          #######################################################
          #           Download Human (FASTQ) -- 27 GB           #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

### Create a folder for FASTQ files and one for human dataset
if [[ ! -d $dataset/$FQ/$HUMAN ]]; then  mkdir -p $dataset/$FQ/$HUMAN;  fi

### Download -- 4.6 GB - 1.4 GB - 12 GB - 488 MB - 8.4 GB
# ERR013103_1: HG00190 - Male   - FIN (Finnish in Finland)     -Low coverage WGS
# ERR015767_2: HG00638 - Female - PUR (Puerto Rican in Puerto Rico)    -Low cov.
# ERR031905_2: HG00501 - Female - CHS (Han Chinese South)              -Exome
# SRR442469_1: HG02108 - Female - ACB (African Caribbean in Barbados)  -Low cov.
# SRR707196_1: HG00126 - Male   - GBR (British in England and Scotland)-Exome
for dual in "ERR013/ERR013103 ERR013103_1"\
            "ERR015/ERR015767 ERR015767_2" "ERR031/ERR031905 ERR031905_2"\
            "SRR442/SRR442469 SRR442469_1" "SRR707/SRR707196 SRR707196_1"; do
    set $dual
    wget $WGET_OP $HUMAN_FQ_URL/$1/$2.fastq.gz;
    gunzip < $2.fastq.gz > $dataset/$FQ/$HUMAN/$HUMAN-$2.$fastq;
    rm -f $2.fastq.gz;
done