          #######################################################
          #          Download viruses (FASTA) -- 350 MB         #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

### Create a folder for FASTA files and one for viruses dataset
if [[ ! -d $dataset/$FA/$VIRUSES ]]; then  mkdir -p $dataset/$FA/$VIRUSES;  fi

### Download
perl ./$script/DownloadViruses.pl

### Remove blank lines in downloaded file and move it to dataset folder
grep -Ev "^$" viruses.fa > $dataset/$FA/$VIRUSES/viruses.$fasta
rm -f viruses.fa