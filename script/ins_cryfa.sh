          #######################################################
          #                    Install Cryfa                    #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

rm -f cryfa

cmake .
make

if [[ ! -d $progs/cryfa ]]; then  mkdir -p $progs/cryfa;  fi
cp cryfa pass.txt  $progs/cryfa/