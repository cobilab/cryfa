          #######################################################
          #        Run & Results for exploring redundancy       #
          #       - - - - - - - - - - - - - - - - - - - -       #
          #        Morteza Hosseini    seyedmorteza@ua.pt       #
          #        Diogo Pratas        pratas@ua.pt             #
          #        Armando J. Pinho    ap@ua.pt                 #
          #######################################################
#!/bin/bash

RES="$result/REDUNDANCY.$INF"
printf "Method\tSpecies\tStrain\t{C. Ratio}\n" > $RES

for s in E.coli; do

for f in 13E0725; do

#for f in E.coli.strain.13E0725 E.coli.strain.13E0767 E.coli.strain.13E0780 \
#         E.coli.sum0; do
#
#    if [[ ! -e $dataset/$redun/$f.$fasta ]]; then
#        echo "Warning: The file \"$f.$fasta\" is not available in" \
#             "\"$dataset/$redun/\" directory.";
#        return;
#    fi

    inName="$s.strain.$f.$fasta"
    in="../../$dataset/$redun/$inName"

    # MFCompress
    cd $progs/mfcompress
    rm -f $f.mfc

#    ./MFCompressC -o $inName.mfc  $in

    origFilesize=`stat --printf="%s" $in`         # Original file size   (bytes)
    compFileSize=`stat --printf="%s" $inName.mfc` # Compressed file size (bytes)
#    ratio=`echo $origFilesize/$compFileSize | bc -l`

#awk '{ print "mori"; }' > s

ratio=`awk "BEGIN { printf "%s", $origFilesize/$compFileSize; }"`
echo $ratio

#    printf "MFCompress\t%s\t%s\t%s" "$s" "$f" "`awk 'BEGIN { print $origFilesize/$compFileSize; }'`" >> ../../$RES;
#
#      printf "MFCompress\t%s\t%s\t%.1f", $s, $f, $origFilesize;
#    cat "../../$RES" | awk '{
#      echo "MFCompress";
#    }' > s

    cd ../..

#    # DELIMINATE
#    cd $progs/delim
#    rm -f $f.dlim
#
#    ./delim a ../../$dataset/$redun/$f.$fasta
#
#    mv ../../$dataset/$redun/$f.$fasta.dlim  .
#
#    cd ../..
done

done