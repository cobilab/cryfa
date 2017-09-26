          #######################################################
          #                 All in one script                   #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   parameters -- set to 1 to activate and 0 to deactivate
#   get/generate datasets, install dependencies, install & run methods
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

### datasets
GET_DATASETS=0
  DL_HUMAN_FA=0         # download Human choromosomes in FASTA
  DL_VIRUSES_FA=0       # download Viruses in FASTA using downloadViruses.pl
  GEN_SYNTH_FA=0        # generate synthetic FASTA dataset using XS
  DL_HUMAN_FQ=0         # download Human in FASTQ
  DL_DENISOVA_FQ=0      # download Denisova in FASTQ
  GEN_SYNTH_FQ=0        # generate synthetic FASTQ dataset using XS

### dependencies
INSTALL_DEPENDENCIES=0  # if this value is 0, no dependencies will be installed
  INS_7ZIP=1            # 7zip
  INS_CMAKE=1           # cmake
  INS_LIBBOOST=1        # boost
  INS_LIBCURL=1         # curl
  INS_VALGRIND=1        # valgrind
  INS_ZLIB=1            # zlib

### install methods
INSTALL_METHODS=0
  INS_CRYFA=0           # cryfa
  # FASTA
  INS_MFCOMPRESS=0      # MFCompress -- error: make -- executables available
  INS_DELIMINATE=0      # DELIMINATE -- error: site not reachable -- exec avail
  # FASTQ
  INS_FQZCOMP=0         # fqzcomp
  INS_QUIP=0            # quip
  INS_DSRC=0            # DSRC
  INS_FQC=0             # FQC -- error: site not reachable -- exec available
  # Encryption
  INS_AESCRYPT=0        # AES crypt

### run methods
RUN_METHODS=1
  # compress/decompress
  RUN_METHODS_COMP=0
      # FASTA
      RUN_GZIP_FA=0         # gzip
      RUN_LZMA_FA=0         # lzma
      RUN_MFCOMPRESS=0      # MFCompress
      RUN_DELIMINATE=0      # DELIMINATE
      RUN_CRYFA_FA=0        # cryfa
      # FASTQ
      RUN_GZIP_FQ=0         # gzip
      RUN_LZMA_FQ=0         # lzma
      RUN_FQZCOMP=0         # fqzcomp
      RUN_QUIP=0            # quip
      RUN_DSRC=0            # DSRC
      RUN_FQC=0             # FQC
      RUN_CRYFA_FQ=0        # cryfa
      # results
      PRINT_RESULTS_COMP=0

  # encrypt/decrypt
  RUN_METHODS_ENC=0
      RUN_AESCRYPT=0        # AES crypt
      # results
      PRINT_RESULTS_ENC=0

  # compress/decompress plus encrypt/decrypt
  RUN_METHODS_COMP_ENC=1
      # FASTA
      RUN_GZIP_FA_AESCRYPT=0         # gzip + AES crypt
      RUN_LZMA_FA_AESCRYPT=0         # lzma + AES crypt
      RUN_MFCOMPRESS_AESCRYPT=0      # MFCompress + AES crypt
      RUN_DELIMINATE_AESCRYPT=1      # DELIMINATE + AES crypt
      # FASTQ
      RUN_GZIP_FQ_AESCRYPT=0         # gzip + AES crypt
      RUN_LZMA_FQ_AESCRYPT=0         # lzma + AES crypt
      RUN_FQZCOMP_AESCRYPT=0         # fqzcomp + AES crypt
      RUN_QUIP_AESCRYPT=0            # quip + AES crypt
      RUN_DSRC_AESCRYPT=0            # DSRC + AES crypt
      RUN_FQC_AESCRYPT=0             # FQC + AES crypt
      # results
      PRINT_RESULTS_COMP_ENC=1


# cryfa exclusive -- test purpose
N_THREADS=8             # cryfa: number of threads
SHUFFLE=1               # cryfa: shuffle -- enabled by default
VERBOSE=1               # cryga: verbose mode -- disabled by default
CRYFA_COMP=0            # cryfa -- compress
CRYFA_DECOMP=0          # cryfa -- decompress
CRYFA_COMP_DECOMP_COMPARE=0   # test -- cryfa: comp. + decomp. + compare results


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   folders
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
dataset="dataset"
progs="progs"
result="result"
FA="FA"
FQ="FQ"
XS="XS"


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   URLs
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
HUMAN_FA_URL="ftp://ftp.ncbi.nlm.nih.gov/genomes/H_sapiens/\
Assembled_chromosomes/seq"
HUMAN_FQ_URL="ftp://ftp.sra.ebi.ac.uk/vol1/fastq"
DENISOVA_FQ_URL="http://cdna.eva.mpg.de/denisova/raw_reads"


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   abbreviated names
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
HUMAN="HS"
VIRUSES="V"
DENISOVA="DS"
Synth="Synth"


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   definitions
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
CHR="chr"
HUMAN_CHR_PREFIX="hs_ref_GRCh38.p7_"
HUMAN_CHROMOSOME="$HUMAN_CHR_PREFIX$CHR"
HS_SEQ_RUN=`seq -s' ' 1 22`; HS_SEQ_RUN+=" X Y MT AL UL UP"
WGET_OP=" --trust-server-names "
INF="dat"         # information (data) file type
fasta="fasta"     # FASTA file extension
fastq="fastq"     # FASTQ file extension


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   download datasets
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GET_DATASETS -eq 1 ]];
then
    ### create a folder, if it doesn't already exist
    if [[ ! -d $dataset ]]; then mkdir -p $dataset; fi

    #----------------------- FASTA -----------------------#
    ### human -- 3.1 GB
    if [[ $DL_HUMAN_FA -eq 1 ]];
    then
        # create a folder for FASTA files and one for human dataset
        if [[ ! -d $dataset/$FA/$HUMAN ]]; then mkdir -p $dataset/$FA/$HUMAN; fi

        # download
        for i in {1..22} X Y MT; do
            wget $WGET_OP $HUMAN_FA_URL/$HUMAN_CHROMOSOME$i.fa.gz;
            gunzip < $HUMAN_CHROMOSOME$i.fa.gz \
                   > $dataset/$FA/$HUMAN/$HUMAN-$i.$fasta
            rm $HUMAN_CHROMOSOME$i.fa.gz
        done
        for dual in "alts AL" "unplaced UP" "unlocalized UL"; do
            set $dual
            wget $WGET_OP $HUMAN_FA_URL/$HUMAN_CHR_PREFIX$1.fa.gz;
            gunzip < $HUMAN_CHR_PREFIX$1.fa.gz \
                   > $dataset/$FA/$HUMAN/$HUMAN-$2.$fasta
            rm $HUMAN_CHR_PREFIX$1.fa.gz;
        done
    fi

    ### viruses -- 350 MB
    if [[ $DL_VIRUSES_FA -eq 1 ]];
    then
        # create a folder for FASTA files and one for viruses dataset
        if [[ ! -d $dataset/$FA/$VIRUSES ]]; then
            mkdir -p $dataset/$FA/$VIRUSES;
        fi

        # download
        perl ./scripts/DownloadViruses.pl

        # rename & move downloaded file to dataset folder
        mv viruses.fa viruses.$fasta
        mv viruses.$fasta $dataset/$FA/$VIRUSES
    fi

    ### synthetic -- 4 GB
    if [[ $GEN_SYNTH_FA -eq 1 ]];
    then
        INSTALL_XS=1

        # create a folder for FASTA files and one for synthetic dataset
        if [[ ! -d $dataset/$FA/$Synth ]]; then mkdir -p $dataset/$FA/$Synth; fi

        # install XS
        if [[ $INSTALL_XS -eq 1 ]]; then
            rm -fr $XS
            git clone https://github.com/pratas/XS.git
            cd $XS
            make
            cd ..
        fi

        # generate dataset -- 2.3 GB - 1.7 GB
        XS/XS -eo -es -t 1 -n 4000000 -ld 70:1000 \
              -f 0.2,0.2,0.2,0.2,0.2       $dataset/$FA/$Synth/Synth-1.$fasta
        XS/XS -eo -es -t 2 -n 3000000 -ls 500 \
              -f 0.23,0.23,0.23,0.23,0.08  $dataset/$FA/$Synth/Synth-2.$fasta
    fi

    #----------------------- FASTQ -----------------------#
    ### human -- 27 GB
    if [[ $DL_HUMAN_FQ -eq 1 ]];
    then
        # create a folder for FASTQ files and one for human dataset
        if [[ ! -d $dataset/$FQ/$HUMAN ]]; then mkdir -p $dataset/$FQ/$HUMAN; fi

        # download -- 4.6 GB - 1.4 GB - 12 GB - 488 MB - 8.4 GB
        # ERR013103_1: HG00190-Male-FIN (Finnish in Finland)-Low coverage WGS
        # ERR015767_2: HG00638-Female-PUR (Puerto Rican in Puerto Rico)-Low cov.
        # ERR031905_2: HG00501-Female-CHS (Han Chinese South)-Exome
        # SRR442469_1: HG02108-Female-ACB (African Caribbean in Barbados)-Low c.
        # SRR707196_1: HG00126-Male-GBR (British in England and Scotland)-Exome
        for dual in "ERR013/ERR013103 ERR013103_1"\
               "ERR015/ERR015767 ERR015767_2" "ERR031/ERR031905 ERR031905_2"\
               "SRR442/SRR442469 SRR442469_1" "SRR707/SRR707196 SRR707196_1"; do
            set $dual
            wget $WGET_OP $HUMAN_FQ_URL/$1/$2.fastq.gz;
            gunzip < $2.fastq.gz > $dataset/$FQ/$HUMAN/$HUMAN-$2.$fastq;
            rm $2.fastq.gz;
        done
    fi

    ### Denisova -- 172 GB
    if [[ $DL_DENISOVA_FQ -eq 1 ]];
    then
        # create a folder for FASTQ files and one for Denisova dataset
        if [[ ! -d $dataset/$FQ/$DENISOVA ]]; then
            mkdir -p $dataset/$FQ/$DENISOVA;
        fi

        # download -- 1.7 GB - 1.2 GB - 59 GB - 51 GB - 59 GB
        for i in B1087 B1088 B1110 B1128 SL3003; do
            wget $WGET_OP $DENISOVA_FQ_URL/${i}_SR.txt.gz;
            gunzip < ${i}_SR.txt.gz \
                   > $dataset/$FQ/$DENISOVA/$DENISOVA-${i}_SR.$fastq
            rm ${i}_SR.txt.gz;
        done
    fi

    ### synthetic -- 6.2 GB
    if [[ $GEN_SYNTH_FQ -eq 1 ]];
    then
        INSTALL_XS=1

        # create a folder for FASTQ files and one for synthetic dataset
        if [[ ! -d $dataset/$FQ/$Synth ]]; then mkdir -p $dataset/$FQ/$Synth; fi

        # install XS
        if [[ $INSTALL_XS -eq 1 ]]; then
            rm -fr $XS
            git clone https://github.com/pratas/XS.git
            cd $XS
            make
            cd ..
        fi

        # generate dataset -- 4.2 GB - 2 GB
        XS/XS -t 1 -n 16000000 -ld 70:100 -o \
              -f 0.2,0.2,0.2,0.2,0.2       $dataset/$FQ/$Synth/Synth-1.$fastq
        XS/XS -t 2 -n 10000000 -ls 70 -qt 2 \
              -f 0.23,0.23,0.23,0.23,0.08  $dataset/$FQ/$Synth/Synth-2.$fastq
    fi
fi

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   install dependencies
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $INSTALL_DEPENDENCIES -eq 1 ]];
then
    ### 7ZIP
    if [[ $INS_7ZIP -eq 1 ]]; then
        sudo apt-get install p7zip-full

##        rm -f FILES
#        url="http://sourceforge.net/projects/p7zip/files/latest";
#        wget $WGET_OP $url/download?source=typ_redirect -O FILES.tar.bz2
#        tar -xjf FILES.tar.bz2
#        cd p7zip*/
#        make all
#        cd ..
    fi

    ### CMAKE
    if [[ $INS_CMAKE -eq 1 ]]; then
        sudo apt-get install cmake

#        rm -f cmake-3.9.2-Linux-x86_64.sh cmake-3.9.2-Linux-x86_64.tar.gz
#
#        url="https://cmake.org/files/v3.9"
#        wget $url/cmake-3.9.2-Linux-x86_64.tar.gz
#        tar -xzf cmake-3.9.2-Linux-x86_64.tar.gz
##        cp cmake-3.9.2-Linux-x86_64/bin/cmake .
#        rm cmake-3.9.2-Linux-x86_64.tar.gz
    fi

    ### LIBBOOST
    if [[ $INS_LIBBOOST -eq 1 ]]; then
        sudo apt-get update
        sudo apt-get install libboost1.54-dev
        sudo apt-get install libboost-system1.54-dev
        sudo apt-get install libboost-system-dev
        sudo apt-get install libboost-filesystem1.54-dev
        sudo apt-get install libboost-filesystem-dev
        sudo apt-get install libboost-iostreams-dev
        sudo apt-get install libboost-iostreams1.54-dev
        sudo apt-get install libboost-thread1.54-dev
        sudo apt-get install libboost-thread-dev
    fi

    ### LIBCURL
    if [[ $INS_LIBCURL -eq 1 ]]; then
        sudo apt-get install libcurl4-nss-dev
        sudo apt-get install libcurl-dev
    fi

    ### VALGRIND AND MASSIF
    if [[ $INS_VALGRIND -eq 1 ]]; then
        sudo apt-get install valgrind
    fi

    ### ZLIB
    if [[ $INS_ZLIB -eq 1 ]]; then
        sudo apt-get install zlib1g-dev

#        rm -f zlib_1.2.8.dfsg.orig.tar.gz
#
#        url="https://launchpad.net/ubuntu/+archive/primary/+files"
#        wget $WGET_OP $url/zlib_1.2.8.dfsg.orig.tar.gz
#        tar -xzf zlib_1.2.8.dfsg.orig.tar.gz
#        cd zlib-1.2.8/
#        ./configure
#        make
#        cd ..
    fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   install methods
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $INSTALL_METHODS -eq 1 ]];
then
    ### create folders, if they don't already exist
    if [[ ! -d $progs ]]; then mkdir -p $progs; fi

    ### cryfa -- FASTA & FASTQ
    if [[ $INS_CRYFA -eq 1 ]];
    then
        rm -f cryfa

        cmake .
        make

        if [[ ! -d $progs/cryfa ]]; then mkdir -p $progs/cryfa; fi
        cp cryfa $progs/cryfa/
        cp pass.txt $progs/cryfa/
    fi

    #----------------------- FASTA -----------------------#
    ### MFCompress
    if [[ $INS_MFCOMPRESS -eq 1 ]];
    then
        rm -f MFCompress-src-1.01.tgz

        url="http://sweet.ua.pt/ap/software/mfcompress"
        wget $WGET_OP $url/MFCompress-src-1.01.tgz
        tar -xzf MFCompress-src-1.01.tgz
        mv MFCompress-src-1.01/ mfcompress/    # rename
        mv mfcompress/ $progs/
        rm -f MFCompress-src-1.01.tgz

        cd $progs/mfcompress/
        cp Makefile.linux Makefile    # make -f Makefile.linux
        make
        cd ../..
    fi

    ### DELIMINATE
    if [[ $INS_DELIMINATE -eq 1 ]];
    then
        rm -f DELIMINATE_LINUX_64bit.tar.gz

         url="http://metagenomics.atc.tcs.com/Compression_archive"
         wget $WGET_OP $url/DELIMINATE_LINUX_64bit.tar.gz
         tar -xzf DELIMINATE_LINUX_64bit.tar.gz
         mv EXECUTABLES deliminate    # rename
         mv deliminate $progs/
         rm -f DELIMINATE_LINUX_64bit.tar.gz
    fi

    #----------------------- FASTQ -----------------------#
    ### fqzcomp
    if [[ $INS_FQZCOMP -eq 1 ]];
    then
        rm -f fqzcomp-4.6.tar.gz

        url="https://downloads.sourceforge.net/project/fqzcomp"
        wget $WGET_OP $url/fqzcomp-4.6.tar.gz
        tar -xzf fqzcomp-4.6.tar.gz
        mv fqzcomp-4.6/ fqzcomp/    # rename
        mv fqzcomp/ $progs/
        rm -f fqzcomp-4.6.tar.gz

        cd $progs/fqzcomp/
        make

        cd ../..
    fi

    ### quip
    if [[ $INS_QUIP -eq 1 ]];
    then
        rm -f quip-1.1.8.tar.gz

        url="http://homes.cs.washington.edu/~dcjones/quip"
        wget $WGET_OP $url/quip-1.1.8.tar.gz
        tar -xzf quip-1.1.8.tar.gz
        mv quip-1.1.8/ quip/    # rename
        mv quip/ $progs/
        rm -f quip-1.1.8.tar.gz

        cd $progs/quip/
        ./configure
        cd src/
        make
        cp quip ../

        cd ../../..
    fi

    ### DSRC
    if [[ $INS_DSRC -eq 1 ]];
    then
        rm -fr dsrc/

        git clone https://github.com/lrog/dsrc.git
        mv dsrc/ $progs/

        cd $progs/dsrc/
        make
        cp bin/dsrc .

        cd ../..
    fi

    ### FQC
    if [[ $INS_FQC -eq 1 ]];
    then
        rm -f FQC_LINUX_64bit.tar.gz

        url="http://metagenomics.atc.tcs.com/Compression_archive/FQC"
        wget $WGET_OP $url/FQC_LINUX_64bit.tar.gz
        tar -xzf FQC_LINUX_64bit.tar.gz
        mv FQC_LINUX_64bit/ fqc/    # rename
        mv fqc/ $progs/
        rm -f FQC_LINUX_64bit.tar.gz
    fi

    ### AES crypt
    if [[ $INS_AESCRYPT -eq 1 ]];
    then
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
    fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   run methods
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $RUN_METHODS -eq 1 ]];
then
  ### create folders, if they don't already exist
  if [[ ! -d $progs   ]]; then mkdir -p $progs;   fi
  if [[ ! -d $result  ]]; then mkdir -p $result;  fi

  #------------------------ functions ------------------------#
  # check if a file is available. $1: file name
  function isAvail
  {
    if [[ ! -e $1 ]]; then
      echo "Warning: The file \"$1\" is not available.";
      return;
    fi
  }

  # memory1
  function progMemoryStart
  {
      echo "0" > mem_ps;
      while true; do
          ps aux | grep $1 | awk '{ print $6; }' | \
          sort -V | tail -n 1 >> mem_ps;
          sleep 5;
      done
  }
  function progMemoryStop
  {
      kill $1 >/dev/null 2>&1
      cat mem_ps | sort -V | tail -n 1 > $2;
  }

  # memory2
  function progMemory2
  {
      valgrind --tool=massif --pages-as-heap=yes \
               --massif-out-file=massif.out ./$1
      cat massif.out | grep mem_heap_B | sed -e 's/mem_heap_B=\(.*\)/\1/' | \
      sort -g | tail -n 1
  }

  # time
  function progTime
  {
      time ./$1
  }

  # methods' names for printing in the result table
  function printMethodName
  {
      methodUpCase="$(echo $1 | tr a-z A-Z)"

      case $methodUpCase in
        "GZIP")                 echo "gzip";;
        "LZMA")                 echo "LZMA";;
        "MFCOMPRESS")           echo "MFCompress";;
        "DELIM"|"DELIMINATE")   echo "DELIMINATE";;
        "CRYFA")                echo "Cryfa";;
        "FQZCOMP")              echo "Fqzcomp";;
        "QUIP")                 echo "Quip";;
        "DSRC")                 echo "DSRC";;
        "FQC")                  echo "FQC";;
        "AESCRYPT")             echo "AEScrypt";;
      esac
  }

  # compress and decompress. $1: program's name, $2: input data
  function compDecomp
  {
      result="../../result"
      in="${2##*/}"                     # input file name
      inwf="${in%.*}"                   # input file name without filetype
      ft="${in##*.}"                    # input filetype
      inPath="${2%/*}"                  # input file's path
      upIn="$(echo $1 | tr a-z A-Z)"    # input program's name in uppercase

      case $1 in
        "gzip")
            cFT="gz"                    # compressed filetype
            cCmd="gzip"                 # compression command
            dProg="gunzip"              # decompress program's name
            dCmd="gunzip";;             # decompress command

        "lzma")
            cFT="lzma";            cCmd="lzma";
            dProg="lzma";          dCmd="lzma -d";;

        "fqzcomp")
            cFT="fqz";             cCmd="./fqz_comp";
            dProg="fqz_comp";      dCmd="./fqz_comp -d";; # "./fqz_comp -d -X"

        "quip")
            cFT="qp";              cCmd="./quip -c";
            dProg="quip";          dCmd="./quip -d -c";;

        "dsrc")
            cFT="dsrc";            cCmd="./dsrc c -m2";
            dProg="dsrc";          dCmd="./dsrc d";;

        "delim")
            cFT="dlim";            cCmd="./delim a";
            dProg="delim";         dCmd="./delim e";;

        "fqc")
            cFT="fqc";             cCmd="./fqc -c";
            dProg="fqc";           dCmd="./fqc -d";;

        "mfcompress")
            cFT="mfc";             cCmd="./MFCompressC";
            dProg="MFCompress";    dCmd="./MFCompressD";;

        "cryfa")
            cFT="cryfa";           cCmd="./cryfa -k pass.txt -t 8";
            dProg="cryfa";         dCmd="./cryfa -k pass.txt -t 8 -d";;
      esac

      ### compress
      progMemoryStart $1 &
      MEMPID=$!

      rm -f $in.$cFT
      case $1 in                                                # time
        "gzip"|"lzma")
            (time $cCmd < $2 > $in.$cFT) &> $result/${upIn}_CT__${inwf}_$ft;;

        "cryfa"|"quip"|"fqzcomp")
            (time $cCmd $2 > $in.$cFT) &> $result/${upIn}_CT__${inwf}_$ft;;

        "dsrc")
            (time $cCmd $2 $in.$cFT) &> $result/${upIn}_CT__${inwf}_$ft;;

        "delim")
            (time $cCmd $2) &> $result/${upIn}_CT__${inwf}_$ft
            mv $inPath/$in.$cFT $in.$cFT;;

        "fqc")
            (time $cCmd -i $2 -o $in.$cFT) &>$result/${upIn}_CT__${inwf}_$ft;;

        "mfcompress")
            (time $cCmd -o $in.$cFT $2) &> $result/${upIn}_CT__${inwf}_$ft;;
      esac

      ls -la $in.$cFT > $result/${upIn}_CS__${inwf}_$ft         # size
      progMemoryStop $MEMPID $result/${upIn}_CM__${inwf}_$ft    # memory

      ### decompress
      progMemoryStart $dProg &
      MEMPID=$!

      case $1 in                                                # time
        "gzip"|"lzma")
            (time $dCmd < $in.$cFT> $in) &> $result/${upIn}_DT__${inwf}_$ft;;

        "cryfa"|"fqzcomp"|"quip")
            (time $dCmd $in.$cFT > $in) &> $result/${upIn}_DT__${inwf}_$ft;;

        "dsrc"|"delim")
            (time $dCmd $in.$cFT $in) &> $result/${upIn}_DT__${inwf}_$ft;;

        "fqc")
            (time $dCmd -i $in.$cFT -o $in)&>$result/${upIn}_DT__${inwf}_$ft;;

        "mfcompress")
            (time $dCmd -o $in $in.$cFT) &> $result/${upIn}_DT__${inwf}_$ft;;
      esac

      progMemoryStop $MEMPID $result/${upIn}_DM__${inwf}_$ft    # memory

      ### verify if input and decompressed files are the same
      cmp $2 $in &> $result/${upIn}_V__${inwf}_$ft;
  }

  # encrypt/decrypt. $1: program's name, $2: input data
  function encDecrypt
  {
      result="../../result"
      in="${2##*/}"                     # input file name
      inwf="${in%.*}"                   # input file name without filetype
      ft="${in##*.}"                    # input filetype
      inPath="${2%/*}"                  # input file's path
      upIn="$(echo $1 | tr a-z A-Z)"    # input program's name in uppercase

      case $1 in
        "aescrypt")
            enFT="aescrypt"                        # encrypted filetype
            enCmd="./aescrypt -e -k pass.txt"      # encryption command
            deProg="aescrypt"                      # decrypt program's name
            deCmd="./aescrypt -d -k pass.txt";;    # decryption command
      esac

      ### encrypt
      progMemoryStart $1 &
      MEMPID=$!

      rm -f $in.$enFT
      case $1 in                                                 # time
        "aescrypt")
            (time $enCmd -o $in.$enFT $2) &> $result/${upIn}_EnT__${inwf}_$ft;;
      esac

      ls -la $in.$enFT > $result/${upIn}_EnS__${inwf}_$ft        # size
      progMemoryStop $MEMPID $result/${upIn}_EnM__${inwf}_$ft    # memory

      ### decrypt
      progMemoryStart $deProg &
      MEMPID=$!

      case $1 in                                                 # time
        "aescrypt")
            (time $deCmd -o $in $in.$enFT) &> $result/${upIn}_DeT__${inwf}_$ft;;
      esac

      progMemoryStop $MEMPID $result/${upIn}_DeM__${inwf}_$ft    # memory
  }


  # comp/decomp plus enc/dec. $1: comp program, $2: input data, $3: enc program
  function compEncDecDecompress
  {
      result="../../result"
      in="${2##*/}"                       # input file name
      inwf="${in%.*}"                     # input file name without filetype
      ft="${in##*.}"                      # input filetype
      inPath="${2%/*}"                    # input file's path
      upInComp="$(echo $1 | tr a-z A-Z)"  # compress program's name in uppercase
      upInEnc="$(echo $3 | tr a-z A-Z)"   # encrypt program's name in uppercase

      ### arguments for compression methods
      case $1 in
        "gzip")
            cFT="gz"                      # compressed filetype
            cCmd="gzip"                   # compression command
            dProg="gunzip"                # decompress program's name
            dCmd="gunzip";;               # decompress command

        "lzma")
            cFT="lzma";            cCmd="lzma";
            dProg="lzma";          dCmd="lzma -d";;

        "fqzcomp")
            cFT="fqz";             cCmd="./fqz_comp";
            dProg="fqz_comp";      dCmd="./fqz_comp -d";; # "./fqz_comp -d -X"

        "quip")
            cFT="qp";              cCmd="./quip -c";
            dProg="quip";          dCmd="./quip -d -c";;

        "dsrc")
            cFT="dsrc";            cCmd="./dsrc c -m2";
            dProg="dsrc";          dCmd="./dsrc d";;

        "delim")
            cFT="dlim";            cCmd="./delim a";
            dProg="delim";         dCmd="./delim e";;

        "fqc")
            cFT="fqc";             cCmd="./fqc -c";
            dProg="fqc";           dCmd="./fqc -d";;

        "mfcompress")
            cFT="mfc";             cCmd="./MFCompressC";
            dProg="MFCompress";    dCmd="./MFCompressD";;

        "cryfa")
            cFT="cryfa";           cCmd="./cryfa -k pass.txt -t 8";
            dProg="cryfa";         dCmd="./cryfa -k pass.txt -t 8 -d";;
      esac

      ### arguments for encryption methods
      case $3 in
        "aescrypt")
            enFT="aescrypt"                        # encrypted filetype
            enCmd="./aescrypt -e -k pass.txt"      # encryption command
            deProg="aescrypt"                      # decrypt program's name
            deCmd="./aescrypt -d -k pass.txt";;    # decryption command
      esac

      ### compress
      cd $progs/$1

      progMemoryStart $1 &
      MEMPID=$!

      rm -f $in.$cFT
      case $1 in                                                        # time
        "gzip"|"lzma")
            (time $cCmd < $2 > $in.$cFT) \
                &> $result/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;

        "cryfa"|"quip"|"fqzcomp")
            (time $cCmd $2 > $in.$cFT) \
                &> $result/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;

        "dsrc")
            (time $cCmd $2 $in.$cFT) \
                &> $result/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;

        "delim")
            (time $cCmd $2) \
                &> $result/${upInComp}_${upInEnc}_CT__${inwf}_$ft
            mv $inPath/$in.$cFT $in.$cFT;;

        "fqc")
            (time $cCmd -i $2 -o $in.$cFT) \
                &>$result/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;

        "mfcompress")
            (time $cCmd -o $in.$cFT $2) \
                &> $result/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;
      esac

      ls -la $in.$cFT > $result/${upInComp}_${upInEnc}_CS__${inwf}_$ft  # size
      progMemoryStop $MEMPID \
                     $result/${upInComp}_${upInEnc}_CM__${inwf}_$ft     # memory

      ### encrypt
      cd ../$3
      compPath="../$1"    # path of compressed file

      progMemoryStart $3 &
      MEMPID=$!

      rm -f $in.$cFT.$enFT
      case $3 in                                                        # time
        "aescrypt")
            (time $enCmd -o $in.$cFT.$enFT $compPath/$in.$cFT) \
                &> $result/${upInComp}_${upInEnc}_EnT__${inwf}_$ft;;
      esac

      ls -la $in.$cFT.$enFT > $result/${upInComp}_${upInEnc}_EnS__${inwf}_$ft
      progMemoryStop $MEMPID \
                     $result/${upInComp}_${upInEnc}_EnM__${inwf}_$ft    # memory

      ### decrypt
      progMemoryStart $deProg &
      MEMPID=$!

      case $3 in                                                        # time
        "aescrypt")
            (time $deCmd -o $in.$cFT $in.$cFT.$enFT) \
                &> $result/${upInComp}_${upInEnc}_DeT__${inwf}_$ft;;
      esac

      progMemoryStop $MEMPID \
                     $result/${upInComp}_${upInEnc}_DeM__${inwf}_$ft    # memory

      ### decompress
      cd ../$1
      encPath="../$3"    # path of encrypted file

      progMemoryStart $dProg &
      MEMPID=$!

      case $1 in                                                        # time
        "gzip"|"lzma")
            (time $dCmd < $encPath/$in.$cFT> $in) \
                &> $result/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;

        "cryfa"|"fqzcomp"|"quip")
            (time $dCmd $encPath/$in.$cFT > $in) \
                &> $result/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;

        "dsrc"|"delim")
            (time $dCmd $encPath/$in.$cFT $in) \
                &> $result/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;

        "fqc")
            (time $dCmd -i $encPath/$in.$cFT -o $in) \
                &> $result/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;

        "mfcompress")
            (time $dCmd -o $in $encPath/$in.$cFT) \
                &> $result/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;
      esac

      progMemoryStop $MEMPID \
                     $result/${upInComp}_${upInEnc}_DM__${inwf}_$ft     # memory

      ### verify if input and decompressed files are the same
      cmp $2 $in &> $result/${upInComp}_${upInEnc}_V__${inwf}_$ft;

      cd ../..
  }

  # compress/decompress on datasets. $1: program's name
  function compDecompOnDataset
  {
      method="$(echo $1 | tr A-Z a-z)"    # method's name in lower case
      if [[ ! -d $progs/$method ]]; then mkdir -p $progs/$method; fi
      cd $progs/$method
      dsPath=../../$dataset

      case $2 in
        "fa"|"FA"|"fasta"|"FASTA")   # FASTA -- human - viruses - synthetic
            for i in $HS_SEQ_RUN; do
                compDecomp $method $dsPath/$FA/$HUMAN/$HUMAN-$i.$fasta
            done
            compDecomp $method $dsPath/$FA/$VIRUSES/viruses.$fasta
            for i in {1..2};do
                compDecomp $method $dsPath/$FA/$Synth/Synth-$i.$fasta
            done;;

        "fq"|"FQ"|"fastq"|"FASTQ")   # FASTQ -- human - Denisova - synthetic
            for i in ERR013103_1 ERR015767_2 ERR031905_2 \
                     SRR442469_1 SRR707196_1; do
                compDecomp $method $dsPath/$FQ/$HUMAN/$HUMAN-$i.$fastq
            done
            for i in B1087 B1088 B1110 B1128 SL3003; do
                compDecomp $method \
                           $dsPath/$FQ/$DENISOVA/$DENISOVA-${i}_SR.$fastq
            done
            for i in {1..2};do
                compDecomp $method $dsPath/$FQ/$Synth/Synth-$i.$fastq
            done;;
      esac

      cd ../..
  }

  # encrypt/decrypt on datasets. $1: program's name
  function encDecOnDataset
  {
      method="$(echo $1 | tr A-Z a-z)"    # method's name in lower case
      if [[ ! -d $progs/$method ]]; then mkdir -p $progs/$method; fi
      cd $progs/$method
      dsPath=../../$dataset

      case $1 in
        "aescrypt")   # AES crypt
            # FASTA
            for i in $HS_SEQ_RUN; do
                encDecrypt $method $dsPath/$FA/$HUMAN/$HUMAN-$i.$fasta
            done
            encDecrypt $method $dsPath/$FA/$VIRUSES/viruses.$fasta
            for i in {1..2};do
                encDecrypt $method $dsPath/$FA/$Synth/Synth-$i.$fasta
            done

            # FASTQ
            for i in ERR013103_1 ERR015767_2 ERR031905_2 \
                     SRR442469_1 SRR707196_1; do
                encDecrypt $method $dsPath/$FQ/$HUMAN/$HUMAN-$i.$fastq
            done
            for i in B1087 B1088 B1110 B1128 SL3003; do
                encDecrypt $method \
                           $dsPath/$FQ/$DENISOVA/$DENISOVA-${i}_SR.$fastq
            done
            for i in {1..2};do
                encDecrypt $method $dsPath/$FQ/$Synth/Synth-$i.$fastq
            done;;
      esac

      cd ../..
  }

  # compress/decompress plus encrypt/decrypt on datasets.
  # $1: compression program, $2: filetype, $3: encryption program
  function compEncDecDecompOnDataset
  {
      methodComp="$(echo $1 | tr A-Z a-z)"    # comp method's name in lower case
      methodEnc="$(echo $3 | tr A-Z a-z)"     # enc  method's name in lower case
      if [[ ! -d $progs/$methodComp ]]; then mkdir -p $progs/$methodComp; fi
      if [[ ! -d $progs/$methodEnc ]];  then mkdir -p $progs/$methodEnc;  fi
      dsPath="../../$dataset"

      case $2 in
        "fa"|"FA"|"fasta"|"FASTA")   # FASTA -- human - viruses - synthetic
            for i in $HS_SEQ_RUN; do
                compEncDecDecompress \
                    $methodComp $dsPath/$FA/$HUMAN/$HUMAN-$i.$fasta $methodEnc
            done
            compEncDecDecompress \
                    $methodComp $dsPath/$FA/$VIRUSES/viruses.$fasta $methodEnc
            for i in {1..2};do
                compEncDecDecompress \
                    $methodComp $dsPath/$FA/$Synth/Synth-$i.$fasta $methodEnc
            done;;

        "fq"|"FQ"|"fastq"|"FASTQ")   # FASTQ -- human - Denisova - synthetic
            for i in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 \
                     SRR707196_1; do
                compEncDecDecompress \
                    $methodComp $dsPath/$FQ/$HUMAN/$HUMAN-$i.$fastq $methodEnc
            done
            for i in B1087 B1088 B1110 B1128 SL3003; do
                compEncDecDecompress $methodComp \
                    $dsPath/$FQ/$DENISOVA/$DENISOVA-${i}_SR.$fastq $methodEnc
            done
            for i in {1..2};do
                compEncDecDecompress \
                    $methodComp $dsPath/$FQ/$Synth/Synth-$i.$fastq $methodEnc
            done;;
      esac
  }

  # print compress/decompress results. $1: program's name, $2: dataset
  function compDecompResult
  {
      CS=`cat $result/${1}_CS__${2} | awk '{ print $5; }'`;
      CT_r=`cat $result/${1}_CT__${2} | tail -n 3 | head -n 1 \
                                      | awk '{ print $2;}'`;              # real
      CT_u=`cat $result/${1}_CT__${2} | tail -n 2 | head -n 1 \
                                      | awk '{ print $2;}'`;              # user
      CT_s=`cat $result/${1}_CT__${2} | tail -n 1 | awk '{ print $2;}'`   # sys
      CM=`cat $result/${1}_CM__${2}`;
      DT_r=`cat $result/${1}_DT__${2} | tail -n 3 | head -n 1 \
                                      | awk '{ print $2;}'`;              # real
      DT_u=`cat $result/${1}_DT__${2} | tail -n 2 | head -n 1 \
                                      | awk '{ print $2;}'`;              # user
      DT_s=`cat $result/${1}_DT__${2} | tail -n 1 | awk '{ print $2;}'`   # sys
      DM=`cat $result/${1}_DM__${2}`;
      V=`cat $result/${1}_V__${2}     | wc -l`;

      dName="${2%_*}"                     # dataset name without filetype
      method=`printMethodName $1`         # methods' name for printing
      c="$CS\t$CT_r\t$CT_u\t$CT_s\t$CM"   # compression results
      d="$DT_r\t$DT_u\t$DT_s\t$DM"        # decompression results

      printf "$dName\t$method\t$c\t$d\t$V\n";
  }

  # print encrypt/decrypt results. $1: program's name, $2: dataset
  function encDecResult
  {
      EnS=`cat $result/${1}_EnS__${2}   | awk '{ print $5; }'`;
      EnT_r=`cat $result/${1}_EnT__${2} | tail -n 3 | head -n 1 \
                                        | awk '{ print $2;}'`;            # real
      EnT_u=`cat $result/${1}_EnT__${2} | tail -n 2 | head -n 1 \
                                        | awk '{ print $2;}'`;            # user
      EnT_s=`cat $result/${1}_EnT__${2} | tail -n 1 | awk '{ print $2;}'` # sys
      EnM=`cat $result/${1}_EnM__${2}`;
      DeT_r=`cat $result/${1}_DeT__${2} | tail -n 3 | head -n 1 \
                                        | awk '{ print $2;}'`;            # real
      DeT_u=`cat $result/${1}_DeT__${2} | tail -n 2 | head -n 1 \
                                        | awk '{ print $2;}'`;            # user
      DeT_s=`cat $result/${1}_DeT__${2} | tail -n 1 | awk '{ print $2;}'` # sys
      DeM=`cat $result/${1}_DeM__${2}`;

      dName="${2%_*}"                            # dataset name without filetype
      method=`printMethodName $1`                # methods' name for printing
      en="$EnS\t$EnT_r\t$EnT_u\t$EnT_s\t$EnM"    # encryption results
      de="$DeT_r\t$DeT_u\t$DeT_s\t$DeM"          # decryption results

      printf "$dName\t$method\t$en\t$de\n";
  }

  # print comp/decomp plus enc/dec results.
  # $1: compression program's name, $2: encryption program's name, $3: dataset.
  function compEncDecDecompResult
  {
      CS=`cat $result/${1}_${2}_CS__${3}     | awk '{ print $5; }'`;
      CT_r=`cat $result/${1}_${2}_CT__${3}   | tail -n 3 | head -n 1 \
                                             | awk '{ print $2;}'`;       # real
      CT_u=`cat $result/${1}_${2}_CT__${3}   | tail -n 2 | head -n 1 \
                                             | awk '{ print $2;}'`;       # user
      CT_s=`cat $result/${1}_${2}_CT__${3}   | tail -n 1 | awk '{ print $2;}'`
      CM=`cat $result/${1}_${2}_CM__${3}`;
      EnS=`cat $result/${1}_${2}_EnS__${3}   | awk '{ print $5; }'`;
      EnT_r=`cat $result/${1}_${2}_EnT__${3} | tail -n 3 | head -n 1 \
                                             | awk '{ print $2;}'`;       # real
      EnT_u=`cat $result/${1}_${2}_EnT__${3} | tail -n 2 | head -n 1 \
                                             | awk '{ print $2;}'`;       # user
      EnT_s=`cat $result/${1}_${2}_EnT__${3} | tail -n 1 | awk '{ print $2;}'`
      EnM=`cat $result/${1}_${2}_EnM__${3}`;
      DeT_r=`cat $result/${1}_${2}_DeT__${3} | tail -n 3 | head -n 1 \
                                             | awk '{ print $2;}'`;       # real
      DeT_u=`cat $result/${1}_${2}_DeT__${3} | tail -n 2 | head -n 1 \
                                             | awk '{ print $2;}'`;       # user
      DeT_s=`cat $result/${1}_${2}_DeT__${3} | tail -n 1 | awk '{ print $2;}'`
      DeM=`cat $result/${1}_${2}_DeM__${3}`;
      DT_r=`cat $result/${1}_${2}_DT__${3}   | tail -n 3 | head -n 1 \
                                             | awk '{ print $2;}'`;       # real
      DT_u=`cat $result/${1}_${2}_DT__${3}   | tail -n 2 | head -n 1 \
                                             | awk '{ print $2;}'`;       # user
      DT_s=`cat $result/${1}_${2}_DT__${3}   | tail -n 1 | awk '{ print $2;}'`
      DM=`cat $result/${1}_${2}_DM__${3}`;
      V=`cat $result/${1}_${2}_V__${3}       | wc -l`;

      dName="${3%_*}"                          # dataset name without filetype
      methodComp=`printMethodName $1`          # comp methods' name for printing
      methodEnc=`printMethodName $2`           # enc methods' name for printing
      c="$CS\t$CT_r\t$CT_u\t$CT_s\t$CM"        # compression results
      en="$EnS\t$EnT_r\t$EnT_u\t$EnT_s\t$EnM"  # encryption results
      de="$DeT_r\t$DeT_u\t$DeT_s\t$DeM"        # decryption results
      d="$DT_r\t$DT_u\t$DT_s\t$DM"             # decompression results

      printf "$dName\t$methodComp\t$methodEnc\t$c\t$en\t$de\t$d\t$V\n";
  }


  #------------------- dataset availablity -------------------#
  # FASTA -- human - viruses - synthetic
  for i in $HS_SEQ_RUN; do
      isAvail "$dataset/$FA/$HUMAN/$HUMAN-$i.$fasta";
  done
  isAvail "$dataset/$FA/$VIRUSES/viruses.$fasta"
  for i in {1..2}; do isAvail "$dataset/$FA/$Synth/Synth-$i.$fasta"; done

  # FASTQ -- human - Denisova - synthetic
  for i in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 SRR707196_1; do
      isAvail "$dataset/$FQ/$HUMAN/$HUMAN-$i.$fastq"
  done
  for i in B1087 B1088 B1110 B1128 SL3003; do
      isAvail "$dataset/$FQ/$DENISOVA/$DENISOVA-${i}_SR.$fastq"
  done
  for i in {1..2}; do isAvail "$dataset/$FQ/$Synth/Synth-$i.$fastq"; done

  #--------------------------- run ---------------------------#
  ### compress/decompress
  if [[ $RUN_METHODS_COMP -eq 1 ]];
  then
      ### FASTA
      if [[ $RUN_GZIP_FA    -eq 1 ]]; then compDecompOnDataset gzip       fa; fi
      if [[ $RUN_LZMA_FA    -eq 1 ]]; then compDecompOnDataset lzma       fa; fi
      if [[ $RUN_MFCOMPRESS -eq 1 ]]; then compDecompOnDataset mfcompress fa; fi
      if [[ $RUN_DELIMINATE -eq 1 ]]; then compDecompOnDataset delim      fa; fi
      if [[ $RUN_CRYFA_FA   -eq 1 ]]; then compDecompOnDataset cryfa      fa; fi

      ### FASTQ
      if [[ $RUN_GZIP_FQ    -eq 1 ]]; then compDecompOnDataset gzip       fq; fi
      if [[ $RUN_LZMA_FQ    -eq 1 ]]; then compDecompOnDataset lzma       fq; fi
      if [[ $RUN_FQZCOMP    -eq 1 ]]; then compDecompOnDataset fqzcomp    fq; fi
      if [[ $RUN_QUIP       -eq 1 ]]; then compDecompOnDataset quip       fq; fi
      if [[ $RUN_DSRC       -eq 1 ]]; then compDecompOnDataset dsrc       fq; fi
      if [[ $RUN_FQC        -eq 1 ]]; then compDecompOnDataset fqc        fq; fi
      if [[ $RUN_CRYFA_FQ   -eq 1 ]]; then compDecompOnDataset cryfa      fq; fi

      #---------------- results ----------------#
      if [[ $PRINT_RESULTS_COMP -eq 1 ]];
      then
          ### result files availability
          result="result"

#          for i in CS CT CM DT DM V; do
#              # FASTA -- human - viruses - synthetic
#              for j in CRYFA GZIP LZMA MFCOMPRESS DELIMINATE; do
#                  for k in $HS_SEQ_RUN; do
#                      isAvail "$result/${j}_${i}__$HUMAN-${k}_$fasta";
#                  done
#                  isAvail "$result/${j}_${i}__viruses_$fasta"
#                  for k in {1..2}; do
#                      isAvail "$result/${j}_${i}__$Synth-${k}_$fasta";
#                  done
#              done
#
#              # FASTQ -- human - Denisova - synthetic
#              for j in CRYFA GZIP LZMA FQZCOMP QUIP DSRC FQC; do
#                  for k in ERR013103_1 ERR015767_2 ERR031905_2 \
#                           SRR442469_1 SRR707196_1; do
#                      isAvail "$result/${j}_${i}__$HUMAN-${k}_$fastq";
#                  done
#                  for k in B1087 B1088 B1110 B1128 SL3003; do
#                      isAvail "$result/${j}_${i}__$DENISOVA-${k}_SR_$fastq";
#                  done
#                  for k in {1..2}; do
#                      isAvail "$result/${j}_${i}__$Synth-${k}_$fastq";
#                  done
#              done
#          done

          ### print results
          c="C_Size\tC_Time(real)\tC_Time(user)\tC_Time(sys)\tC_Mem"
          d="D_Time(real)\tD_Time(user)\tD_Time(sys)\tD_Mem"
          printf "Dataset\tMethod\t$c\t$d\tEq\n" > result_comp.$INF;

          # FASTA -- human - viruses - synthetic
          for i in CRYFA GZIP LZMA MFCOMPRESS DELIMINATE; do
              for j in $HS_SEQ_RUN; do
                  compDecompResult $i $HUMAN-${j}_$fasta >> result_comp.$INF;
              done
              compDecompResult $i viruses_$fasta >> result_comp.$INF;
              for j in {1..2}; do
                  compDecompResult $i $Synth-${j}_$fasta >> result_comp.$INF;
              done
          done

          # FASTQ -- human - Denisova - synthetic
          for i in CRYFA GZIP LZMA FQZCOMP QUIP DSRC FQC; do
              for j in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 \
                       SRR707196_1; do
                  compDecompResult $i $HUMAN-${j}_$fastq >> result_comp.$INF;
              done
              for j in B1087 B1088 B1110 B1128 SL3003; do
                  compDecompResult $i \
                                   $DENISOVA-${j}_SR_$fastq >> result_comp.$INF;
              done
              for j in {1..2}; do
                  compDecompResult $i $Synth-${j}_$fastq >> result_comp.$INF;
              done
          done
      fi
  fi

  ### encrypt/decrypt
  if [[ $RUN_METHODS_ENC -eq 1 ]];
  then
      ### AES crypt
      if [[ $RUN_AESCRYPT -eq 1 ]]; then encDecOnDataset aescrypt; fi

      #---------------- results ----------------#
      if [[ $PRINT_RESULTS_ENC -eq 1 ]];
      then
          ### result files availability
          result="result"

#          for i in AESCRYPT; do
#              for j in EnS EnT EnM DeT DeM; do
#                  # FASTA -- human - viruses - synthetic
#                  for k in $HS_SEQ_RUN; do
#                      isAvail "$result/${i}_${j}__$HUMAN-${k}_$fasta";
#                  done
#                  isAvail "$result/${i}_${j}__viruses_$fasta"
#                  for k in {1..2}; do
#                      isAvail "$result/${i}_${j}__$Synth-${k}_$fasta";
#                  done
#
#                  # FASTQ -- human - Denisova - synthetic
#                  for k in ERR013103_1 ERR015767_2 ERR031905_2 \
#                           SRR442469_1 SRR707196_1; do
#                      isAvail "$result/${i}_${j}__$HUMAN-${k}_$fastq";
#                  done
#                  for k in B1087 B1088 B1110 B1128 SL3003; do
#                      isAvail "$result/${i}_${j}__$DENISOVA-${k}_SR_$fastq";
#                  done
#                  for k in {1..2}; do
#                      isAvail "$result/${i}_${j}__$Synth-${k}_$fastq";
#                  done
#              done
#          done

          ### print results
          en="En_Size\tEn_Time(real)\tEn_Time(user)\tEn_Time(sys)\tEn_Mem"
          de="De_Time(real)\tDe_Time(user)\tDe_Time(sys)\tDe_Mem"
          printf "Dataset\tMethod\t$en\t$de\n" > result_enc.$INF;

          for i in AESCRYPT;
          do
              # FASTA -- human - viruses - synthetic
              for j in $HS_SEQ_RUN; do
                  encDecResult $i $HUMAN-${j}_$fasta >> result_enc.$INF;
              done
              encDecResult $i viruses_$fasta >> result_enc.$INF;
              for j in {1..2}; do
                  encDecResult $i $Synth-${j}_$fasta >> result_enc.$INF;
              done

              # FASTQ -- human - Denisova - synthetic
              for j in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 \
                       SRR707196_1; do
                  encDecResult $i $HUMAN-${j}_$fastq >> result_enc.$INF;
              done
              for j in B1087 B1088 B1110 B1128 SL3003; do
                  encDecResult $i $DENISOVA-${j}_SR_$fastq >> result_enc.$INF;
              done
              for j in {1..2}; do
                  encDecResult $i $Synth-${j}_$fastq >> result_enc.$INF;
              done
          done
      fi
  fi

  ### compress/decompress and encrypt/decrypt
  if [[ $RUN_METHODS_COMP_ENC -eq 1 ]];
  then
      ### FASTA + AEScrypt
      if [[ $RUN_GZIP_FA_AESCRYPT -eq 1 ]]; then
        compEncDecDecompOnDataset gzip fa aescrypt;
      fi
      if [[ $RUN_LZMA_FA_AESCRYPT -eq 1 ]]; then
        compEncDecDecompOnDataset lzma fa aescrypt;
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
      if [[ $RUN_LZMA_FQ_AESCRYPT -eq 1 ]]; then
        compEncDecDecompOnDataset lzma fq aescrypt;
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

      #---------------- results ----------------#
      if [[ $PRINT_RESULTS_COMP_ENC -eq 1 ]];
      then
          ### result files availability
          result="result"

#          for i in CS CT CM DT DM EnS EnT EnM EnT EnM V; do
#              for j in AESCRYPT; do
#                  # FASTA -- human - viruses - synthetic
#                  for k in CRYFA GZIP LZMA MFCOMPRESS DELIMINATE; do
#                      for l in $HS_SEQ_RUN; do
#                          isAvail "$result/${k}_${j}_${i}__$HUMAN-${l}_$fasta";
#                      done
#                      isAvail "$result/${k}_${j}_${i}__viruses_$fasta"
#                      for l in {1..2}; do
#                          isAvail "$result/${k}_${j}_${i}__$Synth-${l}_$fasta";
#                      done
#                  done
#
#                  # FASTQ -- human - Denisova - synthetic
#                  for k in CRYFA GZIP LZMA FQZCOMP QUIP DSRC FQC; do
#                      for l in ERR013103_1 ERR015767_2 ERR031905_2 \
#                               SRR442469_1 SRR707196_1; do
#                          isAvail "$result/${k}_${j}_${i}__$HUMAN-${l}_$fastq";
#                      done
#                      for l in B1087 B1088 B1110 B1128 SL3003; do
#                          isAvail \
#                            "$result/${k}_${j}_${i}__$DENISOVA-${l}_SR_$fastq";
#                      done
#                      for l in {1..2}; do
#                          isAvail "$result/${k}_${j}_${i}__$Synth-${l}_$fastq";
#                      done
#                  done
#              done
#          done

          ### print results
          c="C_Size\tC_Time(real)\tC_Time(user)\tC_Time(sys)\tC_Mem"
          en="En_Size\tEn_Time(real)\tEn_Time(user)\tEn_Time(sys)\tEn_Mem"
          d="D_Time(real)\tD_Time(user)\tD_Time(sys)\tD_Mem"
          de="De_Time(real)\tDe_Time(user)\tDe_Time(sys)\tDe_Mem"
          printf "Dataset\tC_Method\tEn_Method\t$c\t$en\t$de\t$d\tEq\n" \
              > result_comp_enc.$INF;

          for i in AESCRYPT; do
             # FASTA -- human - viruses - synthetic
             for j in GZIP LZMA MFCOMPRESS DELIMINATE; do
                 for k in $HS_SEQ_RUN; do
                     compEncDecDecompResult $j $i $HUMAN-${k}_$fasta \
                         >> result_comp_enc.$INF;
                 done
                 compEncDecDecompResult $j $i viruses_$fasta \
                     >> result_comp_enc.$INF;
                 for k in {1..2}; do
                     compEncDecDecompResult $j $i $Synth-${k}_$fasta \
                         >> result_comp_enc.$INF;
                 done
             done

             # FASTQ -- human - Denisova - synthetic
             for j in GZIP LZMA FQZCOMP QUIP DSRC FQC; do
                 for k in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 \
                          SRR707196_1; do
                     compEncDecDecompResult $j $i $HUMAN-${k}_$fastq \
                         >> result_comp_enc.$INF;
                 done
                 for k in B1087 B1088 B1110 B1128 SL3003; do
                     compEncDecDecompResult $j $i $DENISOVA-${k}_SR_$fastq \
                         >> result_comp_enc.$INF;
                 done
                 for k in {1..2}; do
                     compEncDecDecompResult $j $i $Synth-${k}_$fastq \
                         >> result_comp_enc.$INF;
                 done
             done
          done
      fi
  fi
fi

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   run cryfa -- compress
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CRYFA_COMP -eq 1 ]]; then

    ### shuffle off + verbose off
    if [[ $SHUFFLE -eq 0 && $VERBOSE -eq 0 ]]; then
        cmake . | make | ./cryfa -t $N_THREADS -k pass.txt -s $1

    ### shuffle off + verbose on
    elif [[ $SHUFFLE -eq 0 && $VERBOSE -eq 1 ]]; then
        cmake . | make | ./cryfa -t $N_THREADS -k pass.txt -s -v $1

    ### shuffle on + verbose off
    elif [[ $SHUFFLE -eq 1 && $VERBOSE -eq 0 ]]; then
        cmake . | make | ./cryfa -t $N_THREADS -k pass.txt $1

    ### shuffle on + verbose on
    elif [[ $SHUFFLE -eq 1 && $VERBOSE -eq 1 ]]; then
        cmake . | make | ./cryfa -t $N_THREADS -k pass.txt -v $1
    fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   run cryfa -- decompress
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CRYFA_DECOMP -eq 1 ]]; then

    ### verbose off
    if [[ $VERBOSE -eq 0 ]]; then
        cmake . | make | ./cryfa -t $N_THREADS -k pass.txt -d $1

    ### verbose on
    elif [[ $VERBOSE -eq 1 ]]; then
        cmake . | make | ./cryfa -t $N_THREADS -k pass.txt -v -d $1
    fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   run cryfa -- testing purpose -- compress + decompress + compare results
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CRYFA_COMP_DECOMP_COMPARE -eq 1 ]]; then

    ### shuffle off + verbose off
    if [[ $SHUFFLE -eq 0 && $VERBOSE -eq 0 ]]; then
        cmake . | make | ./cryfa -t $N_THREADS -k pass.txt -s $1 > $2#compress
        ./cryfa -t $N_THREADS -k pass.txt -d $2>"CRYFA_DECOMPRESSED" #decompress

    ### shuffle off + verbose on
    elif [[ $SHUFFLE -eq 0 && $VERBOSE -eq 1 ]]; then
        cmake . | make | ./cryfa -t $N_THREADS -k pass.txt -sv $1 >$2#compress
        ./cryfa -t $N_THREADS -k pass.txt -vd $2>"CRYFA_DECOMPRESSED"#decompress

    ### shuffle on + verbose off
    elif [[ $SHUFFLE -eq 1 && $VERBOSE -eq 0 ]]; then
        cmake . | make | ./cryfa -t $N_THREADS -k pass.txt $1 > $2   #compress
        ./cryfa -t $N_THREADS -k pass.txt -d $2>"CRYFA_DECOMPRESSED" #decompress

    ### shuffle on + verbose on
    elif [[ $SHUFFLE -eq 1 && $VERBOSE -eq 1 ]]; then
        cmake . | make | ./cryfa -t $N_THREADS -k pass.txt -v $1 >$2 #compress
        ./cryfa -t $N_THREADS -k pass.txt -vd $2>"CRYFA_DECOMPRESSED"#decompress
    fi

    ### compare results
    cmp $1 "CRYFA_DECOMPRESSED"
fi