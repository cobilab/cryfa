          #######################################################
          #              Install fqzcomp -- FASTQ               #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

rm -f fqzcomp-4.6.tar.gz

url="https://downloads.sourceforge.net/project/fqzcomp"
wget $WGET_OP $url/fqzcomp-4.6.tar.gz
tar -xzf fqzcomp-4.6.tar.gz
mv fqzcomp-4.6/ fqzcomp/    # Rename
mv fqzcomp/ $progs/
rm -f fqzcomp-4.6.tar.gz

cd $progs/fqzcomp/
make

cd ../..