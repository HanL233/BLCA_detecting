1. bedtools makewindows -b <(cat ./CaptureRegion/probe.sorted.merge.bed | perl -wlane 'print "$F[0]\t$F[1]\t$F[2]\t$F[0]:$F[1]-$F[2]"') -w 500 -i srcwinnum | perl -wlane 'chomp; if($.==1){print "#BinChr\tBinStart\tBinEnd\tCaptureRegion\tBinLength"} $len=$F[2]-$F[1]; print "$_\t$len"' > probe.sorted.merge.fixedBin_500bp.bed

2. cat clinical.txt | perl -wlane 'chomp; if($F[0]=~/^#/){print "$_\tcytosine_report"}; if(-e "./RRBS/Analysis/$F[0]/03.Methyl/$F[0].cytosine_report.txt.gz"){print "$_\t./RRBS/Analysis/$F[0]/03.Methyl/$F[0].cytosine_report.txt.gz"} if(-e "./RRBS/Analysis2/$F[0]/03.Methyl/$F[0].cytosine_report.txt.gz"){print "$_\t./RRBS/Analysis2/$F[0]/03.Methyl/$F[0].cytosine_report.txt.gz"}' > cytosine_report.list

3. cat cytosine_report.list | perl -wlane 'next if (/^#/); print "perl ./script/Arctangent_unMethylation.pl $F[5] ./RRBS/Advanced_Analysis/D-score/probe.sorted.merge.fixedBin_500bp.bed ./RRBS/Advanced_Analysis/D-score/01Arctangent $F[0]"' > Arctangent_unMethylation.sh
 
4. bash Arctangent_unMethylation.sh

5. cat clinical.txt | perl -wlane 'chomp; if($F[0]=~/^#/){print "$_\tarctangent"}; if(-e "./RRBS/Advanced_Analysis/D-score/01Arctangent/$F[0].arctangent.fixedBin_500bp.bed.gz"){print "$_\t./RRBS/Advanced_Analysis/D-score/01Arctangent/$F[0].arctangent.fixedBin_500bp.bed.gz"}' > arctangent.list

6. perl Arctangent_Matrix.pl arctangent.list ./ BLCA-SGYY.RRBS

7. grep -E "Healthy|Primary" arctangent.list | cut -f 1,5 | sort | uniq > 02TenFoldCV/BLCA-SGYY.Training.Healthy_Primary.txt

8. for i in "ALL" "CG" "CHG" "CHH";do perl K_Fold_Cross_Validation.Extract-v1.0.pl -groupInfo 02TenFoldCV/BLCA-SGYY.Training.Healthy_Primary.txt -matrix ./BLCA-SGYY.RRBS.$i.matrix.csv.gz -kfold 10 -seed 100 -outdir 02TenFoldCV/$i;done

9. for i in "ALL" "CG" "CHG" "CHH";do for j in {0..9} "x";do perl -e 'print "perl ./script/K_Fold_Cross_Validation.Train-v1.0.pl -groupA Healthy -groupB Primary -round $ARGV[1] -traindir ./RRBS/Advanced_Analysis/D-score/02TenFoldCV/$ARGV[0]/$ARGV[1] -mincov 0.5 -maxcut 500 -outdir ./RRBS/Advanced_Analysis/D-score/02TenFoldCV/$ARGV[0]/$ARGV[1]\n"' $i T$j;done;done > 02TenFoldCV/02train.sh
   bash 02TenFoldCV/02train.sh

10. grep -Ff <(awk '$5=="Non-recurrent"{print $1","}' arctangent.list) <(gzip -dc ./BLCA-SGYY.RRBS.ALL.matrix.csv.gz) | pee 'cut -f 1 -d "," | gzip > 03Validation/Non-recurrent.SampleInfo.txt.gz' 'sed -r 's/^[^,]+,//' | gzip > 03Validation/Non-recurrent.matrix.csv.gz'
    grep -Ff <(awk '$5=="Recurrent"{print $1","}' arctangent.list) <(gzip -dc ./BLCA-SGYY.RRBS.ALL.matrix.csv.gz) | pee 'cut -f 1 -d "," | gzip > 03Validation/Recurrent.SampleInfo.txt.gz' 'sed -r 's/^[^,]+,//' | gzip > 03Validation/Recurrent.matrix.csv.gz'
    grep -Ff <(awk '$5=="Benign"||$5=="Inflammation"{print $1","}' arctangent.list) <(gzip -dc ./BLCA-SGYY.RRBS.ALL.matrix.csv.gz) | pee 'cut -f 1 -d "," | gzip > 03Validation/Benign.SampleInfo.txt.gz' 'sed -r 's/^[^,]+,//' | gzip > 03Validation/Benign.matrix.csv.gz' 

11. for i in "ALL" "CG" "CHH" "CHG";do for j in "Benign" "Non-recurrent" "Recurrent";do for k in {0..9} "x";do mkdir -p 03Validation/$i/Healthy.Primary.Cov50.balance500; perl K_Fold_Cross_Validation.Validate-v1.0.pl -validatematrix 03Validation/$j.matrix.csv.gz -validatesample 03Validation/$j.SampleInfo.txt.gz -groupApattern 02TenFoldCV/$i/T$k/T$k.Healthy.Primary.Cov50.balance500.txt.gz -groupBpattern 02TenFoldCV/$i/T$k/T$k.Primary.Healthy.Cov50.balance500.txt.gz -groupAname Healthy -groupBname Primary -output 03Validation/$i/Healthy.Primary.Cov50.balance500/T$k.$j.score.txt.gz;done;done;done

12. for i in "ALL" "CG" "CHG" "CHH";do for j in {0..9} "x";do perl -e 'print "perl ./script/K_Fold_Cross_Validation.Train-v1.0.pl -groupA Healthy -groupB Primary -round $ARGV[1] -traindir ./RRBS/Advanced_Analysis/D-score/02TenFoldCV/$ARGV[0]/$ARGV[1] -mincov 0.5 -maxcut 500 -outdir ./RRBS/Advanced_Analysis/D-score/02TenFoldCV_V2/$ARGV[0]/$ARGV[1]\n"' $i T$j;done;done > 02TenFoldCV_V2/02train.sh
    bash 02TenFoldCV_V2/02train.sh

############ filter BGI-BCa77-P and BGI-H22-H ############
7. grep -E "Healthy|Primary" arctangent.list | grep -vE "BGI-BCa77-P|BGI-H22-H" |cut -f 1,5 | sort | uniq > 04TenFoldCV/BLCA-SGYY.Training.Healthy_Primary.txt

8. for i in "ALL" "CG" "CHG" "CHH";do perl K_Fold_Cross_Validation.Extract-v1.0.pl -groupInfo 04TenFoldCV/BLCA-SGYY.Training.Healthy_Primary.txt -matrix ./BLCA-SGYY.RRBS.$i.matrix.csv.gz -kfold 10 -seed 100 -outdir 04TenFoldCV/$i;done

9. for i in "ALL" "CG" "CHG" "CHH";do for j in {0..9} "x";do perl -e 'print "perl ./script/K_Fold_Cross_Validation.Train-v1.2.pl -groupA Healthy -groupB Primary -round $ARGV[1] -traindir ./RRBS/Advanced_Analysis/D-score/04TenFoldCV/$ARGV[0]/$ARGV[1] -mincov 0.5 -maxcut 500 -outdir ./RRBS/Advanced_Analysis/D-score/04TenFoldCV/$ARGV[0]/$ARGV[1]\n"' $i T$j;done;done > 04TenFoldCV/02train.sh
    bash 04TenFoldCV/02train.sh

10. grep -Ff <(awk '$5=="Non-recurrent"{print $1","}' arctangent.list) <(gzip -dc ./BLCA-SGYY.RRBS.ALL.matrix.csv.gz) | pee 'cut -f 1 -d "," | gzip > 05Validation/Non-recurrent.SampleInfo.txt.gz' 'sed -r 's/^[^,]+,//' | gzip > 05Validation/Non-recurrent.matrix.csv.gz'
    grep -Ff <(awk '$5=="Recurrent"{print $1","}' arctangent.list) <(gzip -dc ./BLCA-SGYY.RRBS.ALL.matrix.csv.gz) | pee 'cut -f 1 -d "," | gzip > 05Validation/Recurrent.SampleInfo.txt.gz' 'sed -r 's/^[^,]+,//' | gzip > 05Validation/Recurrent.matrix.csv.gz'
    grep -Ff <(awk '$5=="Benign"||$5=="Inflammation"{print $1","}' arctangent.list) <(gzip -dc ./BLCA-SGYY.RRBS.ALL.matrix.csv.gz) | pee 'cut -f 1 -d "," | gzip > 05Validation/Benign.SampleInfo.txt.gz' 'sed -r 's/^[^,]+,//' | gzip > 05Validation/Benign.matrix.csv.gz'

11. for i in "ALL" "CG" "CHH" "CHG";do for j in "Benign" "Non-recurrent" "Recurrent";do for k in {0..9} "x";do mkdir -p 05Validation/$i/Healthy.Primary.Cov50.balance500; perl K_Fold_Cross_Validation.Validate-v1.1.pl -validatematrix 05Validation/$j.matrix.csv.gz -validatesample 05Validation/$j.SampleInfo.txt.gz -groupApattern 04TenFoldCV/$i/T$k/T$k.Healthy.Primary.Cov50.balance500.txt.gz -groupBpattern 04TenFoldCV/$i/T$k/T$k.Primary.Healthy.Cov50.balance500.txt.gz -groupAname Healthy -groupBname Primary -output 05Validation/$i/Healthy.Primary.Cov50.balance500/T$k.$j.score.txt.gz;done;done;done
