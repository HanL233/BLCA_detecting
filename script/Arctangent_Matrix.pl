#perl -w
use strict;
use Cwd;
use Cwd 'abs_path';
die "usage: perl $0 <FI: arctangent.list> <STR: outdir> <STR: output prefix>\n" if (@ARGV != 3);

my $atanlist = abs_path($ARGV[0]);
my $outdir = abs_path($ARGV[1]);
my $prefix = $ARGV[2];

unless(-e $outdir){system("bash",  "-c", "mkdir -p $outdir")}

open OA, "| gzip > $outdir/$prefix.CG.matrix.csv.gz" || die $!;
open OB, "| gzip > $outdir/$prefix.CHH.matrix.csv.gz" || die $!;
open OC, "| gzip > $outdir/$prefix.CHG.matrix.csv.gz" || die $!;
open OD, "| gzip > $outdir/$prefix.ALL.matrix.csv.gz" || die $!;

my $flagfile = 0;

open LA, $atanlist || die $!;
while(my $line=<LA>){
    chomp($line);
    next if ($line =~ /^#/);
    my ($samp, $file) = (split /\t/, $line)[0, 5];

    if($flagfile == 0){
        my $header = "#Sample";
        open FT, "gzip -dc $file |" || die $!;
        while(<FT>){
            chomp;
            next if (/^#/);
            my @F = split /\t/, $_;
            $header .= ",$F[0]:$F[1]-$F[2]";
        }
        close FT;
        print OA "$header\n";
        print OB "$header\n";
        print OC "$header\n";
        print OD "$header\n";
    }

    my $flagline = 0;
    open FA, "gzip -dc $file |" || die $!;
    while(<FA>){
        chomp;
        next if (/^#/);
        my @F = split /\t/, $_;
        my $atanCG = (split /:/, $F[6])[2];
        my $atanCHH = (split /:/, $F[7])[2];
        my $atanCHG = (split /:/, $F[8])[2];
        my $atanALL = (split /:/, $F[9])[2];
        
        if($flagline == 0){
            print OA "$samp,$atanCG";
            print OB "$samp,$atanCHH";
            print OC "$samp,$atanCHG";
            print OD "$samp,$atanALL";
            $flagline += 1;
        }
        else{
            print OA ",$atanCG";
            print OB ",$atanCHH";
            print OC ",$atanCHG";
            print OD ",$atanALL";
            $flagline += 1;
        }
    } 
    close FA;
    print STDERR "Total $flagline bins for $samp\n";
    print OA "\n";
    print OB "\n";
    print OC "\n";
    print OD "\n";

    $flagfile += 1;
    print STDERR "Total $flagfile files have been processed\n";
}
close LA;
close OA;
close OB;
close OC;
close OD;

system("bash",  "-c", "gzip -t $outdir/$prefix.CG.matrix.csv.gz");
system("bash",  "-c", "gzip -t $outdir/$prefix.CHH.matrix.csv.gz");
system("bash",  "-c", "gzip -t $outdir/$prefix.CHG.matrix.csv.gz");
system("bash",  "-c", "gzip -t $outdir/$prefix.ALL.matrix.csv.gz");
