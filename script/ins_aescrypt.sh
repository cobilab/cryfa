          #######################################################
          #                  Install AES Crypt                  #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

rm -fr aescrypt-3.13/

url="https://www.aescrypt.com/download/v3/linux"
wget $WGET_OP $url/aescrypt-3.13.tgz
tar -xzvf aescrypt-3.13.tgz
mv aescrypt-3.13/ aescrypt/
mv aescrypt/ $progs/
rm -f aescrypt-3.13.tgz

cd $progs/aescrypt/src
make
sudo make install