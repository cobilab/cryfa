          #######################################################
          #    Generate synthetic dataset (FASTQ) -- 6.7 GB     #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

INSTALL_XS=1    # Install XS simulator

### Create a folder for FASTQ files and one for synthetic dataset
if [[ ! -d $dataset/$FQ/$Synth ]]; then  mkdir -p $dataset/$FQ/$Synth;  fi

### Install XS
if [[ $INSTALL_XS -eq 1 ]]; then
    rm -fr $XS
    git clone https://github.com/pratas/XS.git
    cd $XS
    make
    cd ..
fi

### Generate dataset -- 4.6 GB - 2.1 GB
XS/XS -t 1 -n 16000000 -ld 70:100 -o \
      -f 0.2,0.2,0.2,0.2,0.2       $dataset/$FQ/$Synth/SynFQ-1.$fastq
XS/XS -t 2 -n 10000000 -ls 70 -qt 2 \
      -f 0.23,0.23,0.23,0.23,0.08  $dataset/$FQ/$Synth/SynFQ-2.$fastq