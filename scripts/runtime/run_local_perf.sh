#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)

LABEL=${LOCAL_PERF_LABEL:-}
COMPARE_TO=${LOCAL_PERF_COMPARE_TO:-}
BIN=${LOCAL_PERF_BIN:-build/cryfa}
KEY_FILE=${LOCAL_PERF_KEY_FILE:-pass.txt}
INPUT=${LOCAL_PERF_INPUT:-example/in.fq}
OUT_DIR=${LOCAL_PERF_OUT_DIR:-results/local_perf}
TARGET_MB=${LOCAL_PERF_TARGET_MB:-200}
THREADS=${LOCAL_PERF_THREADS:-1 4 8}
RUNS=${LOCAL_PERF_RUNS:-1}
MODES=${LOCAL_PERF_MODES:-default stop-shuffle}
INTERACTIVE=${LOCAL_PERF_INTERACTIVE:-auto}

RUN_TIMESTAMP=""
LABEL_BASE=""
RUN_LABEL=""
LABEL_SAFE=""
DATASET=""
INPUT_BYTES=0
COMPARE_DIR=""
COMPARE_LABEL=""
RAW_CSV=""
SUMMARY_CSV=""
SUMMARY_MD=""
COMPARE_CSV=""
COMPARE_MD=""

function usage {
  cat <<'EOF'
Usage: bash scripts/runtime/run_local_perf.sh [options]

Options:
  --label NAME          Base label for this run; a timestamp is always appended
  --compare-to NAME     Compare against an exact prior run label or a label prefix
  --bin PATH            Cryfa binary path (default: build/cryfa)
  --key-file PATH       Key file path (default: pass.txt)
  --input PATH          Seed input or dataset path (default: example/in.fq)
  --out-dir PATH        Output folder (default: results/local_perf)
  --target-mb N         Expand the seed input to at least N MiB (default: 200)
  --threads "LIST"      Thread counts to test (default: "1 4 8")
  --runs N              Runs per configuration (default: 1)
  --modes "LIST"        Modes: default, stop-shuffle, or both
  --interactive         Prompt for the main run options before starting
  --no-prompt           Run non-interactively with the current values
  --help                Show this help text
EOF
}

function timestamp {
  date '+%Y-%m-%d %H:%M:%S'
}

function log {
  printf '[%s] %s\n' "$(timestamp)" "$*"
}

function log_section {
  printf '\n[%s] %s\n' "$(timestamp)" "$*"
}

function fail {
  log "Error: $*"
  exit 1
}

function is_tty {
  [[ -t 0 && -t 1 ]]
}

function prompt_with_default {
  local prompt_text=$1
  local default_value=$2
  local reply
  printf '%s [%s]: ' "$prompt_text" "$default_value" >&2
  IFS= read -r reply || exit 1
  if [[ -n $reply ]]; then
    printf '%s\n' "$reply"
  else
    printf '%s\n' "$default_value"
  fi
}

function confirm_yes_no {
  local prompt_text=$1
  local default_value=$2
  local reply
  local normalized

  while true; do
    printf '%s [%s]: ' "$prompt_text" "$default_value" >&2
    IFS= read -r reply || exit 1
    if [[ -z $reply ]]; then
      reply=$default_value
    fi
    normalized=$(printf '%s' "$reply" | tr '[:upper:]' '[:lower:]')
    case "$normalized" in
    y | yes)
      return 0
      ;;
    n | no)
      return 1
      ;;
    esac
    printf 'Please answer yes or no.\n' >&2
  done
}

function sanitize_name {
  printf '%s' "$1" | tr -c 'A-Za-z0-9._-' '_'
}

function resolve_path {
  local path=$1
  if [[ $path = /* ]]; then
    printf '%s\n' "$path"
  else
    printf '%s/%s\n' "$ROOT_DIR" "$path"
  fi
}

function file_size_bytes {
  wc -c <"$1" | awk '{print $1}'
}

function format_mib {
  awk -v bytes="$1" 'BEGIN { printf "%.2f", bytes / 1048576.0 }'
}

function mib_per_second {
  awk -v bytes="$1" -v seconds="$2" 'BEGIN {
    if (seconds + 0 <= 0) {
      printf "0.00"
    } else {
      printf "%.2f", (bytes / 1048576.0) / seconds
    }
  }'
}

function parse_time_value {
  local key=$1
  local log_file=$2
  awk -v key="$key" '$1 == key { print $2 }' "$log_file" | tail -n 1
}

function require_file {
  local path=$1
  local label=$2
  if [[ ! -e $path ]]; then
    fail "$label \"$path\" does not exist."
  fi
}

function is_probably_script_executable {
  local path=$1
  if ! command -v file >/dev/null 2>&1; then
    return 1
  fi

  file "$path" | grep -qi 'script'
}

function resolve_existing_binary {
  local requested=$1
  local candidate

  if [[ -x $requested ]] && ! is_probably_script_executable "$requested"; then
    printf '%s\n' "$requested"
    return
  fi

  for candidate in "$ROOT_DIR/build/cryfa" "$ROOT_DIR/cryfa"; do
    if [[ -x $candidate ]] && ! is_probably_script_executable "$candidate"; then
      printf '%s\n' "$candidate"
      return
    fi
  done

  fail "Cryfa binary \"$requested\" does not exist."
}

function csv_escape {
  local value=$1
  value=${value//$'\r'/}
  if [[ $value == *','* || $value == *'"'* || $value == *$'\n'* ]]; then
    value=${value//\"/\"\"}
    printf '"%s"' "$value"
  else
    printf '%s' "$value"
  fi
}

function append_csv_row {
  local output_file=$1
  shift
  local first=1
  local field

  {
    for field in "$@"; do
      if ((first)); then
        first=0
      else
        printf ','
      fi
      csv_escape "$field"
    done
    printf '\n'
  } >>"$output_file"
}

function convert_tsv_file_to_csv {
  local input_file=$1
  local output_file=$2
  awk -F '\t' '
    {
      for (i = 1; i <= NF; ++i) {
        gsub(/\r/, "", $i)
        gsub(/"/, "\"\"", $i)
        if ($i ~ /[",\n]/) {
          $i = "\"" $i "\""
        }
      }
      for (i = 1; i <= NF; ++i) {
        printf "%s", $i
        if (i < NF) {
          printf ","
        }
      }
      printf "\n"
    }
  ' "$input_file" >"$output_file"
}

function ask_for_options {
  log "Interactive benchmark setup"
  LABEL=$(prompt_with_default "Base label" "${LABEL:-baseline}")
  INPUT=$(prompt_with_default "Input file" "$INPUT")
  TARGET_MB=$(prompt_with_default "Target dataset size in MiB (0 keeps the input as-is)" "$TARGET_MB")
  THREADS=$(prompt_with_default "Thread counts (space or comma separated)" "$THREADS")
  RUNS=$(prompt_with_default "Runs per case" "$RUNS")
  MODES=$(prompt_with_default "Modes (default, stop-shuffle, both)" "$MODES")
  COMPARE_TO=$(prompt_with_default "Compare to previous label (leave empty to skip)" "$COMPARE_TO")
}

function build_run_label {
  RUN_TIMESTAMP=$(date '+%Y%m%d_%H%M%S')
  LABEL_BASE=${LABEL:-baseline}
  LABEL_BASE=${LABEL_BASE//$'\r'/}
  LABEL_BASE=${LABEL_BASE//$'\n'/ }
  LABEL_BASE=${LABEL_BASE//,/ _}
  LABEL_BASE=${LABEL_BASE//\"/_}
  RUN_LABEL="${LABEL_BASE}_${RUN_TIMESTAMP}"
  LABEL_SAFE=$(sanitize_name "$RUN_LABEL")
}

function build_dataset {
  local seed_bytes
  seed_bytes=$(file_size_bytes "$INPUT")

  if (( TARGET_MB <= 0 )) || (( seed_bytes >= TARGET_MB * 1024 * 1024 )); then
    DATASET=$INPUT
    log "Using input file directly as benchmark dataset: $DATASET ($(format_mib "$seed_bytes") MiB)"
    return
  fi

  local target_bytes=$((TARGET_MB * 1024 * 1024))
  local copies=$(((target_bytes + seed_bytes - 1) / seed_bytes))
  local dataset_dir="$OUT_DIR/datasets"
  local base_name
  local ext
  local tmp

  mkdir -p "$dataset_dir"
  base_name=$(basename "${INPUT%.*}")
  ext=${INPUT##*.}
  if [[ $ext == "$INPUT" ]]; then
    ext="dat"
  fi

  DATASET="$dataset_dir/${base_name}_${TARGET_MB}mb_x${copies}.${ext}"

  if [[ -f $DATASET && -f $DATASET.meta ]] &&
    grep -qx "copies=$copies" "$DATASET.meta"; then
    log "Reusing cached generated dataset: $DATASET ($(format_mib "$(file_size_bytes "$DATASET")") MiB)"
    return
  fi

  if (( copies > 1 )) && ! command -v perl >/dev/null 2>&1; then
    fail "perl is required to expand \"$INPUT\" into a local dataset."
  fi

  tmp="$DATASET.tmp"
  rm -f "$tmp"

  log "Generating synthetic benchmark dataset from $INPUT"
  log "Seed size: $(format_mib "$seed_bytes") MiB | Copies: $copies | Target: ${TARGET_MB} MiB+"

  if (( copies == 1 )); then
    cp "$INPUT" "$tmp"
  else
    perl -e '
      use strict;
      use warnings;

      my ($input, $copies, $output) = @ARGV;
      open my $in, "<", $input or die "Unable to open $input: $!";
      binmode $in;
      local $/;
      my $chunk = <$in>;
      close $in;

      open my $out, ">", $output or die "Unable to open $output: $!";
      binmode $out;
      for (1 .. $copies) {
        print {$out} $chunk;
      }
      close $out or die "Unable to write $output: $!";
    ' "$INPUT" "$copies" "$tmp"
  fi

  mv "$tmp" "$DATASET"
  cat >"$DATASET.meta" <<EOF
source=$INPUT
seed_bytes=$seed_bytes
copies=$copies
target_mb=$TARGET_MB
EOF

  log "Dataset ready: $DATASET ($(format_mib "$(file_size_bytes "$DATASET")") MiB)"
}

function resolve_compare_target {
  [[ -z $COMPARE_TO ]] && return

  local compare_safe
  compare_safe=$(sanitize_name "$COMPARE_TO")

  if [[ -d "$OUT_DIR/$compare_safe" ]] && [[ "$OUT_DIR/$compare_safe" != "$RUN_DIR" ]]; then
    COMPARE_DIR="$OUT_DIR/$compare_safe"
  else
    COMPARE_DIR=$(find "$OUT_DIR" -maxdepth 1 -mindepth 1 -type d \
      -name "${compare_safe}_*" ! -path "$RUN_DIR" | sort | tail -n 1)
  fi

  if [[ -z $COMPARE_DIR ]]; then
    fail "comparison baseline \"$COMPARE_TO\" was not found under \"$OUT_DIR\"."
  fi

  COMPARE_LABEL=$(basename "$COMPARE_DIR")
  log "Resolved comparison baseline \"$COMPARE_TO\" to \"$COMPARE_LABEL\""
}

function run_case {
  local mode=$1
  local thread=$2
  local -a mode_args=()
  local -a compress_cmd
  local -a decompress_cmd

  case "$mode" in
  default)
    ;;
  stop-shuffle)
    mode_args=(-s)
    ;;
  *)
    fail "unsupported mode \"$mode\"."
    ;;
  esac

  local run_index
  for ((run_index = 1; run_index <= RUNS; ++run_index)); do
    local prefix="${mode}_t${thread}_r${run_index}"
    local compressed="$RUN_DIR/${prefix}.crf"
    local decompressed="$RUN_DIR/${prefix}.out"
    local c_log="$DETAILS_DIR/${prefix}_compress.log"
    local d_log="$DETAILS_DIR/${prefix}_decompress.log"

    rm -f "$compressed" "$decompressed" "$c_log" "$d_log"

    compress_cmd=("$BIN" -k "$KEY_FILE" -t "$thread")
    if ((${#mode_args[@]})); then
      compress_cmd+=("${mode_args[@]}")
    fi
    compress_cmd+=("$DATASET")

    log_section "Running case: mode=$mode | threads=$thread | run=$run_index/$RUNS"
    log "Compress command: ${compress_cmd[*]}"
    if ! { time -p "${compress_cmd[@]}" >"$compressed"; } 2>"$c_log"; then
      fail "compression failed for mode=$mode thread=$thread run=$run_index. See $c_log"
    fi

    decompress_cmd=("$BIN" -k "$KEY_FILE" -t "$thread" -d "$compressed")
    log "Decompress command: ${decompress_cmd[*]}"
    if ! { time -p "${decompress_cmd[@]}" >"$decompressed"; } 2>"$d_log"; then
      fail "decompression failed for mode=$mode thread=$thread run=$run_index. See $d_log"
    fi

    if ! cmp -s "$DATASET" "$decompressed"; then
      fail "round-trip mismatch for mode=$mode thread=$thread run=$run_index."
    fi

    local compressed_bytes
    local c_real
    local c_user
    local c_sys
    local d_real
    local d_user
    local d_sys
    local c_rate
    local d_rate

    compressed_bytes=$(file_size_bytes "$compressed")
    c_real=$(parse_time_value real "$c_log")
    c_user=$(parse_time_value user "$c_log")
    c_sys=$(parse_time_value sys "$c_log")
    d_real=$(parse_time_value real "$d_log")
    d_user=$(parse_time_value user "$d_log")
    d_sys=$(parse_time_value sys "$d_log")
    c_rate=$(mib_per_second "$INPUT_BYTES" "$c_real")
    d_rate=$(mib_per_second "$INPUT_BYTES" "$d_real")

    append_csv_row "$RAW_CSV" \
      "$RUN_LABEL" "$mode" "$thread" "$run_index" "$INPUT_BYTES" "$compressed_bytes" \
      "$c_real" "$c_user" "$c_sys" "$c_rate" \
      "$d_real" "$d_user" "$d_sys" "$d_rate" "ok"

    log "Case complete: compressed $(format_mib "$compressed_bytes") MiB | c=${c_real}s (${c_rate} MiB/s) | d=${d_real}s (${d_rate} MiB/s) | verified=ok"

    rm -f "$compressed" "$decompressed"
  done
}

function write_summary_csv {
  append_csv_row "$SUMMARY_CSV" \
    label mode threads runs input_bytes compressed_bytes avg_c_real avg_c_user avg_c_sys avg_c_mib_s avg_d_real avg_d_user avg_d_sys avg_d_mib_s verified

  local mode
  local thread
  for mode in $MODES; do
    for thread in $THREADS; do
      awk -F ',' -v label="$RUN_LABEL" -v mode="$mode" -v thread="$thread" '
        NR > 1 && $2 == mode && $3 == thread {
          count++
          input_bytes = $5
          compressed_bytes = $6
          c_real += $7
          c_user += $8
          c_sys += $9
          c_mib += $10
          d_real += $11
          d_user += $12
          d_sys += $13
          d_mib += $14
          if ($15 != "ok") verified = "FAIL"
        }
        END {
          if (count > 0) {
            if (verified == "") verified = "ok"
            printf "%s,%s,%s,%d,%s,%s,%.6f,%.6f,%.6f,%.2f,%.6f,%.6f,%.6f,%.2f,%s\n",
              label, mode, thread, count, input_bytes, compressed_bytes,
              c_real / count, c_user / count, c_sys / count, c_mib / count,
              d_real / count, d_user / count, d_sys / count, d_mib / count,
              verified
          }
        }
      ' "$RAW_CSV" >>"$SUMMARY_CSV"
    done
  done
}

function write_summary_markdown {
  {
    echo "# Local Performance Summary"
    echo
    echo "- Label: $RUN_LABEL"
    echo "- Dataset: $DATASET"
    echo "- Dataset size: $(format_mib "$INPUT_BYTES") MiB"
    echo "- Runs per case: $RUNS"
    echo "- Threads: $THREADS"
    echo "- Modes: $MODES"
    echo "- Raw data: $(basename "$RAW_CSV")"
    echo "- Summary data: $(basename "$SUMMARY_CSV")"
    echo
    echo "| Mode | Threads | Runs | Compressed Size (MiB) | Ratio | Compress Time (s) | Compress MiB/s | Decompress Time (s) | Decompress MiB/s | Verified |"
    echo "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |"
    awk -F ',' '
      NR > 1 {
        printf "| %s | %s | %s | %.2f | %.3f | %.3f | %.2f | %.3f | %.2f | %s |\n",
          $2, $3, $4, $6 / 1048576.0, $6 / $5, $7, $10, $11, $14, $15
      }
    ' "$SUMMARY_CSV"
  } >"$SUMMARY_MD"
}

function summary_input_for_compare {
  local dir=$1
  if [[ -f "$dir/summary.csv" ]]; then
    printf '%s\n' "$dir/summary.csv"
    return
  fi
  if [[ -f "$dir/summary.tsv" ]]; then
    printf '%s\n' "$dir/summary.tsv"
    return
  fi
  fail "comparison summary was not found in \"$dir\"."
}

function write_compare_report {
  [[ -z $COMPARE_DIR ]] && return

  local baseline_summary
  local baseline_csv
  baseline_summary=$(summary_input_for_compare "$COMPARE_DIR")

  append_csv_row "$COMPARE_CSV" \
    mode threads compress_speedup decompress_speedup size_ratio_delta baseline_verified current_verified

  if [[ $baseline_summary == *.tsv ]]; then
    baseline_csv=$(mktemp "${TMPDIR:-/tmp}/cryfa-compare-baseline.XXXXXX.csv")
    convert_tsv_file_to_csv "$baseline_summary" "$baseline_csv"
  else
    baseline_csv=$baseline_summary
  fi

  awk -F ',' '
    NR == FNR {
      if (FNR == 1) next
      key = $2 SUBSEP $3
      base_c[key] = $7
      base_d[key] = $11
      base_ratio[key] = $6 / $5
      base_verified[key] = $15
      next
    }
    FNR == 1 {
      next
    }
    {
      key = $2 SUBSEP $3
      if (!(key in base_c)) next
      compress_speedup = ($7 + 0 > 0) ? base_c[key] / $7 : 0
      decompress_speedup = ($11 + 0 > 0) ? base_d[key] / $11 : 0
      ratio_delta = ($6 / $5) - base_ratio[key]
      printf "%s,%s,%.3f,%.3f,%.6f,%s,%s\n",
        $2, $3, compress_speedup, decompress_speedup, ratio_delta,
        base_verified[key], $15
    }
  ' "$baseline_csv" "$SUMMARY_CSV" >>"$COMPARE_CSV"

  if [[ $baseline_summary == *.tsv ]]; then
    rm -f "$baseline_csv"
  fi

  {
    echo "# Before/After Comparison"
    echo
    echo "- Before: $COMPARE_LABEL"
    echo "- After: $RUN_LABEL"
    echo "- Speedup values above 1.00 are faster than the baseline."
    echo "- Negative size deltas mean the new run produced smaller output."
    echo "- Comparison data: $(basename "$COMPARE_CSV")"
    echo
    echo "| Mode | Threads | Compress Speedup | Decompress Speedup | Size Ratio Delta | Baseline Verified | Current Verified |"
    echo "| --- | ---: | ---: | ---: | ---: | --- | --- |"
    awk -F ',' '
      NR > 1 {
        printf "| %s | %s | %.3f | %.3f | %.6f | %s | %s |\n",
          $1, $2, $3, $4, $5, $6, $7
      }
    ' "$COMPARE_CSV"
  } >"$COMPARE_MD"
}

while [[ $# -gt 0 ]]; do
  case "$1" in
  --label)
    LABEL=$2
    shift 2
    ;;
  --compare-to)
    COMPARE_TO=$2
    shift 2
    ;;
  --bin)
    BIN=$2
    shift 2
    ;;
  --key-file)
    KEY_FILE=$2
    shift 2
    ;;
  --input)
    INPUT=$2
    shift 2
    ;;
  --out-dir)
    OUT_DIR=$2
    shift 2
    ;;
  --target-mb)
    TARGET_MB=$2
    shift 2
    ;;
  --threads)
    THREADS=$2
    shift 2
    ;;
  --runs)
    RUNS=$2
    shift 2
    ;;
  --modes)
    MODES=$2
    shift 2
    ;;
  --interactive)
    INTERACTIVE=yes
    shift
    ;;
  --no-prompt)
    INTERACTIVE=no
    shift
    ;;
  -h | --help)
    usage
    exit 0
    ;;
  *)
    echo "Error: unknown argument \"$1\"." >&2
    usage >&2
    exit 1
    ;;
  esac
done

if [[ $INTERACTIVE == auto ]] && is_tty; then
  INTERACTIVE=yes
fi

if [[ $INTERACTIVE == yes ]] && is_tty; then
  ask_for_options
fi

THREADS=${THREADS//,/ }
case "$MODES" in
both)
  MODES="default stop-shuffle"
  ;;
*)
  MODES=${MODES//,/ }
  ;;
esac

if [[ ! $RUNS =~ ^[1-9][0-9]*$ ]]; then
  fail "--runs must be a positive integer."
fi

if [[ ! $TARGET_MB =~ ^[0-9]+$ ]]; then
  fail "--target-mb must be a non-negative integer."
fi

BIN=$(resolve_existing_binary "$(resolve_path "$BIN")")
KEY_FILE=$(resolve_path "$KEY_FILE")
INPUT=$(resolve_path "$INPUT")
OUT_DIR=$(resolve_path "$OUT_DIR")

require_file "$KEY_FILE" "key file"
require_file "$INPUT" "input file"

if [[ $INTERACTIVE == yes ]] && is_tty; then
  log "Planned benchmark settings:"
  log "  Base label: ${LABEL:-baseline}"
  log "  Input file: $INPUT"
  log "  Target size: $TARGET_MB MiB"
  log "  Threads: $THREADS"
  log "  Runs per case: $RUNS"
  log "  Modes: $MODES"
  if [[ -n $COMPARE_TO ]]; then
    log "  Compare to: $COMPARE_TO"
  else
    log "  Compare to: none"
  fi
  if ! confirm_yes_no "Start this benchmark run?" "yes"; then
    fail "benchmark cancelled."
  fi
fi

build_run_label

mkdir -p "$OUT_DIR"
RUN_DIR="$OUT_DIR/$LABEL_SAFE"
DETAILS_DIR="$RUN_DIR/details"
RAW_CSV="$RUN_DIR/raw.csv"
SUMMARY_CSV="$RUN_DIR/summary.csv"
SUMMARY_MD="$RUN_DIR/summary.md"
COMPARE_CSV="$RUN_DIR/compare.csv"
COMPARE_MD="$RUN_DIR/compare.md"

rm -rf "$RUN_DIR"
mkdir -p "$DETAILS_DIR"

log_section "Local performance benchmark"
log "Run label: $RUN_LABEL"
log "Binary: $BIN"
log "Key file: $KEY_FILE"
log "Input: $INPUT"
log "Output directory: $RUN_DIR"
log "Requested threads: $THREADS"
log "Runs per case: $RUNS"
log "Modes: $MODES"

build_dataset
INPUT_BYTES=$(file_size_bytes "$DATASET")
log "Measured dataset size: $(format_mib "$INPUT_BYTES") MiB"

resolve_compare_target

append_csv_row "$RAW_CSV" \
  label mode threads run input_bytes compressed_bytes c_real c_user c_sys c_mib_s d_real d_user d_sys d_mib_s verified

for mode in $MODES; do
  for thread in $THREADS; do
    run_case "$mode" "$thread"
  done
done

log_section "Writing reports"
write_summary_csv
write_summary_markdown
write_compare_report

log "Raw CSV: $RAW_CSV"
log "Summary CSV: $SUMMARY_CSV"
log "Summary markdown: $SUMMARY_MD"
if [[ -f $COMPARE_MD ]]; then
  log "Comparison CSV: $COMPARE_CSV"
  log "Comparison markdown: $COMPARE_MD"
fi

log_section "Benchmark complete"
