          #######################################################
          #            Download Human (SAM) -- 0.5 GB           #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

### Create a folder for SAM files and one for Human dataset
if [[ ! -d $dataset/$SAM/$HUMAN ]]; then  mkdir -p $dataset/$SAM/$HUMAN;  fi

### Download
# HG00096.unmapped.ILLUMINA.bwa.GBR.low_coverage.20120522 -- British male in
# England and Scotland, aligned to the human genome - low coverage
file="HG00096.unmapped.ILLUMINA.bwa.GBR.low_coverage.20120522"
wget $WGET_OP $HUMAN_SAM_URL/$file.$bam;

### Convert BAM to SAM using "samtools"
samtools view -h -o $dataset/$SAM/$HUMAN/HS-n.sam $file.$bam

rm -f $file.$bam;