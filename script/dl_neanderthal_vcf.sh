          #######################################################
          #         Download Neanderthal (VCF) -- 0.8 GB        #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

### Create a folder for VCF files and one for Neanderthal dataset
if [[ ! -d $dataset/$VCF/$NEANDERTHAL ]]; then
    mkdir -p $dataset/$VCF/$NEANDERTHAL;
fi

### Download
# SS6004472.hg19_1000g.nonchrom.mod - High coverage
file="SS6004472.hg19_1000g.nonchrom.mod"
wget $WGET_OP $NEANDERTHAL_VCF_URL/$file.$vcf.gz;
gunzip < $file.$vcf.gz > $dataset/$VCF/$NEANDERTHAL/N-n.$vcf;
rm -f $file.$vcf.gz;