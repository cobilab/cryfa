          #######################################################
          #         Download Denisova (FASTQ) -- 2.9 GB         #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

### Create a folder for FASTQ files and one for Denisova dataset
if [[ ! -d $dataset/$FQ/$DENISOVA ]]; then  mkdir -p $dataset/$FQ/$DENISOVA;  fi

### Download -- 1.7 GB - 1.2 GB
for i in B1087 B1088; do
    wget $WGET_OP $DENISOVA_FQ_URL/${i}_SR.txt.gz;
    gunzip < ${i}_SR.txt.gz > $dataset/$FQ/$DENISOVA/$DENISOVA-${i}_SR.$fastq
    rm -f ${i}_SR.txt.gz;
done