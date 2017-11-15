          #######################################################
          #                Install DSRC -- FASTQ                #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

rm -fr dsrc/

git clone https://github.com/lrog/dsrc.git
mv dsrc/ $progs/

cd $progs/dsrc/
make
cp bin/dsrc .

cd ../..