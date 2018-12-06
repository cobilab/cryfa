          #######################################################
          #            Compression+encryption results           #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Functions
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
### Results of compression+encryption and decryption+decompression.
### $1: compression program's name, $2: encryption program's name, $3: dataset.
function compEncDecDecompRes
{
    in="${3##*/}"                      # Dataset name
    dName="${in%.*}"                   # Dataset name without filetype
    ft="${in##*.}"                     # Dataset filetype
    fsize=$(stat --printf="%s" $3)      # File size (bytes)
    methodComp=$(printMethodName $1)    # Compression methods' name for printing
    methodEnc=$(printMethodName $2)     # Encryption methods' name for printing

    CS="";       CT_r="";     CT_u="";     CT_s="";     CM="";
    EnS="";      EnT_r="";    EnT_u="";    EnT_s="";    EnM="";
    DeT_r="";    DeT_u="";    DeT_s="";    DeM="";
    DT_r="";     DT_u="";     DT_s="";     DM="";       V="";

    ### Compressed file size
    cs_file="$result/${1}_${2}_CS__${dName}_$ft"
    if [[ -e $cs_file ]]; then  CS=$(awk '{ print $5; }') $cs_file;  fi

    ### Compression time -- real - user - system
    ct_file="$result/${1}_${2}_CT__${dName}_$ft"
    if [[ -e $ct_file ]]; then
        CT_r=$(< $ct_file tail -n 3 | head -n 1 | awk '{ print $2;}');
        CT_u=$(< $ct_file tail -n 2 | head -n 1 | awk '{ print $2;}');
        CT_s=$(< $ct_file tail -n 1 | awk '{ print $2;}');
    fi

    ### Compression memory
    cm_file="$result/${1}_${2}_CM__${dName}_$ft"
    if [[ -e $cm_file ]]; then  CM=$(cat $cm_file);  fi

    ### Encrypted file size
    ens_file="$result/${1}_${2}_EnS__${dName}_$ft"
    if [[ -e $ens_file ]]; then  EnS=$(awk '{ print $5; }' $ens_file);  fi

    ### Encryption time -- real - user - system
    ent_file="$result/${1}_${2}_EnT__${dName}_$ft"
    if [[ -e $ent_file ]]; then
        EnT_r=$(< $ent_file tail -n 3 | head -n 1 | awk '{ print $2;}');
        EnT_u=$(< $ent_file tail -n 2 | head -n 1 | awk '{ print $2;}');
        EnT_s=$(< $ent_file tail -n 1 | awk '{ print $2;}');
    fi

    ### Encryption memory
    enm_file="$result/${1}_${2}_EnM__${dName}_$ft"
    if [[ -e $enm_file ]]; then  EnM=$(cat $enm_file);  fi

    ### Decryption time -- real - user - system
    det_file="$result/${1}_${2}_DeT__${dName}_$ft"
    if [[ -e $det_file ]]; then
        DeT_r=$(< $det_file tail -n 3 | head -n 1 | awk '{ print $2;}');
        DeT_u=$(< $det_file tail -n 2 | head -n 1 | awk '{ print $2;}');
        DeT_s=$(< $det_file tail -n 1 | awk '{ print $2;}');
    fi

    ### Decryption memory
    dem_file="$result/${1}_${2}_DeM__${dName}_$ft"
    if [[ -e $dem_file ]]; then  DeM=$(cat $dem_file);  fi

    ### Decompression time -- real - user - system
    dt_file="$result/${1}_${2}_DT__${dName}_$ft"
    if [[ -e $dt_file ]]; then
        DT_r=$(< $dt_file tail -n 3 | head -n 1 | awk '{ print $2;}');
        DT_u=$(< $dt_file tail -n 2 | head -n 1 | awk '{ print $2;}');
        DT_s=$(< $dt_file tail -n 1 | awk '{ print $2;}');
    fi

    ### Decompression memory
    dm_file="$result/${1}_${2}_DM__${dName}_$ft"
    if [[ -e $dm_file ]]; then  DM=$(cat $dm_file);  fi

    ### Verify if the decompressed and the original files are equal
    v_file="$result/${1}_${2}_V__${dName}_$ft"
    if [[ -e $v_file ]]; then  V=$(wc -l $v_file);  fi

    ### Remove extra files
    for xf in $cs_file $cm_file $dem_file $dm_file $enm_file $ens_file; do
        if [[ -e $xf ]]; then  rm -f $xf;  fi
    done
    if [[ $V -eq 0 ]]; then  rm -f $v_file;  fi

    ### Move possible informative files to details folder
    # Create a folder for details, if it doesn't already exist
    if [[ ! -d $details ]]; then  mkdir -p $result/$details;  fi
    # Move
    for pif in $ct_file $det_file $dt_file $ent_file $v_file; do
        if [[ -e $pif ]]; then  mv $pif  $result/$details;  fi
    done

    ### Remove extra files
    for xf in $cs_file $cm_file $dem_file $dm_file $enm_file $ens_file; do
        rm -f $xf;
    done
    if [[ $V -eq 0 ]]; then  rm -f $v_file;  fi

    ### Print results
    c="$CS\t$CT_r\t$CT_u\t$CT_s\t$CM"          # Compression
    en="$EnS\t$EnT_r\t$EnT_u\t$EnT_s\t$EnM"    # Encryption
    de="$DeT_r\t$DeT_u\t$DeT_s\t$DeM"          # Decryption
    d="$DT_r\t$DT_u\t$DT_s\t$DM"               # Decompression
    printf "$dName\t$fsize\t$methodComp\t$methodEnc\t$c\t$en\t$de\t$d\t$V\n";
}


### Results of compression+encryption and decryption+decompression on datasets.
### $1: output file
function compEncDecDecompResOnDataset
{
    FAdsPath=$dataset/$FA
    FQdsPath=$dataset/$FQ
    OUT_FILE=$1
    methods="C_Method\tEn_Method"
    c="C_Size(B)\tC_Time_real(s)\tC_Time_user(s)\tC_Time_sys(s)\tC_Mem(KB)"
    en="En_Size(B)\tEn_Time_real(s)\tEn_Time_user(s)\tEn_Time_sys(s)"
    en+="\tEn_Mem(KB)"
    de="De_Time_real(s)\tDe_Time_user(s)\tDe_Time_sys(s)\tDe_Mem(KB)"
    d="D_Time_real(s)\tD_Time_user(s)\tD_Time_sys(s)\tD_Mem(KB)"

    printf "Dataset\tSize(B)\t$methods\t$c\t$en\t$de\t$d\tEq\n" > $OUT_FILE;

    for i in $ENC_METHODS; do
       ### FASTA -- human - viruses - synthetic
       for j in $FASTA_METHODS; do
           compEncDecDecompRes $j $i $FAdsPath/$HUMAN/HS.$fasta >> $OUT_FILE;
           compEncDecDecompRes $j $i \
                               $FAdsPath/$VIRUSES/viruses.$fasta >> $OUT_FILE;
           for k in 1 2; do
               compEncDecDecompRes $j $i \
                               $FAdsPath/$Synth/SynFA-${k}.$fasta >> $OUT_FILE;
           done
       done

       ### FASTQ -- human - Denisova - synthetic
       for j in $FASTQ_METHODS; do
           for k in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 \
                    SRR707196_1; do
               compEncDecDecompRes $j $i \
                             $FQdsPath/$HUMAN/HS-${k}.$fastq >> $OUT_FILE;
           done
           for k in B1087 B1088; do
               compEncDecDecompRes $j $i \
                             $FQdsPath/$DENISOVA/DS-${k}_SR.$fastq >> $OUT_FILE;
           done
           for k in 1 2; do
               compEncDecDecompRes $j $i \
                             $FQdsPath/$Synth/SynFQ-${k}.$fastq >> $OUT_FILE;
           done
       done
    done
}


### Convert memory numbers scale to MB and times to fractional minutes in
### result files. $1: input file name
function compEncResHumanReadable
{
    IN=$1              # Input file name
    INWF="${IN%.*}"    # Input file name without filetype
    cen="CEn_Method\tC_Ratio\tC_Size(MB)\tCEn_Time_real(m)\tCEn_Time_cpu(m)"
    ded="DeD_Time_real(m)\tDeD_Time_cpu(m)"

    printf "Dataset\tSize(MB)\t$cen\tCEn_Mem(MB)\t$ded\tDeD_Mem(MB)\tEq\n" \
        > $INWF.tmp
    awk 'NR>1' $IN | tr ',' . | awk 'BEGIN {}{
      printf "%s\t%.f\t%s+%s\t%.1f\t%.f",
             $1, $2/(1024*1024), $3, $4, $2/$5, $10/(1024*1024);

      split($6, c_arrMinReal, "m");                 c_minReal=c_arrMinReal[1];
      split(c_arrMinReal[2], c_arrSecReal, "s");    c_secReal=c_arrSecReal[1];
      c_realTime=c_minReal*60+c_secReal;
      split($11, en_arrMinReal, "m");               en_minReal=en_arrMinReal[1];
      split(en_arrMinReal[2], en_arrSecReal, "s");  en_secReal=en_arrSecReal[1];
      en_realTime=en_minReal*60+en_secReal;
      cen_realTime=c_realTime+en_realTime;

      printf "\t%.1f", cen_realTime/60;

      split($7, c_arrMinUser, "m");                 c_minUser=c_arrMinUser[1];
      split(c_arrMinUser[2], c_arrSecUser, "s");    c_secUser=c_arrSecUser[1];
      c_userTime=c_minUser*60+c_secUser;
      split($8, c_arrMinSys, "m");                  c_minSys=c_arrMinSys[1];
      split(c_arrMinSys[2], c_arrSecSys, "s");      c_secSys=c_arrSecSys[1];
      c_sysTime=c_minSys*60+c_secSys;
      c_cpuTime=c_userTime+c_sysTime;
      split($12, en_arrMinUser, "m");               en_minUser=en_arrMinUser[1];
      split(en_arrMinUser[2], en_arrSecUser, "s");  en_secUser=en_arrSecUser[1];
      en_userTime=en_minUser*60+en_secUser;
      split($13, en_arrMinSys, "m");                en_minSys=en_arrMinSys[1];
      split(en_arrMinSys[2], en_arrSecSys, "s");    en_secSys=en_arrSecSys[1];
      en_sysTime=en_minSys*60+en_secSys;
      en_cpuTime=en_userTime+en_sysTime;
      cen_cpuTime=c_cpuTime+en_cpuTime;

      printf "\t%.1f", cen_cpuTime/60;

      max_cen_mem=($9 > $14 ? $9 : $14);
      cen_mem=max_cen_mem;

      printf "\t%.f", cen_mem/1024;

      split($15, de_arrMinReal, "m");               de_minReal=de_arrMinReal[1];
      split(de_arrMinReal[2], de_arrSecReal, "s");  de_secReal=de_arrSecReal[1];
      de_realTime=de_minReal*60+de_secReal;
      split($19, d_arrMinReal, "m");                d_minReal=d_arrMinReal[1];
      split(d_arrMinReal[2], d_arrSecReal, "s");    d_secReal=d_arrSecReal[1];
      d_realTime=d_minReal*60+d_secReal;
      ded_realTime=de_realTime+d_realTime;

      printf "\t%.1f", ded_realTime/60;

      split($16, de_arrMinUser, "m");               de_minUser=de_arrMinUser[1];
      split(de_arrMinUser[2], de_arrSecUser, "s");  de_secUser=de_arrSecUser[1];
      de_userTime=de_minUser*60+de_secUser;
      split($17, de_arrMinSys, "m");                de_minSys=de_arrMinSys[1];
      split(de_arrMinSys[2], de_arrSecSys, "s");    de_secSys=de_arrSecSys[1];
      de_sysTime=de_minSys*60+de_secSys;
      de_cpuTime=de_userTime+de_sysTime;
      split($20, d_arrMinUser, "m");                d_minUser=d_arrMinUser[1];
      split(d_arrMinUser[2], d_arrSecUser, "s");    d_secUser=d_arrSecUser[1];
      d_userTime=d_minUser*60+d_secUser;
      split($21, d_arrMinSys, "m");                 d_minSys=d_arrMinSys[1];
      split(d_arrMinSys[2], d_arrSecSys, "s");      d_secSys=d_arrSecSys[1];
      d_sysTime=d_minSys*60+d_secSys;
      d_cpuTime=d_userTime+d_sysTime;
      ded_cpuTime=de_cpuTime+d_cpuTime;

      printf "\t%.1f", ded_cpuTime/60;

      max_ded_mem=($18 > $22 ? $18 : $22);
      ded_mem=max_ded_mem;

      printf "\t%.f\t%d\n", ded_mem/1024, $23;
    }' >> $INWF.tmp

    ### FASTA
    # Details -- 1 row for headers and 1 row after all
    removeFromRow=$(echo $((FASTA_DATASET_SIZE*FASTA_METHODS_SIZE+1+1)))
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
    printf "Size(MB)\t$cen\t$ded\tEq\n" > ${INWF}_tot_FA.$INF;
    < ${INWF}_FA.$INF tr ',' '.' | awk 'NR>1' \
      | awk -v dsSize=$FASTA_DATASET_SIZE 'BEGIN{}{
      s+=$2;  cenS+=$5;  cenTR+=$6;  cenTC+=$7;  dedTR+=$9; dedTC+=$10; eq+=$12;
      if (NR % dsSize==0) {
          printf "%.f\t%s\t%.1f\t%.f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n",
                 s, $3, s/cenS, cenS, cenTR, cenTC, dedTR, dedTC, eq;
          s=0;cenS=0;    cenTR=0;    cenTC=0;    dedTR=0;   dedTC=0;    eq=0;
      }
    }' >> ${INWF}_tot_FA.$INF
    # Extract from Cryfa
    awk 'NR>1' $result/COMP_tot_FA.$INF \
      | awk 'BEGIN{}{if ($2=="Cryfa") print;}' >> ${INWF}_tot_FA.$INF

    ### FASTQ
    # Details -- 1 row for headers and 1 row after all
    removeUpToRow=$(echo $((removeFromRow-1)))
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
    printf "Size(MB)\t$cen\t$ded\tEq\n" > ${INWF}_tot_FQ.$INF;
    < ${INWF}_FQ.$INF tr ',' '.' | awk 'NR>1' \
      | awk -v dsSize=$FASTQ_DATASET_SIZE 'BEGIN{}{
      s+=$2;  cenS+=$5;  cenTR+=$6;  cenTC+=$7;  dedTR+=$9; dedTC+=$10; eq+=$12;
      if (NR % dsSize==0) {
          printf "%.f\t%s\t%.1f\t%.f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n",
                 s, $3, s/cenS, cenS, cenTR, cenTC, dedTR, dedTC, eq;
          s=0;cenS=0;    cenTR=0;    cenTC=0;    dedTR=0;   dedTC=0;    eq=0;
      }
    }' >> ${INWF}_tot_FQ.$INF
    # Extract from Cryfa
    awk 'NR>1' $result/COMP_tot_FQ.$INF \
      | awk 'BEGIN{}{if ($2=="Cryfa") print;}' >> ${INWF}_tot_FQ.$INF

    rm -f $INWF.tmp
}


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Print compression+encryption results
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
OUT="$result/COMP_ENC.$RES"    # Output file name

### Results on datasets
compEncDecDecompResOnDataset $OUT;

### Make the result file human readable
compEncResHumanReadable $OUT;