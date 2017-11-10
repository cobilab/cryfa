          #######################################################
          #      Generate synthetic dataset (FASTA) -- 4 GB     #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

. par.sh        # Internal parameters

INSTALL_XS=1    # To install XS simulator

### Create a folder for FASTA files and one for synthetic dataset
if [[ ! -d $dataset/$FA/$Synth ]]; then  mkdir -p $dataset/$FA/$Synth;  fi

### Install XS
if [[ $INSTALL_XS -eq 1 ]]; then
    rm -fr $XS
    git clone https://github.com/pratas/XS.git
    cd $XS
    make
    cd ..
fi

### Generate dataset -- 2.3 GB - 1.7 GB
XS/XS -eo -es -t 1 -n 4000000 -ld 70:1000 \
      -f 0.2,0.2,0.2,0.2,0.2       $dataset/$FA/$Synth/SynFA-1.$fasta
XS/XS -eo -es -t 2 -n 3000000 -ls 500 \
      -f 0.23,0.23,0.23,0.23,0.08  $dataset/$FA/$Synth/SynFA-2.$fasta

### Replace @ symbol with > for the headers
for i in 1 2; do  sed -i 's/@/>/g' "$dataset/$FA/$Synth/SynFA-$i.$fasta";  done