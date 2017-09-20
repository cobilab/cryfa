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
N_THREADS=8             # number of threads

### datasets
GET_HUMAN_FA=0          # download Human choromosomes in FASTA
GET_VIRUSES_FA=0        # download Viruses in FASTA using downloadViruses.pl
GEN_SYNTH_FA=0          # generate synthetic FASTA dataset using XS
GET_HUMAN_FQ=0          # download Human in FASTQ
GET_DENISOVA_FQ=0       # download Denisova in FASTQ
GEN_SYNTH_FQ=0          # generate synthetic FASTQ dataset using XS

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
  # FASTA
  INS_MFCOMPRESS=0      # MFCompress -- error: make
  INS_DELIMINATE=0      # DELIMINATE -- error: site not reachable

  # FASTQ
  INS_FQZCOMP=0         # fqzcomp
  INS_QUIP=0            # quip
  INS_DSRC=0            # DSRC
  INS_FQC=0             # FQC -- error: site not reachable

### run methods


# cryfa
SHUFFLE=1               # cryfa: shuffle -- enabled by default
VERBOSE=1               # cryga: verbose mode -- disabled by default
CRYFA_COMP=0            # cryfa -- compress
CRYFA_DECOMP=0          # cryfa -- decompress
CRYFA_COMP_DECOMP_COMPARE=0   # test -- cryfa: comp. + decomp. + compare results


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   folders
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
dataset="dataset"
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
#   scientific & abbreviated names
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
HUMAN_SNAME="Homo sapiens"
VIRUSES_SNAME="Viruses"

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


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   create folders, if they don't already exist
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ ! -d $dataset ]]; then mkdir -p $dataset; fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   get human FASTA -- 3.1 GB
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GET_HUMAN_FA -eq 1 ]]; then

    ### create a folder for FASTA files and one for human dataset
    if [[ ! -d $dataset/$FA/$HUMAN ]]; then mkdir -p $dataset/$FA/$HUMAN; fi

    ### download
    for i in {1..22} X Y MT; do
        wget $WGET_OP $HUMAN_FA_URL/$HUMAN_CHROMOSOME$i.fa.gz;
        gunzip < $HUMAN_CHROMOSOME$i.fa.gz > $dataset/$FA/$HUMAN/$HUMAN-$i.fa;
        rm $HUMAN_CHROMOSOME$i.fa.gz
    done

    for dual in "alts AL" "unplaced UP" "unlocalized UL"; do
        set $dual
        wget $WGET_OP $HUMAN_FA_URL/$HUMAN_CHR_PREFIX$1.fa.gz;
        gunzip < $HUMAN_CHR_PREFIX$1.fa.gz > $dataset/$FA/$HUMAN/$HUMAN-$2.fa;
        rm $HUMAN_CHR_PREFIX$1.fa.gz;
    done
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   get viruses FASTA -- 350 MB
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GET_VIRUSES_FA -eq 1 ]]; then

    ### create a folder for FASTA files and one for viruses dataset
    if [[ ! -d $dataset/$FA/$VIRUSES ]]; then mkdir -p $dataset/$FA/$VIRUSES; fi

    ### download
    perl ./scripts/DownloadViruses.pl

    ### move downloaded file to dataset folder
    mv viruses.fa $dataset/$FA/$VIRUSES
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   generate synthetic dataset in FASTA -- 4 GB
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GEN_SYNTH_FA -eq 1 ]]; then

    INSTALL_XS=1

    ### create a folder for FASTA files and one for synthetic dataset
    if [[ ! -d $dataset/$FA/$Synth ]]; then mkdir -p $dataset/$FA/$Synth; fi
    
    ### install XS
    if [[ $INSTALL_XS -eq 1 ]]; then
        rm -fr $XS
        git clone https://github.com/pratas/XS.git
        cd $XS
        make
        cd ..
    fi

    ### generate dataset -- 2.3 GB - 1.7 GB
    XS/XS -eo -es -t 1 -n 4000000 -ld 70:1000 -f 0.2,0.2,0.2,0.2,0.2  Synth-1.fa
    XS/XS -eo -es -t 2 -n 3000000 -ls 500 -f 0.23,0.23,0.23,0.23,0.08 Synth-2.fa

    for i in {1..2}; do mv Synth-$i.fa $dataset/$FA/$Synth/Synth-$i.fa; done
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   get Human in FASTQ -- 27 GB
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GET_HUMAN_FQ -eq 1 ]]; then

    ### create a folder for FASTQ files and one for human dataset
    if [[ ! -d $dataset/$FQ/$HUMAN ]]; then mkdir -p $dataset/$FQ/$HUMAN; fi

    ### download -- 4.6 GB - 1.4 GB - 12 GB - 488 MB - 8.4 GB
    # ERR013103_1: HG00190--Male--FIN (Finnish in Finland)--Low coverage WGS
    # ERR015767_2: HG00638--Female--PUR (Puerto Rican in Puerto Rico)--Low cov.
    # ERR031905_2: HG00501--Female--CHS (Han Chinese South)--Exome
    # SRR442469_1: HG02108--Female--ACB (African Caribbean in Barbados)--Low co.
    # SRR707196_1: HG00126--Male--GBR (British in England and Scotland)--Exome
    for tuple in "ERR013 ERR013103 ERR013103_1" "ERR015 ERR015767 ERR015767_2"\
                 "ERR031 ERR031905 ERR031905_2" "SRR442 SRR442469 SRR442469_1"\
                 "SRR707 SRR707196 SRR707196_1"; do
        set $tuple
        wget $WGET_OP $HUMAN_FQ_URL/$1/$2/$3.fastq.gz;
        gunzip < $3.fastq.gz > $dataset/$FQ/$HUMAN/$HUMAN-$3.fq;
        rm $3.fastq.gz;
    done
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   get Denisova in FASTQ -- 172 GB
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GET_DENISOVA_FQ -eq 1 ]]; then

    ### create a folder for FASTQ files and one for Denisova dataset
    if [[ ! -d $dataset/$FQ/$DENISOVA ]];then mkdir -p $dataset/$FQ/$DENISOVA;fi

    ### download -- 1.7 GB - 1.2 GB - 59 GB - 51 GB - 59 GB
    for i in B1087 B1088 B1110 B1128 SL3003; do
        wget $WGET_OP $DENISOVA_FQ_URL/${i}_SR.txt.gz;
        gunzip < ${i}_SR.txt.gz > $dataset/$FQ/$DENISOVA/$DENISOVA-${i}_SR.fq;
        rm ${i}_SR.txt.gz;
    done
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   generate synthetic dataset in FASTQ -- 6.2 GB
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GEN_SYNTH_FQ -eq 1 ]]; then

    INSTALL_XS=1

    ### create a folder for FASTQ files and one for synthetic dataset
    if [[ ! -d $dataset/$FQ/$Synth ]]; then mkdir -p $dataset/$FQ/$Synth; fi

    ### install XS
    if [[ $INSTALL_XS -eq 1 ]]; then
        rm -fr $XS
        git clone https://github.com/pratas/XS.git
        cd $XS
        make
        cd ..
    fi

    ### generate dataset -- 4.2 GB - 2 GB
    XS/XS -t 1 -n 16000000 -ld 70:100 -o -f 0.2,0.2,0.2,0.2,0.2      Synth-1.fq
    XS/XS -t 2 -n 10000000 -ls 70 -qt 2 -f 0.23,0.23,0.23,0.23,0.08  Synth-2.fq

    for i in {1..2}; do mv Synth-$i.fq $dataset/$FQ/$Synth/Synth-$i.fq; done
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   install dependencies
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $INSTALL_DEPENDENCIES -eq 1 ]]; then

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
if [[ $INSTALL_METHODS -eq 1 ]]; then

    ###################   FASTA   ###################
    ### MFCompress
    if [[ $INS_MFCOMPRESS -eq 1 ]]; then
        rm -f MFCompress-src-1.01.tgz

        url="http://sweet.ua.pt/ap/software/mfcompress"
        wget $WGET_OP $url/MFCompress-src-1.01.tgz
        tar -xzf MFCompress-src-1.01.tgz
        mv MFCompress-src-1.01/ mfcompress
        rm -f MFCompress-src-1.01.tgz

        cd mfcompress/
        cp Makefile.linux Makefile    # make -f Makefile.linux
        make
        cd ..
    fi

    ### DELIMINATE
    if [[ $INS_DELIMINATE -eq 1 ]]; then

        rm -f DELIMINATE_LINUX_64bit.tar.gz

        url="http://metagenomics.atc.tcs.com/Compression_archive"
        wget $WGET_OP $url/DELIMINATE_LINUX_64bit.tar.gz
        tar -xzf DELIMINATE_LINUX_64bit.tar.gz
        mv EXECUTABLES deliminate
        rm -f DELIMINATE_LINUX_64bit.tar.gz
    fi

    ###################   FASTQ   ###################
    ### fqzcomp
    if [[ $INS_FQZCOMP -eq 1 ]]; then

        rm -f fqzcomp-4.6.tar.gz

        url="https://downloads.sourceforge.net/project/fqzcomp"
        wget $WGET_OP $url/fqzcomp-4.6.tar.gz
        tar -xzf fqzcomp-4.6.tar.gz
        mv fqzcomp-4.6/ fqzcomp/
        rm -f fqzcomp-4.6.tar.gz

        cd fqzcomp/
        make
        cd ..
    fi

    ### quip
    if [[ $INS_QUIP -eq 1 ]]; then

        rm -f quip-1.1.8.tar.gz

        url="http://homes.cs.washington.edu/~dcjones/quip"
        wget $WGET_OP $url/quip-1.1.8.tar.gz
        tar -xzf quip-1.1.8.tar.gz
        mv quip-1.1.8/ quip/
        rm -f quip-1.1.8.tar.gz

        cd quip/
        ./configure
        cd src/
        make
        cp quip ../
        cd ../../
    fi

    ### DSRC
    if [[ $INS_DSRC -eq 1 ]]; then

        rm -fr dsrc/

        git clone https://github.com/lrog/dsrc.git
        cd dsrc/
        make
        cp bin/dsrc .
        cd ..
    fi

    ### FQC
    if [[ $INS_FQC -eq 1 ]]; then

        rm -f FQC_LINUX_64bit.tar.gz

        url="http://metagenomics.atc.tcs.com/Compression_archive/FQC"
        wget $WGET_OP $url/FQC_LINUX_64bit.tar.gz
        tar -xzf FQC_LINUX_64bit.tar.gz
        mv FQC_LINUX_64bit/ fqc/
        rm -f FQC_LINUX_64bit.tar.gz

    #    cd fqc/
    #    #cp 7za ../
    #    #cp fcompfastq ../
    #    #cp fdecompfastq ../
    #    #cp fqc ../
    #    cd ../
    fi
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   functions
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
### check if file exists
function FExists
{
  file="$1"
  if [[ ! -e $file ]]; then
    echo "Warning: The file \"$file\" is not available.";
    return;
  fi
}

### memory1
function ProgMemoryStart
{
    echo "0" > mem_ps;
    while true; do
        ps aux | grep $1 | awk '{ print $6; }' | sort -V | tail -n 1 >> mem_ps;
        sleep 5;
    done
}
function ProgMemoryStop
{
    kill $1 >/dev/null 2>&1
    cat mem_ps | sort -V | tail -n 1 > $2;
}

### memory2
function ProgMemory2
{
    valgrind --tool=massif --pages-as-heap=yes --massif-out-file=massif.out ./$1
    cat massif.out | grep mem_heap_B | sed -e 's/mem_heap_B=\(.*\)/\1/' | \
    sort -g | tail -n 1
}

### time
function ProgTime
{
    time ./$1
}

### gzip method
function compGzip
{
    ProgMemoryStart "gzip" &
    MEMPID=$!
    (time gzip $1 ) &> ../../results/C_GZIP_$2
    ls -la $1.gz > ../../results/BC_GZIP_$2
    ProgMemoryStop $MEMPID "../../results/MC_GZIP_$2";
    ProgMemoryStart "gunzip" &
    MEMPID=$!
    (time gunzip $1.gz ) &> ../../results/D_GZIP_$2
    ProgMemoryStop $MEMPID "../../results/MD_GZIP_$2";
}

### lzma method
function compLzma
{
    ProgMemoryStart "lzma" &
    MEMPID=$!
    (time lzma $1 ) &> ../../results/LZMA_CT_$2
    ls -la $1.lzma > ../../results/LZMA_CB_$2
    ProgMemoryStop $MEMPID "../../results/LZMA_CM_$2";

    ProgMemoryStart "lzma" &
    MEMPID=$!
    (time lzma -d $1.lzma ) &> ../../results/LZMA_DT_$2
    ProgMemoryStop $MEMPID "../../results/LZMA_DM_$2";
}
#function compLzma
#{
#    ProgMemoryStart "lzma" &
#    MEMPID=$!
#    (time lzma $1 ) &> ../../results/C_LZMA_$2
#    ls -la $1.lzma > ../../results/BC_LZMA_$2
#    ProgMemoryStop $MEMPID "../../results/MC_LZMA_$2";
#    ProgMemoryStart "lzma" &
#    MEMPID=$!
#    (time lzma -d $1.lzma ) &> ../../results/D_LZMA_$2
#    ProgMemoryStop $MEMPID "../../results/MD_LZMA_$2";
#}


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