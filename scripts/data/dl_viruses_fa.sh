#!/usr/bin/env bash

# Download viruses (FASTA) -- 350 MB

# Create a folder for FASTA files and one for viruses dataset
if [[ ! -d $dataset/$FA/$VIRUSES ]]; then
  mkdir -p $dataset/$FA/$VIRUSES
fi

# Download
perl "./$scripts_data_perl/DownloadViruses.pl"

# Remove blank lines in downloaded file and move it to dataset folder
grep -Ev "^$" viruses.fa >$dataset/$FA/$VIRUSES/viruses.$fasta
rm -f viruses.fa
