          #######################################################
          #                   Install Samtools                  #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

prog="samtools-1.8"
wget $WGET_OP \
     https://github.com/samtools/samtools/releases/download/1.8/$prog.tar.bz2
tar xvjf $prog.tar.bz2
rm $prog.tar.bz2
cd $prog/
./configure
make    # make -j7
sudo make install
cd ..