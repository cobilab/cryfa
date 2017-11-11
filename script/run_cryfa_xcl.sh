          #######################################################
          #                Run cryfa, exclusively                #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

. run_fn.sh    # Common Functions

cmake .
make
cp cryfa pass.txt  $cryfa_xcl;
cd $cryfa_xcl

for nThr in $CRYFA_THR_RUN; do
    cFT="cryfa";
    cCmd="./cryfa -k $CRYFA_KEY_FILE -t $nThr";
    dCmd="./cryfa -k $CRYFA_KEY_FILE -t $nThr -d";

    ### Compress
    progMemoryStart cryfa &
    MEMPID=$!

    rm -f CRYFA_THR_${nThr}_CT__${inDataWF}_$ft

    (time $cCmd $inData > $in.$cFT) \
        &> $result_FLD/CRYFA_THR_${nThr}_CT__${inDataWF}_$ft              # Time

    ls -la $in.$cFT > $result_FLD/CRYFA_THR_${nThr}_CS__${inDataWF}_$ft   # Size
                                                                          # Mem
    progMemoryStop $MEMPID $result_FLD/CRYFA_THR_${nThr}_CM__${inDataWF}_$ft

    ### Decompress
    progMemoryStart cryfa &
    MEMPID=$!

    (time $dCmd $in.$cFT > $in) \
        &> $result_FLD/CRYFA_THR_${nThr}_DT__${inDataWF}_$ft              # Time
                                                                          # Mem
    progMemoryStop $MEMPID $result_FLD/CRYFA_THR_${nThr}_DM__${inDataWF}_$ft

    ### Verify if input and decompressed files are the same
    cmp $inData $in &>$result_FLD/CRYFA_THR_${nThr}_V__${inDataWF}_$ft
done

cd ..