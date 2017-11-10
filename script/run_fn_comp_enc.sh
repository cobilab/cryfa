          #######################################################
          #     Functions for running compression+encryption    #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

. par.sh        # Internal parameters
. run_fn.sh


### Comp/decomp plus enc/dec. $1: comp program, $2: input data, $3: enc program
function compEncDecDecompress
{
    result_FLD="../../$result"
    in="${2##*/}"                       # input file name
    inwf="${in%.*}"                     # input file name without filetype
    ft="${in##*.}"                      # input filetype
    inPath="${2%/*}"                    # input file's path
    upInComp="$(echo $1 | tr a-z A-Z)"  # compress program's name in uppercase
    upInEnc="$(echo $3 | tr a-z A-Z)"   # encrypt program's name in uppercase

    ### arguments for compression methods
    case $1 in
      "gzip")
          cFT="gz"                      # compressed filetype
          cCmd="gzip"                   # compression command
          cProg="gzip"                  # compress program's name
          dProg="gunzip"                # decompress program's name
          dCmd="gunzip";;               # decompress command

      "bzip2")
          cFT="bz2";             cCmd="bzip2";             cProg="bzip2";
          dProg="bzip2";         dCmd="bunzip2";;

      "lzma")
          cFT="lzma";            cCmd="lzma";              cProg="lzma";
          dProg="lzma";          dCmd="lzma -d";;

      "fqzcomp")
          cFT="fqz";             cCmd="./fqz_comp";        cProg="fqz_comp";
          dProg="fqz_comp";      dCmd="./fqz_comp -d -X";; # "./fqz_comp -d"

      "quip")
          cFT="qp";              cCmd="./quip -c";         cProg="quip";
          dProg="quip";          dCmd="./quip -d -c";;

      "dsrc")
          cFT="dsrc";            cCmd="./dsrc c -m2";      cProg="dsrc";
          dProg="dsrc";          dCmd="./dsrc d";;

      "delim")
          cFT="dlim";            cCmd="./delim a";         cProg="delim";
          dProg="delim";         dCmd="./delim e";;

      "fqc")
          cFT="fqc";             cCmd="./fqc -c";          cProg="fqc";
          dProg="fqc";           dCmd="./fqc -d";;

      "mfcompress")
          cFT="mfc";             cCmd="./MFCompressC";     cProg="MFCompress";
          dProg="MFCompress";    dCmd="./MFCompressD";;
    esac

    ### arguments for encryption methods
    case $3 in
      "aescrypt")
          enFT="aescrypt"                        # encrypted filetype
          enCmd="./aescrypt -e -k pass.txt"      # encryption command
          deProg="aescrypt"                      # decrypt program's name
          deCmd="./aescrypt -d -k pass.txt";;    # decryption command
    esac

    ### compress
    cd $progs/$1

    progMemoryStart $cProg &
    MEMPID=$!

    rm -f $in $in.$cFT
    case $1 in                                                          # time
      "gzip"|"bzip2"|"lzma")
          (time $cCmd < $2 > $in.$cFT) \
              &> $result_FLD/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;

      "cryfa"|"quip"|"fqzcomp")
          (time $cCmd $2 > $in.$cFT) \
              &> $result_FLD/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;

      "dsrc")
          (time $cCmd $2 $in.$cFT) \
              &> $result_FLD/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;

      "delim")
          (time $cCmd $2) \
              &> $result_FLD/${upInComp}_${upInEnc}_CT__${inwf}_$ft
          mv $inPath/$in.$cFT $in.$cFT;;

      "fqc")
          (time $cCmd -i $2 -o $in.$cFT) \
              &>$result_FLD/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;

      "mfcompress")
          (time $cCmd -o $in.$cFT $2) \
              &> $result_FLD/${upInComp}_${upInEnc}_CT__${inwf}_$ft;;
    esac

    ls -la $in.$cFT > $result_FLD/${upInComp}_${upInEnc}_CS__${inwf}_$ft #size
    progMemoryStop $MEMPID \
                   $result_FLD/${upInComp}_${upInEnc}_CM__${inwf}_$ft    # mem

    ### encrypt
    cd ../$3
    compPath="../$1"    # path of compressed file

    progMemoryStart $3 &
    MEMPID=$!

    rm -f $in.$cFT $in.$cFT.$enFT
    case $3 in                                                          # time
      "aescrypt")
          (time $enCmd -o $in.$cFT.$enFT $compPath/$in.$cFT) \
              &> $result_FLD/${upInComp}_${upInEnc}_EnT__${inwf}_$ft;;
    esac

    ls -la $in.$cFT.$enFT >$result_FLD/${upInComp}_${upInEnc}_EnS__${inwf}_$ft
    progMemoryStop $MEMPID \
                   $result_FLD/${upInComp}_${upInEnc}_EnM__${inwf}_$ft   # mem

    ### decrypt
    progMemoryStart $deProg &
    MEMPID=$!

    case $3 in                                                          # time
      "aescrypt")
          (time $deCmd -o $in.$cFT $in.$cFT.$enFT) \
              &> $result_FLD/${upInComp}_${upInEnc}_DeT__${inwf}_$ft;;
    esac

    progMemoryStop $MEMPID \
                   $result_FLD/${upInComp}_${upInEnc}_DeM__${inwf}_$ft  # mem

    ### decompress
    cd ../$1
    encPath="../$3"    # path of encrypted file

    progMemoryStart $dProg &
    MEMPID=$!

    case $1 in                                                          # time
      "gzip"|"bzip2"|"lzma")
          (time $dCmd < $encPath/$in.$cFT> $in) \
              &> $result_FLD/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;

      "cryfa"|"fqzcomp"|"quip")
          (time $dCmd $encPath/$in.$cFT > $in) \
              &> $result_FLD/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;

      "dsrc"|"delim")
          (time $dCmd $encPath/$in.$cFT $in) \
              &> $result_FLD/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;

      "fqc")
          (time $dCmd -i $encPath/$in.$cFT -o $in) \
              &> $result_FLD/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;

      "mfcompress")
          (time $dCmd -o $in $encPath/$in.$cFT) \
              &> $result_FLD/${upInComp}_${upInEnc}_DT__${inwf}_$ft;;
    esac

    progMemoryStop $MEMPID \
                   $result_FLD/${upInComp}_${upInEnc}_DM__${inwf}_$ft    # mem

    ### verify if input and decompressed files are the same
    cmp $2 $in &> $result_FLD/${upInComp}_${upInEnc}_V__${inwf}_$ft;

    cd ../..
}


### Compress/decompress plus encrypt/decrypt on datasets.
### $1: compression program, $2: filetype, $3: encryption program
function compEncDecDecompOnDataset
{
    methodComp="$(echo $1 | tr A-Z a-z)"    # comp method's name in lower case
    methodEnc="$(echo $3 | tr A-Z a-z)"     # enc  method's name in lower case
    if [[ ! -d $progs/$methodComp ]]; then mkdir -p $progs/$methodComp; fi
    if [[ ! -d $progs/$methodEnc ]];  then mkdir -p $progs/$methodEnc;  fi
    dsPath="../../$dataset"

    case $2 in
      "fa"|"FA"|"fasta"|"FASTA")   # FASTA -- human - viruses - synthetic
          compEncDecDecompress \
                  $methodComp $dsPath/$FA/$HUMAN/HS.$fasta $methodEnc
          compEncDecDecompress \
                  $methodComp $dsPath/$FA/$VIRUSES/viruses.$fasta $methodEnc
          for i in 1 2; do
              compEncDecDecompress \
                  $methodComp $dsPath/$FA/$Synth/SynFA-$i.$fasta $methodEnc
          done;;

      "fq"|"FQ"|"fastq"|"FASTQ")   # FASTQ -- human - Denisova - synthetic
          for i in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 \
                   SRR707196_1; do
              compEncDecDecompress \
                  $methodComp $dsPath/$FQ/$HUMAN/$HUMAN-$i.$fastq $methodEnc
          done
          for i in B1087 B1088 B1110 B1128 SL3003; do
              compEncDecDecompress $methodComp \
                  $dsPath/$FQ/$DENISOVA/$DENISOVA-${i}_SR.$fastq $methodEnc
          done
          for i in 1 2; do
              compEncDecDecompress \
                  $methodComp $dsPath/$FQ/$Synth/SynFQ-$i.$fastq $methodEnc
          done;;
    esac
}