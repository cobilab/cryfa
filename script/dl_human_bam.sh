          #######################################################
          #            Download Human (BAM) -- 0.6 GB           #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

### Create a folder for BAM files and one for Human dataset
if [[ ! -d $dataset/$BAM/$HUMAN ]]; then  mkdir -p $dataset/$BAM/$HUMAN;  fi

### Download
# HG00096.chrom11.ILLUMINA.bwa.GBR.low_coverage.20120522 -- British male in
# England and Scotland, aligned to the human genome - low coverage
file="HG00096.chrom11.ILLUMINA.bwa.GBR.low_coverage.20120522"
wget $WGET_OP $HUMAN_BAM_URL/$file.$bam;
mv -f $file.$bam $dataset/$BAM/$HUMAN/HS-11.$bam