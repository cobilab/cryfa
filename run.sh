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
RUN_METHODS_COMP=0
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
    MAX_N_THR=8                  # max number of threads
    CRYFA_XCL_DATASET="dataset/FA/V/viruses.fasta"
    RUN_CRYFA_XCL=1
    PRINT_RESULTS_CRYFA_XCL=1



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

    ### Check if dataset is available
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
    ### create folders, if they don't already exist
    if [[ ! -d $progs   ]]; then  mkdir -p $progs;   fi
    if [[ ! -d $result  ]]; then  mkdir -p $result;  fi

    ### Dataset availablity
    . $script/avail_dataset.sh;

    ### Functions
    . $script/run_fn_enc.sh

    ### AEScrypt
    if [[ $RUN_AESCRYPT -eq 1 ]]; then  encDecOnDataset aescrypt;  fi

    ### Results
    if [[ $RESULTS_ENC  -eq 1 ]]; then  . $script/res_enc.sh;      fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Run compression+encryption methods
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $RUN_METHODS_COMP_ENC -eq 1 ]];
then
    ### create folders, if they don't already exist
    if [[ ! -d $progs   ]]; then  mkdir -p $progs;   fi
    if [[ ! -d $result  ]]; then  mkdir -p $result;  fi

    ### Dataset availablity
    . $script/avail_dataset.sh;

    ### Functions
    . $script/run_fn_comp_enc.sh

    ### FASTA + AEScrypt
    if [[ $RUN_GZIP_FA_AESCRYPT -eq 1 ]]; then
      compEncDecDecompOnDataset gzip fa aescrypt;
    fi
    if [[ $RUN_BZIP2_FA_AESCRYPT -eq 1 ]]; then
      compEncDecDecompOnDataset bzip2 fa aescrypt;
    fi
    if [[ $RUN_MFCOMPRESS_AESCRYPT -eq 1 ]]; then
      compEncDecDecompOnDataset mfcompress fa aescrypt;
    fi
    if [[ $RUN_DELIMINATE_AESCRYPT -eq 1 ]]; then
      compEncDecDecompOnDataset delim fa aescrypt;
    fi

    ### FASTQ + AEScrypt
    if [[ $RUN_GZIP_FQ_AESCRYPT -eq 1 ]]; then
      compEncDecDecompOnDataset gzip fq aescrypt;
    fi
    if [[ $RUN_BZIP2_FQ_AESCRYPT -eq 1 ]]; then
      compEncDecDecompOnDataset bzip2 fq aescrypt;
    fi
    if [[ $RUN_FQZCOMP_AESCRYPT -eq 1 ]]; then
      compEncDecDecompOnDataset fqzcomp fq aescrypt;
    fi
    if [[ $RUN_QUIP_AESCRYPT -eq 1 ]]; then
      compEncDecDecompOnDataset quip fq aescrypt;
    fi
    if [[ $RUN_DSRC_AESCRYPT -eq 1 ]]; then
      compEncDecDecompOnDataset dsrc fq aescrypt;
    fi
    if [[ $RUN_FQC_AESCRYPT -eq 1 ]]; then
      compEncDecDecompOnDataset fqc fq aescrypt;
    fi

    ### Results
    if [[ $RESULTS_COMP_ENC -eq 1 ]]; then  . $script/res_comp_enc.sh;  fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Run cryfa, exclusively
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CRYFA_EXCLUSIVE -eq 1 ]];
then
    ### create folder, if it doesn't already exist
    if [[ ! -d $cryfa_xcl ]]; then mkdir -p $cryfa_xcl; fi

    inData="../$CRYFA_XCL_DATASET"
    in="${inData##*/}"                  # input file name
    inDataWF="${in%.*}"                 # input file name without filetype
    ft="${in##*.}"                      # input filetype
    fsize=`stat --printf="%s" $CRYFA_XCL_DATASET`    # file size (bytes)
    result_FLD="../$result"
    CRYFA_THR_RUN=`seq -s' ' 1 $MAX_N_THR`;
#    CRYFA_THR_RUN=$MAX_N_THR;

    ### run for different number of threads
    if [[ $RUN_CRYFA_XCL -eq 1 ]];
    then
        cmake .
        make
        cp cryfa pass.txt  $cryfa_xcl;

        cd $cryfa_xcl

        for nThr in $CRYFA_THR_RUN; do
            cFT="cryfa";
            cCmd="./cryfa -k $CRYFA_KEY_FILE -t $nThr";
            dCmd="./cryfa -k $CRYFA_KEY_FILE -t $nThr -d";

            # compress
            progMemoryStart cryfa &
            MEMPID=$!

            rm -f CRYFA_THR_${nThr}_CT__${inDataWF}_$ft

            (time $cCmd $inData > $in.$cFT) \
                &> $result_FLD/CRYFA_THR_${nThr}_CT__${inDataWF}_$ft

            ls -la $in.$cFT \
                > $result_FLD/CRYFA_THR_${nThr}_CS__${inDataWF}_$ft

            progMemoryStop $MEMPID \
                           $result_FLD/CRYFA_THR_${nThr}_CM__${inDataWF}_$ft

            # decompress
            progMemoryStart cryfa &
            MEMPID=$!

            (time $dCmd $in.$cFT > $in) \
                &> $result_FLD/CRYFA_THR_${nThr}_DT__${inDataWF}_$ft

            progMemoryStop $MEMPID \
                           $result_FLD/CRYFA_THR_${nThr}_DM__${inDataWF}_$ft

            # verify if input and decompressed files are the same
            cmp $inData $in &>$result_FLD/CRYFA_THR_${nThr}_V__${inDataWF}_$ft
        done

        cd ..
    fi

    ### print results for different number of threads
    if [[ $PRINT_RESULTS_CRYFA_XCL -eq 1 ]];
    then
        result_FLD="../$result"
        OUT="CRYFA_THR.$RES"       # output file name
        cd $cryfa_xcl

        c="C_Size(B)\tC_Time_real(s)\tC_Time_user(s)\tC_Time_sys(s)\t"
        c+="C_Mem(KB)"
        d="D_Time_real(s)\tD_Time_user(s)\tD_Time_sys(s)\tD_Mem(KB)"
        printf "Dataset\tSize(B)\tThread\t$c\t$d\tEq\n" > $OUT;

        for nThr in $CRYFA_THR_RUN; do
            ### print compress/decompress results
            CS="";       CT_r="";     CT_u="";     CT_s="";     CM="";
            DT_r="";     DT_u="";     DT_s="";     DM="";       V="";

            ### compressed file size
            cs_file="$result_FLD/CRYFA_THR_${nThr}_CS__${inDataWF}_$ft"
            if [[ -e $cs_file ]];
                then CS=`cat $cs_file | awk '{ print $5; }'`;
            fi

            ### compression time -- real - user - system
            ct_file="$result_FLD/CRYFA_THR_${nThr}_CT__${inDataWF}_$ft"
            if [[ -e $ct_file ]]; then
                CT_r=`cat $ct_file |tail -n 3 |head -n 1 | awk '{ print $2;}'`
                CT_u=`cat $ct_file |tail -n 2 |head -n 1 | awk '{ print $2;}'`
                CT_s=`cat $ct_file |tail -n 1 |awk '{ print $2;}'`
            fi

            ### compression memory
            cm_file="$result_FLD/CRYFA_THR_${nThr}_CM__${inDataWF}_$ft"
            if [[ -e $cm_file ]]; then CM=`cat $cm_file`; fi

            ### decompression time -- real - user - system
            dt_file="$result_FLD/CRYFA_THR_${nThr}_DT__${inDataWF}_$ft"
            if [[ -e $dt_file ]]; then
                DT_r=`cat $dt_file |tail -n 3 |head -n 1 | awk '{ print $2;}'`
                DT_u=`cat $dt_file |tail -n 2 |head -n 1 | awk '{ print $2;}'`
                DT_s=`cat $dt_file |tail -n 1 |awk '{ print $2;}'`
            fi

            ### decompression memory
            dm_file="$result_FLD/CRYFA_THR_${nThr}_DM__${inDataWF}_$ft"
            if [[ -e $dm_file ]]; then DM=`cat $dm_file`; fi

            ### if decompressed file is the same as the original file
            v_file="$result_FLD/CRYFA_THR_${nThr}_V__${inDataWF}_$ft"
            if [[ -e $v_file ]]; then V=`cat $v_file | wc -l`; fi

            c="$CS\t$CT_r\t$CT_u\t$CT_s\t$CM"   # compression results
            d="$DT_r\t$DT_u\t$DT_s\t$DM"        # decompression results

            printf "$inDataWF\t$fsize\t$nThr\t$c\t$d\t$V\n" >> $OUT;
        done

        ### convert the result file into a human readable file
        cryfaXclResHumanReadable $OUT;

        cd ..
    fi
fi