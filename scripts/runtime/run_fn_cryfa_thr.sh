#!/usr/bin/env bash

# Run Cryfa with different number of threads

. "$scripts_runtime/run_fn.sh" # Common Functions

# Run Cryfa
function runCryfa {
  local result_FLD="../../$result"
  local inData="../../$CRYFA_THR_DATASET"
  local in="${inData##*/}"  # Input file name
  local inDataWF="${in%.*}" # Input file name
  # without filetype
  local ft="${in##*.}" # Input filetype
  local CRYFA_THR_RUN
  CRYFA_THR_RUN=$(seq -s' ' 1 $MAX_N_THR) # 1 2 3 ... $MAX_N_THR
  cd $progs/cryfa

  for nThr in $CRYFA_THR_RUN; do
    cFT="cryfa"
    cCmd="./cryfa -k $CRYFA_KEY_FILE -t $nThr"
    dCmd="./cryfa -k $CRYFA_KEY_FILE -t $nThr -d"

    # Compress
    progMemoryStart cryfa &
    MEMPID=$!

    (time $cCmd $inData >$in.$cFT) &>$result_FLD/CRYFA_THR_${nThr}_CT__${inDataWF}_$ft # Time
    ls -la $in.$cFT >$result_FLD/CRYFA_THR_${nThr}_CS__${inDataWF}_$ft                 # Size
    progMemoryStop $MEMPID $result_FLD/CRYFA_THR_${nThr}_CM__${inDataWF}_$ft           # Mem

    # Decompress
    progMemoryStart cryfa &
    MEMPID=$!

    (time $dCmd $in.$cFT >$in) &>$result_FLD/CRYFA_THR_${nThr}_DT__${inDataWF}_$ft # Time
    progMemoryStop $MEMPID $result_FLD/CRYFA_THR_${nThr}_DM__${inDataWF}_$ft       # Mem

    # Verify if input and decompressed files are the same
    cmp $inData $in &>$result_FLD/CRYFA_THR_${nThr}_V__${inDataWF}_$ft
  done

  rm -f mem_ps

  cd ../..
}
