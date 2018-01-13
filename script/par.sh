          #######################################################
          #                     Parameters                      #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Folders
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
script="script"
dataset="dataset"
    redun="Redundancy"
progs="progs"
result="result"
    details="details"
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
#   Abbreviated names
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
HUMAN="HS"
ARCHAEA="A"
BACTERIA="B"
FUNGI="F"
PLANTS="P"
VIRUSES="V"
DENISOVA="DS"
Synth="Synth"


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Definitions
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
CRYFA_DEFAULT_N_THR=8
CRYFA_KEY_FILE="pass.txt"

CHR="chr"
HUMAN_CHR_PREFIX="hs_ref_GRCh38.p7_"
HUMAN_CHROMOSOME="$HUMAN_CHR_PREFIX$CHR"

HS_SEQ_RUN=`seq -s' ' 1 22`; HS_SEQ_RUN+=" X Y MT AL UL UP"

WGET_OP=" --trust-server-names "

INF="dat"        # Information (data) file type
RES="res"        # Result file type
fasta="fasta"    # FASTA file extension
fastq="fastq"    # FASTQ file extension

FASTA_METHODS="GZIP BZIP2 MFCOMPRESS DELIM"
FASTQ_METHODS="GZIP BZIP2 FQZCOMP QUIP DSRC FQC"
ENC_METHODS="AESCRYPT"

FASTA_DATASET="HS viruses SynFA-1 SynFA-2"
FASTQ_DATASET="HS-ERR013103_1 HS-ERR015767_2 HS-ERR031905_2 HS-SRR442469_1"
    FASTQ_DATASET+=" HS-SRR707196_1 DS-B1087_SR DS-B1088_SR SynFQ-1 SynFQ-2"

FASTA_DATASET_SIZE=`echo $FASTA_DATASET | wc -w`
FASTQ_DATASET_SIZE=`echo $FASTQ_DATASET | wc -w`
FASTA_METHODS_SIZE=`echo $FASTA_METHODS | wc -w`    # Except cryfa
FASTQ_METHODS_SIZE=`echo $FASTQ_METHODS | wc -w`    # Except cryfa
ENC_METHODS_SIZE=`echo $ENC_METHODS | wc -w`