          #######################################################
          #          Functions for running encryption           #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

. par.sh        # Parameters
. run_fn.sh


### Encrypt/decrypt. $1: program's name, $2: input data
function encDecrypt
{
    result_FLD="../../$result"
    in="${2##*/}"                     # input file name
    inwf="${in%.*}"                   # input file name without filetype
    ft="${in##*.}"                    # input filetype
    inPath="${2%/*}"                  # input file's path
    upIn="$(echo $1 | tr a-z A-Z)"    # input program's name in uppercase

    case $1 in
      "aescrypt")
          enFT="aescrypt"                        # encrypted filetype
          enCmd="./aescrypt -e -k pass.txt"      # encryption command
          enProg="aescrypt"                      # encrypt program's name
          deProg="aescrypt"                      # decrypt program's name
          deCmd="./aescrypt -d -k pass.txt";;    # decryption command
    esac

    ### encrypt
    progMemoryStart $enProg &
    MEMPID=$!

    rm -f $in.$enFT
    case $1 in                                                     # time
      "aescrypt")
          (time $enCmd -o $in.$enFT $2) \
              &> $result_FLD/${upIn}_EnT__${inwf}_$ft;;
    esac

    ls -la $in.$enFT > $result_FLD/${upIn}_EnS__${inwf}_$ft        # size
    progMemoryStop $MEMPID $result_FLD/${upIn}_EnM__${inwf}_$ft    # memory

    ### decrypt
    progMemoryStart $deProg &
    MEMPID=$!

    case $1 in                                                     # time
      "aescrypt")
          (time $deCmd -o $in $in.$enFT) \
              &> $result_FLD/${upIn}_DeT__${inwf}_$ft;;
    esac

    progMemoryStop $MEMPID $result_FLD/${upIn}_DeM__${inwf}_$ft    # memory
}


### Encrypt/decrypt on datasets. $1: program's name
function encDecOnDataset
{
    method="$(echo $1 | tr A-Z a-z)"    # method's name in lower case
    if [[ ! -d $progs/$method ]]; then mkdir -p $progs/$method; fi
    cd $progs/$method
    dsPath=../../$dataset

    case $1 in
      "aescrypt")   # AES crypt
          # FASTA
          encDecrypt $method $dsPath/$FA/$HUMAN/HS.$fasta
          encDecrypt $method $dsPath/$FA/$VIRUSES/viruses.$fasta
          for i in 1 2; do
              encDecrypt $method $dsPath/$FA/$Synth/SynFA-$i.$fasta
          done

          # FASTQ
          for i in ERR013103_1 ERR015767_2 ERR031905_2 \
                   SRR442469_1 SRR707196_1; do
              encDecrypt $method $dsPath/$FQ/$HUMAN/$HUMAN-$i.$fastq
          done
          for i in B1087 B1088 B1110 B1128 SL3003; do
              encDecrypt $method \
                         $dsPath/$FQ/$DENISOVA/$DENISOVA-${i}_SR.$fastq
          done
          for i in 1 2; do
              encDecrypt $method $dsPath/$FQ/$Synth/SynFQ-$i.$fastq
          done;;
    esac

    cd ../..
}