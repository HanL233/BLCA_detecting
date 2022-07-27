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

char getWeight(const char *fileName,double ***data,int *width,int *height){
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
                        double *d=(double*)malloc(len*sizeof(double));
                        g_pushListP(&gp,(void*)d);
                        for(i=0;i<len;i++){
                                d[i]=atof(arr[i]);
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

int main(int argc,char **argv){
	if(argc<3){
		fprintf(stderr,"%s <FI:weight> <FI:table>\n",argv[0]);
		return 0;
	}
	int width=0,height=0,i=0,twidth=0,theight=0;
	unsigned char **data=NULL;
	double **theta=NULL;
	if(getWeight(argv[1],&theta,&twidth,&theight)){
		fprintf(stderr,"%s: Error with %s\n",argv[0],argv[1]);
		exit(0);
	}
	if(getData(argv[2],&data,&width,&height)){
                fprintf(stderr,"%s: Error with %s\n",argv[0],argv[2]);
                exit(0);
        }
	if(twidth==width){
		char *label=(char*)malloc(height*sizeof(char));
		predict(theta,twidth,theight,data,width,height,label);
		for(i=0;i<height;i++){
			printf("%d\n",label[i]);
		}
		free(label);
	}else{
		fprintf(stderr,"Error: weight mismathes with data\n");
	}
	clearMatrix(&theta,theight);
	clearData(data,height);
	return 0;
}
