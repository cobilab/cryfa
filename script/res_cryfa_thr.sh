          #######################################################
          #  Results of cryfa with different number of threads  #
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
function cryfaThrRes
{
    fsize=$(stat --printf="%s" $CRYFA_THR_DATASET)    # File size (bytes)
    CRYFA_THR_RUN=$(seq -s' ' 1 $MAX_N_THR);          # 1 2 3 ... $MAX_N_THR

    for nThr in $CRYFA_THR_RUN; do
        CS="";       CT_r="";     CT_u="";     CT_s="";     CM="";
        DT_r="";     DT_u="";     DT_s="";     DM="";       V="";

        ### Compressed file size
        cs_file="$result/${1}_${nThr}_CS__${2}_$3"
        if [[ -e $cs_file ]]; then  CS=$(awk '{ print $5; }' $cs_file);  fi

        ### Compression time -- real - user - system
        ct_file="$result/${1}_${nThr}_CT__${2}_$3"
        if [[ -e $ct_file ]]; then
            CT_r=$(< $ct_file tail -n 3 | head -n 1 | awk '{ print $2;}')
            CT_u=$(< $ct_file tail -n 2 | head -n 1 | awk '{ print $2;}')
            CT_s=$(< $ct_file tail -n 1 | awk '{ print $2;}')
        fi

        ### Compression memory
        cm_file="$result/${1}_${nThr}_CM__${2}_$3"
        if [[ -e $cm_file ]]; then  CM=$(cat $cm_file);  fi

        ### Decompression time -- real - user - system
        dt_file="$result/${1}_${nThr}_DT__${2}_$3"
        if [[ -e $dt_file ]]; then
            DT_r=$(< $dt_file tail -n 3 | head -n 1 | awk '{ print $2;}')
            DT_u=$(< $dt_file tail -n 2 | head -n 1 | awk '{ print $2;}')
            DT_s=$(< $dt_file tail -n 1 | awk '{ print $2;}')
        fi

        ### Decompression memory
        dm_file="$result/${1}_${nThr}_DM__${2}_$3"
        if [[ -e $dm_file ]]; then  DM=$(cat $dm_file);  fi

        ### Verify if the decompressed and the original files are equal
        v_file="$result/${1}_${nThr}_V__${2}_$3"
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
        printf "$inDataWF\t$fsize\t$nThr\t$c\t$d\t$V\n";
    done
}


### Results of compression/decompression on datasets. $1: output file
function cryfaThrResOnDataset
{
    OUT_FILE=$1
    c="C_Size(B)\tC_Time_real(s)\tC_Time_user(s)\tC_Time_sys(s)\tC_Mem(KB)"
    d="D_Time_real(s)\tD_Time_user(s)\tD_Time_sys(s)\tD_Mem(KB)"

    printf "Dataset\tSize(B)\tThread\t$c\t$d\tEq\n" > $OUT_FILE;

    cryfaThrRes "CRYFA_THR" $inDataWF $ft >> $OUT_FILE;
}


### Convert memory numbers scale to MB and times to fractional minutes in
### result files. $1: input file name
function cryfaThrResHumanReadable
{
    IN=$1              # Input file name
    INWF="${IN%.*}"    # Input file name without filetype

    c="C_Ratio\tC_Size(MB)\tC_Time_real(m)\tC_Time_cpu(m)\tC_Mem(MB)"
    d="D_Time_real(m)\tD_Time_cpu(m)\tD_Mem(MB)"

    printf "Size(MB)\tThread\t$c\t$d\tEq\n" > $INWF.$INF
    awk 'NR>1' $IN | tr ',' . | awk 'BEGIN {}{
      printf "%.f\t%s\t%.1f\t%.f", $2/(1024*1024), $3, $2/$4, $4/(1024*1024);

      split($5, c_arrMinReal, "m");                 c_minReal=c_arrMinReal[1];
      split(c_arrMinReal[2], c_arrSecReal, "s");    c_secReal=c_arrSecReal[1];
      c_realTime=c_minReal*60+c_secReal;

      printf "\t%.2f", c_realTime/60;

      split($6, c_arrMinUser, "m");                 c_minUser=c_arrMinUser[1];
      split(c_arrMinUser[2], c_arrSecUser, "s");    c_secUser=c_arrSecUser[1];
      c_userTime=c_minUser*60+c_secUser;
      split($7, c_arrMinSys, "m");                  c_minSys=c_arrMinSys[1];
      split(c_arrMinSys[2], c_arrSecSys, "s");      c_secSys=c_arrSecSys[1];
      c_sysTime=c_minSys*60+c_secSys;
      c_cpuTime=c_userTime+c_sysTime;

      printf "\t%.2f\t%.f", c_cpuTime/60, $8/1024;

      split($9, d_arrMinReal, "m");                 d_minReal=d_arrMinReal[1];
      split(d_arrMinReal[2], d_arrSecReal, "s");    d_secReal=d_arrSecReal[1];
      d_realTime=d_minReal*60+d_secReal;

      printf "\t%.2f", d_realTime/60;

      split($10, d_arrMinUser, "m");                d_minUser=d_arrMinUser[1];
      split(d_arrMinUser[2], d_arrSecUser, "s");    d_secUser=d_arrSecUser[1];
      d_userTime=d_minUser*60+d_secUser;
      split($11, d_arrMinSys, "m");                 d_minSys=d_arrMinSys[1];
      split(d_arrMinSys[2], d_arrSecSys, "s");      d_secSys=d_arrSecSys[1];
      d_sysTime=d_minSys*60+d_secSys;
      d_cpuTime=d_userTime+d_sysTime;

      printf "\t%.2f\t%.f\t%d\n", d_cpuTime/60, $12/1024, $13;
    }' >> $INWF.$INF
}


#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#   Print compression results
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
inData="../../$CRYFA_THR_DATASET"
in="${inData##*/}"                            # Input file name
inDataWF="${in%.*}"                           # Input file name without filetype
ft="${in##*.}"                                # Input filetype
OUT="$result/CRYFA_THR__${inDataWF}_$ft.$RES" # Output file name

### Results on datasets
cryfaThrResOnDataset $OUT;

### Make the result file human readable
cryfaThrResHumanReadable $OUT;