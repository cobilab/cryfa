          #######################################################
          #      Functions for running compression methods      #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

. $script/run_fn.sh    # Common Functions

### Compress and decompress. $1: program's name, $2: input data
function compDecomp
{
    result_FLD="../../$result"
    in="${2##*/}"                     # Input file name
    inwf="${in%.*}"                   # Input file name without filetype
    ft="${in##*.}"                    # Input filetype
    inPath="${2%/*}"                  # Input file's path
    upIn="$(echo $1 | tr a-z A-Z)"    # Input program's name in uppercase
    case $1 in
      "gzip")
          cFT="gz"                    # Compressed filetype
          cCmd="gzip"                 # Compression command
          cProg="gzip"                # Compress program's name
          dProg="gunzip"              # Decompress program's name
          dCmd="gunzip";;             # Decompress command

      "bzip2")
          cFT="bz2";             cCmd="bzip2";             cProg="bzip2";
          dProg="bzip2";         dCmd="bunzip2";;

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

      "cryfa")
          cFT="cryfa";
          cCmd="./cryfa -k $CRYFA_KEY_FILE -t $CRYFA_DEFAULT_N_THR";
          cProg="cryfa";   dProg="cryfa";
          dCmd="./cryfa -k $CRYFA_KEY_FILE -t $CRYFA_DEFAULT_N_THR -d";;
    esac

    ### Compress
    progMemoryStart $cProg &
    MEMPID=$!

    rm -f $in.$cFT
    case $1 in                                                    # Time
      "gzip"|"bzip2")
          (time $cCmd < $2 > $in.$cFT) &> $result_FLD/${upIn}_CT__${inwf}_$ft;;

      "cryfa"|"quip"|"fqzcomp")
          (time $cCmd $2 > $in.$cFT) &> $result_FLD/${upIn}_CT__${inwf}_$ft;;

      "dsrc")
          (time $cCmd $2 $in.$cFT) &> $result_FLD/${upIn}_CT__${inwf}_$ft;;

      "delim")
          (time $cCmd $2) &> $result_FLD/${upIn}_CT__${inwf}_$ft
          mv $inPath/$in.$cFT $in.$cFT;;

      "fqc")
          (time $cCmd -i $2 -o $in.$cFT)&> $result_FLD/${upIn}_CT__${inwf}_$ft;;

      "mfcompress")
          (time $cCmd -o $in.$cFT $2) &> $result_FLD/${upIn}_CT__${inwf}_$ft;;
    esac

    ls -la $in.$cFT > $result_FLD/${upIn}_CS__${inwf}_$ft         # Size
    progMemoryStop $MEMPID $result_FLD/${upIn}_CM__${inwf}_$ft    # Memory

    # Wait 2 seconds
    sleep 2s

    ### Decompress
    progMemoryStart $dProg &
    MEMPID=$!

    case $1 in                                                    # Time
      "gzip"|"bzip2")
          (time $dCmd < $in.$cFT> $in) &> $result_FLD/${upIn}_DT__${inwf}_$ft;;

      "cryfa"|"fqzcomp"|"quip")
          (time $dCmd $in.$cFT > $in) &> $result_FLD/${upIn}_DT__${inwf}_$ft;;

      "dsrc"|"delim")
          (time $dCmd $in.$cFT $in) &> $result_FLD/${upIn}_DT__${inwf}_$ft;;

      "fqc")
          (time $dCmd -i $in.$cFT -o $in)&>$result_FLD/${upIn}_DT__${inwf}_$ft;;

      "mfcompress")
          (time $dCmd -o $in $in.$cFT) &> $result_FLD/${upIn}_DT__${inwf}_$ft;;
    esac

    progMemoryStop $MEMPID $result_FLD/${upIn}_DM__${inwf}_$ft    # Memory

    # Wait 2 seconds
    sleep 2s

    ### Verify if input and decompressed files are the same
    cmp $2 $in &> $result_FLD/${upIn}_V__${inwf}_$ft;
}


### Compress/decompress on datasets. $1: program's name
function compDecompOnDataset
{
    method="$(echo $1 | tr A-Z a-z)"    # Method's name in lower case
    if [[ ! -d $progs/$method ]]; then  mkdir -p $progs/$method;  fi
    cd $progs/$method
    dsPath=../../$dataset

    case $2 in
      "fa"|"FA"|"fasta"|"FASTA")        # FASTA -- human - viruses - synthetic
          compDecomp $method $dsPath/$FA/$HUMAN/HS.$fasta
          compDecomp $method $dsPath/$FA/$VIRUSES/viruses.$fasta
          for i in 1 2; do
              compDecomp $method $dsPath/$FA/$Synth/SynFA-$i.$fasta
          done;;

      "fq"|"FQ"|"fastq"|"FASTQ")        # FASTQ -- human - Denisova - synthetic
          for i in ERR013103_1 ERR015767_2 ERR031905_2 SRR442469_1 \
                   SRR707196_1; do
              compDecomp $method $dsPath/$FQ/$HUMAN/HS-$i.$fastq
          done
          for i in B1087 B1088; do
              compDecomp $method $dsPath/$FQ/$DENISOVA/DS-${i}_SR.$fastq
          done
          for i in 1 2; do
              compDecomp $method $dsPath/$FQ/$Synth/SynFQ-$i.$fastq
          done;;

      "vcf"|"VCF")                      # VCF -- Denisova - Neanderthal
          compDecomp $method $dsPath/$VCF/$DENISOVA/DS-22.$vcf
          compDecomp $method $dsPath/$VCF/$NEANDERTHAL/N-n.$vcf;;

      "sam"|"SAM")                      # SAM -- human - Neanderthal
          compDecomp $method $dsPath/$SAM/$HUMAN/HS-n.$sam
          compDecomp $method $dsPath/$SAM/$NEANDERTHAL/N-y.$sam;;

      "bam"|"BAM")                      # BAM -- human - Neanderthal
          compDecomp $method $dsPath/$BAM/$HUMAN/HS-11.$bam
          compDecomp $method $dsPath/$BAM/$NEANDERTHAL/N-21.$bam;;
    esac

    rm -f mem_ps

    cd ../..
}