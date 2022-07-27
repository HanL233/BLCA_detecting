#include <stdio.h>
#include <argp.h>
#include "data.h"
#include "branch.h"

int main(int argc,const char **argv){
	argv_pra pra[4];
	int aip=-1,aipp=-1,asp=-1,aspp=-1,ahpp=-1;
        pra[0].position=&aipp;pra[0].value_position=&aip;memcpy(pra[0].item_name,"-input",ARGV_STR_MAX);
        pra[1].position=&aspp;pra[1].value_position=&asp;memcpy(pra[1].item_name,"-coverage",ARGV_STR_MAX);
        pra[2].position=&ahpp;pra[2].value_position=NULL;memcpy(pra[2].item_name,"-help",ARGV_STR_MAX);
	pra[3].position=&ahpp;pra[3].value_position=NULL;memcpy(pra[3].item_name,"h",ARGV_STR_MAX);
	int are=getArgvValue(pra,4,0,NULL,0,argc,argv);
        if(are<0){
                printf("unkown item: %s\n",argv[-1*are]);
                ahpp=0;
        }else if(are){
                printf("-%s requires value\n",pra[are-1].item_name);//note
                ahpp=0;
        }
        if(aip<0 || asp<0 || ahpp>=0){
                fprintf(stderr,"tree v0.1\n");
                fprintf(stderr,"  --input       file    matrix of number\n");
                fprintf(stderr,"  --coverage    float   minima support-ratio\n");
                fprintf(stderr,"  -h/--help             show this message and exit\n");
                exit(0);
        }
	int X=0,Y=0;
	float **matrix=NULL;
	if(getDataFromFile(argv[aip],&matrix,&X,&Y))exit(-1);
	float per=atof(argv[asp]);
	int support=(int)(per*(float)(Y)+0.5f);
	//showMatrix(matrix,X,Y);
	LEAF *head=NULL,*tail=NULL;
	initLEAF(matrix,X,Y,&head,&tail,Y,support);
	//showLEAF(head);
	float percentage=100.0f;
	int sup=0;
	while(percentage>=per*100.0f){
		//fprintf(stderr,"per:%g - %g\n",percentage,per*100.0f);
		sup=(int)(percentage*(float)(Y)/100.0f+0.5f);
		updateLEAF(matrix,X,Y,&head,&tail,sup);
		percentage-=1.0f;
	}
	LEAF *tmp=head;
	while(tmp){
		showMiniSingleLEAF(tmp,Y);
		tmp=tmp->next;
	}
	clearLEAF(&head,&tail);
	clearMatrix(&matrix,&X,&Y);
	//printf("total=%d\n",transSum);
	return 0;
}
