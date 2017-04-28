#!/bin/bash


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   get datasets, install dependencies, run programs
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
INSTALL_DELIMINATE=1;
INSTALL_MFCOMPRESS=1;
INSTALL_CRYFA=1;
CREATE_PASS=1;
GET_HUMAN=1;
RUN_CRYFA=1;
RUN_MFCOMPRESS=1;
RUN_DELIMINATE=1;


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   get MFCompress
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $INSTALL_MFCOMPRESS -eq 1 ]]; then
    rm -f MFCompress-src-1.01.tgz;
    wget http://sweet.ua.pt/ap/software/mfcompress/MFCompress-src-1.01.tgz
    tar -xzf MFCompress-src-1.01.tgz
    mv MFCompress-src-1.01/ mfcompress
    cd mfcompress/
    cp Makefile.linux Makefile # make -f Makefile.linux
    make
    cp MFCompressC ..
    cp MFCompressD ..
    cd ..
    rm -fr MFCompress-src-1.01.tgz MFCompress-src-1.01/
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   get DELIMINATE
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $INSTALL_DELIMINATE -eq 1 ]]; then
    rm -f DELIMINATE_LINUX_64bit.tar.gz;
    DELSEV="metagenomics.atc.tcs.com/Compression_archive";
    wget http://$DELSEV/DELIMINATE_LINUX_64bit.tar.gz
    tar -xzf DELIMINATE_LINUX_64bit.tar.gz
    mv EXECUTABLES deliminate
    cd deliminate
    cp 7za ../
    cp delim ../
    cp delimcs ../ 
    cp delimds ../
    cd ..
    rm -fr DELIMINATE_LINUX_64bit.tar.gz deliminate;
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   get cryfa
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $INSTALL_CRYFA -eq 1 ]]; then
    git clone https://github.com/pratas/cryfa.git
    cd cryfa/src/
    git clone https://github.com/weidai11/cryptopp
    cd cryptopp/
    make
    cp libcryptopp.a ..
    cd ..
    g++ -std=c++0x -I cryptopp -o cryfa cryfa.cpp defs.h libcryptopp.a
    cp cryfa ../../CRYF
    cd ../../
    rm -fr cryfa/
    mv CRYF cryfa
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   create pass
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CREATE_PASS -eq 1 ]]; then echo "TestPassword" > pass.txt; fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   get human
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GET_HUMAN -eq 1 ]]; then
    git clone https://github.com/pratas/goose
    cp goose/scripts/GetHumanGRC37Parse.sh .
    . GetHumanGRC37Parse.sh
    rm -fr goose GetHumanGRC37Parse.sh;
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   run cryfa
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $RUN_CRYFA -eq 1 ]]; then
    for x in {1..24}; do
        echo "Running cryfa HS$x";
        (time ./cryfa -k pass.txt HS$x > HS-ENC-CRYFA$x ) &> REPORT-CRYFA-HS$x;
        ls -la HS-ENC-CRYFA$x >> REPORT-CRYFA-HS$x;
    done
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   run MFCompress
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $RUN_MFCOMPRESS -eq 1 ]]; then
    for x in {1..24}; do
        echo "Running mfc HS$x";
        (time ./MFCompressC -o HS-ENC-MFC$x HS$x ) &> REPORT-MFC-HS$x;
        ls -la HS-ENC-MFC$x >> REPORT-MFC-HS$x;
    done
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   run DELIMINATE
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $RUN_DELIMINATE -eq 1 ]]; then
    for x in {1..24}; do
        echo "Running DELIM HS$x";
        (time ./delim a HS$x ) &> REPORT-DEL-HS$x;
        ls -la HS$x.dlim >> REPORT-DEL-HS$x;
    done
fi
