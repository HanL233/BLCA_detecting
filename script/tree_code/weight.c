#include <stdio.h>
#include <stdlib.h>
#include <gListp.h>
#include <readp.h>
#include <splitp.h>
#include "softmax.h"

#define SAMPLING_RATE 1.0f
#define MAX_GENERATION 100
#define MAX_ACC 1.0f

void clearData(unsigned char **data,int height){
	int i=0;
	for(i=0;i<height;i++){
		free(data[i]);
	}
	free(data);
}

char getData(const char *fileName,unsigned char ***data,int *width,int *height){
	FILE *fp=fopen(fileName,"r");
	if(fp==NULL)return -1;
	*width=0;
	g_list_p *gp=g_createListP();
	while(!feof(fp)){
		char *str=NULL;
		readP(fp,&str);
		if(str && str[0] && str[0]!='#'){
			char **arr=NULL;
			int len=splitP(&arr,str,","),i=0;
			unsigned char *d=(unsigned char*)malloc(len*sizeof(unsigned char));
			g_pushListP(&gp,(void*)d);
			for(i=0;i<len;i++){
				d[i]=(unsigned char)atoi(arr[i]);
				free(arr[i]);
			}
			free(arr);
			if(*width){
				if(*width!=len){
					free(str);
					fclose(fp);
					g_clearListP(gp,free);
					g_deleteListP(&gp);
					return -2;
				}
			}else{
				*width=len;
			}
		}
		free(str);
	}
	fclose(fp);
	*height=g_getListArr(gp,(void***)data);
	g_deleteListP(&gp);
	return 0;
}

double getWeight(int step){
	return 1e-4;
}

double getStudyRate(int step){
	return 0.1f;
}

int main(int argc,char **argv){
	if(argc<2){
		fprintf(stderr,"%s <FI:table>\n",argv[0]);
		return 0;
	}
	int width=0,height=0,i=0;
	unsigned char **data=NULL;
	if(getData(argv[1],&data,&width,&height)){
		fprintf(stderr,"%s: Error with %s\n",argv[0],argv[1]);
		exit(0);
	}	
	char *label=(char*)malloc(height*sizeof(char));
	unsigned char maxLevel=0;
	for(i=0;i<height;i++){
		if(maxLevel<(label[i]=data[i][width-1]))maxLevel=label[i];
	}
	int labelLevel=(int)maxLevel+1;
	fprintf(stderr,"labelLevel=%d\n",labelLevel);
	double **theta=createMatrix(width-1,labelLevel);
	softmax_main(theta,width-1,labelLevel,data,width-1,height,label,getWeight,getStudyRate,SAMPLING_RATE,MAX_GENERATION,MAX_ACC);
	showMatrix(theta,width-1,labelLevel);
	if(argc==3){
		int twidth=0,theight=0;
		unsigned char **tdata=NULL;
		if(getData(argv[2],&tdata,&twidth,&theight)){
			fprintf(stderr,"%s: Error with %s\n",argv[0],argv[2]);
			exit(0);
		}
		if(twidth==width-1 || twidth==width){
			if(twidth==width)twidth--;
			char *label=(char*)malloc(theight*sizeof(char));
			predict(theta,width-1,labelLevel,tdata,twidth,theight,label);
			for(i=0;i<theight;i++){
				fprintf(stderr,"%d\n",label[i]);
			}
			free(label);
		}
		clearData(tdata,theight);
	}
	clearMatrix(&theta,labelLevel);
	clearData(data,height);
	free(label);
	return 0;
}
