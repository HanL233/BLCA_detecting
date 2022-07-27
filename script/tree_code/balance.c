#include <math.h>
#include <argp.h>
#include <gUniq.h>
#include <gListp.h>
#include <fisher_routine.h>
#include "data.h"

#define PVALUE 0.01f
#define DIFF 0.0f
#define TRY_TIME 10

typedef struct FISHER_ITEM{
	int len;
	int *arr;
	int a;
	int b;
	int c;
	int d;
	float pValue;
}FI;

void clearFI(FI ***_fi,int len){
	FI **fi=*_fi;
	int i=0;
	for(i=0;i<len;i++){
		free(fi[i]->arr);
		free(fi[i]);
	}
	free(fi);
	*_fi=NULL;
}

float pValue2Score(float pValue){
	if(pValue<0.0f || pValue>0.05f)return 0.0f;
	return -1.0f*log(pValue>1e-40?pValue:1e-40);
}

int isMatched(float *ele,int *set,int length){
	int b=0;
	for(b=1;b<length;b++){
		if(ele[set[b]]>=ele[set[b-1]])break;
	}
	if(b==length)return 1;
	return 0;
}


int getMatched(float **ele,int x,int y,FI **fi,int length,char *table,char bit){
	int i=0,j=0,sum=0;
	char bias=1<<bit;
	for(i=0;i<x;i++){
		for(j=0;j<length;j++){
			if(isMatched(ele[i],fi[j]->arr,fi[j]->len)){
				table[i*length+j]|=bias;
				sum++;
			}
		}
	}
	return sum;
}

int getFIfromFile(const char *fileName,FI ***fi){
	FILE *fp=fopen(fileName,"r");
	if(fp==NULL)return -1;
	int sum=0;
	float pValue=0.0f;
        g_list_p *gp=g_createListP();
	while(!feof(fp)){
		char *str=NULL;
		readP(fp,&str);
		int i=0;
		if(str && str[0] && str[0]!='#'){
			char **arr=NULL;
			int al=splitP(&arr,str,"\t");
			if(al<6){
				for(i=0;i<al;i++){
					free(arr[i]);
				}
				free(arr);
				g_deleteListP(&gp);
				fclose(fp);
				return -1;
			}
			if(arr[al-1][0] !='-' && (pValue=atof(arr[al-1]))<=PVALUE){
				FI *tmp=(FI*)malloc(sizeof(FI));
				char **brr=NULL;
				tmp->len=splitP(&brr,arr[0],",");
				tmp->arr=(int*)malloc(sizeof(int)*tmp->len);
				for(i=0;i<tmp->len;i++){
					(tmp->arr)[i]=atoi(brr[i]);
					free(brr[i]);
				}
				free(brr);
				tmp->a=atoi(arr[1]);
				tmp->b=atoi(arr[2]);
				tmp->c=atoi(arr[3]);
				tmp->d=atoi(arr[4]);
				tmp->pValue=pValue;
				g_pushListP(&gp,(void*)tmp);
			}
			for(i=0;i<al;i++){
				free(arr[i]);
			}
			free(arr);
		}
		free(str);
	}
	fclose(fp);
	sum=g_getListArr(gp,(void***)fi);
	g_deleteListP(&gp);
	return sum;
}

int compareFI(const void *_a,const void *_b){
	FI *a=*(FI**)_a,*b=*(FI**)_b;
	return a->pValue>b->pValue?1:(a->pValue<b->pValue?-1:(a->len-b->len));
}

void sortFI(FI **fi,int len){
	qsort(fi,len,sizeof(FI*),compareFI);
}

void showFI(FI **fi,int len,FILE *fp){
	int i=0,j=0;
	if(fp==NULL){
		fp=stdout;
	}
	for(i=0;i<len;i++){
		for(j=0;j<fi[i]->len;j++){
			if(j)fprintf(fp,",");
			fprintf(fp,"%d",(fi[i]->arr)[j]);
		}
		fprintf(fp,"\t%d\t%d\t%d\t%d\t%g\n",fi[i]->a,fi[i]->b,fi[i]->c,fi[i]->d,fi[i]->pValue);
	}
}

void getScore(float **ele,int x,int y,FI **fi,int len,char *table,char bit,int point,float *value){
	int i=0,j=0;
	char bias=1<<bit;
	point=point<len?point:len;
	for(i=0;i<x;i++){
		float score=0.0f;
		for(j=0;j<point;j++){
			if(table[i*len+j]&bias)score+=pValue2Score(fi[j]->pValue);
			else score-=pValue2Score(fi[j]->pValue);//TODO new inserted
		}
		value[i]=score;
	}
}

float getFisher(float *val00,float *val01,int len0,float *val10,float *val11,int len1){
	int sts[4]={0};
	int i=0;
	for(i=0;i<len0;i++){
		if(val00[i]>val01[i]+DIFF){
			sts[0]++;
		}else if(val01[i]>val00[i]+DIFF){
			sts[1]++;
                }
	}
	for(i=0;i<len1;i++){
		if(val10[i]>val11[i]+DIFF){
			sts[2]++;
		}else if(val11[i]>val10[i]+DIFF){
			sts[3]++;
                }
	}
	if(sts[1]+sts[3]==0)return -1.0f;
	float pValue=fisher_exact_tiss(sts[0],sts[1],sts[2],sts[3]);
	//fprintf(stderr,"%d,%d,%d,%d\t%g\n",sts[0],sts[1],sts[2],sts[3],pValue);
	return pValue>=0.0f?pValue:1.0f;
}

int getPointA(float **eleA,int xA,int yA,float **eleB,int xB,int yB,FI **fiA,int lenA,FI **fiB,int lenB,char *table,char base[],int max,int pointB){
	int i=0,pointSEL=max<lenA?max:lenA;
	float pValueSEL=1.0f;
	float *val00=(float*)malloc(xA*sizeof(float));
	float *val01=(float*)malloc(xA*sizeof(float));
	float *val10=(float*)malloc(xB*sizeof(float));
	float *val11=(float*)malloc(xB*sizeof(float));
	getScore(eleA,xA,yA,fiB,lenB,table,base[1],pointB,val01);
	getScore(eleB,xB,yB,fiB,lenB,table,base[3],pointB,val11);
	for(i=1;i<(max<lenA?max:lenA);i++){
		getScore(eleA,xA,yA,fiA,lenA,table,base[0],i,val00);
		getScore(eleB,xB,yB,fiA,lenA,table,base[2],i,val10);
		float pValue=getFisher(val00,val01,xA,val10,val11,xB);
		if(pValue<0.0f)break;
		//fprintf(stderr,"--> %d,%d\n",i,pointB);
		if(pValue<=pValueSEL){
			//fprintf(stderr,"pointA=%d,pointB=%d,pValue=%g\n",i,pointB,pValue);
			pValueSEL=pValue;
			pointSEL=i;
		}
	}
	free(val00);
	free(val01);
	free(val10);
	free(val11);
	return pointSEL;
}

void getNewMax(FI **fi0,int len0,FI **fi1,int len1,int max,int *max0,int *max1){
	//fi must be sorted according to mindiff first
	int flag=0,m0=0,m1=0;
	float s0=pValue2Score(fi0[0]->pValue);
	float s1=pValue2Score(fi1[0]->pValue);
	do{
		flag=0;
		if(s0<=s1 && m0<max-1 && m0<len0-1){
			s0+=pValue2Score(fi0[++m0]->pValue);
			flag=1;
		}
		if(s1<=s0 && m1<max-1 && m1<len1-1){
			s1+=pValue2Score(fi1[++m1]->pValue);
			flag=1;
		}
	}while(flag);
	*max0=m0;
	*max1=m1;
}

int main(int argc,const char **argv){
	argv_pra pra[10];
	int args[]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},argp[]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	pra[0].position=argp+0;pra[0].value_position=args+0;memcpy(pra[0].item_name,"-input0",ARGV_STR_MAX);
	pra[1].position=argp+1;pra[1].value_position=args+1;memcpy(pra[1].item_name,"-complete0",ARGV_STR_MAX);
	pra[2].position=argp+2;pra[2].value_position=args+2;memcpy(pra[2].item_name,"-input1",ARGV_STR_MAX);
	pra[3].position=argp+3;pra[3].value_position=args+3;memcpy(pra[3].item_name,"-complete1",ARGV_STR_MAX);
	pra[4].position=argp+4;pra[4].value_position=args+4;memcpy(pra[4].item_name,"-max",ARGV_STR_MAX);
	pra[5].position=argp+5;pra[5].value_position=args+5;memcpy(pra[5].item_name,"-balance0",ARGV_STR_MAX);
	pra[6].position=argp+6;pra[6].value_position=args+6;memcpy(pra[6].item_name,"-balance1",ARGV_STR_MAX);
	pra[7].position=argp+7;pra[7].value_position=NULL;memcpy(pra[7].item_name,"-help",ARGV_STR_MAX);
	pra[8].position=argp+8;pra[8].value_position=args+8;memcpy(pra[8].item_name,"-mindiff",ARGV_STR_MAX);
	pra[9].position=argp+9;pra[9].value_position=NULL;memcpy(pra[9].item_name,"-cover",ARGV_STR_MAX);
	int are=getArgvValue(pra,10,0,NULL,0,argc,argv);
	if(are<0){
		fprintf(stderr,"unkown item: %s\n",argv[-1*are]);
		argp[7]=0;
	}else if(are){
		fprintf(stderr,"-%s requires value\n",pra[are-1].item_name);//note
		argp[7]=0;
	}
	if(args[0]<=0 || args[1]<=0 || args[2]<=0 || args[3]<=0 || args[4]<=0 || args[5]<=0 || args[6]<=0 || argp[7]>=0){
		if(args[0]>0 && args[1]>0 && args[3]>0 && args[2]<=0 && args[4]<=0 && args[5]<=0 && args[6]<=0 &&  argp[7]<=0 && args[8]<=0){
			float **ele=NULL;
			int i=0,j=0,x=0,y=0,fl0=0,fl1=0;
			if(getDataFromFile(argv[args[0]],&ele,&y,&x)<0)exit(0);
			FI **fi0=NULL,**fi1=NULL;
			if((fl0=getFIfromFile(argv[args[1]],&fi0))<0)exit(0);
			if((fl1=getFIfromFile(argv[args[3]],&fi1))<0)exit(0);
			sortFI(fi0,fl0);
			sortFI(fi1,fl1);
			char *matchedTable=calloc((fl0>fl1?fl0:fl1)*x,sizeof(char));
			getMatched(ele,x,y,fi0,fl0,matchedTable,0);
			getMatched(ele,x,y,fi1,fl1,matchedTable,1);
			float *score0=(float*)malloc(x*sizeof(float));
			float *score1=(float*)malloc(x*sizeof(float));
			getScore(ele,x,y,fi0,fl0,matchedTable,0,fl0,score0);
			getScore(ele,x,y,fi1,fl1,matchedTable,1,fl1,score1);
			int sum0=0,sum1=0;
			for(i=0;i<x;i++){
				if(score0[i]>score1[i]+DIFF){
					sum0++;
				}else if(score1[i]>score0[i]+DIFF){
					sum1++;
				}
				fprintf(stdout,"%g,%g",score0[i],score1[i]);
				if(argp[9]<0){
					fprintf(stdout,"\n");
				}else{
					fprintf(stdout,"\t");
					for(j=0;j<fl0;j++){
						if(j)fprintf(stdout,",");
						fprintf(stdout,"%d",(matchedTable[i*fl0+j]&1)?1:0);
					}
					fprintf(stdout,"\t");
					for(j=0;j<fl1;j++){
						if(j)fprintf(stdout,",");
						fprintf(stdout,"%d",(matchedTable[i*fl1+j]&2)?1:0);
					}
					fprintf(stdout,"\n");
				}
			}
			free(score0);
			free(score1);
			free(matchedTable);
			clearFI(&fi0,fl0);
			clearFI(&fi1,fl1);
			fprintf(stderr,"%d/%d=%g,%d/%d=%g\n",sum0,x,(float)sum0/(float)(x),sum1,x,(float)sum1/(float)(x));
			clearMatrix(&ele,&y,&x);
		}else if(args[0]>0 && args[1]>0 && args[2]<=0 && args[3]<=0 && args[4]<=0 && args[5]<=0 && args[6]<=0 &&  argp[7]<=0 && args[8]<=0 && argp[9]>=0){
			float **ele=NULL;
			int i=0,j=0,x=0,y=0,fl=0;
			if(getDataFromFile(argv[args[0]],&ele,&y,&x)<0)exit(0);
			FI **fi=NULL;
			if((fl=getFIfromFile(argv[args[1]],&fi))<0)exit(0);
			char *matchedTable=(char*)calloc(fl*x,sizeof(char));
			getMatched(ele,x,y,fi,fl,matchedTable,0);
			for(i=0;i<x;i++){
				for(j=0;j<fl;j++){
					if(j)fprintf(stdout,",");
					//fprintf(stdout,"%g",(matchedTable[i*fl+j]&1)?(pValue2Score(fi[j]->pValue)):0);
					fprintf(stdout,"%d",(matchedTable[i*fl+j]&1)?1:0);
				}
				fprintf(stdout,"\n");
			}
			free(matchedTable);
			clearFI(&fi,fl);
		}else{
			fprintf(stderr,"balance v0.1\n");
			fprintf(stderr,"  --input0        file      with format \"sample gene[,gene,...]\" of each line\n");
			fprintf(stderr,"  --input1        file      same format with --input0\n");
			fprintf(stderr,"  --complete0     file      output of fisher (shape)\n");
			fprintf(stderr,"  --complete1     file      same format with --fisher0\n");
			fprintf(stderr,"  --balance0      file      filename to output resized --fisher0\n");
			fprintf(stderr,"  --balance1      file      filename to output resized --fisher1\n");
			fprintf(stderr,"  --max           integer   maximum sum of traits\n");
			fprintf(stderr,"  --help                    show this message and exit\n\n");
		}
		exit(0);
	}
	float **ele0=NULL,**ele1=NULL;
	int x0=0,y0=0,x1=0,y1=0,fl0=0,fl1=0;
	if(getDataFromFile(argv[args[0]],&ele0,&y0,&x0)<0){fprintf(stderr,"Error with %s\n",argv[args[0]]);exit(0);}
	if(getDataFromFile(argv[args[2]],&ele1,&y1,&x1)<0){fprintf(stderr,"Error with %s\n",argv[args[2]]);exit(0);}
	FI **fi0=NULL,**fi1=NULL;
	if((fl0=getFIfromFile(argv[args[1]],&fi0))<0){fprintf(stderr,"Error with %s\n",argv[args[1]]);exit(0);}
	if((fl1=getFIfromFile(argv[args[3]],&fi1))<0){fprintf(stderr,"Error with %s\n",argv[args[3]]);exit(0);}
	if(fl0 && fl1){
		int _fl0=fl0,_fl1=fl1;
		sortFI(fi0,fl0);
		sortFI(fi1,fl1);
		int max=atoi(argv[args[4]]),max0=0,max1=0;
		getNewMax(fi0,fl0,fi1,fl1,max,&max0,&max1);
		fl0=max0;
		fl1=max1;
		char *matchedTable=calloc((fl0>fl1?fl0:fl1)*(x0>x1?x0:x1),sizeof(char));
		getMatched(ele0,x0,y0,fi0,fl0,matchedTable,0);
		getMatched(ele0,x0,y0,fi1,fl1,matchedTable,1);
		getMatched(ele1,x1,y1,fi0,fl0,matchedTable,2);
		getMatched(ele1,x1,y1,fi1,fl1,matchedTable,3);
		int point0=atoi(argv[args[4]]);point0=point0<fl0?point0:fl0;
		int point1=atoi(argv[args[4]]);point1=point1<fl1?point1:fl1;
		int pointO0=0,pointO1=0;
		char bias[4];
		do{
			pointO0=point0;
			pointO1=point1;
			//fprintf(stderr,"--- 1 ---\n");
			bias[0]=0;bias[1]=1;bias[2]=2;bias[3]=3;
			//int getPointA(float **eleA,int xA,int yA,float **eleB,int xB,int yB,FI **fiA,int lenA,FI **fiB,int lenB,char *table,char base[],int max,int pointB)
			point0=getPointA(ele0,x0,y0,ele1,x1,y1,fi0,fl0,fi1,fl1,matchedTable,bias,max0,point1);
			//fprintf(stderr,"--- 2 ---\n");
			bias[0]=3;bias[1]=2;bias[2]=1;bias[3]=0;
			point1=getPointA(ele1,x1,y1,ele0,x0,y0,fi1,fl1,fi0,fl0,matchedTable,bias,max1,point0);
		}while(point0!=pointO0 || point1!=pointO1);
		FILE *fp=fopen(argv[args[5]],"w");
		showFI(fi0,point0+1,fp);
		fclose(fp);
		fp=fopen(argv[args[6]],"w");
		showFI(fi1,point1+1,fp);
		fclose(fp);
		free(matchedTable);
		fl0=_fl0;fl1=_fl1;
	}
	clearFI(&fi0,fl0);
	clearFI(&fi1,fl1);
	clearMatrix(&ele0,&y0,&x0);
	clearMatrix(&ele1,&y1,&x1);
	return 0;
}
