#include <readp.h>
#include <splitp.h>
#include "bp.h"

#ifndef __HAN__BPEX__
#define __HAN__BPEX__

int bp_readFile(const char *file,bpData **_data){
	FILE *fp=fopen(file,"r");
	if(fp==NULL)return 1;
	int a=0;
	bpNode *head=NULL;
	int numIn=0,numOut=0;
	while(!feof(fp)){
		char *str=NULL;
		int len=readP(fp,&str);
		if(str && str[0] && str[0]!='#'){
			char **arr=NULL;
			int ll=splitP(&arr,str,":");
			int err=0;
			if(ll==2){
				char **in=NULL;
				char **out=NULL;
				int li=splitP(&in,arr[0],",");
				int lo=splitP(&out,arr[1],",");
				if((numIn && numIn!=li) || (numOut && numOut!=lo)){
					err=1;
					for(a=0;a<li;a++){
						free(in[a]);
					}
					free(in);
					for(a=0;a<lo;a++){
						free(out[a]);
					}
					free(out);
				}else{
					numIn=li;
					numOut=lo;
					bpNode *tmp=(bpNode*)malloc(sizeof(bpNode));
					double *i=(double*)malloc(li*sizeof(double));
					for(a=0;a<li;a++){
						i[a]=atof(in[a]);
						free(in[a]);
					}
					free(in);
					double *o=(double*)malloc(lo*sizeof(double));
					for(a=0;a<lo;a++){
						o[a]=atof(out[a]);
						free(out[a]);
					}
					free(out);
					tmp->in=i;
					tmp->out=o;
					tmp->next=head;
					head=tmp;
				}
			}
			for(a=0;a<ll;a++){
				free(arr[a]);
			}
			free(arr);
			if(err || ll!=2){
				bpNode *tp=NULL;
				while(head){
					tp=head;
					head=head->next;
					free(tp->in);
					free(tp->out);
					free(tp);
				}
				free(str);
				fclose(fp);
				return 2;
			}

		}
		free(str);
	}
	bpData *data=(bpData*)malloc(sizeof(bpData));
	data->beNormalized=0;
	data->numIn=numIn;
	data->numOut=numOut;
	data->data=head;
	*_data=data;
	fclose(fp);
	return 0;
}

void bp_showData(bpData *data){
	int a=0;
	bpNode *head=data->data;
	while(head){
		for(a=0;a<data->numIn;a++){
			if(a)printf(",");
			printf("%g",(head->in)[a]);
		}
		printf(":");
		for(a=0;a<data->numOut;a++){
			if(a)printf(",");
			printf("%g",(head->out)[a]);
		}
		printf("\n");
		head=head->next;
	}
}

void bp_writeArr(FILE *fp,double *arr,int len){
	int a=0;
	for(a=0;a<len;a++){
		if(a)fprintf(fp," ");
		fprintf(fp,"%g",arr[a]);
	}
	fprintf(fp,"\n");
}

int bp_writeBPnet(bpNet *bp,const char *file){
	FILE *fp=fopen(file,"w");
	if(fp==NULL)return 1;
	fprintf(fp,"%d %d %d %d\n",bp->numIn,bp->numHidden,bp->numOut,bp->numHiddenLayer);
	bp_writeArr(fp,bp->inMax,bp->numIn);
	bp_writeArr(fp,bp->inMin,bp->numIn);
	bp_writeArr(fp,bp->outMax,bp->numOut);
	bp_writeArr(fp,bp->outMin,bp->numOut);
	bp_writeArr(fp,bp->layerIn->weight,bp->numIn*bp->numHidden);
	bp_writeArr(fp,bp->layerIn->dWeight,bp->numIn*bp->numHidden);
	bp_writeArr(fp,bp->layerOut->weight,bp->numOut*bp->numHidden);
	bp_writeArr(fp,bp->layerOut->dWeight,bp->numOut*bp->numHidden);
	int a=0,size=bp->numHidden*bp->numHidden;
	for(a=0;a<bp->numHiddenLayer;a++){
		bp_writeArr(fp,((bp->layerHidden)[a])->weight,size);
		bp_writeArr(fp,((bp->layerHidden)[a])->dWeight,size);
	}
	fclose(fp);
	return 0;
}

bpNet *bp_resumeBPnet(const char *file){
	FILE *fp=fopen(file,"r");
	if(fp==NULL)return NULL;
	int numIn=0,numOut=0,numHidden=0,numHiddenLayer=0;
	fscanf(fp,"%d %d %d %d",&numIn,&numHidden,&numOut,&numHiddenLayer);
	int a=0,b=0;
	bpNet *bp=(bpNet*)malloc(sizeof(bpNet));
	bp->inMax=(double*)malloc(numIn*sizeof(double));for(a=0;a<numIn;a++)fscanf(fp,"%lf",bp->inMax+a);
	bp->inMin=(double*)malloc(numIn*sizeof(double));for(a=0;a<numIn;a++)fscanf(fp,"%lf",bp->inMin+a);
	bp->outMax=(double*)malloc(numOut*sizeof(double));for(a=0;a<numOut;a++)fscanf(fp,"%lf",bp->outMax+a);
	bp->outMin=(double*)malloc(numOut*sizeof(double));for(a=0;a<numOut;a++)fscanf(fp,"%lf",bp->outMin+a);
	bpLayer *layIn=(bpLayer*)malloc(sizeof(bpLayer));
	layIn->weight=(double*)malloc(numIn*numHidden*sizeof(double));for(a=0;a<numIn*numHidden;a++)fscanf(fp,"%lf",layIn->weight+a);
	layIn->dWeight=(double*)malloc(numIn*numHidden*sizeof(double));for(a=0;a<numIn*numHidden;a++)fscanf(fp,"%lf",layIn->dWeight+a);
	layIn->value=(double*)malloc(numIn*sizeof(double));
	bpLayer *layOut=(bpLayer*)malloc(sizeof(bpLayer));
	layOut->weight=(double*)malloc(numOut*numHidden*sizeof(double));for(a=0;a<numOut*numHidden;a++)fscanf(fp,"%lf",layOut->weight+a);
	layOut->dWeight=(double*)malloc(numOut*numHidden*sizeof(double));for(a=0;a<numOut*numHidden;a++)fscanf(fp,"%lf",layOut->dWeight+a);
	layOut->value=(double*)malloc(numOut*sizeof(double));
	bpLayer **layerHidden=(bpLayer**)malloc(numHiddenLayer*sizeof(bpLayer*));
	for(a=0;a<numHiddenLayer;a++){
		bpLayer *ly=(bpLayer*)malloc(sizeof(bpLayer));
		double *wg=(double*)malloc(numHidden*numHidden*sizeof(double));for(b=0;b<numHidden*numHidden;b++)fscanf(fp,"%lf",wg+b);
		double *dw=(double*)malloc(numHidden*numHidden*sizeof(double));for(b=0;b<numHidden*numHidden;b++)fscanf(fp,"%lf",dw+b);
		double *vl=(double*)malloc(numHidden*sizeof(double));
		ly->weight=wg;
		ly->dWeight=dw;
		ly->value=vl;
		layerHidden[a]=ly;
	}
	bp->numIn=numIn;
	bp->numHidden=numHidden;
	bp->numOut=numOut;
	bp->numHiddenLayer=numHiddenLayer;
	bp->layerIn=layIn;
	bp->layerHidden=layerHidden;
	bp->layerOut=layOut;
	fclose(fp);
	return bp;
}

/*
   int err=bp_readFile("test.data",&data);
   int numHiddenLayer=1;
   int numHidden=100;
   double bias=0.0;
   double dev=0.01;
   int times=1000;
   double rate[]={0.2,0.4,0.2,0.3};
   bpNet *bp=bp_createBPnet(data->numIn,numHidden,data->numOut,numHiddenLayer);
   bp_normalize(bp,data);
   bp_train(bp,data,NULL,rate,times,dev,showProgress);
   bp_writeBPnet(bp,"bp.data");
   bp_deleteBPnet(&bp);
   bp=bp_resumeBPnet("bp.data");
   bp_normalize(bp,data);
   bp_train(bp,data,NULL,rate,times,dev/2.0,showProgress);
   bp_writeBPnet(bp,"bp.data3");
   int a=0;
   double *result=(double*)malloc(bp->numOut*sizeof(double));
   double vals[]={3.0,4.0,5.0,6.0,7.0};
   bp_compute(bp,vals,result);
   for(a=0;a<bp->numIn;a++){
   if(a)printf(",");
   printf("%g",vals[a]);
   }
   printf(":");
   for(a=0;a<bp->numOut;a++){
   if(a)printf(",");
   printf("%g",result[a]);
   }
   printf("\n");
   bp_deleteBPnet(&bp);
   //bp_showData(data);
   clearData(&data);
   free(result);
 */

#endif
