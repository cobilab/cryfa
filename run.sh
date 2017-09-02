#!/bin/bash

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   arguments
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
N_THRD=8                # number of threads

in=$1
comFile="CRYFA_COMPRESSED"
decomFile="CRYFA_DECOMPRESSED"


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   get datasets, install dependencies, run cryfa, plot results
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
GET_HUMAN_FA=0          # download Human choromosomes in FASTA
GET_VIRUSES_FA=0        # get Viruses in FASTA using downloadViruses.pl
GEN_SYNTH_DATASET=0     # generate synthetic dataset using XS
CRYFA_COMP=0            # cryfa -- compress
CRYFA_DECOMP=0          # cryfa -- decompress
CRYFA_COMPARE_COMP_DECOMP=0     # cryfa -- compare comp. & decomp. results
CRYFA_COMP_DECOMP_COMPARE=1     # cryfa: comp. + decomp. + compare results
#GET_CHIMPANZEE=0       # download Chimpanzee chrs and make SEQ out of FASTA
#GET_GORILLA=0          # download Gorilla chrs and make SEQ out of FASTA
#GET_CHICKEN=0          # download Chicken chrs and make SEQ out of FASTA
#GET_TURKEY=0           # download Turkey chrs and make SEQ out of FASTA
#GET_ARCHAEA=0          # get Archaea SEQ using "GOOSE" & downloadArchaea.pl
#GET_FUNGI=0            # get Fungi SEQ using "GOOSE" & downloadFungi.pl
#GET_BACTERIA=0         # get Bacteria SEQ using "GOOSE" & downloadBacteria.pl
#INSTALL_XS=0           # install "XS" from Github
#INSTALL_GOOSE=0        # install "GOOSE" from Github
#INSTALL_GULL=0         # install "GULL" from Github
#GEN_DATASET=0          # generate datasets using "XS"
#GEN_MUTATIONS=0        # generate mutations using "GOOSE"
#RUN_PHOENIX=0          # run Phoenix
#PLOT_RESULT=0          # plot results using "gnuplot"
#BUILD_MATRIX=0         # build matrix from datasets
#FILTER=0               # filter total & diff by threshold
#PLOT_AlCoB=0	        # plot matrix -- AlCoB conference
#PLOT_MATRIX=0          # plot matrix from datasets
#PLOT_MATRIX_ARCHEA=0   # plot matrix Archaea from datasets


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   folders
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
FLD_dataset="dataset"
FLD_XS="XS"
#FLD_chromosomes="chromosomes"
#FLD_GOOSE="goose"
#FLD_GULL="GULL"
#FLD_dat="dat"
#FLD_archaea="archaea"
#FLD_fungi="fungi"
#FLD_bacteria="bacteria"
#FLD_viruses="viruses"
#FLD_src="src"
#FLD_script="script"


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   URLs
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
HUMAN_URL="ftp://ftp.ncbi.nlm.nih.gov/genomes/H_sapiens/\
Assembled_chromosomes/seq"
#CHIMPANZEE_URL="ftp://ftp.ncbi.nlm.nih.gov/genomes/Pan_troglodytes/\
#Assembled_chromosomes/seq"
#GORILLA_URL="ftp://ftp.ncbi.nlm.nih.gov/genomes/Gorilla_gorilla/\
#Assembled_chromosomes/seq"
#CHICKEN_URL="ftp://ftp.ncbi.nlm.nih.gov/genomes/Gallus_gallus/\
#Assembled_chromosomes/seq"
#TURKEY_URL="ftp://ftp.ncbi.nlm.nih.gov/genomes/Meleagris_gallopavo/\
#Assembled_chromosomes/seq"


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   scientific names
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
HUMAN_SNAME="Homo sapiens"
#CHIMPANZEE_SNAME="Pan troglodytes"
#GORILLA_SNAME="Gorilla gorilla"
#CHICKEN_SNAME="Gallus gallus"
#TURKEY_SNAME="Meleagris gallopavo"
#ARCHAEA_SNAME="Archaea"
#FUNGI_SNAME="Fungi"
#BACTERIA_SNAME="Bacteria"
#VIRUSES_SNAME="Viruses"


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   abbreviated names
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
HUMAN="HS"
#CHIMPANZEE="PT"
#GORILLA="GG"
#CHICKEN="GGA"
#TURKEY="MGA"
#ARCHAEA="A"
#FUNGI="F"
#BACTERIA="B"
#VIRUSES="V"


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   sequence runs
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
HS_SEQ_RUN=`seq -s' ' 1 22`; HS_SEQ_RUN+=" X Y MT AL UL UP"
#PT_SEQ_RUN="1 2A 2B "; PT_SEQ_RUN+=`seq -s' ' 3 22`; PT_SEQ_RUN+=" X Y MT UL UP"
#GG_SEQ_RUN="1 2A 2B "; GG_SEQ_RUN+=`seq -s' ' 3 22`; GG_SEQ_RUN+=" X MT UL UP"
#GGA_SEQ_RUN=`seq -s' ' 1 28`; GGA_SEQ_RUN+=" ";
#    GGA_SEQ_RUN+=`seq -s' ' 30 33`; GGA_SEQ_RUN+=" LG MT W Z UL UP"
#MGA_SEQ_RUN=`seq -s' ' 1 30`; MGA_SEQ_RUN+=" MT W Z UL UP"
#A_SEQ_RUN=`seq -s' ' 1 206`
#F_SEQ_RUN=`seq -s' ' 1 235`
#B_SEQ_RUN=`seq -s' ' 1 3219`
#V_SEQ_RUN=`seq -s' ' 1 5687`


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   prefixes
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
HUMAN_CHR_PREFIX="hs_ref_GRCh38.p7_"
#CHIMPANZEE_CHR_PREFIX="ptr_ref_Pan_tro_3.0_"
#GORILLA_CHR_PREFIX="9595_ref_gorGor4_"
#CHICKEN_CHR_PREFIX="gga_ref_Gallus_gallus-5.0_"
#TURKEY_CHR_PREFIX="mga_ref_Turkey_5.0_"


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   file types and formats
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
FA_FTYPE="fa"     # fasta file type
FQ_FTYPE="fq"     # fastq file type
COMP_FTYPE="gz"   # compressed file type
INF_FTYPE="dat"   # information (data) file type
#INF_FTYPE="csv"  # information (data) file type

PIX_FORMAT=pdf    # output format: pdf, png, svg, eps, epslatex (set output x.y)


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   definitions
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
CHR="chr"

CURR_CHR="21"
chromosomes="$HUMAN_CHR_PREFIX$CHR$CURR_CHR"

HUMAN_CHROMOSOME="$HUMAN_CHR_PREFIX$CHR"
#CHIMPANZEE_CHROMOSOME="$CHIMPANZEE_CHR_PREFIX$CHR"
#GORILLA_CHROMOSOME="$GORILLA_CHR_PREFIX$CHR"
#CHICKEN_CHROMOSOME="$CHICKEN_CHR_PREFIX$CHR"
#TURKEY_CHROMOSOME="$TURKEY_CHR_PREFIX$CHR"


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   create folders, if they don't already exist
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [ ! -d $FLD_dataset     ]; then mkdir -p $FLD_dataset;     fi
#if [ ! -d $FLD_chromosomes ]; then mkdir -p $FLD_chromosomes; fi
#if [ ! -d $FLD_dat         ]; then mkdir -p $FLD_dat;         fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   get human FA
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GET_HUMAN_FA -eq 1 ]]; then

    ### download FASTA
    for i in {1..22} X Y MT; do
        wget $HUMAN_URL/$HUMAN_CHROMOSOME$i.$FA_FTYPE.$COMP_FTYPE;
        gunzip < $HUMAN_CHROMOSOME$i.$FA_FTYPE.$COMP_FTYPE \
               > $FLD_dataset/$HUMAN$i.$FA_FTYPE;
        rm $HUMAN_CHROMOSOME$i.$FA_FTYPE.$COMP_FTYPE
    done

    for i in alts unplaced unlocalized; do
        wget $HUMAN_URL/$HUMAN_CHR_PREFIX$i.$FA_FTYPE.$COMP_FTYPE;
        gunzip < $HUMAN_CHR_PREFIX$i.$FA_FTYPE.$COMP_FTYPE \
               > $FLD_dataset/$HUMAN$i.$FA_FTYPE;
        rm $HUMAN_CHR_PREFIX$i.$FA_FTYPE.$COMP_FTYPE
    done

    ### rename: HSalts -> HSAL, HSunplaced -> HSUP, HSunlocalized -> HSUL
    mv $FLD_dataset/$HUMAN"alts".$FA_FTYPE     $FLD_dataset/$HUMAN"AL".$FA_FTYPE
    mv $FLD_dataset/$HUMAN"unplaced".$FA_FTYPE $FLD_dataset/$HUMAN"UP".$FA_FTYPE
    mv $FLD_dataset/$HUMAN"unlocalized".$FA_FTYPE \
           $FLD_dataset/$HUMAN"UL".$FA_FTYPE
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   get viruses FA
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GET_VIRUSES_FA -eq 1 ]]; then
    perl ./scripts/DownloadViruses.pl
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   generate synthetic dataset
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $GEN_SYNTH_DATASET -eq 1 ]]; then

    ### install XS
#    rm -fr $FLD_XS
#    git clone https://github.com/pratas/XS.git
#    cd $FLD_XS
#    make
#    cd ..

    ### generate dataset -- 1000000 lines = 100 MB
    XS/XS -ls 100 -n 1000000 -rn 0 -f 0.20,0.20,0.20,0.20,0.20 \
                                                              -eh -eo -es dtstXS
    echo ">X" > HEADER    # add ">X" as header of the sequence (build "nonRepX")
    cat HEADER dtstXS > synth_dataset
    rm -f HEADER dtstXS
fi



#if [[ $GET_CHIMPANZEE -eq 1 ]];    then . $FLD_script/get-chimpanzee.sh;      fi
#if [[ $GET_GORILLA    -eq 1 ]];    then . $FLD_script/get-gorilla.sh;         fi
#if [[ $GET_CHICKEN    -eq 1 ]];    then . $FLD_script/get-chicken.sh;         fi
#if [[ $GET_TURKEY     -eq 1 ]];    then . $FLD_script/get-turkey.sh;          fi
#if [[ $GET_ARCHAEA    -eq 1 ]];    then . $FLD_script/get-archaea.sh;         fi
#if [[ $GET_FUNGI      -eq 1 ]];    then . $FLD_script/get-fungi.sh;           fi
#if [[ $GET_BACTERIA   -eq 1 ]];    then . $FLD_script/get-bacteria.sh;        fi
#if [[ $INSTALL_XS     -eq 1 ]];    then . $FLD_script/install-XS.sh;          fi
#if [[ $INSTALL_GOOSE  -eq 1 ]];    then . $FLD_script/install-GOOSE.sh;       fi
#if [[ $INSTALL_GULL   -eq 1 ]];    then . $FLD_script/install-GULL.sh;        fi
#if [[ $GEN_DATASET    -eq 1 ]];    then . $FLD_script/generate-dataset.sh;    fi
#if [[ $GEN_MUTATIONS  -eq 1 ]];    then . $FLD_script/generate-mutation.sh;   fi
#if [[ $RUN_PHOENIX    -eq 1 ]];    then . $FLD_script/run-phoenix.sh;         fi
#if [[ $PLOT_RESULT    -eq 1 ]];    then . $FLD_script/plot-result.sh;         fi
#if [[ $BUILD_MATRIX   -eq 1 ]];    then . $FLD_script/build-matrix.sh;        fi
#if [[ $FILTER         -eq 1 ]];    then . $FLD_script/filter.sh;              fi
#if [[ $PLOT_AlCoB     -eq 1 ]];    then . $FLD_script/plot--AlCoB.sh;         fi
#if [[ $PLOT_MATRIX    -eq 1 ]];    then . $FLD_script/plot-matrix.sh;         fi
#if [[ $PLOT_MATRIX_ARCHEA -eq 1 ]];then . $FLD_script/plot-matrix-archaea.sh; fi

########################
#cd $FLD_script


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   run cryfa -- compression
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CRYFA_COMP -eq 1 ]]; then
    cmake .
    make

    ./cryfa -t $N_THRD -k pass.txt $in > $comFile        # -s: disable shuffling
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   run cryfa -- decompression
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CRYFA_DECOMP -eq 1 ]]; then
    cmake .
    make

    ./cryfa -t $N_THRD -dk pass.txt $comFile > $decomFile #-s: disable shuffling
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   compare comp. & decomp. results of running cryfa
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CRYFA_COMPARE_COMP_DECOMP -eq 1 ]]; then
    cmp $in $decomFile
fi


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   run cryfa -- compression + decompression + compare results
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if [[ $CRYFA_COMP_DECOMP_COMPARE -eq 1 ]]; then
    cmake .
    make

    ./cryfa -t $N_THRD -k pass.txt $in       > $comFile   #-s: disable shuffling
    ./cryfa -t $N_THRD -dk pass.txt $comFile > $decomFile #-s: disable shuffling
    cmp $in $decomFile
fi