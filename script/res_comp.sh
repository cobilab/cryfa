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
    fsize=`stat --printf="%s" $2`    # File size (bytes)
    method=`printMethodName $1`      # Methods' name for printing

    CS="";      CT_r="";    CT_u="";    CT_s="";    CM="";
    DT_r="";    DT_u="";    DT_s="";    DM="";      V="";

    ### Compressed file size
    cs_file="$result/${1}_CS__${dName}_$ft"
    if [[ -e $cs_file ]]; then
        CS=`cat $cs_file | awk '{ print $5; }'`;
#        rm -f $cs_file;
    fi

    ### Compression time -- real - user - system
    ct_file="$result/${1}_CT__${dName}_$ft"
    if [[ -e $ct_file ]]; then
        CT_r=`cat $ct_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
        CT_u=`cat $ct_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
        CT_s=`cat $ct_file | tail -n 1 | awk '{ print $2;}'`;
#        rm -f $ct_file;
    fi

    ### Compression memory
    cm_file="$result/${1}_CM__${dName}_$ft"
    if [[ -e $cm_file ]]; then
        CM=`cat $cm_file`;
#        rm -f $cm_file;
    fi

    ### Decompression time -- real - user - system
    dt_file="$result/${1}_DT__${dName}_$ft"
    if [[ -e $dt_file ]]; then
        DT_r=`cat $dt_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
        DT_u=`cat $dt_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
        DT_s=`cat $dt_file | tail -n 1 | awk '{ print $2;}'`;
#        rm -f $dt_file;
    fi

    ### Decompression memory
    dm_file="$result/${1}_DM__${dName}_$ft"
    if [[ -e $dm_file ]]; then
        DM=`cat $dm_file`;
#        rm -f $dm_file;
    fi

    ### Verify if the decompressed and the original files are equal
    v_file="$result/${1}_V__${dName}_$ft"
    if [[ -e $v_file ]]; then
        V=`cat $v_file | wc -l`;
#        rm -f $v_file;
    fi

    c="$CS\t$CT_r\t$CT_u\t$CT_s\t$CM"    # Compression results
    d="$DT_r\t$DT_u\t$DT_s\t$DM"         # Decompression results

    printf "$dName\t$fsize\t$method\t$c\t$d\t$V\n";
}


### Convert memory numbers scale to MB and times to fractional minutes in
### result files. $1: input file name
function compResHumanReadable
{
    IN=$1              # Input file name
    INWF="${IN%.*}"    # Input file name without filetype

    c="C_Size(B)\tC_Time_real(s)\tC_Time_cpu(s)\tC_Mem(KB)"
    d="D_Time_real(s)\tD_Time_cpu(s)\tD_Mem(KB)"

    printf "Dataset\tSize(B)\tMethod\t$c\t$d\tEq\n" > $INWF.tmp
    cat $IN | awk 'NR>1' | tr ',' . | awk 'BEGIN {}{
      printf "%s\t%.f\t%s\t%.f", $1, $2, $3, $4;

      split($5, c_arrMinReal, "m");                 c_minReal=c_arrMinReal[1];
      split(c_arrMinReal[2], c_arrSecReal, "s");    c_secReal=c_arrSecReal[1];
      c_realTime=c_minReal*60+c_secReal;
      printf "\t%.3f", c_realTime;

      split($6, c_arrMinUser, "m");                 c_minUser=c_arrMinUser[1];
      split(c_arrMinUser[2], c_arrSecUser, "s");    c_secUser=c_arrSecUser[1];
      c_userTime=c_minUser*60+c_secUser;
      split($7, c_arrMinSys, "m");                  c_minSys=c_arrMinSys[1];
      split(c_arrMinSys[2], c_arrSecSys, "s");      c_secSys=c_arrSecSys[1];
      c_sysTime=c_minSys*60+c_secSys;
      c_cpuTime=c_userTime+c_sysTime;
      printf "\t%.3f", c_cpuTime;

      printf "\t%.f", $8;

      split($9, d_arrMinReal, "m");                 d_minReal=d_arrMinReal[1];
      split(d_arrMinReal[2], d_arrSecReal, "s");    d_secReal=d_arrSecReal[1];
      d_realTime=d_minReal*60+d_secReal;
      printf "\t%.3f", d_realTime;

      split($10, d_arrMinUser, "m");                d_minUser=d_arrMinUser[1];
      split(d_arrMinUser[2], d_arrSecUser, "s");    d_secUser=d_arrSecUser[1];
      d_userTime=d_minUser*60+d_secUser;
      split($11, d_arrMinSys, "m");                 d_minSys=d_arrMinSys[1];
      split(d_arrMinSys[2], d_arrSecSys, "s");      d_secSys=d_arrSecSys[1];
      d_sysTime=d_minSys*60+d_secSys;
      d_cpuTime=d_userTime+d_sysTime;
      printf "\t%.3f", d_cpuTime;

      printf "\t%.f\t%d\n", $12, $13;
    }' >> $INWF.tmp

    ### FASTA
    # Details -- 1 row for headers and 1 row after all
    removeFromRow=`echo $((FASTA_DATASET_SIZE*(FASTA_METHODS_SIZE+1)+1+1))`
    sed "$removeFromRow,$ d" $INWF.tmp > ${INWF}_detail_FA.$INF;

    # For each dataset
    c_each="C_Ratio\tC_Speed(MB/s)\tC_Time_cpu(m)\tC_Mem(MB)"
    d_each="D_Speed(MB/s)\tD_Time_cpu(m)\tD_Mem(MB)"

    printf "Dataset\tSize(MB)\tC_Method\t$c_each\t$d_each\tEq\n" > ${INWF}_FA.tmp
    cat ${INWF}_detail_FA.$INF | awk 'NR>1' | awk 'BEGIN{}{
      printf "%s\t%.f\t%s\t%.1f\t%.f\t%.2f\t%.f\t%.f\t%.2f\t%.f\t%.1f\n",
             $1, $2/(1024*1024), $3, $2/$4, $2/(1024*1024*$5), $6/60, $7/1024,
             $4/(1024*1024*$8), $9/60, $10/1024, $11;
    }' >> ${INWF}_FA.tmp

    for i in $FASTA_DATASET; do
        sed "2,$ d" ${INWF}_FA.tmp > ${INWF}_${i}_FA.$INF;
        cat ${INWF}_FA.tmp | awk 'NR>1' \
         | awk -v i=$i 'BEGIN{}{if ($1==i) print;}' >> ${INWF}_${i}_FA.$INF;
    done
    rm -f ${INWF}_FA.tmp

    # Total
    c_tot="C_Ratio\tC_Speed(MB/s)\tC_Time_cpu(m)"
    d_tot="D_Speed(MB/s)\tD_Time_cpu(m)"

    printf "Size(MB)\tC_Method\t$c_tot\t$d_tot\tEq\n" > ${INWF}_tot_FA.$INF;
    cat ${INWF}_detail_FA.$INF | awk 'NR>1' \
      | awk -v dsSize=$FASTA_DATASET_SIZE 'BEGIN{}{
      s+=$2;   cS+=$4;   cTR+=$5;   cTC+=$6;   dTR+=$8;   dTC+=$9;   eq+=$11;
      if (NR % dsSize==0) {
          printf "%.f\t%s\t%.1f\t%.f\t%.2f\t%.f\t%.2f\t%.1f\n",
                 s/(1024*1024), $3, s/cS, s/(1024*1024*cTR), cTC/60,
                 cS/(1024*1024*dTR), dTC/60, eq;
          s=0; cS=0;     cTR=0;     cTC=0;     dTR=0;     dTC=0;     eq=0; }
    }' >> ${INWF}_tot_FA.$INF

    ### FASTQ
    # Details -- 1 row for headers and 1 row after all
    removeUpToRow=`echo $((removeFromRow-1))`
    sed "2,$removeUpToRow d" $INWF.tmp > ${INWF}_detail_FQ.$INF;

    # For each dataset
    printf "Dataset\tSize(MB)\tMethod\t$c_each\t$d_each\tEq\n" > ${INWF}_FQ.tmp
    cat ${INWF}_detail_FQ.$INF | awk 'NR>1' | awk 'BEGIN{}{
      printf "%s\t%.f\t%s\t%.1f\t%.f\t%.2f\t%.f\t%.f\t%.2f\t%.f\t%.1f\n",
             $1, $2/(1024*1024), $3, $2/$4, $2/(1024*1024*$5), $6/60, $7/1024,
             $4/(1024*1024*$8), $9/60, $10/1024, $11;
    }' >> ${INWF}_FQ.tmp

    for i in $FASTQ_DATASET; do
        sed "2,$ d" ${INWF}_FQ.tmp > ${INWF}_${i}_FQ.$INF;
        cat ${INWF}_FQ.tmp | awk 'NR>1' \
          | awk -v i=$i 'BEGIN{}{if ($1==i) print;}' >> ${INWF}_${i}_FQ.$INF;
    done
    rm -f ${INWF}_FQ.tmp

    # Total
    printf "Size(MB)\tMethod\t$c_tot\t$d_tot\tEq\n" > ${INWF}_tot_FQ.$INF;
    cat ${INWF}_detail_FQ.$INF | awk 'NR>1' \
      | awk -v dsSize=$FASTQ_DATASET_SIZE 'BEGIN{}{
      s+=$2;   cS+=$4;   cTR+=$5;   cTC+=$6;   dTR+=$8;   dTC+=$9;   eq+=$11;
      if (NR % dsSize==0) {
          printf "%.f\t%s\t%.1f\t%.f\t%.2f\t%.f\t%.2f\t%.1f\n",
                 s/(1024*1024), $3, s/cS, s/(1024*1024*cTR), cTC/60,
                 cS/(1024*1024*dTR), dTC/60, eq;
          s=0; cS=0;     cTR=0;     cTC=0;     dTR=0;     dTC=0;     eq=0; }
    }' >> ${INWF}_tot_FQ.$INF

    rm -f $INWF.tmp
}


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Print compression results
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
OUT="$result/COMP.$RES"    # Output file name
FAdsPath=$dataset/$FA
FQdsPath=$dataset/$FQ
c="C_Size(B)\tC_Time_real(s)\tC_Time_user(s)\tC_Time_sys(s)\tC_Mem(KB)"
d="D_Time_real(s)\tD_Time_user(s)\tD_Time_sys(s)\tD_Mem(KB)"

printf "Dataset\tSize(B)\tMethod\t$c\t$d\tEq\n" > $OUT;

### FASTA -- human - viruses - synthetic
for i in CRYFA $FASTA_METHODS; do
    compDecompRes $i $FAdsPath/$HUMAN/HS.$fasta >> $OUT;
    compDecompRes $i $FAdsPath/$VIRUSES/viruses.$fasta >> $OUT;
    for j in 1 2; do
        compDecompRes $i $FAdsPath/$Synth/SynFA-${j}.$fasta >> $OUT;
    done
done

### FASTQ -- human - Denisova - synthetic
for i in CRYFA $FASTQ_METHODS; do
    for j in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 SRR707196_1; do
        compDecompRes $i $FQdsPath/$HUMAN/HS-${j}.$fastq >> $OUT;
    done
    for j in B1087 B1088 B1110 B1128 SL3003; do
        compDecompRes $i $FQdsPath/$DENISOVA/DS-${j}_SR.$fastq >> $OUT
    done
    for j in 1 2; do
        compDecompRes $i $FQdsPath/$Synth/SynFQ-${j}.$fastq >> $OUT;
    done
done

### Make the result file human readable
compResHumanReadable $OUT;