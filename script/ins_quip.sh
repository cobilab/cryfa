          #######################################################
          #                Install Quip -- FASTQ                #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

rm -f quip-1.1.8.tar.gz

url="http://homes.cs.washington.edu/~dcjones/quip"
wget $WGET_OP $url/quip-1.1.8.tar.gz
tar -xzf quip-1.1.8.tar.gz
mv quip-1.1.8/ quip/    # Rename
mv quip/ $progs/
rm -f quip-1.1.8.tar.gz

cd $progs/quip/
./configure
cd src/
make
cp quip ../

cd ../../..