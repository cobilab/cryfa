          #######################################################
          #      Generate synthetic dataset (FASTA) -- 2.8 GB   #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

INSTALL_XS=1    # Install XS simulator

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

### Generate dataset -- 1.9 GB - 0.9 GB
XS/XS -eo -es -t 2 -n 20000 -ls 100000       -f 0.2,0.2,0.2,0.2,0.2       A
XS/XS -eo -es -t 4 -n 10000 -ld 75000:100000 -f 0.24,0.24,0.24,0.24,0.04  B

sed -i 's/@/>/g' "A" "B"    # Replace @ symbol with > for the headers
mv A $dataset/$FA/$Synth/SynFA-1.$fasta
mv B $dataset/$FA/$Synth/SynFA-2.$fasta