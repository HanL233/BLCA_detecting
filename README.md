# BLCA_detecting
Detecting and Monitoring Bladder Cancer with Exfoliated Cells in Urine
Author : lianghan@genomics.cn,  algorithm
Author : lifuqiang@genomics.cn, pipeline

## Preparation
1. compile neccessary tools <br />
cd script <br />
gcc -Wall -g -o balance.plus tree_code/balance.plus.c -lm -I ./include/ <br />
gcc -Wall -g -o fisher tree_code/fisher.c -lm -I ./include/ <br />
gcc -Wall -g -o tree tree_code/tree.c -lm -I ./include/ <br />
2. edit the Perl scripts to replace default paths of bedtools and anaconda2 pee <br />
3. add bedtools & pee into $PATH

## Processing
Please see 00work.sh, an example of training data (used in step 9) had been put in folder "data", you don't need to go through the whole pipeline

