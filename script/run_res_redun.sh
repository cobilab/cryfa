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


#DATA_SET="A B F V P";
DATA_SET="A B F V";
printf "Method;Dataset;Size;CompSize;CRatio;1/CRatio\n" > red

### cryfa
if [[ $CRYFA -eq 1 ]];
then
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
            printf "cryfa;$d/$inwf;$origSize;$compSize;%s;%s\n" \
                   "`echo "scale=5; $origSize/$compSize" | bc -l`" \
                   "`echo "scale=5; $compSize/$origSize" | bc -l`" >> ../../red
            rm -f $i.cryfa
        done
    done
    cd ../..
fi


### MFCompress
if [[ $MFCOMPRESS -eq 1 ]];
then
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
            printf "MFCompress;$d/$inwf;$origSize;$compSize;%s;%s\n" \
                   "`echo "scale=5; $origSize/$compSize" | bc -l`" \
                   "`echo "scale=5; $compSize/$origSize" | bc -l`" >> ../../red
            rm -f $i.mfc
        done
    done
    cd ../..
fi


### DELIMINATE
if [[ $DELIMINATE -eq 1 ]];
then
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
            printf "DELIMINATE;$d/$inwf;$origSize;$compSize;%s;%s\n" \
                   "`echo "scale=5; $origSize/$compSize" | bc -l`" \
                   "`echo "scale=5; $compSize/$origSize" | bc -l`" >> ../../red
            rm -f $i.dlim
        done
    done
    cd ../..
fi
