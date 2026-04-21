#!/usr/bin/env bash

# Benchmark Cryfa Against Other Methods
#
# Maintainers:
# - Morteza Hosseini <seyedmorteza.hosseini@manchester.ac.uk>
# - Diogo Pratas     <pratas@ua.pt>
# - Armando J. Pinho <ap@ua.pt>
#
# Usage:
# - Set section flags to 1 to enable a task.
# - Leave flags at 0 to skip a task.

# Dataset
GET_DATASET=0        # 54 GB free disk space
DL_HUMAN_FA=0        # Download Human             (FASTA) -- 3.1  GB
DL_VIRUSES_FA=0      # Download Viruses           (FASTA) -- 0.3  GB
GEN_SYNTH_FA=0       # Generate synthetic dataset (FASTA) -- 2.8  GB
DL_HUMAN_FQ=0        # Download Human             (FASTQ) -- 27.0 GB
DL_DENISOVA_FQ=0     # Download Denisova          (FASTQ) -- 2.9  GB
GEN_SYNTH_FQ=0       # Generate synthetic dataset (FASTQ) -- 6.1  GB
DL_DENISOVA_VCF=0    # Download Denisova          (VCF)   -- 6.3  GB
DL_NEANDERTHAL_VCF=0 # Download Neanderthal       (VCF)   -- 0.8  GB
DL_HUMAN_SAM=0       # Download Human             (SAM)   -- 0.5  GB
DL_NEANDERTHAL_SAM=0 # Download Neanderthal       (SAM)   -- 1.5  GB
DL_HUMAN_BAM=0       # Download Human             (BAM)   -- 0.6  GB
DL_NEANDERTHAL_BAM=0 # Download Neanderthal       (BAM)   -- 1.3  GB

# Dependencies
INSTALL_DEPENDENCIES=0
INS_7ZIP=1     # 7zip
INS_CMAKE=1    # Cmake
INS_BOOST=0    # Boost
INS_CURL=0     # Curl
INS_VALGRIND=0 # Valgrind
INS_ZLIB=0     # Zlib
INS_SAMTOOLS=0 # Samtools

# Install methods
INSTALL_METHODS=0
# FASTA/FASTQ/VCF/SAM/BAM
INS_CRYFA=1 # Cryfa
# FASTA
INS_MFCOMPRESS=0 # MFCompress -- error: make -- executables available
INS_DELIMINATE=0 # DELIMINATE -- error: site not reachable -- exec avail.
# FASTQ
INS_FQZCOMP=0 # fqzcomp
INS_QUIP=0    # Quip
INS_DSRC=0    # DSRC
INS_FQC=0     # FQC -- error: site not reachable -- exec available
# Encryption -- FASTA/FASTQ/VCF/SAM/BAM
INS_AESCRYPT=0 # AES Crypt

# Run compression methods
RUN_METHODS_COMP=0 # 100 GB free disk space
# FASTA
RUN_CRYFA_FA=0 # Cryfa
# FASTQ
RUN_CRYFA_FQ=0 # Cryfa
# VCF
RUN_CRYFA_VCF=0 # Cryfa
# SAM
RUN_CRYFA_SAM=0 # Cryfa
# BAM
RUN_CRYFA_BAM=0 # Cryfa
# Results
RESULTS_COMP=0

# Run compression & encryption methods
RUN_METHODS_COMP_ENC=0
# FASTA -- 10 GB free disk space
RUN_GZIP_FA_AESCRYPT=0    # gzip       + AES Crypt
RUN_BZIP2_FA_AESCRYPT=0   # bzip2      + AES Crypt
RUN_MFCOMPRESS_AESCRYPT=0 # MFCompress + AES Crypt
RUN_DELIMINATE_AESCRYPT=0 # DELIMINATE + AES Crypt
# FASTQ -- 50 GB free disk space
RUN_GZIP_FQ_AESCRYPT=0  # gzip    + AES Crypt
RUN_BZIP2_FQ_AESCRYPT=0 # bzip2   + AES Crypt
RUN_FQZCOMP_AESCRYPT=0  # fqzcomp + AES Crypt
RUN_QUIP_AESCRYPT=0     # Quip    + AES Crypt
RUN_DSRC_AESCRYPT=0     # DSRC    + AES Crypt
RUN_FQC_AESCRYPT=0      # FQC     + AES Crypt
# Results
RESULTS_COMP_ENC=0

# Run encryption methods
RUN_METHODS_ENC=0 # 110 GB free disk space
RUN_AESCRYPT=0    # AES Crypt
# Results
RESULTS_ENC=0

# Run Cryfa with different number of threads
RUN_CRYFA_THREADS=0
MAX_N_THR=8 # Max number of threads
CRYFA_THR_DATASET="dataset/FA/V/viruses.fasta"
# CRYFA_THR_DATASET="dataset/FQ/DS/DS-B1088_SR.fastq"
# Run
RUN_CRYFA_THR=1
# Results
RESULTS_CRYFA_THR=0

# Run different methods to explore redundancy
RUN_REDUNDANCY=0 # Cryfa, DELIMINATE, MFCompress
# Dataset (FASTA) -- archaea, bacteria, fungi, plants, viruses
GET_DATASET_REDUN=1 # 12 GB free disk space
# Run & Results
RUN_RES_REDUN=1

################################################################################
# DO NOT CHANGE
################################################################################
source scripts/config/par.sh
source "$scripts_orchestration/benchmark_orchestrator.sh"

function main {
  run_benchmark
}

main "$@"
