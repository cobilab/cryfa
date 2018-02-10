          #######################################################
          #                  Encryption results                 #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Functions
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
### Results of encrypt/decrypt. $1: program's name, $2: dataset
function encDecRes
{
    in="${2##*/}"                    # Dataset name
    dName="${in%.*}"                 # Dataset name without filetype
    ft="${in##*.}"                   # Dataset filetype
    fsize=`stat --printf="%s" $2`    # File size (bytes)
    method=`printMethodName $1`      # Methods' name for printing

    EnS="";      EnT_r="";    EnT_u="";    EnT_s="";    EnM="";
    DeT_r="";    DeT_u="";    DeT_s="";    DeM="";

    ### Encrypted file size
    ens_file="$result/${1}_EnS__${dName}_$ft"
    if [[ -e $ens_file ]]; then  EnS=`cat $ens_file | awk '{ print $5; }'`;  fi

    ### Encryption time -- real - user - system
    ent_file="$result/${1}_EnT__${dName}_$ft"
    if [[ -e $ent_file ]]; then
        EnT_r=`cat $ent_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
        EnT_u=`cat $ent_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
        EnT_s=`cat $ent_file | tail -n 1 | awk '{ print $2;}'`;
    fi

    ### Encryption memory
    enm_file="$result/${1}_EnM__${dName}_$ft"
    if [[ -e $enm_file ]]; then  EnM=`cat $enm_file`;  fi

    ### Decryption time -- real - user - system
    det_file="$result/${1}_DeT__${dName}_$ft"
    if [[ -e $det_file ]]; then
        DeT_r=`cat $det_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
        DeT_u=`cat $det_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
        DeT_s=`cat $det_file | tail -n 1 | awk '{ print $2;}'`;
    fi

    ### Decryption memory
    dem_file="$result/${1}_DeM__${dName}_$ft"
    if [[ -e $dem_file ]]; then  DeM=`cat $dem_file`;  fi

    ### Remove extra files
    for xf in $ens_file $enm_file $dem_file; do
        if [[ -e $xf ]]; then  rm -f $xf;  fi
    done

    ### Move possible informative files to details folder
    # Create a folder for details, if it doesn't already exist
    if [[ ! -d $details ]]; then  mkdir -p $result/$details;  fi
    # Move
    for pif in $ent_file $det_file; do
        if [[ -e $pif ]]; then  mv $pif  $result/$details;  fi
    done

    ### Print results
    en="$EnS\t$EnT_r\t$EnT_u\t$EnT_s\t$EnM"    # Encryption
    de="$DeT_r\t$DeT_u\t$DeT_s\t$DeM"          # Decryption
    printf "$dName\t$fsize\t$method\t$en\t$de\n";
}


### Results of encrypt/decrypt on datasets. $1: output file
function encResOnDataset
{
    FAdsPath=$dataset/$FA
    FQdsPath=$dataset/$FQ
    OUT_FILE=$1
    en="Method\tEn_Size(B)\tEn_Time_real(s)\tEn_Time_user(s)\tEn_Time_sys(s)"
    en+="\tEn_Mem(KB)"
    de="De_Time_real(s)\tDe_Time_user(s)\tDe_Time_sys(s)\tDe_Mem(KB)"

    printf "Dataset\tSize(B)\t$en\t$de\n" > $OUT_FILE;

    for i in $ENC_METHODS; do
        ### FASTA -- human - viruses - synthetic
        encDecRes $i $FAdsPath/$HUMAN/HS.$fasta >> $OUT_FILE;
        encDecRes $i $FAdsPath/$VIRUSES/viruses.$fasta >> $OUT_FILE;
        for j in 1 2; do
            encDecRes $i $FAdsPath/$Synth/SynFA-${j}.$fasta >> $OUT_FILE;
        done

        ### FASTQ -- human - Denisova - synthetic
        for j in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 SRR707196_1; do
            encDecRes $i $FQdsPath/$HUMAN/HS-${j}.$fastq >> $OUT_FILE;
        done
        for j in B1087 B1088; do
            encDecRes $i $FQdsPath/$DENISOVA/DS-${j}_SR.$fastq >> $OUT_FILE;
        done
        for j in 1 2; do
            encDecRes $i $FQdsPath/$Synth/SynFQ-${j}.$fastq >> $OUT_FILE;
        done
    done
}


### Convert memory numbers scale to MB and times to fractional minutes in
### result files. $1: input file name
function encResHumanReadable
{
    IN=$1              # Input file name
    INWF="${IN%.*}"    # Input file name without filetype
    en="Method\tC_Ratio\tEn_Size(MB)\tEn_Time_real(m)\tEn_Time_cpu(m)"
    de="De_Time_real(m)\tDe_Time_cpu(m)"

    printf "Dataset\tSize(MB)\t$en\tEn_Mem(MB)\t$de\tDe_Mem(MB)\tEq\n" \
        > $INWF.tmp
    cat $IN | awk 'NR>1' | tr ',' . | awk 'BEGIN {}{
      printf "%s\t%.f\t%s\t%.1f\t%.f",
             $1, $2/(1024*1024), $3, $2/$4, $4/(1024*1024);

      split($5, en_arrMinReal, "m");                en_minReal=en_arrMinReal[1];
      split(en_arrMinReal[2], en_arrSecReal, "s");  en_secReal=en_arrSecReal[1];
      en_realTime=en_minReal*60+en_secReal;

      printf "\t%.1f", en_realTime/60;

      split($6, en_arrMinUser, "m");                en_minUser=en_arrMinUser[1];
      split(en_arrMinUser[2], en_arrSecUser, "s");  en_secUser=en_arrSecUser[1];
      en_userTime=en_minUser*60+en_secUser;
      split($7, en_arrMinSys, "m");                 en_minSys=en_arrMinSys[1];
      split(en_arrMinSys[2], en_arrSecSys, "s");    en_secSys=en_arrSecSys[1];
      en_sysTime=en_minSys*60+en_secSys;
      en_cpuTime=en_userTime+en_sysTime;

      printf "\t%.1f\t%.f", en_cpuTime/60, $8/1024;

      split($9, de_arrMinReal, "m");                de_minReal=de_arrMinReal[1];
      split(de_arrMinReal[2], de_arrSecReal, "s");  de_secReal=de_arrSecReal[1];
      de_realTime=de_minReal*60+de_secReal;

      printf "\t%.1f", de_realTime/60;

      split($10, de_arrMinUser, "m");               de_minUser=de_arrMinUser[1];
      split(de_arrMinUser[2], de_arrSecUser, "s");  de_secUser=de_arrSecUser[1];
      de_userTime=de_minUser*60+de_secUser;
      split($11, de_arrMinSys, "m");                de_minSys=de_arrMinSys[1];
      split(de_arrMinSys[2], de_arrSecSys, "s");    de_secSys=de_arrSecSys[1];
      de_sysTime=de_minSys*60+de_secSys;
      de_cpuTime=de_userTime+de_sysTime;

      printf "\t%.1f\t%.f\t%d\n", de_cpuTime/60, $12/1024, $13;
    }' >> $INWF.tmp

    ### FASTA
    # Details -- 1 row for headers and 1 row after all
    removeFromRow=`echo $((FASTA_DATASET_SIZE*ENC_METHODS_SIZE+1+1))`
    sed "$removeFromRow,$ d" $INWF.tmp > ${INWF}_FA.$INF;

#    # For each dataset
#    for i in $FASTA_DATASET; do
#        sed "2,$ d" ${INWF}_FA.$INF > ${INWF}_${i}_FA.$INF;
#        cat ${INWF}_FA.$INF | awk 'NR>1' \
#          | awk -v i=$i 'BEGIN{}{if ($1==i) print;}' >> ${INWF}_${i}_FA.$INF;
#
#        # Extract from Cryfa
#        cat $result/COMP_${i}_FA.$INF | awk 'NR>1' \
#          | awk 'BEGIN{}{if ($3=="Cryfa") print;}' >> ${INWF}_${i}_FA.$INF;
#    done

    # Total
    printf "Size(MB)\t$en\t$de\tEq\n" > ${INWF}_tot_FA.$INF;
    cat ${INWF}_FA.$INF | tr ',' '.' | awk 'NR>1' \
      | awk -v dsSize=$FASTA_DATASET_SIZE 'BEGIN{}{
      s+=$2;   cS+=$5;   cTR+=$6;   cTC+=$7;   dTR+=$9;   dTC+=$10;   eq+=$12;
      if (NR % dsSize==0) {
          printf "%.f\t%s\t%.1f\t%.f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n",
                 s, $3, s/cS, cS, cTR, cTC, dTR, dTC, eq;
          s=0; cS=0;     cTR=0;     cTC=0;     dTR=0;     dTC=0;      eq=0;
      }
    }' >> ${INWF}_tot_FA.$INF
    # Extract from Cryfa
    cat $result/COMP_tot_FA.$INF | awk 'NR>1' \
      | awk 'BEGIN{}{if ($2=="Cryfa") print;}' >> ${INWF}_tot_FA.$INF

    ### FASTQ
    # Details -- 1 row for headers and 1 row after all
    removeUpToRow=`echo $((removeFromRow-1))`
    sed "2,$removeUpToRow d" $INWF.tmp > ${INWF}_FQ.$INF;

#    # For each dataset
#    for i in $FASTQ_DATASET; do
#        sed "2,$ d" ${INWF}_FQ.$INF > ${INWF}_${i}_FQ.$INF;
#        cat ${INWF}_FQ.$INF | awk 'NR>1' \
#          | awk -v i=$i 'BEGIN{}{if ($1==i) print;}' >> ${INWF}_${i}_FQ.$INF;
#
#        # Extract from Cryfa
#        cat $result/COMP_${i}_FQ.$INF | awk 'NR>1' \
#          | awk 'BEGIN{}{if ($3=="Cryfa") print;}' >> ${INWF}_${i}_FQ.$INF;
#    done

    # Total
    printf "Size(MB)\t$en\t$de\tEq\n" > ${INWF}_tot_FQ.$INF;
    cat ${INWF}_FQ.$INF | tr ',' '.' | awk 'NR>1' \
      | awk -v dsSize=$FASTQ_DATASET_SIZE 'BEGIN{}{
      s+=$2;   cS+=$5;   cTR+=$6;   cTC+=$7;   dTR+=$9;   dTC+=$10;   eq+=$12;
      if (NR % dsSize==0) {
          printf "%.f\t%s\t%.1f\t%.f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n",
                 s, $3, s/cS, cS, cTR, cTC, dTR, dTC, eq;
          s=0; cS=0;     cTR=0;     cTC=0;     dTR=0;     dTC=0;      eq=0;
      }
    }' >> ${INWF}_tot_FQ.$INF
    # Extract from Cryfa
    cat $result/COMP_tot_FQ.$INF | awk 'NR>1' \
      | awk 'BEGIN{}{if ($2=="Cryfa") print;}' >> ${INWF}_tot_FQ.$INF

    rm -f $INWF.tmp
}


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Print compression results
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
OUT="$result/ENC.$RES"    # Output file name

### Results on datasets
encResOnDataset $OUT;

### Make the result file human readable
encResHumanReadable $OUT;