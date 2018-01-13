          #######################################################
          #  Download Datasets for redundancy (FASTA) --  GB #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

### Create a folder for redundancy exploration datasets
if [[ ! -d $dataset/$redun ]]; then  mkdir -p $dataset/$redun;  fi

### Get 'goose' for splitting reads
git clone https://github.com/pratas/goose.git
cd goose/src/
make
cd ../..

### Download
# Archaea
if [[ ! -d $dataset/$redun/$ARCHAEA ]];
then
    mkdir -p $dataset/$redun/$ARCHAEA;
fi

perl ./$script/DownloadArchaea.pl

### Remove blank lines and move it to dataset folder
cat archaea.fa | grep -Ev "^$" | ./goose/src/goose-splitreads "complete genome" \
    > $dataset/$redun/$ARCHAEA
rm -f archaea.fa

# Bacteria
# Fungi
# Plants
# Viruses
#perl ./$script/DownloadViruses.pl

### Remove blank lines in downloaded file and move it to dataset folder
#cat viruses.fa | grep -Ev "^$" > $dataset/$FA/$VIRUSES/viruses.$fasta
#rm -f viruses.fa