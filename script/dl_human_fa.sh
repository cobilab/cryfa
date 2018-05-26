          #######################################################
          #          Download Human (FASTA) -- 3.1 GB           #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

### Create a folder for FASTA files and one for human dataset
if [[ ! -d $dataset/$FA/$HUMAN ]]; then  mkdir -p $dataset/$FA/$HUMAN;  fi

### Download and remove blank lines
for i in {1..22} X Y MT; do
    wget $WGET_OP $HUMAN_FA_URL/$HUMAN_CHROMOSOME$i.fa.gz;
    gunzip < $HUMAN_CHROMOSOME$i.fa.gz | grep -Ev "^$" \
           > $dataset/$FA/$HUMAN/$HUMAN-$i.$fasta
    rm -f $HUMAN_CHROMOSOME$i.fa.gz
done
for dual in "alts AL" "unplaced UP" "unlocalized UL"; do
    set $dual
    wget $WGET_OP $HUMAN_FA_URL/$HUMAN_CHR_PREFIX$1.fa.gz;
    gunzip < $HUMAN_CHR_PREFIX$1.fa.gz | grep -Ev "^$" \
           > $HOME/$dataset/$FA/$HUMAN/$HUMAN-$2.$fasta
    rm -f $HUMAN_CHR_PREFIX$1.fa.gz;
done

### Merge all chromosomes into one file
cd $dataset/$FA/$HUMAN
for i in $HS_SEQ_RUN; do
    cat $HUMAN-$i.$fasta >> HS.$fasta;
    rm -f $HUMAN-$i.$fasta    # Remove individual chromosomes
done