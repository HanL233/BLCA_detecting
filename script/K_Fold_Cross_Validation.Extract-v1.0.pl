#!perl -w
use strict;
use Getopt::Long;
use File::Basename;
use Cwd qw(abs_path);
use List::Util qw(shuffle); # Returns the values of the input in a random order, https://perldoc.perl.org/List/Util.html

my $usage = <<'USAGE';

Step 1 of  K-Fold Cross Validation.
Extract sub-matrix of bin (or window) readcount (or arctangent unmathylation, ...).
Version 1.0 : 20200906
Author : lianghan@genomics.cn,  algorithm
Author : lifuqiang@genomics.cn, pipeline

Options:
        -groupInfo          FILE   Group info of samples for training and testing.  <require>
                                   Sample   Group
        -matrix             FILE   Readcount (or arctangent unmathylation, ...) matrix of all samples. <require>
        -kfold              INT    Specific K value for K-Fold Cross Validation. <require>
                                   Should be >= 1. Recommendation is 10.
        -seed               INT    Specitic seed value for reproducible shuffle (random). <require>
                                   Samples will be sorted randomly. The same seed will generate reproducible sub-matrix.
                                   Note1: Returns the values of the input in a random order, https://perldoc.perl.org/List/Util.html
                                   Note2: Reproducible shuffle, https://www.perlmonks.org/?node_id=665918
        -outdir             DIR    The output directory. <require>
        -pee                EXE    Program that tee standard input to pipes. [/ldfssz1/ST_CANCER/POL/USER/lifuqiang/tools/anaconda2/bin/pee]
        -h                         Help information
USAGE

my($groupinfo, $matrix, $kfold, $seed, $outdir, $pee);

GetOptions(
        'groupInfo=s'       =>       \$groupinfo,
        'matrix=s'          =>       \$matrix,
        'kfold=i'           =>       \$kfold,
        'seed=i'            =>       \$seed,
        'outdir=s'          =>       \$outdir,
        'pee=s'             =>       \$pee,
        'h|?'               =>       sub{die "$usage\n";},
);
die "$usage\n" unless (defined($groupinfo) && defined($matrix)  && defined($kfold)  && defined($seed) && defined($outdir));
unless(-e $groupinfo){die "The $groupinfo not exists!\n"}
$groupinfo = abs_path($groupinfo);
unless(-e $matrix){die "The $matrix not exists!\n"}
$matrix = abs_path($matrix);
unless(-e $outdir){system("bash",  "-c", "mkdir -p $outdir")}
$outdir = abs_path($outdir);
$pee ||= "/ldfssz1/ST_CANCER/POL/USER/lifuqiang/tools/anaconda2/bin/pee";
unless(-e $pee){die "The $pee not exists!\n"}
unless(int($kfold) == $kfold){die "The kfold should bed INT, whereas $kfold was provided!\n"}
srand($seed); # reproducible shuffle, https://www.perlmonks.org/?node_id=665918

my $groupinfoname = basename($groupinfo);
$groupinfoname =~ s/\.txt$//;
my (%roundH, %groupH, %count, %pattern);

system("bash",  "-c", "mkdir -p $outdir/Tx"); # use all samples from groupA and groupB for training.

if($groupinfo =~ /\.gz$/){open IA, "gzip -dc $groupinfo |" || die $!;}
else{open IA, $groupinfo || die $!;}
while(<IA>){
    chomp;
    next if (/^#/);
    my ($samp, $group) = (split /\t/, $_)[0, 1];
    push @{$groupH{$group}}, $samp;
    $pattern{"Tx"}{$group}{"Train"}{"extract"} .= "$samp,|";
    $pattern{"Tx"}{$group}{"Train"}{"substitute"} .= " -e 's/$samp,//' ";
}
close IA;

for my $group (sort keys %groupH){
    my @groupR = shuffle @{$groupH{$group}};
    my $numTest = int(@groupR / $kfold);
    my $remainder = @groupR % $kfold;
    my @arrayTest;
    for (my $i=0; $i<$kfold; $i++){
        if($i < $remainder){
            push @arrayTest, ($numTest + 1);
        }
        else{
            push @arrayTest, $numTest;
        }
    }
    print STDERR "Number of samples for testing with $group samples at each round:\t", join ("\t", @arrayTest), "\n";

    open OA, "| gzip > $outdir/$groupinfoname.SubgroupInfo.txt.gz" || die $!;
    print OA "Sample\tGroup";
    my $sumTest = 0;
    for (my $i=0; $i<$kfold; $i++){
        print OA "\tT$i";
        unless(-e "$outdir/T$i"){system("bash",  "-c", "mkdir -p $outdir/T$i")}

        $sumTest += $arrayTest[$i];
        for (my $j=0; $j<=$#groupR; $j++){
            my $samp = $groupR[$j];
            if($j <= $sumTest - 1 && $j >= $sumTest - $arrayTest[$i]){
                $roundH{$group}{$samp}{"T$i"} = "Test";
                $count{$group}{$samp}{"Test"} += 1;
                $pattern{"T$i"}{$group}{"Test"}{"extract"} .= "$samp,|";
                $pattern{"T$i"}{$group}{"Test"}{"substitute"} .= " -e 's/$samp,//' ";
            }
            else{
                $roundH{$group}{$samp}{"T$i"} = "Train";
                $count{$group}{$samp}{"Train"} += 1;
                $pattern{"T$i"}{$group}{"Train"}{"extract"} .= "$samp,|";
                $pattern{"T$i"}{$group}{"Train"}{"substitute"} .= " -e 's/$samp,//' ";
            }
        }
    }
    print OA "\n";
}

for my $group (sort keys %roundH){
    for my $samp (sort keys %{$roundH{$group}}){
        if($count{$group}{$samp}{"Test"} != 1){
            die "The sample ($samp) in Testing group with $kfold-Fold Cross Validation should be 1 time(s), but $count{$group}{$samp}{Test} present.\n";
        }
        if($count{$group}{$samp}{"Train"} != $kfold - 1){
            die "The sample ($samp) in Training group with $kfold-Fold Cross Validation should be $kfold-1 time(s), but $count{$group}{$samp}{Train} present.\n";
        }
        print OA "$samp\t$group";
        for my $round (sort keys %{$roundH{$group}{$samp}}){
            print OA "\t$roundH{$group}{$samp}{$round}";
        }
        print OA "\n";
    }
}
close OA;
system("bash", "-c", "gzip -t $outdir/$groupinfoname.SubgroupInfo.txt.gz");

for my $round (sort keys %pattern){
    for my $group (sort keys %{$pattern{$round}}){
        for my $type ("Test", "Train"){
            next if ($round eq "Tx" && $type eq "Test");
            my $extract = $pattern{$round}{$group}{$type}{"extract"};
            $extract =~ s/\|$//;
            my $substitute = $pattern{$round}{$group}{$type}{"substitute"};
            system("bash",  "-c", "gzip -dc $matrix | grep -E \"$extract\" | $pee 'cut -f 1 -d \",\" | gzip > $outdir/$round/$round.$group.$type.SampleInfo.txt.gz' 'sed $substitute | gzip > $outdir/$round/$round.$group.$type.matrix.csv.gz'");
            system("bash",  "-c", "gzip -t $outdir/$round/$round.$group.$type.SampleInfo.txt.gz $outdir/$round/$round.$group.$type.matrix.csv.gz");
        }
    }
}
