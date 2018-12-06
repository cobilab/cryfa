          #######################################################
          #                 Compression results                 #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Functions
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
### Results of compress/decompress. $1: program's name, $2: dataset
function compDecompRes
{
    in="${2##*/}"                    # Dataset name
    dName="${in%.*}"                 # Dataset name without filetype
    ft="${in##*.}"                   # Dataset filetype
    fsize=$(stat --printf="%s" $2)    # File size (bytes)
    method=$(printMethodName $1)      # Methods' name for printing

    CS="";      CT_r="";    CT_u="";    CT_s="";    CM="";
    DT_r="";    DT_u="";    DT_s="";    DM="";      V="";

    ### Compressed file size
    cs_file="$result/${1}_CS__${dName}_$ft"
    if [[ -e $cs_file ]]; then  CS=$(awk '{ print $5; }' $cs_file);  fi

    ### Compression time -- real - user - system
    ct_file="$result/${1}_CT__${dName}_$ft"
    if [[ -e $ct_file ]]; then
        CT_r=$(< $ct_file tail -n 3 | head -n 1 | awk '{ print $2;}');
        CT_u=$(< $ct_file tail -n 2 | head -n 1 | awk '{ print $2;}');
        CT_s=$(< $ct_file tail -n 1 | awk '{ print $2;}');
    fi

    ### Compression memory
    cm_file="$result/${1}_CM__${dName}_$ft"
    if [[ -e $cm_file ]]; then  CM=$(cat $cm_file);  fi

    ### Decompression time -- real - user - system
    dt_file="$result/${1}_DT__${dName}_$ft"
    if [[ -e $dt_file ]]; then
        DT_r=$(< $dt_file tail -n 3 | head -n 1 | awk '{ print $2;}');
        DT_u=$(< $dt_file tail -n 2 | head -n 1 | awk '{ print $2;}');
        DT_s=$(< $dt_file tail -n 1 | awk '{ print $2;}');
    fi

    ### Decompression memory
    dm_file="$result/${1}_DM__${dName}_$ft"
    if [[ -e $dm_file ]]; then  DM=$(cat $dm_file);  fi

    ### Verify if the decompressed and the original files are equal
    v_file="$result/${1}_V__${dName}_$ft"
    if [[ -e $v_file ]]; then  V=$(wc -l $v_file);  fi

    ### Remove extra files
    for xf in $cs_file $cm_file $dm_file; do
        if [[ -e $xf ]]; then  rm -f $xf;  fi
    done
    if [[ $V -eq 0 ]]; then  rm -f $v_file;  fi

    ### Move possible informative files to details folder
    # Create a folder for details, if it doesn't already exist
    if [[ ! -d $details ]]; then  mkdir -p $result/$details;  fi
    # Move
    for pif in $ct_file $dt_file $v_file; do
        if [[ -e $pif ]]; then  mv $pif  $result/$details;  fi
    done

    ### Print results
    c="$CS\t$CT_r\t$CT_u\t$CT_s\t$CM"    # Compression
    d="$DT_r\t$DT_u\t$DT_s\t$DM"         # Decompression
    printf "$dName\t$fsize\t$method\t$c\t$d\t$V\n";
}


### Results of compress/decompress on datasets. $1: output file
function compDecompResOnDataset
{
    FAdsPath=$dataset/$FA
    FQdsPath=$dataset/$FQ
    VCFdsPath=$dataset/$VCF
    SAMdsPath=$dataset/$SAM
    BAMdsPath=$dataset/$BAM
    OUT_FILE=$1
    c="Method\tC_Size(B)\tC_Time_real(s)\tC_Time_user(s)\tC_Time_sys(s)"
    c+="\tC_Mem(KB)"
    d="D_Time_real(s)\tD_Time_user(s)\tD_Time_sys(s)\tD_Mem(KB)"

    printf "Dataset\tSize(B)\t$c\t$d\tEq\n" > $OUT_FILE;

    ### FASTA -- human - viruses - synthetic
    #for i in CRYFA $FASTA_METHODS; do
    for i in CRYFA; do
        compDecompRes $i $FAdsPath/$HUMAN/HS.$fasta >> $OUT_FILE;
        compDecompRes $i $FAdsPath/$VIRUSES/viruses.$fasta >> $OUT_FILE;
        for j in 1 2; do
            compDecompRes $i $FAdsPath/$Synth/SynFA-${j}.$fasta >> $OUT_FILE;
        done
    done

    ### FASTQ -- human - Denisova - synthetic
    #for i in CRYFA $FASTQ_METHODS; do
    for i in CRYFA; do
        for j in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 SRR707196_1; do
            compDecompRes $i $FQdsPath/$HUMAN/HS-${j}.$fastq >> $OUT_FILE;
        done
        for j in B1087 B1088; do
            compDecompRes $i $FQdsPath/$DENISOVA/DS-${j}_SR.$fastq >> $OUT_FILE
        done
        for j in 1 2; do
            compDecompRes $i $FQdsPath/$Synth/SynFQ-${j}.$fastq >> $OUT_FILE;
        done
    done

    ### VCF -- Denisova - Neanderthal
    for i in CRYFA; do
        compDecompRes $i $VCFdsPath/$DENISOVA/DS-22.$vcf >> $OUT_FILE;
        compDecompRes $i $VCFdsPath/$NEANDERTHAL/N-n.$vcf >> $OUT_FILE;
    done

    ### SAM -- human - Neanderthal
    for i in CRYFA; do
        compDecompRes $i $SAMdsPath/$SAM/$HUMAN/HS-n.$sam >> $OUT_FILE;
        compDecompRes $i $SAMdsPath/$SAM/$NEANDERTHAL/N-y.$sam >> $OUT_FILE;
    done

    ### BAM -- human - Neanderthal
    for i in CRYFA; do
        compDecompRes $i $BAMdsPath/$BAM/$HUMAN/HS-11.$bam >> $OUT_FILE;
        compDecompRes $i $BAMdsPath/$BAM/$NEANDERTHAL/N-21.$bam >> $OUT_FILE;
    done
}


### Convert memory numbers scale to MB and times to fractional minutes in
### result files. $1: input file name
function compResHumanReadable
{
    IN=$1              # Input file name
    INWF="${IN%.*}"    # Input file name without filetype
    c="Method\tC_Ratio\tC_Size(MB)\tC_Time_real(m)\tC_Time_cpu(m)"
    d="D_Time_real(m)\tD_Time_cpu(m)"

    printf "Dataset\tSize(MB)\t$c\tC_Mem(MB)\t$d\tD_Mem(MB)\tEq\n" > $INWF.tmp
    awk 'NR>1' $IN | tr ',' . | awk 'BEGIN {}{
      printf "%s\t%.f\t%s\t%.1f\t%.f",
             $1, $2/(1024*1024), $3, $2/$4, $4/(1024*1024);

      split($5, c_arrMinReal, "m");                 c_minReal=c_arrMinReal[1];
      split(c_arrMinReal[2], c_arrSecReal, "s");    c_secReal=c_arrSecReal[1];
      c_realTime=c_minReal*60+c_secReal;

      printf "\t%.1f", c_realTime/60;

      split($6, c_arrMinUser, "m");                 c_minUser=c_arrMinUser[1];
      split(c_arrMinUser[2], c_arrSecUser, "s");    c_secUser=c_arrSecUser[1];
      c_userTime=c_minUser*60+c_secUser;
      split($7, c_arrMinSys, "m");                  c_minSys=c_arrMinSys[1];
      split(c_arrMinSys[2], c_arrSecSys, "s");      c_secSys=c_arrSecSys[1];
      c_sysTime=c_minSys*60+c_secSys;
      c_cpuTime=c_userTime+c_sysTime;

      printf "\t%.1f\t%.f", c_cpuTime/60, $8/1024;

      split($9, d_arrMinReal, "m");                 d_minReal=d_arrMinReal[1];
      split(d_arrMinReal[2], d_arrSecReal, "s");    d_secReal=d_arrSecReal[1];
      d_realTime=d_minReal*60+d_secReal;

      printf "\t%.1f", d_realTime/60;

      split($10, d_arrMinUser, "m");                d_minUser=d_arrMinUser[1];
      split(d_arrMinUser[2], d_arrSecUser, "s");    d_secUser=d_arrSecUser[1];
      d_userTime=d_minUser*60+d_secUser;
      split($11, d_arrMinSys, "m");                 d_minSys=d_arrMinSys[1];
      split(d_arrMinSys[2], d_arrSecSys, "s");      d_secSys=d_arrSecSys[1];
      d_sysTime=d_minSys*60+d_secSys;
      d_cpuTime=d_userTime+d_sysTime;

      printf "\t%.1f\t%.f\t%d\n", d_cpuTime/60, $12/1024, $13;
    }' >> $INWF.tmp

    ### FASTA
    # Details -- 1 row for headers and 1 row after all
    FASTA_METHODS_SIZE=0    # Just when Cryfa is the only compression method run
    removeFromRow=$(echo $((FASTA_DATASET_SIZE*(FASTA_METHODS_SIZE+1)+1+1)));
    sed "$removeFromRow,$ d" $INWF.tmp > ${INWF}_FA.$INF;

#    # For each dataset
#    for i in $FASTA_DATASET; do
#        sed "2,$ d" ${INWF}_FA.$INF > ${INWF}_${i}_FA.$INF;
#        cat ${INWF}_FA.$INF | awk 'NR>1' \
#         | awk -v i=$i 'BEGIN{}{if ($1==i) print;}' >> ${INWF}_${i}_FA.$INF;
#    done

    # Total
    printf "Size(MB)\t$c\t$d\tEq\n" > ${INWF}_tot_FA.$INF;
    < ${INWF}_FA.$INF tr ',' '.' | awk 'NR>1' \
      | awk -v dsSize=$FASTA_DATASET_SIZE 'BEGIN{}{
      s+=$2;   cS+=$5;   cTR+=$6;   cTC+=$7;   dTR+=$9;   dTC+=$10;   eq+=$12;
      if (NR % dsSize==0) {
          printf "%.f\t%s\t%.1f\t%.f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n",
                 s, $3, s/cS, cS, cTR, cTC, dTR, dTC, eq;
          s=0; cS=0;     cTR=0;     cTC=0;     dTR=0;     dTC=0;      eq=0;
      }
    }' >> ${INWF}_tot_FA.$INF

    ### FASTQ
    # Details -- 1 row for headers and 1 row after all
    removeUpToRow=$(echo $((removeFromRow-1)))
    sed "2,$removeUpToRow d" $INWF.tmp > ${INWF}_FQ.$INF;

#    # For each dataset
#    for i in $FASTQ_DATASET; do
#        sed "2,$ d" ${INWF}_FQ.$INF > ${INWF}_${i}_FQ.$INF;
#        cat ${INWF}_FQ.$INF | awk 'NR>1' \
#          | awk -v i=$i 'BEGIN{}{if ($1==i) print;}' >> ${INWF}_${i}_FQ.$INF;
#    done

    # Total
    printf "Size(MB)\t$c\t$d\tEq\n" > ${INWF}_tot_FQ.$INF;
    < ${INWF}_FQ.$INF tr ',' '.' | awk 'NR>1' \
      | awk -v dsSize=$FASTQ_DATASET_SIZE 'BEGIN{}{
      s+=$2;   cS+=$5;   cTR+=$6;   cTC+=$7;   dTR+=$9;   dTC+=$10;   eq+=$12;
      if (NR % dsSize==0) {
          printf "%.f\t%s\t%.1f\t%.f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n",
                 s, $3, s/cS, cS, cTR, cTC, dTR, dTC, eq;
          s=0; cS=0;     cTR=0;     cTC=0;     dTR=0;     dTC=0;      eq=0;
      }
    }' >> ${INWF}_tot_FQ.$INF

    ### VCF
    # Details -- 1 row for headers and 1 row after all
    VCF_METHODS_SIZE=0    # Just when Cryfa is the only compression method run
    removeFromRow=$(echo $((VCF_DATASET_SIZE*(VCF_METHODS_SIZE+1)+1+1)));
    sed "$removeFromRow,$ d" $INWF.tmp > ${INWF}_VCF.$INF;

#    # For each dataset
#    for i in $VCF_DATASET; do
#        sed "2,$ d" ${INWF}_VCF.$INF > ${INWF}_${i}_VCF.$INF;
#        cat ${INWF}_VCF.$INF | awk 'NR>1' \
#         | awk -v i=$i 'BEGIN{}{if ($1==i) print;}' >> ${INWF}_${i}_VCF.$INF;
#    done

    # Total
    printf "Size(MB)\t$c\t$d\tEq\n" > ${INWF}_tot_VCF.$INF;
    < ${INWF}_VCF.$INF tr ',' '.' | awk 'NR>1' \
      | awk -v dsSize=$VCF_DATASET_SIZE 'BEGIN{}{
      s+=$2;   cS+=$5;   cTR+=$6;   cTC+=$7;   dTR+=$9;   dTC+=$10;   eq+=$12;
      if (NR % dsSize==0) {
          printf "%.f\t%s\t%.1f\t%.f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n",
                 s, $3, s/cS, cS, cTR, cTC, dTR, dTC, eq;
          s=0; cS=0;     cTR=0;     cTC=0;     dTR=0;     dTC=0;      eq=0;
      }
    }' >> ${INWF}_tot_VCF.$INF

    ### SAM
    # Details -- 1 row for headers and 1 row after all
    SAM_METHODS_SIZE=0    # Just when Cryfa is the only compression method run
    removeFromRow=$(echo $((SAM_DATASET_SIZE*(SAM_METHODS_SIZE+1)+1+1)));
    sed "$removeFromRow,$ d" $INWF.tmp > ${INWF}_SAM.$INF;

#    # For each dataset
#    for i in $SAM_DATASET; do
#        sed "2,$ d" ${INWF}_SAM.$INF > ${INWF}_${i}_SAM.$INF;
#        cat ${INWF}_SAM.$INF | awk 'NR>1' \
#         | awk -v i=$i 'BEGIN{}{if ($1==i) print;}' >> ${INWF}_${i}_SAM.$INF;
#    done

    # Total
    printf "Size(MB)\t$c\t$d\tEq\n" > ${INWF}_tot_SAM.$INF;
    < ${INWF}_SAM.$INF tr ',' '.' | awk 'NR>1' \
      | awk -v dsSize=$SAM_DATASET_SIZE 'BEGIN{}{
      s+=$2;   cS+=$5;   cTR+=$6;   cTC+=$7;   dTR+=$9;   dTC+=$10;   eq+=$12;
      if (NR % dsSize==0) {
          printf "%.f\t%s\t%.1f\t%.f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n",
                 s, $3, s/cS, cS, cTR, cTC, dTR, dTC, eq;
          s=0; cS=0;     cTR=0;     cTC=0;     dTR=0;     dTC=0;      eq=0;
      }
    }' >> ${INWF}_tot_SAM.$INF

    ### BAM
    # Details -- 1 row for headers and 1 row after all
    BAM_METHODS_SIZE=0    # Just when Cryfa is the only compression method run
    removeFromRow=$(echo $((BAM_DATASET_SIZE*(BAM_METHODS_SIZE+1)+1+1)));
    sed "$removeFromRow,$ d" $INWF.tmp > ${INWF}_BAM.$INF;

#    # For each dataset
#    for i in $BAM_DATASET; do
#        sed "2,$ d" ${INWF}_BAM.$INF > ${INWF}_${i}_BAM.$INF;
#        cat ${INWF}_BAM.$INF | awk 'NR>1' \
#         | awk -v i=$i 'BEGIN{}{if ($1==i) print;}' >> ${INWF}_${i}_BAM.$INF;
#    done

    # Total
    printf "Size(MB)\t$c\t$d\tEq\n" > ${INWF}_tot_BAM.$INF;
    < ${INWF}_BAM.$INF tr ',' '.' | awk 'NR>1' \
      | awk -v dsSize=$BAM_DATASET_SIZE 'BEGIN{}{
      s+=$2;   cS+=$5;   cTR+=$6;   cTC+=$7;   dTR+=$9;   dTC+=$10;   eq+=$12;
      if (NR % dsSize==0) {
          printf "%.f\t%s\t%.1f\t%.f\t%.1f\t%.1f\t%.1f\t%.1f\t%.1f\n",
                 s, $3, s/cS, cS, cTR, cTC, dTR, dTC, eq;
          s=0; cS=0;     cTR=0;     cTC=0;     dTR=0;     dTC=0;      eq=0;
      }
    }' >> ${INWF}_tot_BAM.$INF

    rm -f $INWF.tmp
}


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Print compression results
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
OUT="$result/COMP.$RES"    # Output file name

### Results on datasets
compDecompResOnDataset $OUT;

### Make the result file human readable
compResHumanReadable $OUT;