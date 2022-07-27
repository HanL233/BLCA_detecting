#!perl -w
use strict;
use Cwd;
use Cwd 'abs_path';
die "usage: <FI: *.cytosine_report.txt.gz> <FI: *.fixedBin_500bp.bed> <STR: outdir> <STR: sample name>\n" if (@ARGV != 4);

my $bedtools = "/ldfssz1/ST_CANCER/POL/SHARE/tools/bedtools/v2.26.0/bin/bedtools";
my $pee = "/ldfssz1/ST_CANCER/POL/USER/lifuqiang/tools/anaconda2/bin/pee";
my $cyfile = $ARGV[0];
my $bedfile = $ARGV[1];
my $outdir = abs_path($ARGV[2]);
my $samp = $ARGV[3];

unless(-e $outdir){system("bash",  "-c", "mkdir -p $outdir")}

open OA, "| gzip > $outdir/$samp.cytosine_report.fixedBin_500bp.bed.gz" || die $!;
print OA "#CytosineChr\tCytosineStart\tCytosineEnd\tCytosineMethylReads\tCytosineUnmethylReads\tCytosineType\tBinChr\tBinStart\tBinEnd\tCaptureRegion\tBinLength\n";
close OA;

system("bash",  "-c", "zcat $cyfile | perl -wlane 'print join \"\\t\", \$F[0],\$F[1]-1,\$F[1],\$F[3],\$F[4],\$F[5]' | $bedtools intersect -a stdin -b $bedfile -wa -wb | $pee 'gzip >> $outdir/$samp.cytosine_report.fixedBin_500bp.bed.gz' 'wc -l > $outdir/$samp.cytosine_report.fixedBin_500bp.bed.gz.stat'");
system("bash",  "-c", "gzip -t $outdir/$samp.cytosine_report.fixedBin_500bp.bed.gz");

my (%sum, %count);
my $inlinecount1 = 0;
# only chr1~chr22, and BinLength >= 500
open IA, "gzip -dc $outdir/$samp.cytosine_report.fixedBin_500bp.bed.gz |" || die $!;
while(<IA>){
    chomp;
    next if (/^#/);
    $inlinecount1 += 1;
    my @F = split /\t/, $_;
    next if ($F[0] =~ /[XYM_-]/i);
    next if ($F[10] < 500);
    $sum{"$F[6]_$F[7]"}{$F[5]}{"methyl"} += $F[3];
    $sum{"$F[6]_$F[7]"}{$F[5]}{"unmethyl"} += $F[4];
    $count{"$F[6]_$F[7]"}{$F[5]} += 1;
}
close IA;

open SA, "$outdir/$samp.cytosine_report.fixedBin_500bp.bed.gz.stat" || die $!;
while(<SA>){
	chomp;
	if($_ != $inlinecount1){
		die "ERROR: the number rows of $outdir/$samp.cytosine_report.fixedBin_500bp.bed.gz ($inlinecount1) is not the same with the stat $outdir/$samp.cytosine_report.fixedBin_500bp.bed.gz.stat ($_)\n";
	}
	else{
		system("bash",  "-c", "rm -rf $outdir/$samp.cytosine_report.fixedBin_500bp.bed.gz.stat");
	}
}
close SA;

my $inlinecount2 = 4; # number of header lines (start with #)
open OB, "| $pee 'gzip > $outdir/$samp.arctangent.fixedBin_500bp.bed.gz' 'wc -l > $outdir/$samp.arctangent.fixedBin_500bp.bed.gz.stat'" || die $!;
print OB "##FORMAT=<ID=NC,Description=\"Number of cytosine sites in the bin\">\n";
print OB "##FORMAT=<ID=DC,Description=\"Sum of alignments supporting unmethylation for cytosine sites in the bin, sum of alignments supporting methylation for cytosine sites in the bin\">\n";
print OB "##FORMAT=<ID=AC,Description=\"Arctangent value between unmethylation and methylation\">\n";
print OB "#BinChr\tBinStart\tBinEnd\tCaptureRegion\tBinLength\tFORMAT\tCG\tCHH\tCHG\tALL\n";

open IB, $bedfile || die $!;
while(<IB>){
    chomp;
    next if (/^#/);
    my @F = split /\t/, $_;
    next if ($F[0] =~ /[XYM_-]/i);
    next if ($F[4] < 500);
    $inlinecount2 += 1;
    print OB "$_\tNC:DC:AC"; 
    for my $cytype ("CG", "CHH", "CHG", "ALL"){
	unless(exists $sum{"$F[0]_$F[1]"}){
		print STDERR "Missing Bin: $_\n";
	}
        $sum{"$F[0]_$F[1]"}{$cytype}{"methyl"} ||= 0;
        $sum{"$F[0]_$F[1]"}{$cytype}{"unmethyl"} ||= 0;
        $count{"$F[0]_$F[1]"}{$cytype} ||= 0;
	unless($cytype eq "ALL"){
		$sum{"$F[0]_$F[1]"}{"ALL"}{"methyl"} += $sum{"$F[0]_$F[1]"}{$cytype}{"methyl"};
		$sum{"$F[0]_$F[1]"}{"ALL"}{"unmethyl"} += $sum{"$F[0]_$F[1]"}{$cytype}{"unmethyl"};
		$count{"$F[0]_$F[1]"}{"ALL"} += $count{"$F[0]_$F[1]"}{$cytype};
	}

	my $sumUnmethyl = $sum{"$F[0]_$F[1]"}{$cytype}{"unmethyl"};
	my $sumMethyl = $sum{"$F[0]_$F[1]"}{$cytype}{"methyl"};
	my $countCytosine = $count{"$F[0]_$F[1]"}{$cytype};
        my $arctan_methyl = atan2($sumUnmethyl, $sumMethyl);
        print OB "\t$countCytosine:$sumUnmethyl,$sumMethyl:$arctan_methyl";
    }
    print OB "\n";
}
close IB;
close OB;

system("bash",  "-c", "gzip -t $outdir/$samp.arctangent.fixedBin_500bp.bed.gz");

open SB, "$outdir/$samp.arctangent.fixedBin_500bp.bed.gz.stat" || die $!;
while(<SB>){
        chomp;
        if($_ != $inlinecount2){
                die "ERROR: the number rows of $outdir/$samp.arctangent.fixedBin_500bp.bed.gz ($inlinecount2) is not the same with the stat $outdir/$samp.arctangent.fixedBin_500bp.bed.gz.stat ($_)\n";
        }
	else{
		system("bash",  "-c", "rm -rf $outdir/$samp.arctangent.fixedBin_500bp.bed.gz.stat");
	}
}
close SB;

