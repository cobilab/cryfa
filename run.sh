          #######################################################
          #                         Run                         #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Parameters -- set to 1 to activate, and 0 to deactivate
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

### Dataset
GET_DATASET=0               # 54 GB free disk space
    DL_HUMAN_FA=0           # Download Human             (FASTA) -- 3.1  GB
    DL_VIRUSES_FA=0         # Download Viruses           (FASTA) -- 0.3  GB
    GEN_SYNTH_FA=0          # Generate synthetic dataset (FASTA) -- 2.8  GB
    DL_HUMAN_FQ=0           # Download Human             (FASTQ) -- 27.0 GB
    DL_DENISOVA_FQ=0        # Download Denisova          (FASTQ) -- 2.9  GB
    GEN_SYNTH_FQ=0          # Generate synthetic dataset (FASTQ) -- 6.1  GB
    DL_DENISOVA_VCF=0       # Download Denisova          (VCF)   -- 6.3  GB
    DL_NEANDERTHAL_VCF=0    # Download Neanderthal       (VCF)   -- 0.8  GB
    DL_HUMAN_SAM=0          # Download Human             (SAM)   -- 0.5  GB
    DL_NEANDERTHAL_SAM=0    # Download Neanderthal       (SAM)   -- 1.5  GB
    DL_HUMAN_BAM=0          # Download Human             (BAM)   -- 0.6  GB
    DL_NEANDERTHAL_BAM=0    # Download Neanderthal       (BAM)   -- 1.3  GB

### Dependencies
INSTALL_DEPENDENCIES=0
    INS_7ZIP=1          # 7zip
    INS_CMAKE=1         # Cmake
    INS_BOOST=0         # Boost
    INS_CURL=0          # Curl
    INS_VALGRIND=0      # Valgrind
    INS_ZLIB=0          # Zlib
    INS_SAMTOOLS=0      # Samtools

### Install methods
INSTALL_METHODS=0
    # FASTA/FASTQ/VCF/SAM/BAM
    INS_CRYFA=1         # Cryfa
    # FASTA
    INS_MFCOMPRESS=0    # MFCompress -- error: make -- executables available
    INS_DELIMINATE=0    # DELIMINATE -- error: site not reachable -- exec avail.
    # FASTQ
    INS_FQZCOMP=0       # fqzcomp
    INS_QUIP=0          # Quip
    INS_DSRC=0          # DSRC
    INS_FQC=0           # FQC -- error: site not reachable -- exec available
    # Encryption -- FASTA/FASTQ/VCF/SAM/BAM
    INS_AESCRYPT=0      # AES Crypt

### Run compression methods
RUN_METHODS_COMP=0      # 100 GB free disk space
    # FASTA
    RUN_CRYFA_FA=0      # Cryfa
    # FASTQ
    RUN_CRYFA_FQ=0      # Cryfa
    # VCF
    RUN_CRYFA_VCF=0     # Cryfa
    # SAM
    RUN_CRYFA_SAM=0     # Cryfa
    # BAM
    RUN_CRYFA_BAM=0     # Cryfa
    # Results
    RESULTS_COMP=0

### Run compression & encryption methods
RUN_METHODS_COMP_ENC=0
    # FASTA -- 10 GB free disk space
    RUN_GZIP_FA_AESCRYPT=0       # gzip       + AES Crypt
    RUN_BZIP2_FA_AESCRYPT=0      # bzip2      + AES Crypt
    RUN_MFCOMPRESS_AESCRYPT=0    # MFCompress + AES Crypt
    RUN_DELIMINATE_AESCRYPT=0    # DELIMINATE + AES Crypt
    # FASTQ -- 50 GB free disk space
    RUN_GZIP_FQ_AESCRYPT=0       # gzip       + AES Crypt
    RUN_BZIP2_FQ_AESCRYPT=0      # bzip2      + AES Crypt
    RUN_FQZCOMP_AESCRYPT=0       # fqzcomp    + AES Crypt
    RUN_QUIP_AESCRYPT=0          # Quip       + AES Crypt
    RUN_DSRC_AESCRYPT=0          # DSRC       + AES Crypt
    RUN_FQC_AESCRYPT=0           # FQC        + AES Crypt
    # Results
    RESULTS_COMP_ENC=0

### Run encryption methods
RUN_METHODS_ENC=0       # 110 GB free disk space
    RUN_AESCRYPT=0      # AES Crypt
    # Results
    RESULTS_ENC=0

### Run Cryfa with different number of threads
RUN_CRYFA_THREADS=0
    MAX_N_THR=8         # Max number of threads
    CRYFA_THR_DATASET="dataset/FA/V/viruses.fasta"
#    CRYFA_THR_DATASET="dataset/FQ/DS/DS-B1088_SR.fastq"
    # Run
    RUN_CRYFA_THR=1
    # Results
    RESULTS_CRYFA_THR=0

### Run different methods to explore redundancy
RUN_REDUNDANCY=0        # Cryfa, DELIMINATE, MFCompress
    # Dataset (FASTA) -- archaea, bacteria, fungi, plants, viruses
    GET_DATASET_REDUN=1 # 12 GB free disk space
    # Run & Results
    RUN_RES_REDUN=1


################################################################################
##################         D O   N O T   C H A N G E         ###################
################################################################################
. script/par.sh    # Parameters

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Get dataset
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GET_DATASET -eq 1 ]];
then
    ### Create a folder for dataset, if it doesn't already exist
    if [[ ! -d $dataset ]]; then  mkdir -p $dataset;  fi

    ### FASTA
    if [[ $DL_HUMAN_FA        -eq 1 ]]; then . $script/dl_human_fa.sh;        fi
    if [[ $DL_VIRUSES_FA      -eq 1 ]]; then . $script/dl_viruses_fa.sh;      fi
    if [[ $GEN_SYNTH_FA       -eq 1 ]]; then . $script/gen_synth_fa.sh;       fi

    ### FASTQ
    if [[ $DL_HUMAN_FQ        -eq 1 ]]; then . $script/dl_human_fq.sh;        fi
    if [[ $DL_DENISOVA_FQ     -eq 1 ]]; then . $script/dl_denisova_fq.sh;     fi
    if [[ $GEN_SYNTH_FQ       -eq 1 ]]; then . $script/gen_synth_fq.sh;       fi

    ### VCF
    if [[ $DL_DENISOVA_VCF    -eq 1 ]]; then . $script/dl_denisova_vcf.sh;    fi
    if [[ $DL_NEANDERTHAL_VCF -eq 1 ]]; then . $script/dl_neanderthal_vcf.sh; fi

    ### SAM
    if [[ $DL_HUMAN_SAM       -eq 1 ]]; then . $script/dl_human_sam.sh;       fi
    if [[ $DL_NEANDERTHAL_SAM -eq 1 ]]; then . $script/dl_neanderthal_sam.sh; fi

    ### BAM
    if [[ $DL_HUMAN_BAM       -eq 1 ]]; then . $script/dl_human_bam.sh;       fi
    if [[ $DL_NEANDERTHAL_BAM -eq 1 ]]; then . $script/dl_neanderthal_bam.sh; fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Install dependencies
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $INSTALL_DEPENDENCIES -eq 1 ]];
then
    if [[ $INS_7ZIP     -eq 1 ]]; then  . $script/dep_7zip.sh;      fi
    if [[ $INS_CMAKE    -eq 1 ]]; then  . $script/dep_cmake.sh;     fi
    if [[ $INS_BOOST    -eq 1 ]]; then  . $script/dep_boost.sh;     fi
    if [[ $INS_CURL     -eq 1 ]]; then  . $script/dep_curl.sh;      fi
    if [[ $INS_VALGRIND -eq 1 ]]; then  . $script/dep_valgrind.sh;  fi
    if [[ $INS_ZLIB     -eq 1 ]]; then  . $script/dep_zlib.sh;      fi
    if [[ $INS_SAMTOOLS -eq 1 ]]; then  . $script/dep_samtools.sh;  fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Install methods
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $INSTALL_METHODS -eq 1 ]];
then
    ### Create a folder for programs, if it doesn't already exist
    if [[ ! -d $progs ]]; then mkdir -p $progs; fi

    ### FASTA/FASTQ
    if [[ $INS_CRYFA      -eq 1 ]]; then  . $script/ins_cryfa.sh;       fi
    if [[ $INS_AESCRYPT   -eq 1 ]]; then  . $script/ins_aescrypt.sh;    fi

    ### FASTA
    if [[ $INS_MFCOMPRESS -eq 1 ]]; then  . $script/ins_mfcompress.sh;  fi
    if [[ $INS_DELIMINATE -eq 1 ]]; then  . $script/ins_deliminate.sh;  fi

    ### FASTQ
    if [[ $INS_FQZCOMP    -eq 1 ]]; then  . $script/ins_fqzcomp.sh;     fi
    if [[ $INS_QUIP       -eq 1 ]]; then  . $script/ins_quip.sh;        fi
    if [[ $INS_DSRC       -eq 1 ]]; then  . $script/ins_dsrc.sh;        fi
    if [[ $INS_FQC        -eq 1 ]]; then  . $script/ins_fqc.sh;         fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Run compression methods
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $RUN_METHODS_COMP -eq 1 ]];
then
    ### Create folders for programs and results, if they don't already exist
    if [[ ! -d $progs  ]]; then  mkdir -p $progs;   fi
    if [[ ! -d $result ]]; then  mkdir -p $result;  fi

    ### Check if the whole dataset is available
    . $script/avail_dataset.sh;

    ### Compression functions
    . $script/run_fn_comp.sh

    ### FASTA
    if [[ $RUN_CRYFA_FA   -eq 1 ]]; then  compDecompOnDataset cryfa fa;  fi

    ### FASTQ
    if [[ $RUN_CRYFA_FQ   -eq 1 ]]; then  compDecompOnDataset cryfa fq;  fi

    ### VCF
    if [[ $RUN_CRYFA_VCF  -eq 1 ]]; then  compDecompOnDataset cryfa vcf; fi

    ### SAM
    if [[ $RUN_CRYFA_SAM  -eq 1 ]]; then  compDecompOnDataset cryfa sam; fi

    ### BAM
    if [[ $RUN_CRYFA_BAM  -eq 1 ]]; then  compDecompOnDataset cryfa bam; fi

    ### Results
    if [[ $RESULTS_COMP   -eq 1 ]]; then  . $script/res_comp.sh;         fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Run encryption methods
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $RUN_METHODS_ENC -eq 1 ]];
then
    ### Create folders for programs and results, if they don't already exist
    if [[ ! -d $progs   ]]; then  mkdir -p $progs;   fi
    if [[ ! -d $result  ]]; then  mkdir -p $result;  fi

    ### Check if the whole dataset is available
    . $script/avail_dataset.sh;

    ### Encryption functions
    . $script/run_fn_enc.sh

    ### FASTA/FASTQ/VCF/SAM/BAM
    if [[ $RUN_AESCRYPT -eq 1 ]]; then  encDecOnDataset aescrypt;  fi

    ### Results
    if [[ $RESULTS_ENC  -eq 1 ]]; then  . $script/res_enc.sh;      fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Run compression+encryption methods
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $RUN_METHODS_COMP_ENC -eq 1 ]];
then
    ### Create folders for programs and results, if they don't already exist
    if [[ ! -d $progs   ]]; then  mkdir -p $progs;   fi
    if [[ ! -d $result  ]]; then  mkdir -p $result;  fi

    ### Check if the whole dataset is available
    . $script/avail_dataset.sh;

    ### Compression+encryption functions
    . $script/run_fn_comp_enc.sh

    ### FASTA + Encryption
    if [[ $RUN_GZIP_FA_AESCRYPT    -eq 1 ]]; then
        compEncDecDecompOnDataset gzip       fa aescrypt;
    fi
    if [[ $RUN_BZIP2_FA_AESCRYPT   -eq 1 ]]; then
        compEncDecDecompOnDataset bzip2      fa aescrypt;
    fi
    if [[ $RUN_MFCOMPRESS_AESCRYPT -eq 1 ]]; then
        compEncDecDecompOnDataset mfcompress fa aescrypt;
    fi
    if [[ $RUN_DELIMINATE_AESCRYPT -eq 1 ]]; then
        compEncDecDecompOnDataset delim      fa aescrypt;
    fi

    ### FASTQ + Encryption
    if [[ $RUN_GZIP_FQ_AESCRYPT    -eq 1 ]]; then
        compEncDecDecompOnDataset gzip       fq aescrypt;
    fi
    if [[ $RUN_BZIP2_FQ_AESCRYPT   -eq 1 ]]; then
        compEncDecDecompOnDataset bzip2      fq aescrypt;
    fi
    if [[ $RUN_FQZCOMP_AESCRYPT    -eq 1 ]]; then
        compEncDecDecompOnDataset fqzcomp    fq aescrypt;
    fi
    if [[ $RUN_QUIP_AESCRYPT       -eq 1 ]]; then
        compEncDecDecompOnDataset quip       fq aescrypt;
    fi
    if [[ $RUN_DSRC_AESCRYPT       -eq 1 ]]; then
        compEncDecDecompOnDataset dsrc       fq aescrypt;
    fi
    if [[ $RUN_FQC_AESCRYPT        -eq 1 ]]; then
        compEncDecDecompOnDataset fqc        fq aescrypt;
    fi

    ### Results
    if [[ $RESULTS_COMP_ENC -eq 1 ]]; then  . $script/res_comp_enc.sh;  fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Run Cryfa with different number of threads
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $RUN_CRYFA_THREADS -eq 1 ]];
then
    ### Create a folder for results, if it doesn't already exist
    if [[ ! -d $result  ]]; then  mkdir -p $result;  fi

    ### Check if the dataset is available
    if [[ ! -e $CRYFA_THR_DATASET ]]; then
        echo "Error: The file \"$CRYFA_THR_DATASET\" is not available.";
        return;
    fi

    ### Cryfa compress/decompress function
    . $script/run_fn_cryfa_thr.sh

    ### Run
    if [[ $RUN_CRYFA_THR     -eq 1 ]]; then  runCryfa;  fi

    ### Results
    if [[ $RESULTS_CRYFA_THR -eq 1 ]]; then  . $script/res_cryfa_thr.sh;  fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Run different methods to check exploring redundancy
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $RUN_REDUNDANCY -eq 1 ]];
then
    ### Create a folder for results, if it doesn't already exist
    if [[ ! -d $result  ]]; then  mkdir -p $result;  fi

    ### Download datasets
    if [[ $GET_DATASET_REDUN -eq 1 ]]; then  . $script/dl_dataset_redun.sh;  fi

    ### Run & Results
    if [[ $RUN_RES_REDUN     -eq 1 ]]; then  . $script/run_res_redun.sh;     fi
fi