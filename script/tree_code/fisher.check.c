#include <stdio.h>
#include <argp.h>
#include <fisher_routine.h>
#include "data.h"

int getSupport(float **matrix,int width,int height,int *position,int length,char *bit){
        int support=0;
        int i=0,j=0;
        for(i=0;i<height;i++){
                for(j=0;j<length-1;j++){
                        if(matrix[i][position[j]]<=matrix[i][position[j+1]])break;
                }
		if(j==length-1){
                	support++;
			bit[i]=1;
		}else{
			bit[i]=0;
		}
        }
        return support;
}

void complete(float **matrix,int width,int height,const char *fileName){
	FILE *fp=fopen(fileName,"r");
	if(fp==NULL)exit(0);
	int staA=0,staB=0,staC=0,staD=0,ol=0;
	float pValue=0.0f;
	int *set=NULL;
	char *bit=(char*)malloc(height*sizeof(char));
	long *mat=(long*)calloc(height*height,sizeof(long));
	while(!feof(fp)){
		char *str=NULL;
		readP(fp,&str);
		int i=0,j=0;
		if(str && str[0] && str[0]!='#'){
			char **arr=NULL;
			int al=splitP(&arr,str,"\t");
			free(str);
			if(al>2){
				str=arr[0];
				staA=atoi(arr[1]);
				staB=atoi(arr[2]);
			}
			for(i=1;i<al;i++){
				free(arr[i]);
			}
			free(arr);
			if(al<3){
				free(str);
				fclose(fp);
				exit(0);
			}
			al=splitP(&arr,str,",");
			if(set && ol!=al){
				free(set);
				set=NULL;
			}
			if(set==NULL)set=(int*)malloc(sizeof(int)*al);
			ol=al;
			for(i=0;i<al;i++){
				set[i]=atoi(arr[i]);
				free(arr[i]);
			}
			free(arr);
			staD=height-(staC=getSupport(matrix,width,height,set,al,bit));
			//pValue=fisher_exact_tiss(staA,staB,staC,staD);
			//if(pValue>=0.0f && pValue<=0.05f)printf("%s\t%d\t%d\t%d\t%d\t%g\t",str,staA,staB,staC,staD,pValue>=0.0f?pValue:1.0f);
			if(staC<height){
				/*
				printf("%s\t",str);
				for(i=1;i<height;i++){
					printf("%d",bit[i]);
				}
				printf("\n");
				*/
				for(i=0;i<height;i++){
					if(bit[i]==0)continue;
					for(j=0;j<height;j++){
						if(bit[j]==0)continue;
						mat[i*height+j]++;
					}
				}
			}
		}
		free(str);
	}
	fclose(fp);
	free(bit);
	int i=0,j=0;
	for(i=0;i<height;i++){
		printf("%ld",mat[i*height]);
		for(j=1;j<height;j++){
			printf(",%ld",mat[i*height+j]);
		}
		printf("\n");
	}
	free(mat);
}

int main(int argc,const char **argv){
	argv_pra pra[4];
	int aip=-1,aipp=-1,asp=-1,aspp=-1,ahpp=-1;
        pra[0].position=&aipp;pra[0].value_position=&aip;memcpy(pra[0].item_name,"-input",ARGV_STR_MAX);
        pra[1].position=&aspp;pra[1].value_position=&asp;memcpy(pra[1].item_name,"-half",ARGV_STR_MAX);
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
                fprintf(stderr,"fish v0.1\n");
                fprintf(stderr,"  --input       file    matrix of number\n");
                fprintf(stderr,"  --half        file    output of tree v0.1\n");
                fprintf(stderr,"  -h/--help             show this message and exit\n");
                exit(0);
        }
	int X=0,Y=0;
	float **matrix=NULL;
	if(getDataFromFile(argv[aip],&matrix,&X,&Y))exit(-1);
	complete(matrix,X,Y,argv[asp]);
	clearMatrix(&matrix,&X,&Y);
	return 0;
}
