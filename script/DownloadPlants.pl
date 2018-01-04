# This script downloads all plants genomes in RefSeq and puts them in plants.fa
# Script is taken from: http://www.ncbi.nlm.nih.gov/books/NBK25498/#chapter3.Application_3_Retrieving_large 
# BY INTERFACE NCBI: http://www.ncbi.nlm.nih.gov/nuccore?term=%22plants%22[PORG]+AND+srcdb_refseq[PROP] , then sent to file: fasta.

use LWP::Simple;

$organism = 'plants';

$query = $organism.'[orgn]+AND+srcdb_refseq[prop]';
print STDERR "Searching RefSeq for $organism: $query\n";
#assemble the esearch URL
$base = 'http://eutils.ncbi.nlm.nih.gov/entrez/eutils/';
$url = $base . "esearch.fcgi?db=nucleotide&term=$query&usehistory=y";

#post the esearch URL
$output = get($url);

#parse WebEnv, QueryKey and Count (# records retrieved)
$web = $1 if ($output =~ /<WebEnv>(\S+)<\/WebEnv>/);
$key = $1 if ($output =~ /<QueryKey>(\d+)<\/QueryKey>/);
$count = $1 if ($output =~ /<Count>(\d+)<\/Count>/);

print STDERR "Found: $count records for $organism\n";
if($count == 0) {
    exit(0);
}

#open output file for writing
open(OUT, ">tmp.$organism.fa") || die "Can't open file!\n";


#retrieve data in batches of 5000
$retmax = 5000;
for ($ret = 0; $ret < $count; ) {
    $efetch_url = $base ."efetch.fcgi?db=nucleotide&WebEnv=$web";
    $efetch_url .= "&query_key=$key&retstart=$ret";
    $efetch_url .= "&retmax=$retmax&rettype=fasta&retmode=text";
    $efetch_out = get($efetch_url);
    $actual_sequences_returned = $efetch_out =~ s/>/\n>/g;  # count number of sequences returned
    $ret += $actual_sequences_returned;
    print OUT "$efetch_out";
    print STDERR "Fetched $ret\n";
}
close OUT;

rename("tmp.$organism.fa", "$organism.fa")
