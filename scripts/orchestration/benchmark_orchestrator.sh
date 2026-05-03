#!/usr/bin/env bash

# Benchmark orchestration functions

function ensureDir {
  if [[ ! -d $1 ]]; then
    mkdir -p $1
  fi
}

function run_benchmark_get_dataset {
  if [[ $GET_DATASET -ne 1 ]]; then
    return
  fi

  ensureDir $dataset

  if [[ $DL_HUMAN_FA -eq 1 ]]; then
    . "$scripts_data/dl_human_fa.sh"
  fi
  if [[ $DL_VIRUSES_FA -eq 1 ]]; then
    . "$scripts_data/dl_viruses_fa.sh"
  fi
  if [[ $GEN_SYNTH_FA -eq 1 ]]; then
    . "$scripts_data/gen_synth_fa.sh"
  fi

  if [[ $DL_HUMAN_FQ -eq 1 ]]; then
    . "$scripts_data/dl_human_fq.sh"
  fi
  if [[ $DL_DENISOVA_FQ -eq 1 ]]; then
    . "$scripts_data/dl_denisova_fq.sh"
  fi
  if [[ $GEN_SYNTH_FQ -eq 1 ]]; then
    . "$scripts_data/gen_synth_fq.sh"
  fi

  if [[ $DL_DENISOVA_VCF -eq 1 ]]; then
    . "$scripts_data/dl_denisova_vcf.sh"
  fi
  if [[ $DL_NEANDERTHAL_VCF -eq 1 ]]; then
    . "$scripts_data/dl_neanderthal_vcf.sh"
  fi

  if [[ $DL_HUMAN_SAM -eq 1 ]]; then
    . "$scripts_data/dl_human_sam.sh"
  fi
  if [[ $DL_NEANDERTHAL_SAM -eq 1 ]]; then
    . "$scripts_data/dl_neanderthal_sam.sh"
  fi

  if [[ $DL_HUMAN_BAM -eq 1 ]]; then
    . "$scripts_data/dl_human_bam.sh"
  fi
  if [[ $DL_NEANDERTHAL_BAM -eq 1 ]]; then
    . "$scripts_data/dl_neanderthal_bam.sh"
  fi
}

function run_benchmark_install_dependencies {
  if [[ $INSTALL_DEPENDENCIES -ne 1 ]]; then
    return
  fi

  if [[ $INS_7ZIP -eq 1 ]]; then
    . "$scripts_install_dependencies/dep_7zip.sh"
  fi
  if [[ $INS_CMAKE -eq 1 ]]; then
    . "$scripts_install_dependencies/dep_cmake.sh"
  fi
  if [[ $INS_BOOST -eq 1 ]]; then
    . "$scripts_install_dependencies/dep_boost.sh"
  fi
  if [[ $INS_CURL -eq 1 ]]; then
    . "$scripts_install_dependencies/dep_curl.sh"
  fi
  if [[ $INS_VALGRIND -eq 1 ]]; then
    . "$scripts_install_dependencies/dep_valgrind.sh"
  fi
  if [[ $INS_ZLIB -eq 1 ]]; then
    . "$scripts_install_dependencies/dep_zlib.sh"
  fi
  if [[ $INS_SAMTOOLS -eq 1 ]]; then
    . "$scripts_install_dependencies/dep_samtools.sh"
  fi
}

function run_benchmark_install_methods {
  if [[ $INSTALL_METHODS -ne 1 ]]; then
    return
  fi

  ensureDir $progs

  if [[ $INS_CRYFA -eq 1 ]]; then
    . "$scripts_install_methods/ins_cryfa.sh"
  fi
  if [[ $INS_AESCRYPT -eq 1 ]]; then
    . "$scripts_install_methods/ins_aescrypt.sh"
  fi

  if [[ $INS_MFCOMPRESS -eq 1 ]]; then
    . "$scripts_install_methods/ins_mfcompress.sh"
  fi
  if [[ $INS_DELIMINATE -eq 1 ]]; then
    . "$scripts_install_methods/ins_deliminate.sh"
  fi

  if [[ $INS_FQZCOMP -eq 1 ]]; then
    . "$scripts_install_methods/ins_fqzcomp.sh"
  fi
  if [[ $INS_QUIP -eq 1 ]]; then
    . "$scripts_install_methods/ins_quip.sh"
  fi
  if [[ $INS_DSRC -eq 1 ]]; then
    . "$scripts_install_methods/ins_dsrc.sh"
  fi
  if [[ $INS_FQC -eq 1 ]]; then
    . "$scripts_install_methods/ins_fqc.sh"
  fi
}

function run_benchmark_compression {
  if [[ $RUN_METHODS_COMP -ne 1 ]]; then
    return
  fi

  ensureDir $progs
  ensureDir $result

  . "$scripts_data/avail_dataset.sh"
  check_dataset_availability

  . "$scripts_runtime/run_fn_comp.sh"

  if [[ $RUN_CRYFA_FA -eq 1 ]]; then
    compDecompOnDataset cryfa fa
  fi
  if [[ $RUN_CRYFA_FQ -eq 1 ]]; then
    compDecompOnDataset cryfa fq
  fi
  if [[ $RUN_CRYFA_VCF -eq 1 ]]; then
    compDecompOnDataset cryfa vcf
  fi
  if [[ $RUN_CRYFA_SAM -eq 1 ]]; then
    compDecompOnDataset cryfa sam
  fi
  if [[ $RUN_CRYFA_BAM -eq 1 ]]; then
    compDecompOnDataset cryfa bam
  fi

  if [[ $RESULTS_COMP -eq 1 ]]; then
    . "$scripts_results/res_comp.sh"
    generate_compression_results
  fi
}

function run_benchmark_encryption {
  if [[ $RUN_METHODS_ENC -ne 1 ]]; then
    return
  fi

  ensureDir $progs
  ensureDir $result

  . "$scripts_data/avail_dataset.sh"
  check_dataset_availability

  . "$scripts_runtime/run_fn_enc.sh"

  if [[ $RUN_AESCRYPT -eq 1 ]]; then
    encDecOnDataset aescrypt
  fi

  if [[ $RESULTS_ENC -eq 1 ]]; then
    . "$scripts_results/res_enc.sh"
    generate_encryption_results
  fi
}

function run_benchmark_compression_encryption {
  if [[ $RUN_METHODS_COMP_ENC -ne 1 ]]; then
    return
  fi

  ensureDir $progs
  ensureDir $result

  . "$scripts_data/avail_dataset.sh"
  check_dataset_availability

  . "$scripts_runtime/run_fn_comp_enc.sh"

  if [[ $RUN_GZIP_FA_AESCRYPT -eq 1 ]]; then
    compEncDecDecompOnDataset gzip fa aescrypt
  fi
  if [[ $RUN_BZIP2_FA_AESCRYPT -eq 1 ]]; then
    compEncDecDecompOnDataset bzip2 fa aescrypt
  fi
  if [[ $RUN_MFCOMPRESS_AESCRYPT -eq 1 ]]; then
    compEncDecDecompOnDataset mfcompress fa aescrypt
  fi
  if [[ $RUN_DELIMINATE_AESCRYPT -eq 1 ]]; then
    compEncDecDecompOnDataset delim fa aescrypt
  fi

  if [[ $RUN_GZIP_FQ_AESCRYPT -eq 1 ]]; then
    compEncDecDecompOnDataset gzip fq aescrypt
  fi
  if [[ $RUN_BZIP2_FQ_AESCRYPT -eq 1 ]]; then
    compEncDecDecompOnDataset bzip2 fq aescrypt
  fi
  if [[ $RUN_FQZCOMP_AESCRYPT -eq 1 ]]; then
    compEncDecDecompOnDataset fqzcomp fq aescrypt
  fi
  if [[ $RUN_QUIP_AESCRYPT -eq 1 ]]; then
    compEncDecDecompOnDataset quip fq aescrypt
  fi
  if [[ $RUN_DSRC_AESCRYPT -eq 1 ]]; then
    compEncDecDecompOnDataset dsrc fq aescrypt
  fi
  if [[ $RUN_FQC_AESCRYPT -eq 1 ]]; then
    compEncDecDecompOnDataset fqc fq aescrypt
  fi

  if [[ $RESULTS_COMP_ENC -eq 1 ]]; then
    . "$scripts_results/res_comp_enc.sh"
    generate_compression_encryption_results
  fi
}

function run_benchmark_cryfa_threads {
  if [[ $RUN_CRYFA_THREADS -ne 1 ]]; then
    return
  fi

  ensureDir $result

  if [[ ! -e $CRYFA_THR_DATASET ]]; then
    echo "Error: The file \"$CRYFA_THR_DATASET\" is not available."
    return 1
  fi

  . "$scripts_runtime/run_fn_cryfa_thr.sh"

  if [[ $RUN_CRYFA_THR -eq 1 ]]; then
    runCryfa
  fi

  if [[ $RESULTS_CRYFA_THR -eq 1 ]]; then
    . "$scripts_results/res_cryfa_thr.sh"
    generate_cryfa_thread_results
  fi
}

function run_benchmark_local_perf {
  if [[ $RUN_LOCAL_PERF -ne 1 ]]; then
    return
  fi

  ensureDir $result

  local label=${LOCAL_PERF_LABEL:-baseline}
  local compare_to=${LOCAL_PERF_COMPARE_TO:-}
  local input=${LOCAL_PERF_INPUT:-example/in.fq}
  local target_mb=${LOCAL_PERF_TARGET_MB:-200}
  local threads=${LOCAL_PERF_THREADS:-1\ 4\ 8}
  local runs=${LOCAL_PERF_RUNS:-1}
  local modes=${LOCAL_PERF_MODES:-default\ stop-shuffle}
  local interactive_mode=${LOCAL_PERF_INTERACTIVE:-auto}
  local bin=${LOCAL_PERF_BIN:-build/cryfa}
  local key_file=${LOCAL_PERF_KEY_FILE:-pass.txt}
  local out_dir=${LOCAL_PERF_OUT_DIR:-results/local_perf}
  local compare_args=()
  local prompt_args=()
  if [[ -n $compare_to ]]; then
    compare_args=(--compare-to "$compare_to")
  fi
  case "$interactive_mode" in
    yes)
      prompt_args=(--interactive)
      ;;
    no)
      prompt_args=(--no-prompt)
      ;;
  esac

  echo "[local_perf] Starting local performance harness..."
  echo "[local_perf] Base label: $label"
  echo "[local_perf] Output dir: $out_dir"

  bash "$scripts_runtime/run_local_perf.sh" \
    --label "$label" \
    "${compare_args[@]}" \
    "${prompt_args[@]}" \
    --bin "$bin" \
    --key-file "$key_file" \
    --input "$input" \
    --out-dir "$out_dir" \
    --target-mb "$target_mb" \
    --threads "$threads" \
    --runs "$runs" \
    --modes "$modes"
}

function run_benchmark_redundancy {
  if [[ $RUN_REDUNDANCY -ne 1 ]]; then
    return
  fi

  ensureDir $result

  if [[ $GET_DATASET_REDUN -eq 1 ]]; then
    . "$scripts_data/dl_dataset_redun.sh"
  fi

  if [[ $RUN_RES_REDUN -eq 1 ]]; then
    . "$scripts_results/run_res_redun.sh"
    run_redundancy_results
  fi
}

function run_benchmark {
  run_benchmark_get_dataset
  run_benchmark_install_dependencies
  run_benchmark_install_methods
  run_benchmark_compression
  run_benchmark_encryption
  run_benchmark_compression_encryption
  run_benchmark_cryfa_threads || return 1
  run_benchmark_local_perf || return 1
  run_benchmark_redundancy
}
