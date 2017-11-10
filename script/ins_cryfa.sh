          #######################################################
          #             Install cryfa -- FASTA/FASTQ            #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

. par.sh        # Internal parameters

rm -f cryfa

cmake .
make

if [[ ! -d $progs/cryfa ]]; then  mkdir -p $progs/cryfa;  fi
cp cryfa pass.txt  $progs/cryfa/