          #######################################################
          #         Download Neanderthal (BAM) -- 1.3 GB        #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

### Create a folder for BAM files and one for Neanderthal dataset
if [[ ! -d $dataset/$BAM/$NEANDERTHAL ]]; then
    mkdir -p $dataset/$BAM/$NEANDERTHAL;
fi

### Download
# Vi33.19.chr21.indel_realn -- High coverage, aligned to the human genome
file="Vi33.19.chr21.indel_realn"
wget $WGET_OP $NEANDERTHAL_BAM_URL/$file.$bam;
mv -f $file.$bam $dataset/$BAM/$NEANDERTHAL/N-21.$bam