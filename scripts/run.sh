#!/bin/bash
###############################################################################
INSTALL_DELIMINATE=0;
INSTALL_MFCOMPRESS=0;
INSTALL_CRYFA=0;
CREATE_PASS=0;
GET_HUMAN=0;
RUN_CRYFA=0;
RUN_MFCOMPRESS=0;
RUN_DELIMINATE=1;
# GET MFCOMPRESS ==============================================================
if [[ "$INSTALL_MFCOMPRESS" -eq "1" ]]; then
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
###############################################################################
# GET DELIMINATE ==============================================================
if [[ "$INSTALL_DELIMINATE" -eq "1" ]]; then
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
###############################################################################
# GET CRYFA ===================================================================
if [[ "$INSTALL_CRYFA" -eq "1" ]]; then
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
###############################################################################
if [[ "$CREATE_PASS" -eq "1" ]]; then
  echo "TestPassword" > pass.txt
fi
###############################################################################
if [[ "$GET_HUMAN" -eq "1" ]]; then
  git clone https://github.com/pratas/goose
  cp goose/scripts/GetHumanGRC37Parse.sh .
  . GetHumanGRC37Parse.sh
  rm -fr goose GetHumanGRC37Parse.sh;
fi
###############################################################################
if [[ "$RUN_CRYFA" -eq "1" ]]; then
for((x=1 ; x<25 ; ++x)); 
  do
  echo "Running cryfa HS$x";
  (time ./cryfa -k pass.txt HS$x > HS-ENC-CRYFA$x ) &> REPORT-CRYFA-HS$x;
  ls -la HS-ENC-CRYFA$x >> REPORT-CRYFA-HS$x;
  done
fi
###############################################################################
if [[ "$RUN_MFCOMPRESS" -eq "1" ]]; then
for((x=1 ; x<25 ; ++x));
  do
  echo "Running mfc HS$x";
  (time ./MFCompressC -o HS-ENC-MFC$x HS$x ) &> REPORT-MFC-HS$x;
  ls -la HS-ENC-MFC$x >> REPORT-MFC-HS$x;
  done
fi
###############################################################################
if [[ "$RUN_DELIMINATE" -eq "1" ]]; then
for((x=1 ; x<25 ; ++x));
  do
  echo "Running DELIM HS$x";
  (time ./delim a HS$x ) &> REPORT-DEL-HS$x;
  ls -la HS$x.dlim >> REPORT-DEL-HS$x;
  done
fi
###############################################################################






