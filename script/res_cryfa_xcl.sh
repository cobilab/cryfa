          #######################################################
          #                    Cryfa results                    #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

. par.sh        # Internal parameters


# convert memory numbers scale to MB and times to fractional minutes in
# result files associated with cryfa method. $1: input file name
function cryfaXclResHumanReadable
{
    IN=$1              # input file name
    INWF="${IN%.*}"    # input file name without filetype

    c="C_Size(B)\tC_Time_real(s)\tC_Time_cpu(s)\tC_Mem(KB)"
    d="D_Time_real(s)\tD_Time_cpu(s)\tD_Mem(KB)"
    printf "Dataset\tSize(B)\tThread\t$c\t$d\tEq\n" > ${INWF}_detail.$INF

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
    printf "\t%.3f\t%.f", c_cpuTime, $8;

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
    printf "\t%.3f\t%.f\t%d\n", d_cpuTime, $12, $13;
    }' >> ${INWF}_detail.$INF

    c_each="C_Ratio\tC_Speed(MB/s)\tC_Time_cpu(m)\tC_Mem(MB)"
    d_each="D_Speed(MB/s)\tD_Time_cpu(m)\tD_Mem(MB)"
    printf "Size(MB)\tThread\t$c_each\t$d_each\tEq\n" > $INWF.$INF
    cat ${INWF}_detail.$INF | awk 'NR>1' | awk 'BEGIN{}{
     printf "%.f\t%s\t%.1f\t%.f\t%.2f\t%.f\t%.f\t%.2f\t%.f\t%.1f\n",
     $2/(1024*1024), $3, $2/$4, $2/(1024*1024*$5), $6/60, $7/1024,
     $4/(1024*1024*$8), $9/60, $10/1024, $11;
     }'  >> $INWF.$INF
}