#!perl -w
use strict;
use Getopt::Long;
use File::Basename;
use Cwd qw(abs_path);

my $usage = <<'USAGE';

Step 3 of  K-Fold Cross Validation.
Calculate scores for testing samples.
Version 1.0 : 20200906
Author : lianghan@genomics.cn,  algorithm
Author : lifuqiang@genomics.cn, pipeline

Options:
        -validatematrix     FILE   The testing matrix to predict.  <require>
                                   e.g. Recurrent.matrix.csv.gz
        -validatesample     FILE   The sample list of validatematrix. <require>
                                   e.g. Recurrent.SampleInfo.txt.gz
        -groupApattern      FILE   The balanced patterns of groupA.  <require>
                                   e.g. Tx.Healthy.Primary.Cov50.balance500.txt.gz
        -groupBpattern      FILE   The balanced patterns of groupB.  <require>
                                   e.g. Tx.Primary.Healthy.Cov50.balance500.txt.gz
        -groupAname         STR    The groupA name. <require>
                                   e.g. Healthy. Should be the same with that used at K_Fold_Cross_Validation.Train-v1.0.pl
        -groupBname         STR    The groupB name. <require>
                                   e.g. Primary. Should be the same with that used at K_Fold_Cross_Validation.Train-v1.0.pl
        -balance            EXE    1) Program to make number of patterns balance between groupA and groupB.
                                   2) Program to calculate pattern score.
                                   [/ldfssz1/ST_CANCER/POL/USER/lianghan/backup/tree/balance.plus]
        -output             FILE   The gzip output file. <require>
                                   e.g. Recurrent.Cov50.balance500.score.txt.gz
        -h                         Help information
USAGE

my($validatematrix, $validatesample, $groupApattern, $groupBpattern, $groupAname, $groupBname, $balance, $output);

GetOptions(
        'validatematrix=s'  =>       \$validatematrix,
        'validatesample=s'  =>       \$validatesample,
        'groupApattern=s'   =>       \$groupApattern,
        'groupBpattern=s'   =>       \$groupBpattern,
        'groupAname=s'      =>       \$groupAname,
        'groupBname=s'      =>       \$groupBname,
        'balance=s'         =>       \$balance,
        'output=s'          =>       \$output,
        'h|?'               =>       sub{die "$usage\n";},
);
die "$usage\n" unless (defined($validatematrix) && defined($validatesample) && defined($groupApattern) && defined($groupBpattern) && defined($groupAname) && defined($groupBname) && defined($output));

if($groupApattern eq $groupBpattern){die "The files of groupA ($groupApattern) and groupB ($groupBpattern) should not the same!\n"}
if($groupAname eq $groupBname){die "The names of groupA ($groupAname) and groupB ($groupBname) should not the same!\n"}

unless(-e $validatematrix){die "The $validatematrix not exists!\n"}
$validatematrix = abs_path($validatematrix);
unless(-e $validatesample){die "The $validatesample not exists!\n"}
$validatesample = abs_path($validatesample);
unless(-e $groupApattern){die "The $groupApattern not exists!\n"}
$groupApattern = abs_path($groupApattern);
unless(-e $groupBpattern){die "The $groupBpattern not exists!\n"}
$groupBpattern = abs_path($groupBpattern);
$balance ||= "/ldfssz1/ST_CANCER/POL/USER/lianghan/backup/tree/balance.plus";
unless(-e $balance){die "The $balance not exists!\n"}
$balance = abs_path($balance);

my $groupAscore = $groupApattern; 
$groupAscore =~ s/(T[0-9,x]+\.$groupAname)\.$groupBname\.(.+)\.txt([^\/]+)$/$1\.Train\.$2\.score.txt$3/;
unless(-e $groupAscore){die "The $groupAscore not exists!\n"}
my $groupBscore = $groupBpattern; 
$groupBscore =~ s/(T[0-9,x]+\.$groupBname)\.$groupAname\.(.+)\.txt([^\/]+)$/$1\.Train\.$2\.score.txt$3/;
unless(-e $groupBscore){die "The $groupBscore not exists!\n"}

my @train_Dscore;
for my $filescore ($groupAscore, $groupBscore){
        my $col = "NA";

        if($filescore =~ /\.gz$/){open IB, "gzip -dc $filescore |" || die $!;}
        else{open IB, $filescore || die $!;}
        while(<IB>){
                chomp;
                my @F = split /\t/, $_;
                if($F[0] eq "#Sample"){
                        for (my $i=0; $i<=$#F; $i++){
                                if ($F[$i] eq "D-score"){
                                        $col = $i;
                                }
                        }
                }
                next if (/^#/);
                push @train_Dscore, $F[$col];
        } 
        close IB;
}
my ($train_Dscore_mean, $train_Dscore_sd) = &Mean_StandardDeviation(\@train_Dscore);

# prediction score for validation samples
system("bash", "-c", "$balance --input0 <(gzip -dc $validatematrix) --complete0 <(gzip -dc $groupApattern) --complete1 <(gzip -dc $groupBpattern) > $output.tmp");
if($output =~ /\.gz$/){open OA, "| gzip > $output" || die $!;}
else{open OA, "> $output" || die $!;}
print OA "##D-score = $groupAname\_raw-score - $groupBname\_raw-score\n";
print OA "##Dz-score = $groupAname\_z-score - $groupBname\_z-score\n";
print OA "#Sample\t$groupAname\_raw-score\t$groupBname\_raw-score\tD-score\t$groupAname\_z-score\t$groupBname\_z-score\tDz-score\n";
open IA, "$output.tmp" || die $!;
if($validatesample =~ /\.gz$/){open SA, "gzip -dc $validatesample |" || die $!;}
else{open SA, $validatesample || die $!;}
while(my $samp=<SA>, my $line=<IA>){
        chomp($samp);
        chomp($line);
        my @F = split /,/, $line;
        my $dscore = $F[0] - $F[1];
        my $groupA_zscore = ($F[0]-$train_Dscore_mean)/$train_Dscore_sd;
        my $groupB_zscore = ($F[1]-$train_Dscore_mean)/$train_Dscore_sd;
        my $dzscore = $groupA_zscore - $groupB_zscore;
        print OA "$samp\t$F[0]\t$F[1]\t$dscore\t$groupA_zscore\t$groupB_zscore\t$dzscore\n";
}
close OA;
close IA;
close SA;
if($output =~ /\.gz$/){system("bash", "-c", "gzip -t $output");}
system("bash", "-c", "rm -rf $output.tmp");

sub Mean_StandardDeviation{
        my @F = @{$_[0]};
        my $sum = 0;
        $sum += $_ for(@F);
        my $mean = $sum/scalar(@F);
        my $var = 0;
        $var += ($_-$mean)*($_-$mean) for(@F);
        $var /= (scalar(@F) - 1);
        my $sd = sqrt($var);
        ($mean,$sd);
}