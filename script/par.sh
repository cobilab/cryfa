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
VCF="VCF"
SAM="SAM"
BAM="BAM"
XS="XS"
goose="goose"


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   URLs
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
HUMAN_FA_URL="ftp://ftp.ncbi.nlm.nih.gov/genomes/H_sapiens/\
Assembled_chromosomes/seq"
HUMAN_FQ_URL="ftp://ftp.sra.ebi.ac.uk/vol1/fastq"
DENISOVA_FQ_URL="http://cdna.eva.mpg.de/denisova/raw_reads"
DENISOVA_VCF_URL="http://cdna.eva.mpg.de/denisova/VCF/human"
NEANDERTHAL_VCF_URL="http://cdna.eva.mpg.de/neandertal/altai/ModernHumans/vcf"
HUMAN_SAM_URL="ftp://ftp.1000genomes.ebi.ac.uk/vol1/ftp/phase3/data/HG00096/\
alignment"
NEANDERTHAL_SAM_URL="http://cdna.eva.mpg.de/neandertal/altai/AltaiNeandertal/\
bam"
HUMAN_BAM_URL="ftp://ftp.1000genomes.ebi.ac.uk/vol1/ftp/phase3/data/HG00096/\
alignment"
NEANDERTHAL_BAM_URL="http://cdna.eva.mpg.de/neandertal/Vindija/bam"


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
NEANDERTHAL="N"


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Definitions
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
CRYFA_DEFAULT_N_THR=8
CRYFA_KEY_FILE="pass.txt"

CHR="chr"
HUMAN_CHR_PREFIX="hs_ref_GRCh38.p7_"
HUMAN_CHROMOSOME="$HUMAN_CHR_PREFIX$CHR"

HS_SEQ_RUN=$(seq -s' ' 1 22); HS_SEQ_RUN+=" X Y MT AL UL UP"

WGET_OP=" --trust-server-names "

INF="dat"        # Information (data) file type
RES="res"        # Result file type
fasta="fasta"    # FASTA file extension
fastq="fastq"    # FASTQ file extension
vcf="vcf"        # VCF   file extension
sam="sam"        # SAM   file extension
bam="bam"        # BAM   file extension

FASTA_METHODS="GZIP BZIP2 MFCOMPRESS DELIM"
FASTQ_METHODS="GZIP BZIP2 FQZCOMP QUIP DSRC FQC"
VCF_METHODS="AESCRYPT"
SAM_METHODS="AESCRYPT"
BAM_METHODS="AESCRYPT"
ENC_METHODS="AESCRYPT"

FASTA_DATASET="HS viruses SynFA-1 SynFA-2"
FASTQ_DATASET="HS-ERR013103_1 HS-ERR015767_2 HS-ERR031905_2 HS-SRR442469_1"
    FASTQ_DATASET+=" HS-SRR707196_1 DS-B1087_SR DS-B1088_SR SynFQ-1 SynFQ-2"
VCF_DATASET="DS-22 N-n"
SAM_DATASET="HS-n N-y"
BAM_DATASET="HS-11 N-21"

FASTA_DATASET_SIZE=$(echo $FASTA_DATASET | wc -w)
FASTQ_DATASET_SIZE=$(echo $FASTQ_DATASET | wc -w)
VCF_DATASET_SIZE=$(echo $VCF_DATASET | wc -w)
SAM_DATASET_SIZE=$(echo $SAM_DATASET | wc -w)
BAM_DATASET_SIZE=$(echo $BAM_DATASET | wc -w)
FASTA_METHODS_SIZE=$(echo $FASTA_METHODS | wc -w)    # Except Cryfa
FASTQ_METHODS_SIZE=$(echo $FASTQ_METHODS | wc -w)    # Except Cryfa
VCF_METHODS_SIZE=$(echo $VCF_METHODS | wc -w)        # Except Cryfa
SAM_METHODS_SIZE=$(echo $SAM_METHODS | wc -w)        # Except Cryfa
BAM_METHODS_SIZE=$(echo $BAM_METHODS | wc -w)        # Except Cryfa
ENC_METHODS_SIZE=$(echo $ENC_METHODS | wc -w)