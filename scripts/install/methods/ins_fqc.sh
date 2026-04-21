#!/usr/bin/env bash

# Install FQC -- FASTQ

rm -f FQC_LINUX_64bit.tar.gz

url="http://metagenomics.atc.tcs.com/Compression_archive/FQC"
wget $WGET_OP $url/FQC_LINUX_64bit.tar.gz
tar -xzf FQC_LINUX_64bit.tar.gz
mv FQC_LINUX_64bit/ fqc/ # Rename
mv fqc/ $progs/
rm -f FQC_LINUX_64bit.tar.gz
