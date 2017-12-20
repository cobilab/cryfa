          #######################################################
          #    Generate synthetic dataset (FASTQ) -- 6.1 GB     #
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

### Generate dataset -- 5.6 GB - 512 MB
XS/XS -t 4 -n 23000000 -ld 70:150 -qc 63:70 -o -f 0.23,0.23,0.23,0.23,0.08  A
XS/XS -t 2 -n 2183500  -ld 70:120 -qc 56:70    -f 0.2,0.2,0.2,0.2,0.2       B

mv A $dataset/$FQ/$Synth/SynFQ-1.$fastq
mv B $dataset/$FQ/$Synth/SynFQ-2.$fastq
