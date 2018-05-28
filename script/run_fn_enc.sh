          #######################################################
          #          Functions for running encryption           #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

. $script/run_fn.sh    # Common Functions

### Encrypt/decrypt. $1: program's name, $2: input data
function encDecrypt
{
    result_FLD="../../$result"
    in="${2##*/}"                            # Input file name
    inwf="${in%.*}"                          # Input file name without filetype
    ft="${in##*.}"                           # Input filetype
    inPath="${2%/*}"                         # Input file's path
    upIn="$(echo $1 | tr a-z A-Z)"           # Input program's name in uppercase
    case $1 in
      "aescrypt")
          enFT="aescrypt"                    # Encrypted filetype
          enCmd="./aescrypt -e -k pass.txt"  # Encryption command
          enProg="aescrypt"                  # Encrypt program's name
          deProg="aescrypt"                  # Decrypt program's name
          deCmd="./aescrypt -d -k pass.txt";;# Decryption command
    esac

    ### Encrypt
    progMemoryStart $enProg &
    MEMPID=$!

    rm -f $in.$enFT
    case $1 in                                                     # Time
      "aescrypt")
          (time $enCmd -o $in.$enFT $2) &>$result_FLD/${upIn}_EnT__${inwf}_$ft;;
    esac

    ls -la $in.$enFT > $result_FLD/${upIn}_EnS__${inwf}_$ft        # Size
    progMemoryStop $MEMPID $result_FLD/${upIn}_EnM__${inwf}_$ft    # Memory

    # Wait 2 seconds
    sleep 2s

    ### Decrypt
    progMemoryStart $deProg &
    MEMPID=$!

    case $1 in                                                     # Time
      "aescrypt")
          (time $deCmd -o $in $in.$enFT)&>$result_FLD/${upIn}_DeT__${inwf}_$ft;;
    esac

    progMemoryStop $MEMPID $result_FLD/${upIn}_DeM__${inwf}_$ft    # Memory

    # Wait 2 seconds
    sleep 2s
}


### Encrypt/decrypt on datasets. $1: program's name
function encDecOnDataset
{
    method="$(echo $1 | tr A-Z a-z)"    # Method's name in lower case
    if [[ ! -d $progs/$method ]]; then mkdir -p $progs/$method; fi
    cd $progs/$method
    dsPath=../../$dataset

    case $1 in
      "aescrypt")   # AEScrypt
          # FASTA
          encDecrypt $method $dsPath/$FA/$HUMAN/HS.$fasta
          encDecrypt $method $dsPath/$FA/$VIRUSES/viruses.$fasta
          for i in 1 2; do
              encDecrypt $method $dsPath/$FA/$Synth/SynFA-$i.$fasta
          done

          # FASTQ
          for i in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 \
                   SRR707196_1; do
              encDecrypt $method $dsPath/$FQ/$HUMAN/$HUMAN-$i.$fastq
          done
          for i in B1087 B1088; do
              encDecrypt $method $dsPath/$FQ/$DENISOVA/$DENISOVA-${i}_SR.$fastq
          done
          for i in 1 2; do
              encDecrypt $method $dsPath/$FQ/$Synth/SynFQ-$i.$fastq
          done

          # VCF
          encDecrypt $method $dsPath/$VCF/$DENISOVA/DS-22.$vcf
          encDecrypt $method $dsPath/$VCF/$NEANDERTHAL/N-n.$vcf

          # SAM
          encDecrypt $method $dsPath/$SAM/$HUMAN/HS-n.$sam
          encDecrypt $method $dsPath/$SAM/$NEANDERTHAL/N-y.$sam

          # BAM
          encDecrypt $method $dsPath/$BAM/$HUMAN/HS-11.$bam
          encDecrypt $method $dsPath/$BAM/$NEANDERTHAL/N-21.$bam;;
    esac

    rm -f mem_ps

    cd ../..
}