          #######################################################
          #             Install MFCompress -- FASTA             #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

rm -f MFCompress-src-1.01.tgz

url="http://sweet.ua.pt/ap/software/mfcompress"
wget $WGET_OP $url/MFCompress-src-1.01.tgz
tar -xzf MFCompress-src-1.01.tgz
mv MFCompress-src-1.01/ mfcompress/    # Rename
mv mfcompress/ $progs/
rm -f MFCompress-src-1.01.tgz

cd $progs/mfcompress/
cp Makefile.linux Makefile    # make -f Makefile.linux
make
cd ../..