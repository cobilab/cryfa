#!/usr/bin/env sh

set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
BUILD_TYPE=${BUILD_TYPE:-Release}
BUILD=${BUILD:-build}
PARALLEL=${PARALLEL:-8}
INTERACTIVE=${INTERACTIVE:-no}
NO_PROMPT=0

usage() {
  cat <<'EOF'
Usage: sh install.sh [options]

Options:
  --build-dir DIR       Build directory (default: build)
  --build-type TYPE     CMake build type (default: Release)
  --parallel N          Parallel build jobs (default: 8)
  --interactive         Confirm the resolved settings before building
  --no-prompt           Run non-interactively with the current values
  --help                Show this help text
EOF
}

log() {
  printf '[install] %s\n' "$*"
}

fail() {
  log "Error: $*"
  exit 1
}

is_tty() {
  [ -t 0 ] && [ -t 1 ]
}

confirm_yes_no() {
  prompt_text=$1
  default_value=$2
  while true; do
    printf '%s [%s]: ' "$prompt_text" "$default_value" >&2
    IFS= read -r reply || exit 1
    if [ -z "$reply" ]; then
      reply=$default_value
    fi
    case $(printf '%s' "$reply" | tr '[:upper:]' '[:lower:]') in
      y | yes) return 0 ;;
      n | no) return 1 ;;
    esac
    printf 'Please answer yes or no.\n' >&2
  done
}

validate_options() {
  case $BUILD_TYPE in
    "" )
      fail "build type must not be empty."
      ;;
  esac

  case $PARALLEL in
    '' | *[!0-9]*)
      fail "parallel build jobs must be a positive integer."
      ;;
    0)
      fail "parallel build jobs must be greater than zero."
      ;;
  esac
}

cache_source_dir() {
  cache_file=$1
  sed -n 's/^CMAKE_HOME_DIRECTORY:INTERNAL=//p' "$cache_file" | tail -n 1
}

clear_stale_cmake_state() {
  build_dir=$1
  if [ ! -d "$build_dir" ]; then
    return
  fi

  log "Resetting stale CMake state in: $build_dir"
  log "Keeping reusable downloaded dependencies under $build_dir/_deps when present"

  rm -f \
    "$build_dir/CMakeCache.txt" \
    "$build_dir/CTestTestfile.cmake" \
    "$build_dir/Makefile" \
    "$build_dir/cmake_install.cmake" \
    "$build_dir/compile_commands.json" \
    "$build_dir/build.ninja" \
    "$build_dir/.ninja_deps" \
    "$build_dir/.ninja_log"

  rm -rf "$build_dir/CMakeFiles"
}

handle_existing_cache() {
  build_dir=$1
  cache_file=$build_dir/CMakeCache.txt

  if [ ! -f "$cache_file" ]; then
    return
  fi

  cache_source=$(cache_source_dir "$cache_file")
  if [ "$cache_source" = "$ROOT_DIR" ]; then
    return
  fi

  log "Detected a build cache from a different source directory."
  log "Current repo: $ROOT_DIR"
  log "Cached source: ${cache_source:-unknown}"

  if [ "$NO_PROMPT" -eq 0 ] && is_tty; then
    if ! confirm_yes_no "Reset the build directory and reconfigure?" "yes"; then
      fail "installation cancelled."
    fi
  fi

  clear_stale_cmake_state "$build_dir"
}

while [ "$#" -gt 0 ]; do
  case "$1" in
    --build-dir)
      BUILD=$2
      shift 2
      ;;
    --build-type)
      BUILD_TYPE=$2
      shift 2
      ;;
    --parallel)
      PARALLEL=$2
      shift 2
      ;;
    --interactive)
      INTERACTIVE=yes
      shift
      ;;
    --no-prompt)
      INTERACTIVE=no
      NO_PROMPT=1
      shift
      ;;
    -h | --help)
      usage
      exit 0
      ;;
    *)
      fail "unknown argument \"$1\"."
      ;;
  esac
done

cd "$ROOT_DIR"

if [ "$INTERACTIVE" = "yes" ] && [ "$NO_PROMPT" -eq 0 ] && is_tty; then
  log "Planned install settings:"
  log "  Build directory: $BUILD"
  log "  Build type: $BUILD_TYPE"
  log "  Parallel jobs: $PARALLEL"
  if ! confirm_yes_no "Continue with these settings?" "yes"; then
    fail "installation cancelled."
  fi
fi

validate_options
handle_existing_cache "$BUILD"

log "Configuring CMake in \"$BUILD\""
cmake -S "$ROOT_DIR" -B "$BUILD" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

log "Building targets with $PARALLEL parallel jobs"
cmake --build "$BUILD" --parallel "$PARALLEL" --config "$BUILD_TYPE"

log "Copying executables to the repository root"
cp "$BUILD/cryfa" "$ROOT_DIR/cryfa"
cp "$BUILD/keygen" "$ROOT_DIR/keygen"

log "Install complete"
