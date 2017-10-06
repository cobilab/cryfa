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
  RUN_METHODS_COMP=1
      # FASTA
      RUN_GZIP_FA=0                # gzip
      RUN_BZIP2_FA=0               # bzip2
###      RUN_LZMA_FA=0                # lzma
      RUN_MFCOMPRESS=0             # MFCompress
      RUN_DELIMINATE=0             # DELIMINATE
      RUN_CRYFA_FA=0               # cryfa
      # FASTQ
      RUN_GZIP_FQ=0                # gzip
      RUN_BZIP2_FQ=0               # bzip2
###      RUN_LZMA_FQ=0                # lzma
      RUN_FQZCOMP=0                # fqzcomp
      RUN_QUIP=0                   # quip
      RUN_DSRC=0                   # DSRC
      RUN_FQC=0                    # FQC
      RUN_CRYFA_FQ=1               # cryfa
      # results
      PRINT_RESULTS_COMP=1

  # encrypt/decrypt
  RUN_METHODS_ENC=0
      RUN_AESCRYPT=0               # AES crypt
      # results
      PRINT_RESULTS_ENC=0

  # compress/decompress plus encrypt/decrypt
  RUN_METHODS_COMP_ENC=1
      # FASTA
      RUN_GZIP_FA_AESCRYPT=0       # gzip + AES crypt
      RUN_BZIP2_FA_AESCRYPT=0      # bzip2 + AES crypt
###      RUN_LZMA_FA_AESCRYPT=0       # lzma + AES crypt
      RUN_MFCOMPRESS_AESCRYPT=0    # MFCompress + AES crypt
      RUN_DELIMINATE_AESCRYPT=0    # DELIMINATE + AES crypt
      # FASTQ
      RUN_GZIP_FQ_AESCRYPT=1       # gzip + AES crypt
      RUN_BZIP2_FQ_AESCRYPT=1      # bzip2 + AES crypt
###      RUN_LZMA_FQ_AESCRYPT=0       # lzma + AES crypt
      RUN_FQZCOMP_AESCRYPT=1       # fqzcomp + AES crypt
      RUN_QUIP_AESCRYPT=1          # quip + AES crypt
      RUN_DSRC_AESCRYPT=1          # DSRC + AES crypt
      RUN_FQC_AESCRYPT=1           # FQC + AES crypt
      # results
      PRINT_RESULTS_COMP_ENC=1

  # cryfa exclusive
  CRYFA_EXCLUSIVE=0
      MAX_N_THR=8                  # max number of threads
#      CRYFA_XCL_DATASET="dataset/FA/V/viruses.fasta"
      CRYFA_XCL_DATASET="dataset/FA/HS/HS.fasta"
#      CRYFA_XCL_DATASET="dataset/FQ/HS/HS-ERR013103_1.fastq"
#      CRYFA_XCL_DATASET="dataset/FQ/DS/DS-B1088_SR.fastq"
      RUN_CRYFA_XCL=1
      PRINT_RESULTS_CRYFA_XCL=1

### plot results
PLOT_RESULTS=0


# test purpose
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
cryfa_xcl="cryfa_xcl"
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
WGET_OP=" --trust-server-names "
INF="dat"         # information (data) file type
RES="res"         # result file type
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

      # download and remove blank lines
      for i in {1..22} X Y MT; do
          wget $WGET_OP $HUMAN_FA_URL/$HUMAN_CHROMOSOME$i.fa.gz;
          gunzip < $HUMAN_CHROMOSOME$i.fa.gz | grep -Ev "^$" \
                 > $dataset/$FA/$HUMAN/$HUMAN-$i.$fasta
          rm $HUMAN_CHROMOSOME$i.fa.gz
      done
      for dual in "alts AL" "unplaced UP" "unlocalized UL"; do
          set $dual
          wget $WGET_OP $HUMAN_FA_URL/$HUMAN_CHR_PREFIX$1.fa.gz;
          gunzip < $HUMAN_CHR_PREFIX$1.fa.gz | grep -Ev "^$" \
                 > $dataset/$FA/$HUMAN/$HUMAN-$2.$fasta
          rm $HUMAN_CHR_PREFIX$1.fa.gz;
      done

      ### join all files
      cat HS-1.$fasta HS-2.$fasta HS-3.$fasta HS-4.$fasta HS-5.$fasta      \
          HS-6.$fasta HS-7.$fasta HS-8.$fasta HS-9.$fasta HS-10.$fasta     \
          HS-11.$fasta HS-12.$fasta HS-13.$fasta HS-14.$fasta HS-15.$fasta \
          HS-16.$fasta HS-17.$fasta HS-18.$fasta HS-19.$fasta HS-20.$fasta \
          HS-21.$fasta HS-22.$fasta HS-X.$fasta HS-Y.$fasta HS-AL.$fasta   \
          HS-UL.$fasta HS-UP.$fasta HS-MT.$fasta > HS.$fasta.tmp
      rm *.$fasta
      mv HS.$fasta.tmp > HS.$fasta
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

      # remove blank lines in downloaded file & move it to dataset folder
      cat viruses.fa | grep -Ev "^$" > $dataset/$FA/$VIRUSES/viruses.$fasta
      rm viruses.fa
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

      # replace @ symbol with > for the headers
      for i in 1 2; do
          sed -i 's/@/>/g' "$dataset/$FA/$Synth/Synth-$i.$fasta";
      done
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

##      rm -f FILES
#      url="http://sourceforge.net/projects/p7zip/files/latest";
#      wget $WGET_OP $url/download?source=typ_redirect -O FILES.tar.bz2
#      tar -xjf FILES.tar.bz2
#      cd p7zip*/
#      make all
#      cd ..
  fi

  ### CMAKE
  if [[ $INS_CMAKE -eq 1 ]]; then
      sudo apt-get install cmake

#      rm -f cmake-3.9.2-Linux-x86_64.sh cmake-3.9.2-Linux-x86_64.tar.gz
#
#      url="https://cmake.org/files/v3.9"
#      wget $url/cmake-3.9.2-Linux-x86_64.tar.gz
#      tar -xzf cmake-3.9.2-Linux-x86_64.tar.gz
##      cp cmake-3.9.2-Linux-x86_64/bin/cmake .
#      rm cmake-3.9.2-Linux-x86_64.tar.gz
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

#      rm -f zlib_1.2.8.dfsg.orig.tar.gz
#
#      url="https://launchpad.net/ubuntu/+archive/primary/+files"
#      wget $WGET_OP $url/zlib_1.2.8.dfsg.orig.tar.gz
#      tar -xzf zlib_1.2.8.dfsg.orig.tar.gz
#      cd zlib-1.2.8/
#      ./configure
#      make
#      cd ..
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
      cp cryfa pass.txt  $progs/cryfa/
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
          ps aux | grep $1 | awk '{print $6;}' | sort -V | tail -n 1 >> mem_ps;
          sleep 0.001;    # 1 milisecond
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
        "BZIP2")                echo "bzip2";;
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
      result_FLD="../../$result"
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

        "bzip2")
            cFT="bz2";             cCmd="bzip2";
            dProg="bzip2";         dCmd="bunzip2";;

        "lzma")
            cFT="lzma";            cCmd="lzma";
            dProg="lzma";          dCmd="lzma -d";;

        "fqzcomp")
            cFT="fqz";             cCmd="./fqz_comp";
            dProg="fqz_comp";      dCmd="./fqz_comp -d -X";; # "./fqz_comp -d"

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
      case $1 in                                                    # time
        "gzip"|"lzma"|"bzip2")
            (time $cCmd < $2 > $in.$cFT)&> $result_FLD/${upIn}_CT__${inwf}_$ft;;

        "cryfa"|"quip"|"fqzcomp")
            (time $cCmd $2 > $in.$cFT) &> $result_FLD/${upIn}_CT__${inwf}_$ft;;

        "dsrc")
            (time $cCmd $2 $in.$cFT) &> $result_FLD/${upIn}_CT__${inwf}_$ft;;

        "delim")
            (time $cCmd $2) &> $result_FLD/${upIn}_CT__${inwf}_$ft
            mv $inPath/$in.$cFT $in.$cFT;;

        "fqc")
            (time $cCmd -i $2 -o $in.$cFT) \
                &> $result_FLD/${upIn}_CT__${inwf}_$ft;;

        "mfcompress")
            (time $cCmd -o $in.$cFT $2) &> $result_FLD/${upIn}_CT__${inwf}_$ft;;
      esac

      ls -la $in.$cFT > $result_FLD/${upIn}_CS__${inwf}_$ft         # size
      progMemoryStop $MEMPID $result_FLD/${upIn}_CM__${inwf}_$ft    # memory

      ### decompress
      progMemoryStart $dProg &
      MEMPID=$!

      case $1 in                                                    # time
        "gzip"|"lzma"|"bzip2")
            (time $dCmd < $in.$cFT> $in)&> $result_FLD/${upIn}_DT__${inwf}_$ft;;

        "cryfa"|"fqzcomp"|"quip")
            (time $dCmd $in.$cFT > $in) &> $result_FLD/${upIn}_DT__${inwf}_$ft;;

        "dsrc"|"delim")
            (time $dCmd $in.$cFT $in) &> $result_FLD/${upIn}_DT__${inwf}_$ft;;

        "fqc")
            (time $dCmd -i $in.$cFT -o $in) \
                &> $result_FLD/${upIn}_DT__${inwf}_$ft;;

        "mfcompress")
            (time $dCmd -o $in $in.$cFT)&> $result_FLD/${upIn}_DT__${inwf}_$ft;;
      esac

      progMemoryStop $MEMPID $result_FLD/${upIn}_DM__${inwf}_$ft    # memory

      ### verify if input and decompressed files are the same
      cmp $2 $in &> $result_FLD/${upIn}_V__${inwf}_$ft;
  }

  # encrypt/decrypt. $1: program's name, $2: input data
  function encDecrypt
  {
      result_FLD="../../$result"
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
      case $1 in                                                     # time
        "aescrypt")
            (time $enCmd -o $in.$enFT $2) \
                &> $result_FLD/${upIn}_EnT__${inwf}_$ft;;
      esac

      ls -la $in.$enFT > $result_FLD/${upIn}_EnS__${inwf}_$ft        # size
      progMemoryStop $MEMPID $result_FLD/${upIn}_EnM__${inwf}_$ft    # memory

      ### decrypt
      progMemoryStart $deProg &
      MEMPID=$!

      case $1 in                                                     # time
        "aescrypt")
            (time $deCmd -o $in $in.$enFT) \
                &> $result_FLD/${upIn}_DeT__${inwf}_$ft;;
      esac

      progMemoryStop $MEMPID $result_FLD/${upIn}_DeM__${inwf}_$ft    # memory
  }

  # comp/decomp plus enc/dec. $1: comp program, $2: input data, $3: enc program
  function compEncDecDecompress
  {
      result_FLD="../../$result"
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

        "bzip2")
            cFT="bz2";             cCmd="bzip2";
            dProg="bzip2";         dCmd="bunzip2";;

        "lzma")
            cFT="lzma";            cCmd="lzma";
            dProg="lzma";          dCmd="lzma -d";;

        "fqzcomp")
            cFT="fqz";             cCmd="./fqz_comp";
            dProg="fqz_comp";      dCmd="./fqz_comp -d -X";; # "./fqz_comp -d"

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

      rm -f $in $in.$cFT
      case $1 in                                                          # time
        "gzip"|"lzma"|"bzip2")
            (time $cCmd < $2 > $in.$cFT) \
                &> $result_FLD/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;

        "cryfa"|"quip"|"fqzcomp")
            (time $cCmd $2 > $in.$cFT) \
                &> $result_FLD/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;

        "dsrc")
            (time $cCmd $2 $in.$cFT) \
                &> $result_FLD/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;

        "delim")
            (time $cCmd $2) \
                &> $result_FLD/${upInComp}_${upInEnc}_CT__${inwf}_$ft
            mv $inPath/$in.$cFT $in.$cFT;;

        "fqc")
            (time $cCmd -i $2 -o $in.$cFT) \
                &>$result_FLD/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;

        "mfcompress")
            (time $cCmd -o $in.$cFT $2) \
                &> $result_FLD/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;
      esac

      ls -la $in.$cFT > $result_FLD/${upInComp}_${upInEnc}_CS__${inwf}_$ft #size
      progMemoryStop $MEMPID \
                     $result_FLD/${upInComp}_${upInEnc}_CM__${inwf}_$ft    # mem

      ### encrypt
      cd ../$3
      compPath="../$1"    # path of compressed file

      progMemoryStart $3 &
      MEMPID=$!

      rm -f $in.$cFT $in.$cFT.$enFT
      case $3 in                                                          # time
        "aescrypt")
            (time $enCmd -o $in.$cFT.$enFT $compPath/$in.$cFT) \
                &> $result_FLD/${upInComp}_${upInEnc}_EnT__${inwf}_$ft;;
      esac

      ls -la $in.$cFT.$enFT >$result_FLD/${upInComp}_${upInEnc}_EnS__${inwf}_$ft
      progMemoryStop $MEMPID \
                     $result_FLD/${upInComp}_${upInEnc}_EnM__${inwf}_$ft   # mem

      ### decrypt
      progMemoryStart $deProg &
      MEMPID=$!

      case $3 in                                                          # time
        "aescrypt")
            (time $deCmd -o $in.$cFT $in.$cFT.$enFT) \
                &> $result_FLD/${upInComp}_${upInEnc}_DeT__${inwf}_$ft;;
      esac

      progMemoryStop $MEMPID \
                     $result_FLD/${upInComp}_${upInEnc}_DeM__${inwf}_$ft  # mem

      ### decompress
      cd ../$1
      encPath="../$3"    # path of encrypted file

      progMemoryStart $dProg &
      MEMPID=$!

      case $1 in                                                          # time
        "gzip"|"lzma"|"bzip2")
            (time $dCmd < $encPath/$in.$cFT> $in) \
                &> $result_FLD/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;

        "cryfa"|"fqzcomp"|"quip")
            (time $dCmd $encPath/$in.$cFT > $in) \
                &> $result_FLD/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;

        "dsrc"|"delim")
            (time $dCmd $encPath/$in.$cFT $in) \
                &> $result_FLD/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;

        "fqc")
            (time $dCmd -i $encPath/$in.$cFT -o $in) \
                &> $result_FLD/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;

        "mfcompress")
            (time $dCmd -o $in $encPath/$in.$cFT) \
                &> $result_FLD/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;
      esac

      progMemoryStop $MEMPID \
                     $result_FLD/${upInComp}_${upInEnc}_DM__${inwf}_$ft    # mem

      ### verify if input and decompressed files are the same
      cmp $2 $in &> $result_FLD/${upInComp}_${upInEnc}_V__${inwf}_$ft;

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
            compDecomp $method $dsPath/$FA/$HUMAN/HS.$fasta
            compDecomp $method $dsPath/$FA/$VIRUSES/viruses.$fasta
            for i in 1 2; do
                compDecomp $method $dsPath/$FA/$Synth/Synth-$i.$fasta
            done;;

        "fq"|"FQ"|"fastq"|"FASTQ")   # FASTQ -- human - Denisova - synthetic
#            for i in ERR013103_1 ERR015767_2 ERR031905_2 \
#                     SRR442469_1 SRR707196_1; do
            for i in ERR013103_1 SRR442469_1; do
                compDecomp $method $dsPath/$FQ/$HUMAN/HS-$i.$fastq
            done
#            for i in B1087 B1088 B1110 B1128 SL3003; do
#                compDecomp $method $dsPath/$FQ/$DENISOVA/DS-${i}_SR.$fastq
#            done
#            for i in 1 2; do
#                compDecomp $method $dsPath/$FQ/$Synth/Synth-$i.$fastq
#            done
            ;;
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
            encDecrypt $method $dsPath/$FA/$HUMAN/HS.$fasta
            encDecrypt $method $dsPath/$FA/$VIRUSES/viruses.$fasta
            for i in 1 2;do
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
            for i in 1 2; do
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
            compEncDecDecompress \
                    $methodComp $dsPath/$FA/$HUMAN/HS.$fasta $methodEnc
            compEncDecDecompress \
                    $methodComp $dsPath/$FA/$VIRUSES/viruses.$fasta $methodEnc
            for i in 1 2; do
                compEncDecDecompress \
                    $methodComp $dsPath/$FA/$Synth/Synth-$i.$fasta $methodEnc
            done;;

        "fq"|"FQ"|"fastq"|"FASTQ")   # FASTQ -- human - Denisova - synthetic
#            for i in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 \
#                     SRR707196_1; do
            for i in ERR013103_1 SRR442469_1; do
                compEncDecDecompress \
                    $methodComp $dsPath/$FQ/$HUMAN/$HUMAN-$i.$fastq $methodEnc
            done
#            for i in B1087 B1088 B1110 B1128 SL3003; do
#                compEncDecDecompress $methodComp \
#                    $dsPath/$FQ/$DENISOVA/$DENISOVA-${i}_SR.$fastq $methodEnc
#            done
#            for i in 1 2; do
#                compEncDecDecompress \
#                    $methodComp $dsPath/$FQ/$Synth/Synth-$i.$fastq $methodEnc
#            done
            ;;
      esac
  }

  # print compress/decompress results. $1: program's name, $2: dataset
  function compDecompRes
  {
      in="${2##*/}"                       # dataset name
      dName="${in%.*}"                    # dataset name without filetype
      ft="${in##*.}"                      # dataset filetype
      fsize=`stat --printf="%s" $2`       # file size (bytes)
      method=`printMethodName $1`         # methods' name for printing

      CS="";
      CT_r="";     CT_u="";     CT_s="";     CM="";
      DT_r="";     DT_u="";     DT_s="";     DM="";
      V="";

      ### compressed file size
      cs_file="$result/${1}_CS__${dName}_$ft"
      if [[ -e $cs_file ]]; then CS=`cat $cs_file | awk '{ print $5; }'`; fi

      ### compression time -- real - user - system
      ct_file="$result/${1}_CT__${dName}_$ft"
      if [[ -e $ct_file ]]; then
          CT_r=`cat $ct_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          CT_u=`cat $ct_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          CT_s=`cat $ct_file | tail -n 1 | awk '{ print $2;}'`;
      fi

      ### compression memory
      cm_file="$result/${1}_CM__${dName}_$ft"
      if [[ -e $cm_file ]]; then CM=`cat $cm_file`; fi

      ### decompression time -- real - user - system
      dt_file="$result/${1}_DT__${dName}_$ft"
      if [[ -e $dt_file ]]; then
          DT_r=`cat $dt_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          DT_u=`cat $dt_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          DT_s=`cat $dt_file | tail -n 1 | awk '{ print $2;}'`;
      fi

      ### decompression memory
      dm_file="$result/${1}_DM__${dName}_$ft"
      if [[ -e $dm_file ]]; then DM=`cat $dm_file`; fi

      ### if decompressed file is the same as the original file
      v_file="$result/${1}_V__${dName}_$ft"
      if [[ -e $v_file ]]; then V=`cat $v_file | wc -l`; fi

      c="$CS\t$CT_r\t$CT_u\t$CT_s\t$CM"   # compression results
      d="$DT_r\t$DT_u\t$DT_s\t$DM"        # decompression results

      printf "$dName\t$fsize\t$method\t$c\t$d\t$V\n";
  }

  # print encrypt/decrypt results. $1: program's name, $2: dataset
  function encDecRes
  {
      in="${2##*/}"                       # dataset name
      dName="${in%.*}"                    # dataset name without filetype
      ft="${in##*.}"                      # dataset filetype
      fsize=`stat --printf="%s" $2`       # file size (bytes)
      method=`printMethodName $1`         # methods' name for printing

      EnS="";
      EnT_r="";     EnT_u="";     EnT_s="";     EnM="";
      DeT_r="";     DeT_u="";     DeT_s="";     DeM="";

      ### encrypted file size
      ens_file="$result/${1}_EnS__${dName}_$ft"
      if [[ -e $ens_file ]]; then EnS=`cat $ens_file | awk '{ print $5; }'`; fi

      ### encryption time -- real - user - system
      ent_file="$result/${1}_EnT__${dName}_$ft"
      if [[ -e $ent_file ]]; then
          EnT_r=`cat $ent_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          EnT_u=`cat $ent_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          EnT_s=`cat $ent_file | tail -n 1 | awk '{ print $2;}'`;
      fi

      ### encryption memory
      enm_file="$result/${1}_EnM__${dName}_$ft"
      if [[ -e $enm_file ]]; then EnM=`cat $enm_file`; fi

      ### decryption time -- real - user - system
      det_file="$result/${1}_DeT__${dName}_$ft"
      if [[ -e $det_file ]]; then
          DeT_r=`cat $det_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          DeT_u=`cat $det_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          DeT_s=`cat $det_file | tail -n 1 | awk '{ print $2;}'`;
      fi

      ### decryption memory
      dem_file="$result/${1}_DeM__${dName}_$ft"
      if [[ -e $dem_file ]]; then DeM=`cat $dem_file`; fi

      en="$EnS\t$EnT_r\t$EnT_u\t$EnT_s\t$EnM"    # encryption results
      de="$DeT_r\t$DeT_u\t$DeT_s\t$DeM"          # decryption results

      printf "$dName\t$fsize\t$method\t$en\t$de\n";
  }

  # print comp/decomp plus enc/dec results.
  # $1: compression program's name, $2: encryption program's name, $3: dataset.
  function compEncDecDecompRes
  {
      in="${3##*/}"                       # dataset name
      dName="${in%.*}"                    # dataset name without filetype
      ft="${in##*.}"                      # dataset filetype
      fsize=`stat --printf="%s" $3`       # file size (bytes)
      methodComp=`printMethodName $1`     # comp methods' name for printing
      methodEnc=`printMethodName $2`      # enc methods' name for printing

      CS="";
      CT_r="";     CT_u="";     CT_s="";     CM="";
      EnS="";
      EnT_r="";    EnT_u="";    EnT_s="";    EnM="";
      DeT_r="";    DeT_u="";    DeT_s="";    DeM="";
      DT_r="";     DT_u="";     DT_s="";     DM="";
      V="";

      ### compressed file size
      cs_file="$result/${1}_${2}_CS__${dName}_$ft"
      if [[ -e $cs_file ]]; then CS=`cat $cs_file | awk '{ print $5; }'`; fi

      ### compression time -- real - user - system
      ct_file="$result/${1}_${2}_CT__${dName}_$ft"
      if [[ -e $ct_file ]]; then
          CT_r=`cat $ct_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          CT_u=`cat $ct_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          CT_s=`cat $ct_file | tail -n 1 | awk '{ print $2;}'`;
      fi

      ### compression memory
      cm_file="$result/${1}_${2}_CM__${dName}_$ft"
      if [[ -e $cm_file ]]; then CM=`cat $cm_file`; fi

      ### encrypted file size
      ens_file="$result/${1}_${2}_EnS__${dName}_$ft"
      if [[ -e $ens_file ]]; then EnS=`cat $ens_file | awk '{ print $5; }'`; fi

      ### encryption time -- real - user - system
      ent_file="$result/${1}_${2}_EnT__${dName}_$ft"
      if [[ -e $ent_file ]]; then
          EnT_r=`cat $ent_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          EnT_u=`cat $ent_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          EnT_s=`cat $ent_file | tail -n 1 | awk '{ print $2;}'`;
      fi

      ### encryption memory
      enm_file="$result/${1}_${2}_EnM__${dName}_$ft"
      if [[ -e $enm_file ]]; then EnM=`cat $enm_file`; fi

      ### decryption time -- real - user - system
      det_file="$result/${1}_${2}_DeT__${dName}_$ft"
      if [[ -e $det_file ]]; then
          DeT_r=`cat $det_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          DeT_u=`cat $det_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          DeT_s=`cat $det_file | tail -n 1 | awk '{ print $2;}'`;
      fi

      ### decryption memory
      dem_file="$result/${1}_${2}_DeM__${dName}_$ft"
      if [[ -e $dem_file ]]; then DeM=`cat $dem_file`; fi

      ### decompression time -- real - user - system
      dt_file="$result/${1}_${2}_DT__${dName}_$ft"
      if [[ -e $dt_file ]]; then
          DT_r=`cat $dt_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          DT_u=`cat $dt_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          DT_s=`cat $dt_file | tail -n 1 | awk '{ print $2;}'`;
      fi

      ### decompression memory
      dm_file="$result/${1}_${2}_DM__${dName}_$ft"
      if [[ -e $dm_file ]]; then DM=`cat $dm_file`; fi

      ### if decompressed file is the same as the original file
      v_file="$result/${1}_${2}_V__${dName}_$ft"
      if [[ -e $v_file ]]; then V=`cat $v_file | wc -l`; fi

      c="$CS\t$CT_r\t$CT_u\t$CT_s\t$CM"        # compression results
      en="$EnS\t$EnT_r\t$EnT_u\t$EnT_s\t$EnM"  # encryption results
      de="$DeT_r\t$DeT_u\t$DeT_s\t$DeM"        # decryption results
      d="$DT_r\t$DT_u\t$DT_s\t$DM"             # decompression results

      printf "$dName\t$fsize\t$methodComp\t$methodEnc\t$c\t$en\t$de\t$d\t$V\n";
  }

  # convert memory numbers scale to MB and times to fractional minutes in
  # result files associated with comression methods. $1: input file name
  function compResHumanReadable
  {
      IN=$1              # input file name
      INWF="${IN%.*}"    # input file name without filetype

      c="C_Size(MB)\tC_Time_real(m)\tC_Time_cpu(m)\tC_Mem(MB)"
      d="D_Time_real(m)\tD_Time_cpu(m)\tD_Mem(MB)"
      printf "Dataset\tSize(MB)\tMethod\t$c\t$d\tEq\n" > $INWF.$INF

      cat $IN | awk 'NR>1' | awk 'BEGIN {}{
      printf "%s\t%.f\t%s\t%.f", $1, $2/1024/1024, $3, $4/1024/1024;

      split($5, c_arrMinReal, "m");                 c_minReal=c_arrMinReal[1];
      split(c_arrMinReal[2], c_arrSecReal, "s");    c_secReal=c_arrSecReal[1];
      c_realTime=(c_minReal*60+c_secReal)/60;
      printf "\t%.2f", c_realTime;

      split($6, c_arrMinUser, "m");                 c_minUser=c_arrMinUser[1];
      split(c_arrMinUser[2], c_arrSecUser, "s");    c_secUser=c_arrSecUser[1];
      c_userTime=(c_minUser*60+c_secUser)/60;
      split($7, c_arrMinSys, "m");                  c_minSys=c_arrMinSys[1];
      split(c_arrMinSys[2], c_arrSecSys, "s");      c_secSys=c_arrSecSys[1];
      c_sysTime=(c_minSys*60+c_secSys)/60;
      c_cpuTime=c_userTime+c_sysTime;
      printf "\t%.2f\t%.2f", c_cpuTime, $8/1024;

      split($9, d_arrMinReal, "m");                 d_minReal=d_arrMinReal[1];
      split(d_arrMinReal[2], d_arrSecReal, "s");    d_secReal=d_arrSecReal[1];
      d_realTime=(d_minReal*60+d_secReal)/60;
      printf "\t%.2f", d_realTime;

      split($10, d_arrMinUser, "m");                d_minUser=d_arrMinUser[1];
      split(d_arrMinUser[2], d_arrSecUser, "s");    d_secUser=d_arrSecUser[1];
      d_userTime=(d_minUser*60+d_secUser)/60;
      split($11, d_arrMinSys, "m");                 d_minSys=d_arrMinSys[1];
      split(d_arrMinSys[2], d_arrSecSys, "s");      d_secSys=d_arrSecSys[1];
      d_sysTime=(d_minSys*60+d_secSys)/60;
      d_cpuTime=d_userTime+d_sysTime;
      printf "\t%.2f\t%.2f\t%d\n", d_cpuTime, $12/1024, $13;
      }' >> $INWF.$INF
  }

  # convert memory numbers scale to MB and times to fractional minutes in
  # result files associated with encryption methods. $1: input file name
  function encResHumanReadable
  {
      IN=$1              # input file name
      INWF="${IN%.*}"    # input file name without filetype

      en="En_Size(MB)\tEn_Time_real(m)\tEn_Time_cpu(m)\tEn_Mem(MB)"
      de="De_Time_real(m)\tDe_Time_cpu(m)\tDe_Mem(MB)"
      printf "Dataset\tSize(MB)\tMethod\t$en\t$de\n" > $INWF.$INF

      cat $IN | awk 'NR>1' | awk 'BEGIN {}{
      printf "%s\t%.f\t%s\t%.f", $1, $2/1024/1024, $3, $4/1024/1024;

      split($5, c_arrMinReal, "m");                 c_minReal=c_arrMinReal[1];
      split(c_arrMinReal[2], c_arrSecReal, "s");    c_secReal=c_arrSecReal[1];
      c_realTime=(c_minReal*60+c_secReal)/60;
      printf "\t%.2f", c_realTime;

      split($6, c_arrMinUser, "m");                 c_minUser=c_arrMinUser[1];
      split(c_arrMinUser[2], c_arrSecUser, "s");    c_secUser=c_arrSecUser[1];
      c_userTime=(c_minUser*60+c_secUser)/60;
      split($7, c_arrMinSys, "m");                  c_minSys=c_arrMinSys[1];
      split(c_arrMinSys[2], c_arrSecSys, "s");      c_secSys=c_arrSecSys[1];
      c_sysTime=(c_minSys*60+c_secSys)/60;
      c_cpuTime=c_userTime+c_sysTime;
      printf "\t%.2f\t%.2f", c_cpuTime, $8/1024;

      split($9, d_arrMinReal, "m");                 d_minReal=d_arrMinReal[1];
      split(d_arrMinReal[2], d_arrSecReal, "s");    d_secReal=d_arrSecReal[1];
      d_realTime=(d_minReal*60+d_secReal)/60;
      printf "\t%.2f", d_realTime;

      split($10, d_arrMinUser, "m");                d_minUser=d_arrMinUser[1];
      split(d_arrMinUser[2], d_arrSecUser, "s");    d_secUser=d_arrSecUser[1];
      d_userTime=(d_minUser*60+d_secUser)/60;
      split($11, d_arrMinSys, "m");                 d_minSys=d_arrMinSys[1];
      split(d_arrMinSys[2], d_arrSecSys, "s");      d_secSys=d_arrSecSys[1];
      d_sysTime=(d_minSys*60+d_secSys)/60;
      d_cpuTime=d_userTime+d_sysTime;
      printf "\t%.2f\t%.2f\n", d_cpuTime, $12/1024;
      }' >> $INWF.$INF
  }

  # convert memory numbers scale to MB and times to fractional minutes in
  # result files associated with comression+encryption methods
  # $1: input file name
    function compEncResHumanReadable
  {
      IN=$1              # input file name
      INWF="${IN%.*}"    # input file name without filetype

      cen="CEn_Size(MB)\tCEn_Time_real(m)\tCEn_Time_cpu(m)\tCEn_Mem(MB)"
      ded="DeD_Time_real(m)\tDeD_Time_cpu(m)\tDeD_Mem(MB)"
      printf "Dataset\tSize(MB)\tC_Method\tEn_Method\t$cen\t$ded\tEq\n" \
             > $INWF.$INF

      cat $IN | awk 'NR>1' | awk 'BEGIN {}{
      printf "%s\t%.f\t%s\t%s\t%.f", $1, $2/1024/1024, $3, $4, $10/1024/1024;

      split($6, c_arrMinReal, "m");                 c_minReal=c_arrMinReal[1];
      split(c_arrMinReal[2], c_arrSecReal, "s");    c_secReal=c_arrSecReal[1];
      c_realTime=(c_minReal*60+c_secReal)/60;
      split($11, en_arrMinReal, "m");               en_minReal=en_arrMinReal[1];
      split(en_arrMinReal[2], en_arrSecReal, "s");  en_secReal=en_arrSecReal[1];
      en_realTime=(en_minReal*60+en_secReal)/60;
      cen_realTime=c_realTime+en_realTime;
      printf "\t%.2f", cen_realTime;

      split($7, c_arrMinUser, "m");                 c_minUser=c_arrMinUser[1];
      split(c_arrMinUser[2], c_arrSecUser, "s");    c_secUser=c_arrSecUser[1];
      c_userTime=(c_minUser*60+c_secUser)/60;
      split($8, c_arrMinSys, "m");                  c_minSys=c_arrMinSys[1];
      split(c_arrMinSys[2], c_arrSecSys, "s");      c_secSys=c_arrSecSys[1];
      c_sysTime=(c_minSys*60+c_secSys)/60;
      c_cpuTime=c_userTime+c_sysTime;
      split($12, en_arrMinUser, "m");               en_minUser=en_arrMinUser[1];
      split(en_arrMinUser[2], en_arrSecUser, "s");  en_secUser=en_arrSecUser[1];
      en_userTime=(en_minUser*60+en_secUser)/60;
      split($13, en_arrMinSys, "m");                en_minSys=en_arrMinSys[1];
      split(en_arrMinSys[2], en_arrSecSys, "s");    en_secSys=en_arrSecSys[1];
      en_sysTime=(en_minSys*60+en_secSys)/60;
      en_cpuTime=en_userTime+en_sysTime;
      cen_cpuTime=c_cpuTime+en_cpuTime;
      printf "\t%.2f", cen_cpuTime;

      max_cen_mem=($9 > $14 ? $9 : $14);
      cen_mem=max_cen_mem/1024;
      printf "\t%.2f", cen_mem;

      split($15, de_arrMinReal, "m");               de_minReal=de_arrMinReal[1];
      split(de_arrMinReal[2], de_arrSecReal, "s");  de_secReal=de_arrSecReal[1];
      de_realTime=(de_minReal*60+de_secReal)/60;
      split($19, d_arrMinReal, "m");                d_minReal=d_arrMinReal[1];
      split(d_arrMinReal[2], d_arrSecReal, "s");    d_secReal=d_arrSecReal[1];
      d_realTime=(d_minReal*60+d_secReal)/60;
      ded_realTime=de_realTime+d_realTime;
      printf "\t%.2f", ded_realTime;

      split($16, de_arrMinUser, "m");               de_minUser=de_arrMinUser[1];
      split(de_arrMinUser[2], de_arrSecUser, "s");  de_secUser=de_arrSecUser[1];
      de_userTime=(de_minUser*60+de_secUser)/60;
      split($17, de_arrMinSys, "m");                de_minSys=de_arrMinSys[1];
      split(de_arrMinSys[2], de_arrSecSys, "s");    de_secSys=de_arrSecSys[1];
      de_sysTime=(de_minSys*60+de_secSys)/60;
      de_cpuTime=de_userTime+de_sysTime;
      split($20, d_arrMinUser, "m");                d_minUser=d_arrMinUser[1];
      split(d_arrMinUser[2], d_arrSecUser, "s");    d_secUser=d_arrSecUser[1];
      d_userTime=(d_minUser*60+d_secUser)/60;
      split($21, d_arrMinSys, "m");                 d_minSys=d_arrMinSys[1];
      split(d_arrMinSys[2], d_arrSecSys, "s");      d_secSys=d_arrSecSys[1];
      d_sysTime=(d_minSys*60+d_secSys)/60;
      d_cpuTime=d_userTime+d_sysTime;
      ded_cpuTime=de_cpuTime+d_cpuTime;
      printf "\t%.2f", ded_cpuTime;

      max_ded_mem=($18 > $22 ? $18 : $22);
      ded_mem=max_ded_mem/1024;
      printf "\t%.2f", ded_mem;

      printf "\t%d\n", $23;
      }' >> $INWF.$INF
  }

  # convert memory numbers scale to MB and times to fractional minutes in
  # result files associated with cryfa method. $1: input file name
  function cryfaXclResHumanReadable
  {
      IN=$1              # input file name
      INWF="${IN%.*}"    # input file name without filetype

      c="C_Size(MB)\tC_Time_real(m)\tC_Time_cpu(m)\tC_Mem(MB)"
      d="D_Time_real(m)\tD_Time_cpu(m)\tD_Mem(MB)"
      printf "Dataset\tSize(MB)\tThr\t$c\t$d\tEq\n" > $INWF.$INF

      cat $IN | awk 'NR>1' | awk 'BEGIN {}{
      printf "%s\t%.f\t%s\t%.f", $1, $2/1024/1024, $3, $4/1024/1024;

      split($5, c_arrMinReal, "m");                 c_minReal=c_arrMinReal[1];
      split(c_arrMinReal[2], c_arrSecReal, "s");    c_secReal=c_arrSecReal[1];
      c_realTime=(c_minReal*60+c_secReal)/60;
      printf "\t%.2f", c_realTime;

      split($6, c_arrMinUser, "m");                 c_minUser=c_arrMinUser[1];
      split(c_arrMinUser[2], c_arrSecUser, "s");    c_secUser=c_arrSecUser[1];
      c_userTime=(c_minUser*60+c_secUser)/60;
      split($7, c_arrMinSys, "m");                  c_minSys=c_arrMinSys[1];
      split(c_arrMinSys[2], c_arrSecSys, "s");      c_secSys=c_arrSecSys[1];
      c_sysTime=(c_minSys*60+c_secSys)/60;
      c_cpuTime=c_userTime+c_sysTime;
      printf "\t%.2f\t%.2f", c_cpuTime, $8/1024;

      split($9, d_arrMinReal, "m");                 d_minReal=d_arrMinReal[1];
      split(d_arrMinReal[2], d_arrSecReal, "s");    d_secReal=d_arrSecReal[1];
      d_realTime=(d_minReal*60+d_secReal)/60;
      printf "\t%.2f", d_realTime;

      split($10, d_arrMinUser, "m");                d_minUser=d_arrMinUser[1];
      split(d_arrMinUser[2], d_arrSecUser, "s");    d_secUser=d_arrSecUser[1];
      d_userTime=(d_minUser*60+d_secUser)/60;
      split($11, d_arrMinSys, "m");                 d_minSys=d_arrMinSys[1];
      split(d_arrMinSys[2], d_arrSecSys, "s");      d_secSys=d_arrSecSys[1];
      d_sysTime=(d_minSys*60+d_secSys)/60;
      d_cpuTime=d_userTime+d_sysTime;
      printf "\t%.2f\t%.2f\t%d\n", d_cpuTime, $12/1024, $13;
      }' >> $INWF.$INF
  }

  #------------------- dataset availablity -------------------#
  # FASTA -- human - viruses - synthetic
  isAvail "$dataset/$FA/$HUMAN/HS.$fasta";
  isAvail "$dataset/$FA/$VIRUSES/viruses.$fasta"
  for i in 1 2; do isAvail "$dataset/$FA/$Synth/Synth-$i.$fasta"; done

  # FASTQ -- human - Denisova - synthetic
  for i in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 SRR707196_1; do
      isAvail "$dataset/$FQ/$HUMAN/$HUMAN-$i.$fastq"
  done
  for i in B1087 B1088 B1110 B1128 SL3003; do
      isAvail "$dataset/$FQ/$DENISOVA/$DENISOVA-${i}_SR.$fastq"
  done
  for i in 1 2; do isAvail "$dataset/$FQ/$Synth/Synth-$i.$fastq"; done

  #--------------------------- run ---------------------------#
  ### compress/decompress
  if [[ $RUN_METHODS_COMP -eq 1 ]];
  then
      ### FASTA
      if [[ $RUN_GZIP_FA    -eq 1 ]]; then compDecompOnDataset gzip       fa; fi
      if [[ $RUN_BZIP2_FA   -eq 1 ]]; then compDecompOnDataset bzip2      fa; fi
      if [[ $RUN_LZMA_FA    -eq 1 ]]; then compDecompOnDataset lzma       fa; fi
      if [[ $RUN_MFCOMPRESS -eq 1 ]]; then compDecompOnDataset mfcompress fa; fi
      if [[ $RUN_DELIMINATE -eq 1 ]]; then compDecompOnDataset delim      fa; fi
      if [[ $RUN_CRYFA_FA   -eq 1 ]];
      then
          cmake .
          make
          if [[ ! -d $progs/cryfa ]]; then mkdir -p $progs/cryfa; fi
          cp cryfa pass.txt  $progs/cryfa/

          compDecompOnDataset cryfa fa;
      fi

      ### FASTQ
      if [[ $RUN_GZIP_FQ    -eq 1 ]]; then compDecompOnDataset gzip       fq; fi
      if [[ $RUN_BZIP2_FQ   -eq 1 ]]; then compDecompOnDataset bzip2      fq; fi
      if [[ $RUN_LZMA_FQ    -eq 1 ]]; then compDecompOnDataset lzma       fq; fi
      if [[ $RUN_FQZCOMP    -eq 1 ]]; then compDecompOnDataset fqzcomp    fq; fi
      if [[ $RUN_QUIP       -eq 1 ]]; then compDecompOnDataset quip       fq; fi
      if [[ $RUN_DSRC       -eq 1 ]]; then compDecompOnDataset dsrc       fq; fi
      if [[ $RUN_FQC        -eq 1 ]]; then compDecompOnDataset fqc        fq; fi
      if [[ $RUN_CRYFA_FQ   -eq 1 ]];
      then
          cmake .
          make
          if [[ ! -d $progs/cryfa ]]; then mkdir -p $progs/cryfa; fi
          cp cryfa pass.txt  $progs/cryfa/

          compDecompOnDataset cryfa fq;
      fi

      #---------------- results ----------------#
      if [[ $PRINT_RESULTS_COMP -eq 1 ]];
      then
          OUT="$result/COMP.$RES"    # output file name
          FAdsPath=$dataset/$FA
          FQdsPath=$dataset/$FQ

          ### print results
          c="C_Size(B)\tC_Time_real(s)\tC_Time_user(s)\tC_Time_sys(s)\t"
          c+="C_Mem(KB)"
          d="D_Time_real(s)\tD_Time_user(s)\tD_Time_sys(s)\tD_Mem(KB)"
          printf "Dataset\tSize\tMethod\t$c\t$d\tEq\n" > $OUT;

          # FASTA -- human - viruses - synthetic
          for i in CRYFA GZIP BZIP2 LZMA MFCOMPRESS DELIMINATE; do
              compDecompRes $i $FAdsPath/$HUMAN/HS.$fasta >> $OUT;
              compDecompRes $i $FAdsPath/$VIRUSES/viruses.$fasta >> $OUT;
              for j in 1 2; do
                  compDecompRes $i $FAdsPath/$Synth/Synth-${j}.$fasta >> $OUT;
              done
          done

          # FASTQ -- human - Denisova - synthetic
          for i in CRYFA GZIP BZIP2 LZMA FQZCOMP QUIP DSRC FQC; do
              for j in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 \
                       SRR707196_1; do
                  compDecompRes $i $FQdsPath/$HUMAN/HS-${j}.$fastq >> $OUT;
              done
              for j in B1087 B1088 B1110 B1128 SL3003; do
                  compDecompRes $i $FQdsPath/$DENISOVA/DS-${j}_SR.$fastq >> $OUT
              done
              for j in 1 2; do
                  compDecompRes $i $FQdsPath/$Synth/Synth-${j}.$fastq >> $OUT;
              done
          done

          ### convert the result file into a human readable file
          compResHumanReadable $OUT;
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
          OUT="$result/ENC.$RES"    # output file name
          FAdsPath=$dataset/$FA
          FQdsPath=$dataset/$FQ

          ### print results
          en="En_Size(B)\tEn_Time_real(s)\tEn_Time_user(s)\tEn_Time_sys(s)\t"
          en+="En_Mem(KB)"
          de="De_Time_real(s)\tDe_Time_user(s)\tDe_Time_sys(s)\tDe_Mem(KB)"
          printf "Dataset\tSize\tMethod\t$en\t$de\n" > $OUT;

          for i in AESCRYPT;
          do
              # FASTA -- human - viruses - synthetic
              encDecRes $i $FAdsPath/$HUMAN/HS.$fasta >> $OUT;
              encDecRes $i $FAdsPath/$VIRUSES/viruses.$fasta >> $OUT;
              for j in 1 2; do
                  encDecRes $i $FAdsPath/$Synth/Synth-${j}.$fasta >> $OUT;
              done

              # FASTQ -- human - Denisova - synthetic
              for j in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 \
                       SRR707196_1; do
                  encDecRes $i $FQdsPath/$HUMAN/HS-${j}.$fastq >> $OUT;
              done
              for j in B1087 B1088 B1110 B1128 SL3003; do
                  encDecRes $i $FQdsPath/$DENISOVA/DS-${j}_SR.$fastq >> $OUT;
              done
              for j in 1 2; do
                  encDecRes $i $FQdsPath/$Synth/Synth-${j}.$fastq >> $OUT;
              done
          done

          ### convert the result file into a human readable file
          encResHumanReadable $OUT;
      fi
  fi

  ### compress/decompress and encrypt/decrypt
  if [[ $RUN_METHODS_COMP_ENC -eq 1 ]];
  then
      ### FASTA + AEScrypt
      if [[ $RUN_GZIP_FA_AESCRYPT -eq 1 ]]; then
        compEncDecDecompOnDataset gzip fa aescrypt;
      fi
      if [[ $RUN_BZIP2_FA_AESCRYPT -eq 1 ]]; then
        compEncDecDecompOnDataset bzip2 fa aescrypt;
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
      if [[ $RUN_BZIP2_FQ_AESCRYPT -eq 1 ]]; then
        compEncDecDecompOnDataset bzip2 fq aescrypt;
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
          OUT="$result/COMP_ENC.$RES"    # output file name
          FAdsPath=$dataset/$FA
          FQdsPath=$dataset/$FQ

          ### print results
          c="C_Size(B)\tC_Time_real(s)\tC_Time_user(s)\tC_Time_sys(s)\t"
          c+="C_Mem(KB)"
          en="En_Size(B)\tEn_Time_real(s)\tEn_Time_user(s)\tEn_Time_sys(s)\t"
          en+="En_Mem(KB)"
          de="De_Time_real(s)\tDe_Time_user(s)\tDe_Time_sys(s)\tDe_Mem(KB)"
          d="D_Time_real(s)\tD_Time_user(s)\tD_Time_sys(s)\tD_Mem(KB)"
          printf "Dataset\tSize\tC_Method\tEn_Method\t$c\t$en\t$de\t$d\tEq\n" \
                 > $OUT;

          for i in AESCRYPT; do
             # FASTA -- human - viruses - synthetic
             for j in GZIP BZIP2 LZMA MFCOMPRESS DELIM; do
                 compEncDecDecompRes $j $i $FAdsPath/$HUMAN/HS.$fasta >> $OUT;
                 compEncDecDecompRes $j $i \
                                     $FAdsPath/$VIRUSES/viruses.$fasta >> $OUT;
                 for k in 1 2; do
                     compEncDecDecompRes $j $i \
                                     $FAdsPath/$Synth/Synth-${k}.$fasta >> $OUT;
                 done
             done

             # FASTQ -- human - Denisova - synthetic
             for j in GZIP BZIP2 LZMA FQZCOMP QUIP DSRC FQC; do
                 for k in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 \
                          SRR707196_1; do
                     compEncDecDecompRes $j $i \
                                  $FQdsPath/$HUMAN/HS-${k}.$fastq >> $OUT;
                 done
                 for k in B1087 B1088 B1110 B1128 SL3003; do
                     compEncDecDecompRes $j $i \
                                  $FQdsPath/$DENISOVA/DS-${k}_SR.$fastq >> $OUT;
                 done
                 for k in 1 2; do
                     compEncDecDecompRes $j $i \
                                  $FQdsPath/$Synth/Synth-${k}.$fastq >> $OUT;
                 done
             done
          done

          ### convert the result file into a human readable file
          compEncResHumanReadable $OUT;
      fi
  fi

  ### cryfa exclusive
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
#      CRYFA_THR_RUN=`seq -s' ' 1 $MAX_N_THR`;
      CRYFA_THR_RUN=$MAX_N_THR;

      ### run for different number of threads
      if [[ $RUN_CRYFA_XCL -eq 1 ]];
      then
          cmake .
          make
          cp cryfa pass.txt  $cryfa_xcl;

          cd $cryfa_xcl

          for nThr in $CRYFA_THR_RUN; do
              cFT="cryfa";           cCmd="./cryfa -k pass.txt -t $nThr";
              dCmd="./cryfa -k pass.txt -t $nThr -d";

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

      ### run for different number of threads
      if [[ $PRINT_RESULTS_CRYFA_XCL -eq 1 ]];
      then
          result_FLD="../$result"
          OUT="CRYFA_THR.$RES"       # output file name
          cd $cryfa_xcl

          c="C_Size\tC_Time(real)\tC_Time(user)\tC_Time(sys)\tC_Mem"
          d="D_Time(real)\tD_Time(user)\tD_Time(sys)\tD_Mem"
          printf "Dataset\tSize\tThread\t$c\t$d\tEq\n" > $OUT;

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
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   plot results
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $PLOT_RESULTS -eq 1 ]];
then
  #------------------------ functions ------------------------#
  ### plot result. $1: input file, $2: dataset
  function plotResultCryfaXcl
  {
      IN=$1             # input file
      DATASET=$2        # dataset name
      PIX_FORMAT=pdf    # output format: pdf, png, svg, eps, epslatex
      TITLE=${DATASET//_/'\\\_'}        # title of figure -- replace _ with \\\_
      dsSize=`grep "$DATASET" $IN | awk '{print $2; exit}'`  #input dataset size

gnuplot <<- EOF
set term $PIX_FORMAT    # terminal for output picture format
set title font ",16" "Cryfa on $TITLE ($dsSize MB)"    # title
set xlabel 'Number of threads'                         # label of x axis
set xrange [0.5:8.5]
set style data histogram
set style histogram cluster gap 1
set style fill solid # border
set boxwidth 1
#set xtics format ""
set grid ytics

# - - - - - - - - - -  memory  - - - - - - - - - - #
set output "CRYFA_THR_MEM.$PIX_FORMAT"  # output file name
set ylabel 'Memory (MB)'                # label of y axis
set key top left                        # legend position
#unset key
plot '$IN' using 7:xtic(3) title "Compression"   linecolor rgb"blue", \
     ''    using 10        title "Decompression" linecolor rgb"red"

# - - - - - - - - -  real time  - - - - - - - - - #
set output "CRYFA_THR_TIME_REAL.$PIX_FORMAT"    # output file name
set ylabel 'Real Time (min)'                    # label of y axis
set key top right                               # legend position
#unset key
plot '$IN' using 5:xtic(3) title "Compression"   linecolor rgb"blue", \
     ''    using 8         title "Decompression" linecolor rgb"red"

# - - - - - - - - -  cpu time  - - - - - - - - - #
set output "CRYFA_THR_TIME_CPU.$PIX_FORMAT"     # output file name
set ylabel 'CPU Time (min)'                     # label of y axis
set key top left                                # legend position
#unset key
plot '$IN' using 6:xtic(3) title "Compression"   linecolor rgb"blue", \
     ''    using 9         title "Decompression" linecolor rgb"red"

### the following line (EOF) MUST be left as it is; i.e. no indent
EOF
  }

  ### plot for cryfa excusive method
  cd $cryfa_xcl

  ### identify unique datasets names
  dataArr=($(cat CRYFA_THR.dat | awk '{print $1}' | uniq));
###  for i in ${dataArr[@]}; do echo $i; done

 plotResultCryfaXcl "CRYFA_THR.dat" ${dataArr[1]}

#  for tuple in "yLbl='Compression Time - real (min)' yCol='4'\
#                out='CRYFA_THR_COMP_TIME_REAL'"                      \
#                "yLbl='Compression Time - cpu (min)' yCol='5'\
#                out='CRYFA_THR_COMP_TIME_CPU'"                       \
#                "yLbl='Compression Memory (MB)' yCol='6'\
#                out='CRYFA_THR_COMP_MEM'"                            \
#                "yLbl='Decompression Time - real (min)' yCol='7'\
#                out='CRYFA_THR_DECOMP_TIME_REAL'"                    \
#                "yLbl='Decompression Time - cpu (min)' yCol='8'\
#                out='CRYFA_THR_DECOMP_TIME_CPU'"                     \
#                "yLbl='Decompression Memory (MB)' yCol='9'\
#                out='CRYFA_THR_DECOMP_MEM'" \
#  ; do
#      eval $tuple;
#      plotResultCryfaXcl "CRYFA_THR.dat" ${dataArr[1]} \
#                         "Number of threads" 2 "$yLbl" "$yCol" "$out";
#  done

  cd ..

fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   test purpose -- run cryfa - compress
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
#   test purpose -- run cryfa - decompress
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
#   test purpose -- run cryfa -- compress + decompress + compare results
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