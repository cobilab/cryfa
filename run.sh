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
GET_DATASET=0
    DL_HUMAN_FA=0       # Download human             (FASTA) -- 3.1 GB
    DL_VIRUSES_FA=0     # Download viruses           (FASTA) -- 350 MB
    GEN_SYNTH_FA=0      # Generate synthetic dataset (FASTA) -- 4   GB
    DL_HUMAN_FQ=0       # Download human             (FASTQ) -- 27  GB
    DL_DENISOVA_FQ=0    # Download denisova          (FASTQ) -- 172 GB
    GEN_SYNTH_FQ=0      # Generate synthetic dataset (FASTQ) -- 6.2 GB

### Dependencies
INSTALL_DEPENDENCIES=0
    INS_7ZIP=1          # 7zip
    INS_CMAKE=1         # Cmake
    INS_BOOST=1         # Boost
    INS_CURL=1          # Curl
    INS_VALGRIND=1      # Valgrind
    INS_ZLIB=1          # Zlib

### Install methods
INSTALL_METHODS=0
    INS_CRYFA=1         # Cryfa
    # FASTA
    INS_MFCOMPRESS=0    # MFCompress -- error: make -- executables available
    INS_DELIMINATE=0    # DELIMINATE -- error: site not reachable -- exec avail.
    # FASTQ
    INS_FQZCOMP=0       # Fqzcomp
    INS_QUIP=0          # Quip
    INS_DSRC=0          # DSRC
    INS_FQC=0           # FQC -- error: site not reachable -- exec available
    # Encryption
    INS_AESCRYPT=0      # AEScrypt

### Run compression methods
RUN_METHODS_COMP=1
    # FASTA
    RUN_GZIP_FA=0       # Gzip
    RUN_BZIP2_FA=0      # Bzip2
    RUN_MFCOMPRESS=0    # MFCompress
    RUN_DELIMINATE=0    # DELIMINATE
    RUN_CRYFA_FA=0      # Cryfa
    # FASTQ
    RUN_GZIP_FQ=0       # Gzip
    RUN_BZIP2_FQ=0      # Bzip2
    RUN_FQZCOMP=0       # Fqzcomp
    RUN_QUIP=0          # Quip
    RUN_DSRC=0          # DSRC
    RUN_FQC=0           # FQC
    RUN_CRYFA_FQ=0      # Cryfa
    # Results
    RESULTS_COMP=1

### Run encryption methods
RUN_METHODS_ENC=0
    RUN_AESCRYPT=0      # AEScrypt
    # Results
    RESULTS_ENC=1

### Run compression+encryption methods
RUN_METHODS_COMP_ENC=0
    # FASTA
    RUN_GZIP_FA_AESCRYPT=0       # Gzip       + AEScrypt
    RUN_BZIP2_FA_AESCRYPT=0      # Bzip2      + AEScrypt
    RUN_MFCOMPRESS_AESCRYPT=0    # MFCompress + AEScrypt
    RUN_DELIMINATE_AESCRYPT=0    # DELIMINATE + AEScrypt
    # FASTQ
    RUN_GZIP_FQ_AESCRYPT=0       # Gzip       + AEScrypt
    RUN_BZIP2_FQ_AESCRYPT=0      # Bzip2      + AEScrypt
    RUN_FQZCOMP_AESCRYPT=0       # Fqzcomp    + AEScrypt
    RUN_QUIP_AESCRYPT=0          # Quip       + AEScrypt
    RUN_DSRC_AESCRYPT=0          # DSRC       + AEScrypt
    RUN_FQC_AESCRYPT=0           # FQC        + AEScrypt
    # Results
    RESULTS_COMP_ENC=1

### Run cryfa, exclusively
CRYFA_EXCLUSIVE=0
    MAX_N_THR=8                  # Max number of threads
    CRYFA_XCL_DATASET="dataset/FA/V/viruses.fasta"
    RUN_CRYFA_XCL=0
    RESULTS_CRYFA_XCL=1



################################################################################
#################          D O   N O T   C H A N G E          ##################
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
    if [[ $DL_HUMAN_FA    -eq 1 ]]; then  . $script/dl_human_fa.sh;     fi
    if [[ $DL_VIRUSES_FA  -eq 1 ]]; then  . $script/dl_viruses_fa.sh;   fi
    if [[ $GEN_SYNTH_FA   -eq 1 ]]; then  . $script/gen_synth_fa.sh;    fi

    ### FASTQ
    if [[ $DL_HUMAN_FQ    -eq 1 ]]; then  . $script/dl_human_fq.sh;     fi
    if [[ $DL_DENISOVA_FQ -eq 1 ]]; then  . $script/dl_denisova_fq.sh;  fi
    if [[ $GEN_SYNTH_FQ   -eq 1 ]]; then  . $script/gen_synth_fq.sh;    fi
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
    if [[ $RUN_GZIP_FA    -eq 1 ]]; then  compDecompOnDataset gzip       fa;  fi
    if [[ $RUN_BZIP2_FA   -eq 1 ]]; then  compDecompOnDataset bzip2      fa;  fi
    if [[ $RUN_MFCOMPRESS -eq 1 ]]; then  compDecompOnDataset mfcompress fa;  fi
    if [[ $RUN_DELIMINATE -eq 1 ]]; then  compDecompOnDataset delim      fa;  fi
    if [[ $RUN_CRYFA_FA   -eq 1 ]]; then  compDecompOnDataset cryfa      fa;  fi

    ### FASTQ
    if [[ $RUN_GZIP_FQ    -eq 1 ]]; then  compDecompOnDataset gzip       fq;  fi
    if [[ $RUN_BZIP2_FQ   -eq 1 ]]; then  compDecompOnDataset bzip2      fq;  fi
    if [[ $RUN_FQZCOMP    -eq 1 ]]; then  compDecompOnDataset fqzcomp    fq;  fi
    if [[ $RUN_QUIP       -eq 1 ]]; then  compDecompOnDataset quip       fq;  fi
    if [[ $RUN_DSRC       -eq 1 ]]; then  compDecompOnDataset dsrc       fq;  fi
    if [[ $RUN_FQC        -eq 1 ]]; then  compDecompOnDataset fqc        fq;  fi
    if [[ $RUN_CRYFA_FQ   -eq 1 ]]; then  compDecompOnDataset cryfa      fq;  fi

    ### Results
    if [[ $RESULTS_COMP -eq 1 ]]; then  . $script/res_comp.sh;  fi
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

    ### FASTA/FASTQ
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
#   Run cryfa, exclusively -- for different number of threads
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CRYFA_EXCLUSIVE -eq 1 ]];
then
    ### Create a folder for cryfa, if it doesn't already exist
    if [[ ! -d $cryfa_xcl ]]; then  mkdir -p $cryfa_xcl;  fi

    ### Check if the dataset is available
    if [[ ! -e $CRYFA_XCL_DATASET ]]; then
        echo "Error: The file \"$CRYFA_XCL_DATASET\" is not available.";
        return;
    fi

    ### Cryfa compress/decompress function
    . $script/run_cryfa_xcl.sh

    ### Run
    if [[ $RUN_CRYFA_XCL     -eq 1 ]]; then  runCryfa;  fi

    ### Results
    if [[ $RESULTS_CRYFA_XCL -eq 1 ]]; then  . $script/res_cryfa_xcl.sh;  fi
fi