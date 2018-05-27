          #######################################################
          #         Download Neanderthal (SAM) -- 1.5 GB        #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

### Create a folder for SAM files and one for Neanderthal dataset
if [[ ! -d $dataset/$SAM/$NEANDERTHAL ]]; then
    mkdir -p $dataset/$SAM/$NEANDERTHAL;
fi

### Download
# AltaiNea.hg19_1000g.Y.dq - High quality, aligned to the human genome
file="AltaiNea.hg19_1000g.Y.dq"
wget $WGET_OP $NEANDERTHAL_SAM_URL/$file.$bam;

### Convert BAM to SAM using "samtools"
samtools view -h -o $dataset/$SAM/$NEANDERTHAL/N-y.sam $file.$bam

rm -f $file.$bam;