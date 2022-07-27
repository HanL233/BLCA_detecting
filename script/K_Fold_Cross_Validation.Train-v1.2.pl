#!perl -w
use strict;
use Getopt::Long;
use File::Basename;
use Cwd qw(abs_path);

my $usage = <<'USAGE';

Step 2 of  K-Fold Cross Validation.
Extract distinguish patterns for groupA and groupB from training matrixs respectively.
Version 1.2 : 20200921
Author : lianghan@genomics.cn,  algorithm
Author : lifuqiang@genomics.cn, pipeline

Options:
        -groupA             STR    Name of groupA of training.  <require>
                                   e.g. Healthy
        -groupB             STR    Name of groupB of training.  <require>
                                   e.g. Primary
        -round              STR    Name of the round in K-Fold Cross Validation.  <require>
                                   e.g. T0, extract distinguish patterns by training samples in T0
                                   e.g. Tx, extract distinguish patterns by all the samples of groupA and groupB in Tx
        -traindir           DIR    The directory with training matrixs for groupA and groupB.  <require>
                                   e.g. /path/to/T0
        -tree               EXE    Program to extract frequency alignment (or arctangent unmathylation, ...) distribution patterns from training matrix.
                                   [/ldfssz1/ST_CANCER/POL/USER/lianghan/backup/tree/tree]
        -mincov             FLOAT  Frequncy pattern should cover at least FLOAT samples in a matrix. <require>
                                   e.g. 0.5.  Parameter for tree.
        -fisher             EXE    Program to extract significant patterns that cloud be distinguish groupA and groupB by fisher p-value.
                                   [/ldfssz1/ST_CANCER/POL/USER/lianghan/backup/tree/fisher]
        -balance            EXE    1) Program to make number of patterns balance between groupA and groupB.
                                   2) Program to calculate pattern score.
                                   [/ldfssz1/ST_CANCER/POL/USER/lianghan/backup/tree/balance.plus]
        -maxcut             INT    At most INT patterns will be kept for groupA and groupB separately. <require>
                                   e.g. 500. Parameter for balance.
        -outdir             DIR    The output directory. <require>
                                   e.g. the same with traindir
        -h                         Help information
USAGE

my($groupA, $groupB, $round, $traindir, $tree, $mincov, $fisher, $balance, $maxcut, $outdir);

GetOptions(
        'groupA=s'          =>       \$groupA,
        'groupB=s'          =>       \$groupB,
        'round=s'           =>       \$round,
        'traindir=s'        =>       \$traindir,
        'tree=s'            =>       \$tree,
        'mincov=f'          =>       \$mincov,
        'fisher=s'          =>       \$fisher,
        'balance=s'         =>       \$balance,
        'maxcut=i'          =>       \$maxcut,
        'outdir=s'          =>       \$outdir,
        'h|?'               =>       sub{die "$usage\n";},
);
die "$usage\n" unless (defined($groupA) && defined($groupB) && defined($round) && defined($traindir) && defined($mincov) && defined($maxcut) && defined($outdir));

unless(-e $traindir){die "The $traindir not exists!\n"}
$traindir = abs_path($traindir);
if($groupA eq $groupB){die "The names of groupA ($groupA) and groupB ($groupB) should not the same!\n"}
unless($round =~ /T\d+/ || $round eq "Tx"){die "The round should be 'T\\d+' (e.g. T0) or 'Tx'!\n"}

my (%matrixList, %sampInfoList);
for my $groupname ($groupA, $groupB){
        for my $datatype ("Train", "Test"){
                $matrixList{$groupname}{$datatype} = "$traindir/$round.$groupname.$datatype.matrix.csv.gz";
                $sampInfoList{$groupname}{$datatype} = "$traindir/$round.$groupname.$datatype.SampleInfo.txt.gz";
                if ($round eq "Tx"){
                        unless(-e $matrixList{$groupname}{"Train"}){die "The file $matrixList{$groupname}{Train} not exists!\n"}
                        unless(-e $sampInfoList{$groupname}{"Train"}){die "The file $sampInfoList{$groupname}{Train} not exists!\n"}
                }
                else{
                        unless(-e $matrixList{$groupname}{$datatype}){die "The file $matrixList{$groupname}{$datatype} not exists!\n"}
                        unless(-e $sampInfoList{$groupname}{$datatype}){die "The file $sampInfoList{$groupname}{$datatype} not exists!\n"}
                }
        }
}

unless(-e $outdir){system("bash",  "-c", "mkdir -p $outdir")}
$outdir = abs_path($outdir);

$tree ||= "/ldfssz1/ST_CANCER/POL/USER/lianghan/backup/tree/tree";
unless(-e $tree){die "The $tree not exists!\n"}
$tree = abs_path($tree);
$fisher ||= "/ldfssz1/ST_CANCER/POL/USER/lianghan/backup/tree/fisher";
unless(-e $fisher){die "The $fisher not exists!\n"}
$fisher = abs_path($fisher);
$balance ||= "/ldfssz1/ST_CANCER/POL/USER/lianghan/backup/tree/balance.plus";
unless(-e $balance){die "The $balance not exists!\n"}
$balance = abs_path($balance);

unless($mincov >=0 && $mincov <=1){die "The mincov should be >=0 && <=1, whereas $mincov was provided!\n"}
unless(int($maxcut) == $maxcut){die "The maxcut should be INT, whereas $maxcut was provided!\n"}

my (%pattern);
for my $groupname ($groupA, $groupB){
        my $datatype = "Train";
        my $opgroupname;
        if($groupname eq $groupA){
                $opgroupname = $groupB;
        }
        elsif($groupname eq $groupB){
                $opgroupname = $groupA;
        }

        # extract frequency patterns
        $pattern{$groupname}{$datatype}{"tree"} = "$outdir/$round.$groupname.$datatype.Cov".($mincov*100).".txt.gz";
        system("bash",  "-c", "$tree --input <(gzip -dc $matrixList{$groupname}{$datatype}) --coverage $mincov | gzip > $pattern{$groupname}{$datatype}{tree} && gzip -t $pattern{$groupname}{$datatype}{tree}");

        # p-value of patterns between groupA and groupB
        $pattern{$groupname}{$datatype}{"fisher"} = "$outdir/$round.$groupname.Train.Cov".($mincov*100).".$opgroupname.txt.gz";
        system("bash",  "-c", "$fisher --input <(gzip -dc $matrixList{$opgroupname}{$datatype}) --half <(gzip -dc $pattern{$groupname}{$datatype}{tree}) | gzip > $pattern{$groupname}{$datatype}{fisher} && gzip -t $pattern{$groupname}{$datatype}{fisher}");

        # balance number of patterns between groupA and groupB
        $pattern{$groupname}{$datatype}{"balance"} = "$outdir/$round.$groupname.$opgroupname.Cov".($mincov*100).".balance$maxcut.txt.gz";
        if($groupname eq $groupB){
                system("bash",  "-c", "$balance --input0 <(gzip -dc $matrixList{$groupA}{$datatype}) --input1 <(gzip -dc $matrixList{$groupB}{$datatype}) --complete0 <(gzip -dc $pattern{$groupA}{$datatype}{fisher}) --complete1 <(gzip -dc $pattern{$groupB}{$datatype}{fisher}) --balance0 >(gzip > $pattern{$groupA}{$datatype}{balance}) --balance1 >(gzip > $pattern{$groupB}{$datatype}{balance}) --max $maxcut && gzip -t $pattern{$groupA}{$datatype}{balance} $pattern{$groupB}{$datatype}{balance}");
        }
}

my @train_Dscore;
for my $groupname ($groupA, $groupB){

        my $scorefile = "$outdir/$round.$groupname.Train.Cov".($mincov*100).".balance$maxcut.score.txt.gz";

        # prediction score for training samples
        system("bash",  "-c", "$balance --input0 <(gzip -dc $matrixList{$groupname}{Train}) --complete0 <(gzip -dc $pattern{$groupA}{Train}{balance}) --complete1 <(gzip -dc $pattern{$groupB}{Train}{balance}) > $scorefile.tmp");
        if($scorefile =~ /\.gz$/){open OA, "| gzip > $scorefile" || die $!;}
        else{open OA, "> $scorefile" || die $!;}
        print OA "##D-score = $groupA\_raw-score - $groupB\_raw-score\n";
        print OA "#Sample\t$groupA\_raw-score\t$groupB\_raw-score\tD-score\n";
        open IA, "$scorefile.tmp" || die $!;
        if($sampInfoList{$groupname}{"Train"} =~ /\.gz$/){open SA, "gzip -dc $sampInfoList{$groupname}{Train} |" || die $!;}
        else{open SA, $sampInfoList{$groupname}{"Train"} || die $!;}
        while(my $samp=<SA>, my $line=<IA>){
                chomp($samp);
                chomp($line);
                my @F = split /,/, $line;
                my $dscore = $F[0] - $F[1];
                print OA "$samp\t$F[0]\t$F[1]\t$dscore\n";
                push @train_Dscore, $dscore;

        }
        close OA;
        close IA;
        close SA;
        if($scorefile =~ /\.gz$/){system("bash", "-c", "gzip -t $scorefile");}
        system("bash", "-c", "rm -rf $scorefile.tmp");
}

for my $groupname ($groupA, $groupB){
        if($round ne "Tx"){
                # prediction score for testing samples
                my ($train_Dscore_mean, $train_Dscore_sd) = &Mean_StandardDeviation(\@train_Dscore);

                my $scorefile = "$outdir/$round.$groupname.Test.Cov".($mincov*100).".balance$maxcut.score.txt.gz";
                system("bash", "-c", "$balance --input0 <(gzip -dc $matrixList{$groupname}{Test}) --complete0 <(gzip -dc $pattern{$groupA}{Train}{balance}) --complete1 <(gzip -dc $pattern{$groupB}{Train}{balance}) > $scorefile.tmp");
                if($scorefile =~ /\.gz$/){open OC, "| gzip > $scorefile" || die $!;}
                else{open OC, "> $scorefile" || die $!;}
                print OC "##D-score = $groupA\_raw-score - $groupB\_raw-score\n";
                print OC "##Dz-score = $groupA\_z-score - $groupB\_z-score\n";
                print OC "#Sample\t$groupA\_raw-score\t$groupB\_raw-score\tD-score\t$groupA\_z-score\t$groupB\_z-score\tDz-score\n";
                open IC, "$scorefile.tmp" || die $!;
                if($sampInfoList{$groupname}{"Test"} =~ /\.gz$/){open SC, "gzip -dc $sampInfoList{$groupname}{Test} |" || die $!;}
                else{open SC, $sampInfoList{$groupname}{"Test"} || die $!;}
                while(my $samp=<SC>, my $line=<IC>){
                        chomp($samp);
                        chomp($line);
                        my @F = split /,/, $line;
                        my $dscore = $F[0] - $F[1];
                        my $groupA_zscore = ($F[0]-$train_Dscore_mean)/$train_Dscore_sd;
                        my $groupB_zscore = ($F[1]-$train_Dscore_mean)/$train_Dscore_sd;
                        my $dzscore = $groupA_zscore - $groupB_zscore;
                        print OC join ("\t", $samp, $F[0], $F[1], $dscore, $groupA_zscore, $groupB_zscore, $dzscore), "\n";
                }
                close OC;
                close IC;
                close SC;
                if($scorefile =~ /\.gz$/){system("bash", "-c", "gzip -t $scorefile");}
                system("bash", "-c", "rm -rf $scorefile.tmp");
        }
}

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