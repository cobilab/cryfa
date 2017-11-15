          #######################################################
          #             Install DELIMINATE -- FASTA             #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

rm -f DELIMINATE_LINUX_64bit.tar.gz

url="http://metagenomics.atc.tcs.com/Compression_archive"
wget $WGET_OP $url/DELIMINATE_LINUX_64bit.tar.gz
tar -xzf DELIMINATE_LINUX_64bit.tar.gz
mv EXECUTABLES deliminate    # Rename
mv deliminate $progs/
rm -f DELIMINATE_LINUX_64bit.tar.gz