          #######################################################
          #            Compression+encryption results           #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

. par.sh        # Internal parameters


  # print comp/decomp plus enc/dec results.
  # $1: compression program's name, $2: encryption program's name, $3: dataset.
  function compEncDecDecompRes
  {
      in="${3##*/}"                       # dataset name
      dName="${in%.*}"                    # dataset name without filetype
      ft="${in##*.}"                      # dataset filetype
      fsize=`stat --printf="%s" $3`       # file size (bytes)
      methodComp=`printMethodName $1`     # comp methods' name for printing
      methodEnc=`printMethodName $2`      # enc methods' name for printing

      CS="";
      CT_r="";     CT_u="";     CT_s="";     CM="";
      EnS="";
      EnT_r="";    EnT_u="";    EnT_s="";    EnM="";
      DeT_r="";    DeT_u="";    DeT_s="";    DeM="";
      DT_r="";     DT_u="";     DT_s="";     DM="";
      V="";

      ### compressed file size
      cs_file="$result/${1}_${2}_CS__${dName}_$ft"
      if [[ -e $cs_file ]]; then
          CS=`cat $cs_file | awk '{ print $5; }'`;
#          rm -f $cs_file;
      fi

      ### compression time -- real - user - system
      ct_file="$result/${1}_${2}_CT__${dName}_$ft"
      if [[ -e $ct_file ]]; then
          CT_r=`cat $ct_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          CT_u=`cat $ct_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          CT_s=`cat $ct_file | tail -n 1 | awk '{ print $2;}'`;
#          rm -f $ct_file;
      fi

      ### compression memory
      cm_file="$result/${1}_${2}_CM__${dName}_$ft"
      if [[ -e $cm_file ]]; then
          CM=`cat $cm_file`;
#          rm -f $cm_file;
      fi

      ### encrypted file size
      ens_file="$result/${1}_${2}_EnS__${dName}_$ft"
      if [[ -e $ens_file ]]; then
          EnS=`cat $ens_file | awk '{ print $5; }'`;
#          rm -f $ens_file;
      fi

      ### encryption time -- real - user - system
      ent_file="$result/${1}_${2}_EnT__${dName}_$ft"
      if [[ -e $ent_file ]]; then
          EnT_r=`cat $ent_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          EnT_u=`cat $ent_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          EnT_s=`cat $ent_file | tail -n 1 | awk '{ print $2;}'`;
#          rm -f $ent_file;
      fi

      ### encryption memory
      enm_file="$result/${1}_${2}_EnM__${dName}_$ft"
      if [[ -e $enm_file ]]; then
          EnM=`cat $enm_file`;
#          rm -f $enm_file;
      fi

      ### decryption time -- real - user - system
      det_file="$result/${1}_${2}_DeT__${dName}_$ft"
      if [[ -e $det_file ]]; then
          DeT_r=`cat $det_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          DeT_u=`cat $det_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          DeT_s=`cat $det_file | tail -n 1 | awk '{ print $2;}'`;
#          rm -f $det_file;
      fi

      ### decryption memory
      dem_file="$result/${1}_${2}_DeM__${dName}_$ft"
      if [[ -e $dem_file ]]; then
          DeM=`cat $dem_file`;
#          rm -f $dem_file;
      fi

      ### decompression time -- real - user - system
      dt_file="$result/${1}_${2}_DT__${dName}_$ft"
      if [[ -e $dt_file ]]; then
          DT_r=`cat $dt_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          DT_u=`cat $dt_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          DT_s=`cat $dt_file | tail -n 1 | awk '{ print $2;}'`;
#          rm -f $dt_file;
      fi

      ### decompression memory
      dm_file="$result/${1}_${2}_DM__${dName}_$ft"
      if [[ -e $dm_file ]]; then
          DM=`cat $dm_file`;
#          rm -f $dm_file;
      fi

      ### if decompressed file is the same as the original file
      v_file="$result/${1}_${2}_V__${dName}_$ft"
      if [[ -e $v_file ]]; then
          V=`cat $v_file | wc -l`;
#          rm -f $v_file;
      fi

      c="$CS\t$CT_r\t$CT_u\t$CT_s\t$CM"        # compression results
      en="$EnS\t$EnT_r\t$EnT_u\t$EnT_s\t$EnM"  # encryption results
      de="$DeT_r\t$DeT_u\t$DeT_s\t$DeM"        # decryption results
      d="$DT_r\t$DT_u\t$DT_s\t$DM"             # decompression results

      printf "$dName\t$fsize\t$methodComp\t$methodEnc\t$c\t$en\t$de\t$d\t$V\n";
  }


  # convert memory numbers scale to MB and times to fractional minutes in
  # result files associated with comression+encryption methods
  # $1: input file name
  function compEncResHumanReadable
  {
      IN=$1              # input file name
      INWF="${IN%.*}"    # input file name without filetype

      cen="CEn_Size(B)\tCEn_Time_real(s)\tCEn_Time_cpu(s)\tCEn_Mem(KB)"
      ded="DeD_Time_real(s)\tDeD_Time_cpu(s)\tDeD_Mem(KB)"
      printf "Dataset\tSize(B)\tC_Method\tEn_Method\t$cen\t$ded\tEq\n" \
             > $INWF.tmp

      cat $IN | awk 'NR>1' | tr ',' . | awk 'BEGIN {}{
      printf "%s\t%.f\t%s\t%s\t%.f", $1, $2, $3, $4, $10;

      split($6, c_arrMinReal, "m");                 c_minReal=c_arrMinReal[1];
      split(c_arrMinReal[2], c_arrSecReal, "s");    c_secReal=c_arrSecReal[1];
      c_realTime=c_minReal*60+c_secReal;
      split($11, en_arrMinReal, "m");               en_minReal=en_arrMinReal[1];
      split(en_arrMinReal[2], en_arrSecReal, "s");  en_secReal=en_arrSecReal[1];
      en_realTime=en_minReal*60+en_secReal;
      cen_realTime=c_realTime+en_realTime;
      printf "\t%.3f", cen_realTime;

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
      printf "\t%.3f", cen_cpuTime;

      max_cen_mem=($9 > $14 ? $9 : $14);
      cen_mem=max_cen_mem;
      printf "\t%.f", cen_mem;

      split($15, de_arrMinReal, "m");               de_minReal=de_arrMinReal[1];
      split(de_arrMinReal[2], de_arrSecReal, "s");  de_secReal=de_arrSecReal[1];
      de_realTime=de_minReal*60+de_secReal;
      split($19, d_arrMinReal, "m");                d_minReal=d_arrMinReal[1];
      split(d_arrMinReal[2], d_arrSecReal, "s");    d_secReal=d_arrSecReal[1];
      d_realTime=d_minReal*60+d_secReal;
      ded_realTime=de_realTime+d_realTime;
      printf "\t%.3f", ded_realTime;

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
      printf "\t%.3f", ded_cpuTime;

      max_ded_mem=($18 > $22 ? $18 : $22);
      ded_mem=max_ded_mem;
      printf "\t%.f", ded_mem;

      printf "\t%d\n", $23;
      }' >> $INWF.tmp

      ### FASTA
      # details -- 1 row for headers and 1 row after all
      removeFromRow=`echo $((FASTA_DATASET_SIZE*FASTA_METHODS_SIZE+1+1))`
      sed "$removeFromRow,$ d" $INWF.tmp > ${INWF}_detail_FA.$INF;

      # for each dataset
      cen_each="C_Ratio\tCEn_Speed(MB/s)\tCEn_Time_cpu(m)\tCEn_Mem(MB)"
      ded_each="DeD_Speed(MB/s)\tDeD_Time_cpu(m)\tDeD_Mem(MB)"
      printf "Dataset\tSize(MB)\tCEn_Method\t$cen_each\t$ded_each\tEq\n" \
          > ${INWF}_FA.tmp
      cat ${INWF}_detail_FA.$INF | awk 'NR>1' | awk 'BEGIN{}{
      printf "%s\t%.f\t%s+%s\t%.1f\t%.f\t%.2f\t%.f\t%.f\t%.2f\t%.f\t%.1f\n",
       $1, $2/(1024*1024), $3, $4, $2/$5, $2/(1024*1024*$6), $7/60, $8/1024,
       $5/(1024*1024*$9), $10/60, $11/1024, $12;
       }'  >> ${INWF}_FA.tmp

       for i in $FASTA_DATASET; do
           sed "2,$ d" ${INWF}_FA.tmp > ${INWF}_${i}_FA.$INF;
           cat ${INWF}_FA.tmp | awk 'NR>1' \
            | awk -v i=$i 'BEGIN{}{if ($1==i) print;}' >> ${INWF}_${i}_FA.$INF;

           # extract from cryfa
           cat $result/COMP_${i}_FA.$INF | awk 'NR>1' \
            | awk 'BEGIN{}{if ($3=="Cryfa") print;}' >> ${INWF}_${i}_FA.$INF;
       done
       rm -f ${INWF}_FA.tmp

      # total
      cen_tot="C_Ratio\tCEn_Speed(MB/s)\tCEn_Time_cpu(m)"
      ded_tot="DeD_Speed(MB/s)\tDeD_Time_cpu(m)"
      printf "Size(MB)\tCEn_Method\t$cen_tot\t$ded_tot\tEq\n" \
          > ${INWF}_tot_FA.$INF;
      cat ${INWF}_detail_FA.$INF | awk 'NR>1' \
       | awk -v dsSize=$FASTA_DATASET_SIZE 'BEGIN{}{
       s+=$2; cenS+=$5; cenTR+=$6; cenTC+=$7; dedTR+=$9; dedTC+=$10; eq+=$12;
       if (NR % dsSize==0) {
        printf "%.f\t%s+%s\t%.1f\t%.f\t%.2f\t%.f\t%.2f\t%.1f\n",
               s/(1024*1024), $3, $4, s/cenS, s/(1024*1024*cenTR), cenTC/60,
               cenS/(1024*1024*dedTR), dedTC/60, eq;
        s=0;  cenS=0;  cenTR=0;  cenTC=0;  dedTR=0;  dedTC=0;  eq=0;
       }
       }' >> ${INWF}_tot_FA.$INF

      # extract from cryfa
      cat $result/COMP_tot_FA.$INF | awk 'NR>1' \
       | awk 'BEGIN{}{if ($2=="Cryfa") print;}' >> ${INWF}_tot_FA.$INF

      ### FASTQ
      # details -- 1 row for headers and 1 row after all
      removeUpToRow=`echo $((removeFromRow-1))`
      sed "2,$removeUpToRow d" $INWF.tmp > ${INWF}_detail_FQ.$INF;

      # for each dataset
      printf "Dataset\tSize(MB)\tCEn_Method\t$cen_each\t$ded_each\tEq\n" \
          > ${INWF}_FQ.tmp
      cat ${INWF}_detail_FQ.$INF | awk 'NR>1' | awk 'BEGIN{}{
       printf "%s\t%.f\t%s+%s\t%.1f\t%.f\t%.2f\t%.f\t%.f\t%.2f\t%.f\t%.1f\n",
       $1, $2/(1024*1024), $3, $4, $2/$5, $2/(1024*1024*$6), $7/60, $8/1024,
       $5/(1024*1024*$9), $10/60, $11/1024, $12;
       }'  >> ${INWF}_FQ.tmp

       for i in $FASTQ_DATASET; do
           sed "2,$ d" ${INWF}_FQ.tmp > ${INWF}_${i}_FQ.$INF;
           cat ${INWF}_FQ.tmp | awk 'NR>1' \
            | awk -v i=$i 'BEGIN{}{if ($1==i) print;}' >> ${INWF}_${i}_FQ.$INF;

           # extract from cryfa
           cat $result/COMP_${i}_FQ.$INF | awk 'NR>1' \
            | awk 'BEGIN{}{if ($3=="Cryfa") print;}' >> ${INWF}_${i}_FQ.$INF;
       done
       rm -f ${INWF}_FQ.tmp

      # total
      printf "Size(MB)\tCEn_Method\t$cen_tot\t$ded_tot\tEq\n" \
          > ${INWF}_tot_FQ.$INF;
      cat ${INWF}_detail_FQ.$INF | awk 'NR>1' \
       | awk -v dsSize=$FASTQ_DATASET_SIZE 'BEGIN{}{
       s+=$2; cenS+=$5; cenTR+=$6; cenTC+=$7; dedTR+=$9; dedTC+=$10; eq+=$12;
       if (NR % dsSize==0) {
        printf "%.f\t%s+%s\t%.1f\t%.f\t%.2f\t%.f\t%.2f\t%.1f\n",
               s/(1024*1024), $3, $4, s/cenS, s/(1024*1024*cenTR), cenTC/60,
               cenS/(1024*1024*dedTR), dedTC/60, eq;
        s=0;  cenS=0;  cenTR=0;  cenTC=0;  dedTR=0;  dedTC=0;  eq=0;
       }
       }' >> ${INWF}_tot_FQ.$INF

      # extract from cryfa
      cat $result/COMP_tot_FQ.$INF | awk 'NR>1' \
       | awk 'BEGIN{}{if ($2=="Cryfa") print;}' >> ${INWF}_tot_FQ.$INF

      rm -f $INWF.tmp
  }