#!/bin/bash

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   parameters -- set to 1 to activate and 0 to deactivate
#   get/generate datasets, install dependencies, run cryfa, plot results
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
N_THRD=8                # number of threads

GET_HUMAN_FA=0          # download Human choromosomes in FASTA
GET_VIRUSES_FA=0        # download Viruses in FASTA using downloadViruses.pl
GEN_SYNTH_FA=0          # generate synthetic FASTA dataset using XS
GET_HUMAN_FQ=0          # download Human in FASTQ
GET_DENISOVA_FQ=0       # download Denisova in FASTQ
GEN_SYNTH_FQ=0          # generate synthetic FASTQ dataset using XS

CRYFA_COMP=1            # cryfa -- compress
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
#   get human FA -- 3.1 GB
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
#   get viruses FA -- 350 MB
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
#   get Human in FASTQ -- 8.6 GB compressed * 4 -> GB decompressed
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GET_HUMAN_FQ -eq 1 ]]; then

    ### create a folder for FASTQ files and one for human dataset
    if [[ ! -d $dataset/$FQ/$HUMAN ]]; then mkdir -p $dataset/$FQ/$HUMAN; fi

    ### download -- 160 MB - 360 MB - 1.7 GB - 2.6 GB - 3.8 GB
    # SRR707196_1: HG00126--Male--GBR (British in England and Scotland)--Exome
    # ERR031905_2: HG00501--Female--CHS (Han Chinese South)--Exome
    # ERR013103_1: HG00190--Male--FIN (Finnish in Finland)--Low coverage WGS
    # ERR015767_2: HG00638--Female--PUR (Puerto Rican in Puerto Rico)--Low cov.
    # SRR442469_1: HG02108--Female--ACB (African Caribbean in Barbados)--Low co.
    for tuple in "SRR442 SRR442469 SRR442469_1" "ERR015 ERR015767 ERR015767_2"\
                 "ERR013 ERR013103 ERR013103_1" "SRR707 SRR707196 SRR707196_1"\
                 "ERR031 ERR031905 ERR031905_2"; do
        set $tuple
        wget $WGET_OP $HUMAN_FQ_URL/$1/$2/$3.fastq.gz;
        gunzip < $3.fastq.gz > $dataset/$FQ/$HUMAN/$HUMAN-$3.fq;
        rm $3.fastq.gz;
    done
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   get Denisova in FASTQ -- 41.7 GB compressed * 4 -> GB decompressed
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GET_DENISOVA_FQ -eq 1 ]]; then

    ### create a folder for FASTQ files and one for Denisova dataset
    if [[ ! -d $dataset/$FQ/$DENISOVA ]];then mkdir -p $dataset/$FQ/$DENISOVA;fi

    ### download -- 292 MB - 396 MB - 11 GB - 15 GB - 15 GB
    for i in B1088 B1087 B1128 B1110 SL3003; do
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
#   run cryfa -- compress
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CRYFA_COMP -eq 1 ]]; then
    cmake . | make | ./cryfa -t $N_THRD -k pass.txt $1   # -s: disable shuffling
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   run cryfa -- decompress
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CRYFA_DECOMP -eq 1 ]]; then
    cmake . | make | ./cryfa -t $N_THRD -dk pass.txt $1
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   run cryfa -- compress + decompress + compare results
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CRYFA_COMP_DECOMP_COMPARE -eq 1 ]]; then
    # compress
    cmake . | make | ./cryfa -t $N_THRD -k pass.txt $1>$2 #-s: disable shuffling

    # decompress
    ./cryfa -t $N_THRD -dk pass.txt $2 > "CRYFA_DECOMPRESSED"

    # compare
    cmp $1 "CRYFA_DECOMPRESSED"
fi