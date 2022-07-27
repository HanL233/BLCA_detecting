#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

double **createMatrix(int weight,int height){
	double **data=(double**)malloc(height*sizeof(double*));
	while(height--){
		data[height]=(double*)malloc(weight*sizeof(double));
	}
	return data;
}

void copyMatrix(double **data,int weight,int height,double **src){
	int i=0;
	for(i=0;i<height;i++){
		memcpy(data[i],src[i],weight*sizeof(double));
	}
}

void showMatrix(double **data,int weight,int height){
	int i=0,j=0;
	for(i=0;i<height;i++){
		for(j=0;j<weight;j++){
			if(j)printf(",");
			printf("%g",data[i][j]);
		}
		printf("\n");
	}
}

void clearMatrix(double ***data,int height){
	while(height--){
		free((*data)[height]);
	}
	free(*data);
	*data=NULL;
}

void initTHETA(double **theta,int X,int Y){
	time_t t;
	srand((unsigned) time(&t));
	int i=0,j=0;
	for(i=0;i<Y;i++){
		for(j=0;j<X;j++){
			theta[i][j]=0.005f*(double)(rand())/(double)(RAND_MAX);
		}
	}
}

double getExpTHETA(double **theta,int X,int Y,unsigned char *x,double *e){
	int i=0,j=0;
	double v=0.0f,sum=0.0f,max=0.0f;
	for(i=0;i<Y;i++){
		v=0.0f;
		for(j=0;j<X;j++){
                        v+=theta[i][j]*(double)(x[j]);
                }
                if(i==0 || max<v)max=v;
		e[i]=v;
	}
	for(i=0;i<Y;i++){
		sum+=(e[i]=exp(e[i]-max));
	}
	return sum;
}

int getMaxIndex(double *v,int len){
	double max=v[--len];
	int index=len;
	while(--len>=0){
		if(max<v[len]){
			max=v[len];
			index=len;
		}
	}
	return index;
}

int derivate(double **theta,int X,int Y,unsigned char **matrix,int width,int height,char *label,char *available,double weight,double studyRate,double **mTheta,double *e,int *best,double **_theta){
	int i=0,j=0,k=0,ava=0,right=0;
	double sum=0.0f,probit=0.0f;
	for(i=0;i<Y;i++){
		memset(mTheta[i],0,sizeof(double)*X);
	}
	for(i=0;i<height;i++){
		if(available[i]==0)continue;
		sum=getExpTHETA(theta,X,Y,matrix[i],e);
		for(j=0;j<Y;j++){
			probit=((int)(label[i])==j?1.0f:0.0f)-e[j]/sum;
			for(k=0;k<X;k++){
				mTheta[j][k]+=(double)(matrix[i][k])*probit;
			}
		}
		ava++;
		if(getMaxIndex(e,Y)==(int)label[i])right++;
	}
	if(right>*best){
		copyMatrix(_theta,X,Y,theta);
		*best=right;
	}
	for(i=0;i<Y;i++){
		for(j=0;j<X;j++){
			theta[i][j]+=studyRate*mTheta[i][j]/(double)ava+weight*theta[i][j];
		}
	}
	return right;
}
void predict(double **theta,int X,int Y,unsigned char **matrix,int width,int height,char *label){
	int i=0;
	double *e=(double*)malloc(Y*sizeof(double));
	for(i=0;i<height;i++){
		getExpTHETA(theta,X,Y,matrix[i],e);
		label[i]=getMaxIndex(e,Y);
	}
	free(e);
}

double getLoss(double **theta,int X,int Y,unsigned char **matrix,int width,int height,char *label,double *e){
	int i=0;
	double loss=0.0,sum=0.0;
	for(i=0;i<height;i++){
		sum=getExpTHETA(theta,X,Y,matrix[i],e);
		//fprintf(stderr,"%g/%g=%g\n",e[(int)(label[i])],sum,e[(int)(label[i])]/sum);
		//loss+=log(e[(int)(label[i])]/sum);
		//fprintf(stderr,"e=%g,sum=%g\n",e[(int)(label[i])],sum);
		loss+=(getMaxIndex(e,Y)==label[i]?1.0f:0.0f);
	}
	//exit(0);
	return loss;//-1.0f*loss/((double)(height));
}

void randAvailable(char *available,int len,int total){
	int p=0;
	memset(available,0,len*sizeof(char));
	while(total--){
		p=rand()%len;
		while(available[p]){
			p++;
			if(p>=len)p=0;
		}
		available[p]=1;
	}
}

typedef double (*refFI)(int);

void softmax_main(double **theta,int X,int Y,unsigned char **matrix,int width,int height,char *label,refFI weight,refFI studyRate,double samplingRate,int maxGeneration,double minAcc){
	int i=0,gen=0,best=0,num=(int)(((double)(height))*samplingRate+0.5),goal=(int)(((double)(height))*minAcc+0.5);
	double **mTheta=createMatrix(X,Y);
	double **_theta=createMatrix(X,Y);
	double *e=(double*)malloc(Y*sizeof(double));
	char *available=(char*)malloc(height*sizeof(char));
	for(i=0;i<height;i++){
		available[i]=1;
	}
	initTHETA(theta,X,Y);
	while(gen++<maxGeneration){
		if(samplingRate<1.0f)randAvailable(available,Y,num);
		fprintf(stderr,"%d -> %d\n",gen,derivate(theta,X,Y,matrix,width,height,label,available,weight(gen),studyRate(gen),mTheta,e,&best,_theta));
		if(best>=goal)break;
	}
	fprintf(stderr,"train: %d/%d=%g\n",best,height,(double)(best)/(double)(height));
	copyMatrix(theta,X,Y,_theta);
	clearMatrix(&mTheta,Y);
	clearMatrix(&_theta,Y);
	free(available);
	free(e);
}
