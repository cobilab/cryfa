          #######################################################
          # Download Datasets to redundancy check (FASTA)--12 GB#
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

DS_PATH="$dataset/$redun"    # Path of datasets
GET_GOOSE=1

### Create a folder for redundancy exploration datasets
if [[ ! -d $DS_PATH ]]; then  mkdir -p $DS_PATH;  fi

### Get 'goose' for splitting reads
if [[ $GET_GOOSE -eq 1 ]];
then
    git clone https://github.com/pratas/goose.git
    cd goose/src/
    make
    cd ../..
fi

### Download
for d in "$ARCHAEA Archaea" "$BACTERIA Bacteria" "$FUNGI Fungi" \
         "$PLANTS  Plants"  "$VIRUSES  Viruses"; do
    set $d
    smallCapsD=$(echo $2 | tr A-Z a-z)

    if [[ ! -d $DS_PATH/$1 ]]; then  mkdir -p $DS_PATH/$1;  fi
    cd $DS_PATH/$1
    cp ../../../$script/Download$2.pl \
       ../../../$goose/src/goose-extractreadbypattern \
       ../../../$goose/src/goose-splitreads .

    perl ./Download$2.pl

    # Remove blank lines and split reads by complete genomes
    grep -Ev "^$" $smallCapsD.fa \
        | ./goose-extractreadbypattern "complete genome" | ./goose-splitreads

    # Rename *.fa to *.fasta
    for i in *.fa; do  mv $i ${i}sta;  done

    rm -f Download$2.pl  goose-extractreadbypattern  goose-splitreads

    # If you want to keep the original file, comment the command below.
    # Take care of disk space.
    rm -f $smallCapsD.fasta

    cd ../../..
done