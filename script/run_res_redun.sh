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
GECO=0


DATA_SET="A B F V P";

### cryfa
if [[ $CRYFA -eq 1 ]];
then
    printf "Method\tCategory\tDataset\tSize\tCompSize\tCRatio\tCRatioRev\n" > REDUN-CRYFA.dat
    cd progs/cryfa
    echo "              %%%%%%%%%%%%%"
    echo "%%%%%%%%%%%%%%%   CRYFA   %%%%%%%%%%%%%%%"
    echo "              %%%%%%%%%%%%%"
    for d in $DATA_SET; do
        for i in ../../dataset/Redundancy/$d/*.fa; do
            ./cryfa -k pass.txt -t 8 $i > $i.cryfa
            in="${i##*/}"      # Input file name
            inwf="${in%.*}"    # Input file name without filetype
            origSize=`stat --printf="%s" $i`
            compSize=`stat --printf="%s" $i.cryfa`
            printf "cryfa\t$d\t$inwf\t$origSize\t$compSize\t%s\t%s\n" \
                   "`echo "scale=5; $origSize/$compSize" | bc -l`" \
                   "`echo "scale=5; $compSize/$origSize" | bc -l`" >> ../../REDUN-CRYFA.dat
            rm -f $i.cryfa
        done
    done
    cd ../..
fi


### MFCompress
if [[ $MFCOMPRESS -eq 1 ]];
then
    printf "Method\tCategory\tDataset\tSize\tCompSize\tCRatio\tCRatioRev\n" > REDUN-MFCOMPRESS.dat
    cd progs/mfcompress
    echo "            %%%%%%%%%%%%%%%%%%"
    echo "%%%%%%%%%%%%%   MFCOMPRESS   %%%%%%%%%%%%%%%"
    echo "            %%%%%%%%%%%%%%%%%%"
    for d in $DATA_SET; do
        for i in ../../dataset/Redundancy/$d/*.fa; do
            ./MFCompressC -o $i.mfc $i;
            in="${i##*/}"      # Input file name
            inwf="${in%.*}"    # Input file name without filetype
            origSize=`stat --printf="%s" $i`
            compSize=`stat --printf="%s" $i.mfc`
            printf "MFCompress\t$d\t$inwf\t$origSize\t$compSize\t%s\t%s\n" \
                   "`echo "scale=5; $origSize/$compSize" | bc -l`" \
                   "`echo "scale=5; $compSize/$origSize" | bc -l`" >> ../../REDUN-MFCOMPRESS.dat
            rm -f $i.mfc
        done
    done
    cd ../..
fi


### DELIMINATE
if [[ $DELIMINATE -eq 1 ]];
then
    printf "Method\tCategory\tDataset\tSize\tCompSize\tCRatio\tCRatioRev\n" > REDUN-DELIMINATE.dat
    cd progs/delim
    echo "            %%%%%%%%%%%%%%%%%%"
    echo "%%%%%%%%%%%%%   DELIMINATE   %%%%%%%%%%%%%%%"
    echo "            %%%%%%%%%%%%%%%%%%"
    for d in $DATA_SET; do
        for i in ../../dataset/Redundancy/$d/*.fa; do
            ./delim a $i;
            in="${i##*/}"      # Input file name
            inwf="${in%.*}"    # Input file name without filetype
            origSize=`stat --printf="%s" $i`
            compSize=`stat --printf="%s" $i.dlim`
            printf "DELIMINATE\t$d\t$inwf\t$origSize\t$compSize\t%s\t%s\n" \
                   "`echo "scale=5; $origSize/$compSize" | bc -l`" \
                   "`echo "scale=5; $compSize/$origSize" | bc -l`" >> ../../REDUN-DELIMINATE.dat
            rm -f $i.dlim
        done
    done
    cd ../..
fi





#if [[ GECO -eq 1 ]];
#then
#    echo "            %%%%%%%%%%%%%%%%%%"
#    echo "%%%%%%%%%%%%%   GECO   %%%%%%%%%%%%%%%"
#    echo "            %%%%%%%%%%%%%%%%%%"
#    for d in $DATA_SET; do
#        for i in dataset/Redundancy/$d/*.fa; do
#            ./GeCo -tm 6:1:0:0/0 -tm 13:10:1:0/0 -g 0.8 $i;
#            in="${i##*/}"      # Input file name
#            inwf="${in%.*}"    # Input file name without filetype
#            origSize=`stat --printf="%s" $i`
#            compSize=`stat --printf="%s" $i.co`
#            printf "GECO\t$d\t$inwf\t$origSize\t$compSize\t%s\t%s\n" \
#                   "`echo "scale=5; $origSize/$compSize" | bc -l`" \
#                   "`echo "scale=5; $compSize/$origSize" | bc -l`" >> red
#            rm -f $i.co
#        done
#    done
#fi