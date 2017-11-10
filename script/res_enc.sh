          #######################################################
          #                  Encryption results                 #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

. par.sh        # Parameters


  # print encrypt/decrypt results. $1: program's name, $2: dataset
  function encDecRes
  {
      in="${2##*/}"                       # dataset name
      dName="${in%.*}"                    # dataset name without filetype
      ft="${in##*.}"                      # dataset filetype
      fsize=`stat --printf="%s" $2`       # file size (bytes)
      method=`printMethodName $1`         # methods' name for printing

      EnS="";
      EnT_r="";     EnT_u="";     EnT_s="";     EnM="";
      DeT_r="";     DeT_u="";     DeT_s="";     DeM="";

      ### encrypted file size
      ens_file="$result/${1}_EnS__${dName}_$ft"
      if [[ -e $ens_file ]]; then
          EnS=`cat $ens_file | awk '{ print $5; }'`;
#          rm -f $ens_file;
      fi

      ### encryption time -- real - user - system
      ent_file="$result/${1}_EnT__${dName}_$ft"
      if [[ -e $ent_file ]]; then
          EnT_r=`cat $ent_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          EnT_u=`cat $ent_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          EnT_s=`cat $ent_file | tail -n 1 | awk '{ print $2;}'`;
#          rm -f $ent_file;
      fi

      ### encryption memory
      enm_file="$result/${1}_EnM__${dName}_$ft"
      if [[ -e $enm_file ]]; then
          EnM=`cat $enm_file`;
#          rm -f $enm_file;
      fi

      ### decryption time -- real - user - system
      det_file="$result/${1}_DeT__${dName}_$ft"
      if [[ -e $det_file ]]; then
          DeT_r=`cat $det_file | tail -n 3 | head -n 1 | awk '{ print $2;}'`;
          DeT_u=`cat $det_file | tail -n 2 | head -n 1 | awk '{ print $2;}'`;
          DeT_s=`cat $det_file | tail -n 1 | awk '{ print $2;}'`;
#          rm -f $det_file;
      fi

      ### decryption memory
      dem_file="$result/${1}_DeM__${dName}_$ft"
      if [[ -e $dem_file ]]; then
          DeM=`cat $dem_file`;
#          rm -f $dem_file;
      fi

      en="$EnS\t$EnT_r\t$EnT_u\t$EnT_s\t$EnM"    # encryption results
      de="$DeT_r\t$DeT_u\t$DeT_s\t$DeM"          # decryption results

      printf "$dName\t$fsize\t$method\t$en\t$de\n";
  }


  # convert memory numbers scale to MB and times to fractional minutes in
  # result files associated with encryption methods. $1: input file name
  function encResHumanReadable
  {
      IN=$1              # input file name
      INWF="${IN%.*}"    # input file name without filetype

      en="En_Size(B)\tEn_Time_real(s)\tEn_Time_cpu(s)\tEn_Mem(KB)"
      de="De_Time_real(s)\tDe_Time_cpu(s)\tDe_Mem(KB)"
      printf "Dataset\tSize(B)\tMethod\t$en\t$de\tEq\n" > $INWF.tmp

      cat $IN | awk 'NR>1' | tr ',' . | awk 'BEGIN {}{
      printf "%s\t%.f\t%s\t%.f", $1, $2, $3, $4;

      split($5, en_arrMinReal, "m");                en_minReal=en_arrMinReal[1];
      split(en_arrMinReal[2], en_arrSecReal, "s");  en_secReal=en_arrSecReal[1];
      en_realTime=en_minReal*60+en_secReal;
      printf "\t%.3f", en_realTime;

      split($6, en_arrMinUser, "m");                en_minUser=en_arrMinUser[1];
      split(en_arrMinUser[2], en_arrSecUser, "s");  en_secUser=en_arrSecUser[1];
      en_userTime=en_minUser*60+en_secUser;
      split($7, en_arrMinSys, "m");                 en_minSys=en_arrMinSys[1];
      split(en_arrMinSys[2], en_arrSecSys, "s");    en_secSys=en_arrSecSys[1];
      en_sysTime=en_minSys*60+en_secSys;
      en_cpuTime=en_userTime+en_sysTime;
      printf "\t%.3f", en_cpuTime;

      printf "\t%.f", $8;

      split($9, de_arrMinReal, "m");                de_minReal=de_arrMinReal[1];
      split(de_arrMinReal[2], de_arrSecReal, "s");  de_secReal=de_arrSecReal[1];
      de_realTime=de_minReal*60+de_secReal;
      printf "\t%.3f", de_realTime;

      split($10, de_arrMinUser, "m");               de_minUser=de_arrMinUser[1];
      split(de_arrMinUser[2], de_arrSecUser, "s");  de_secUser=de_arrSecUser[1];
      de_userTime=de_minUser*60+de_secUser;
      split($11, de_arrMinSys, "m");                de_minSys=de_arrMinSys[1];
      split(de_arrMinSys[2], de_arrSecSys, "s");    de_secSys=de_arrSecSys[1];
      de_sysTime=de_minSys*60+de_secSys;
      de_cpuTime=de_userTime+de_sysTime;
      printf "\t%.3f", de_cpuTime;

      printf "\t%.f\t%d\n", $12, $13;
      }' >> $INWF.tmp

      ### FASTA
      # details -- 1 row for headers and 1 row after all
      removeFromRow=`echo $((FASTA_DATASET_SIZE+1+1))`
      sed "$removeFromRow,$ d" $INWF.tmp > ${INWF}_detail_FA.$INF;

      # for each dataset
      en_each="C_Ratio\tEn_Speed(MB/s)\tEn_Time_cpu(m)\tEn_Mem(MB)"
      de_each="De_Speed(MB/s)\tDe_Time_cpu(m)\tDe_Mem(MB)"
      printf "Dataset\tSize(MB)\tEn_Method\t$en_each\t$de_each\tEq\n" \
          > ${INWF}_FA.tmp
      cat ${INWF}_detail_FA.$INF | awk 'NR>1' | awk 'BEGIN{}{
       printf "%s\t%.f\t%s\t%.1f\t%.f\t%.2f\t%.f\t%.f\t%.2f\t%.f\t%.1f\n",
       $1, $2/(1024*1024), $3, $2/$4, $2/(1024*1024*$5), $6/60, $7/1024,
       $4/(1024*1024*$8), $9/60, $10/1024, $11;
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
      en_tot="C_Ratio\tEn_Speed(MB/s)\tEn_Time_cpu(m)"
      de_tot="De_Speed(MB/s)\tDe_Time_cpu(m)"
      printf "Size(MB)\tEn_Method\t$en_tot\t$de_tot\tEq\n" \
          > ${INWF}_tot_FA.$INF;
      cat ${INWF}_detail_FA.$INF | awk 'NR>1' \
       | awk -v dsSize=$FASTA_DATASET_SIZE 'BEGIN{}{
       s+=$2;  cS+=$4;  cTR+=$5;  cTC+=$6;  dTR+=$8;  dTC+=$9;  eq+=$11;
       if (NR % dsSize==0) {
        printf "%.f\t%s\t%.1f\t%.f\t%.2f\t%.f\t%.2f\t%.1f\n",
               s/(1024*1024), $3, s/cS, s/(1024*1024*cTR), cTC/60,
               cS/(1024*1024*dTR), dTC/60, eq;
        s=0;   cS=0;   cTR=0;   cTC=0;   dTR=0;   dTC=0;   eq=0;
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
      printf "Dataset\tSize(MB)\tMethod\t$en_each\t$de_each\tEq\n" \
          > ${INWF}_FQ.tmp
      cat ${INWF}_detail_FQ.$INF | awk 'NR>1' | awk 'BEGIN{}{
       printf "%s\t%.f\t%s\t%.1f\t%.f\t%.2f\t%.f\t%.f\t%.2f\t%.f\t%.1f\n",
       $1, $2/(1024*1024), $3, $2/$4, $2/(1024*1024*$5), $6/60, $7/1024,
       $4/(1024*1024*$8), $9/60, $10/1024, $11;
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
      printf "Size(MB)\tMethod\t$en_tot\t$de_tot\tEq\n" > ${INWF}_tot_FQ.$INF;
      cat ${INWF}_detail_FQ.$INF | awk 'NR>1' \
       | awk -v dsSize=$FASTQ_DATASET_SIZE 'BEGIN{}{
       s+=$2;  cS+=$4;  cTR+=$5;  cTC+=$6;  dTR+=$8;  dTC+=$9;  eq+=$11;
       if (NR % dsSize==0) {
        printf "%.f\t%s\t%.1f\t%.f\t%.2f\t%.f\t%.2f\t%.1f\n",
               s/(1024*1024), $3, s/cS, s/(1024*1024*cTR), cTC/60,
               cS/(1024*1024*dTR), dTC/60, eq;
        s=0;   cS=0;   cTR=0;   cTC=0;   dTR=0;   dTC=0;   eq=0;
       }
       }' >> ${INWF}_tot_FQ.$INF

      # extract from cryfa
      cat $result/COMP_tot_FQ.$INF | awk 'NR>1' \
       | awk 'BEGIN{}{if ($2=="Cryfa") print;}' >> ${INWF}_tot_FQ.$INF

      rm -f $INWF.tmp
  }