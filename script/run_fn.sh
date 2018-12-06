          #######################################################
          #         Common functions for running methods        #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

### Memory
function progMemoryStart
{
    echo "0" > mem_ps;
    while true; do
        ps -T aux | grep $1 | grep -v "grep --color=auto" | awk '{print $7}' \
         | sort -V | tail -n 1 >> mem_ps;

#        ps aux | grep $1 | awk '{print $6}' | sort -V | tail -n +2 \
#         | awk 'BEGIN {tot=0} {tot+=$1} END {print tot}' >> mem_ps;

        sleep 0.001;    # 1 milisecond
    done
}
function progMemoryStop
{
    kill $1 >/dev/null 2>&1
    < mem_ps sort -V | tail -n 1 > $2;
}


### Memory2
#function progMemory2
#{
#    valgrind --tool=massif --pages-as-heap=yes \
#             --massif-out-file=massif.out ./$1
#    cat massif.out | grep mem_heap_B | sed -e 's/mem_heap_B=\(.*\)/\1/' | \
#    sort -g | tail -n 1
#}


### Time
function progTime
{
    time ./$1
}


### Methods' names for printing in the result table
function printMethodName
{
    methodUpCase="$(echo $1 | tr a-z A-Z)"

    case $methodUpCase in
      "GZIP")                  echo "gzip";;
      "BZIP2")                 echo "bzip2";;
      "MFCOMPRESS")            echo "MFCompress";;
      "DELIM"|"DELIMINATE")    echo "DELIMINATE";;
      "CRYFA")                 echo "Cryfa";;
      "FQZCOMP")               echo "fqzcomp";;
      "QUIP")                  echo "Quip";;
      "DSRC")                  echo "DSRC";;
      "FQC")                   echo "FQC";;
      "AESCRYPT")              echo "AESCrypt";;
    esac
}