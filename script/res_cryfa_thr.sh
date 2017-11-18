          #######################################################
          #  Results of cryfa with different number of threads  #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

. $script/run_fn_cryfa_thr.sh    # Dataset parameters

#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Functions
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
### Convert memory numbers scale to MB and times to fractional minutes in
### result files. $1: input file name
function cryfaThrResHumanReadable
{
    IN=$1              # Input file name
    INWF="${IN%.*}"    # Input file name without filetype

    c="C_Ratio\tC_Size(MB)\tC_Time_real(m)\tC_Time_cpu(m)\tC_Mem(MB)"
    d="D_Time_real(m)\tD_Time_cpu(m)\tD_Mem(MB)"

    printf "Size(MB)\tThread\t$c\t$d\tEq\n" > $INWF.$INF
    cat $IN | awk 'NR>1' | tr ',' . | awk 'BEGIN {}{
      printf "%.f\t%s\t%.1f\t%.f", $2/(1024*1024), $3, $2/$4, $4/(1024*1024);

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
    }' >> $INWF.$INF
}


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Print compression results
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
OUT="$result/CRYFA_THR__${inDataWF}_$ft.$RES"    # Output file name
c="C_Size(B)\tC_Time_real(s)\tC_Time_user(s)\tC_Time_sys(s)\tC_Mem(KB)"
d="D_Time_real(s)\tD_Time_user(s)\tD_Time_sys(s)\tD_Mem(KB)"

printf "Dataset\tSize(B)\tThread\t$c\t$d\tEq\n" > $OUT;

for nThr in $CRYFA_THR_RUN; do
    CS="";       CT_r="";     CT_u="";     CT_s="";     CM="";
    DT_r="";     DT_u="";     DT_s="";     DM="";       V="";

    ### Compressed file size
    cs_file="$result/CRYFA_THR_${nThr}_CS__${inDataWF}_$ft"
    if [[ -e $cs_file ]]; then
        CS=`cat $cs_file | awk '{ print $5; }'`;
#        rm -f $cs_file;
    fi

    ### Compression time -- real - user - system
    ct_file="$result/CRYFA_THR_${nThr}_CT__${inDataWF}_$ft"
    if [[ -e $ct_file ]]; then
        CT_r=`cat $ct_file |tail -n 3 |head -n 1 | awk '{ print $2;}'`
        CT_u=`cat $ct_file |tail -n 2 |head -n 1 | awk '{ print $2;}'`
        CT_s=`cat $ct_file |tail -n 1 |awk '{ print $2;}'`
#        rm -f $ct_file;
    fi

    ### Compression memory
    cm_file="$result/CRYFA_THR_${nThr}_CM__${inDataWF}_$ft"
    if [[ -e $cm_file ]]; then
        CM=`cat $cm_file`;
#        rm -f $cm_file;
    fi

    ### Decompression time -- real - user - system
    dt_file="$result/CRYFA_THR_${nThr}_DT__${inDataWF}_$ft"
    if [[ -e $dt_file ]]; then
        DT_r=`cat $dt_file |tail -n 3 |head -n 1 | awk '{ print $2;}'`
        DT_u=`cat $dt_file |tail -n 2 |head -n 1 | awk '{ print $2;}'`
        DT_s=`cat $dt_file |tail -n 1 |awk '{ print $2;}'`
#        rm -f $dt_file;
    fi

    ### Decompression memory
    dm_file="$result/CRYFA_THR_${nThr}_DM__${inDataWF}_$ft"
    if [[ -e $dm_file ]]; then
        DM=`cat $dm_file`;
#        rm -f $dm_file;
    fi

    ### Verify if the decompressed and the original files are equal
    v_file="$result/CRYFA_THR_${nThr}_V__${inDataWF}_$ft"
    if [[ -e $v_file ]]; then
        V=`cat $v_file | wc -l`;
#        rm -f $v_file;
    fi

    c="$CS\t$CT_r\t$CT_u\t$CT_s\t$CM"    # Compression results
    d="$DT_r\t$DT_u\t$DT_s\t$DM"         # Decompression results

    printf "$inDataWF\t$fsize\t$nThr\t$c\t$d\t$V\n" >> $OUT;
done

### Make the result file human readable
cryfaThrResHumanReadable $OUT;