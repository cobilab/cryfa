          #######################################################
          #        Run & Results for exploring redundancy       #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

CRYFA=1
MFCOMPRESS=1
DELIMINATE=1


DATA_SET="$ARCHAEA $BACTERIA $FUNGI $PLANTS $VIRUSES"

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Cryfa
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CRYFA -eq 1 ]];
then
    printf "Method\tCategory\tDataset\tSize\tCompSize\tNRC\n" \
        > $result/REDUN-CRYFA.$INF
    cd progs/cryfa
    echo "              %%%%%%%%%%%%%"
    echo "%%%%%%%%%%%%%%%   CRYFA   %%%%%%%%%%%%%%%"
    echo "              %%%%%%%%%%%%%"
    for d in $DATA_SET; do
        for i in ../../$dataset/$redun/$d/*.$fasta; do
            ./cryfa -k $CRYFA_KEY_FILE -t $CRYFA_DEFAULT_N_THR $i > $i.cryfa
            in="${i##*/}"      # Input file name
            inwf="${in%.*}"    # Input file name without filetype
            origSize=$(stat --printf="%s" $i)
            compSize=$(stat --printf="%s" $i.cryfa)
            printf "cryfa\t$d\t$inwf\t$origSize\t$compSize\t%s\n"      \
                   "$(echo "scale=5; ($compSize*8)/($origSize*2)" | bc -l)" \
                   >> ../../$result/REDUN-CRYFA.$INF
            rm -f $i.cryfa
        done
    done
    cd ../..

    ### Split the results for A, B, F, V, P
    for d in $DATA_SET; do
        cd $result
        grep Method  REDUN-CRYFA.$INF  > REDUN-CRYFA-$d;
        grep $'\t'$d REDUN-CRYFA.$INF >> REDUN-CRYFA-$d;
        cd ..
    done
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   MFCompress
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $MFCOMPRESS -eq 1 ]];
then
    printf "Method\tCategory\tDataset\tSize\tCompSize\tNRC\n" \
        > $result/REDUN-MFCOMPRESS.$INF
    cd progs/mfcompress
    echo "            %%%%%%%%%%%%%%%%%%"
    echo "%%%%%%%%%%%%%   MFCOMPRESS   %%%%%%%%%%%%%%%"
    echo "            %%%%%%%%%%%%%%%%%%"
    for d in $DATA_SET; do
        for i in ../../$dataset/$redun/$d/*.$fasta; do
            ./MFCompressC -o $i.mfc $i;
            in="${i##*/}"      # Input file name
            inwf="${in%.*}"    # Input file name without filetype
            origSize=$(stat --printf="%s" $i)
            compSize=$(stat --printf="%s" $i.mfc)
            printf "MFCompress\t$d\t$inwf\t$origSize\t$compSize\t%s\n" \
                   "$(echo "scale=5; ($compSize*8)/($origSize*2)" | bc -l)" \
                   >> ../../$result/REDUN-MFCOMPRESS.$INF
            rm -f $i.mfc
        done
    done
    cd ../..

    ### Split the results for A, B, F, V, P
    for d in $DATA_SET; do
        cd $result
        grep Method  REDUN-MFCOMPRESS.$INF  > REDUN-MFCOMPRESS-$d;
        grep $'\t'$d REDUN-MFCOMPRESS.$INF >> REDUN-MFCOMPRESS-$d;
        cd ..
    done
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   DELIMINATE
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $DELIMINATE -eq 1 ]];
then
    printf "Method\tCategory\tDataset\tSize\tCompSize\tNRC\n" \
        > $result/REDUN-DELIMINATE.$INF
    cd progs/delim
    echo "            %%%%%%%%%%%%%%%%%%"
    echo "%%%%%%%%%%%%%   DELIMINATE   %%%%%%%%%%%%%%%"
    echo "            %%%%%%%%%%%%%%%%%%"
    for d in $DATA_SET; do
        for i in ../../$dataset/$redun/$d/*.$fasta; do
            ./delim a $i;
            in="${i##*/}"      # Input file name
            inwf="${in%.*}"    # Input file name without filetype
            origSize=$(stat --printf="%s" $i)
            compSize=$(stat --printf="%s" $i.dlim)
            printf "DELIMINATE\t$d\t$inwf\t$origSize\t$compSize\t%s\n" \
                   "$(echo "scale=5; ($compSize*8)/($origSize*2)" | bc -l)" \
                   >> ../../$result/REDUN-DELIMINATE.$INF
            rm -f $i.dlim
        done
    done
    cd ../..

    ### Split the results for A, B, F, V, P
    for d in $DATA_SET; do
        cd $result
        grep Method  REDUN-DELIMINATE.$INF  > REDUN-DELIMINATE-$d;
        grep $'\t'$d REDUN-DELIMINATE.$INF >> REDUN-DELIMINATE-$d;
        cd ..
    done
fi