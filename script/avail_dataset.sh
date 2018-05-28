          #######################################################
          #             Check dataset availabality              #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Functions
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
### Check if a file is available. $1: file name
function isAvail
{
    if [[ ! -e $1 ]]; then
        echo "Warning: The file \"$1\" is not available in" "\"$dataset/\" \
             directory.";
        return;
    fi
}


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Check availabality
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
### FASTA -- human - viruses - synthetic
isAvail "$dataset/$FA/$HUMAN/HS.$fasta";
isAvail "$dataset/$FA/$VIRUSES/viruses.$fasta"
for i in 1 2; do  isAvail "$dataset/$FA/$Synth/SynFA-$i.$fasta";  done

### FASTQ -- human - Denisova - synthetic
for i in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 SRR707196_1; do
    isAvail "$dataset/$FQ/$HUMAN/$HUMAN-$i.$fastq"
done
for i in B1087 B1088; do
    isAvail "$dataset/$FQ/$DENISOVA/$DENISOVA-${i}_SR.$fastq"
done
for i in 1 2; do  isAvail "$dataset/$FQ/$Synth/SynFQ-$i.$fastq";  done

### VCF -- Denisova - Neanderthal
isAvail "$dataset/$VCF/$DENISOVA/DS-22.$vcf";
isAvail "$dataset/$VCF/$NEANDERTHAL/N-n.$vcf";

### SAM -- human - Neanderthal
isAvail "$dataset/$SAM/$HUMAN/HS-n.$sam";
isAvail "$dataset/$SAM/$NEANDERTHAL/N-y.$sam";

### BAM -- human - Neanderthal
isAvail "$dataset/$BAM/$HUMAN/HS-11.$bam";
isAvail "$dataset/$BAM/$NEANDERTHAL/N-21.$bam";