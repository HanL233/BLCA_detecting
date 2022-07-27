#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef __HAN__BP__
#define __HAN__BP__

typedef struct BPNODE{
	double *in;
	double *out;
	struct BPNODE *next;
}bpNode;

typedef struct BPDATA{
	char beNormalized;
	int numIn;
	int numOut;
	bpNode *data;
}bpData;

typedef struct BPLAYER{
	double *weight;
	double *value;
	double *dWeight;
}bpLayer;

typedef struct BPNET{
	bpLayer *layerIn;
	bpLayer **layerHidden;
	bpLayer *layerOut;
	int numIn;
	int numHidden;
	int numOut;
	int numHiddenLayer;
	double *inMax;
	double *inMin;
	double *outMax;
	double *outMin;
}bpNet;

typedef void (*bpRate)(double*,int);

typedef void (*bpPro)(int,double);

double bp_rand_max=RAND_MAX;

double bp_getRand(){
	return (double)(rand()*2)/bp_rand_max-1.0;
}

void bp_normalize(bpNet *bp,bpData *data){
	if(data->beNormalized)return;
	data->beNormalized=1;
	double *inMax=bp->inMax;
	double *inMin=bp->inMin;
	double *outMax=bp->outMax;
	double *outMin=bp->outMin;
	bpNode *head=data->data;
	int a=0;
	if(head){
		for(a=0;a<data->numIn;a++){
			inMax[a]=(head->in)[a];
			inMin[a]=(head->in)[a];
		}
		for(a=0;a<data->numOut;a++){
			outMax[a]=(head->out)[a];
			outMin[a]=(head->out)[a];
		}
		head=head->next;
	}
	while(head){
		for(a=0;a<data->numIn;a++){
			if(inMax[a]<(head->in)[a])inMax[a]=(head->in)[a];
			else if(inMin[a]>(head->in)[a])inMin[a]=(head->in)[a];
		}
		for(a=0;a<data->numOut;a++){
			if(outMax[a]<(head->out)[a])outMax[a]=(head->out)[a];
			else if(outMin[a]>(head->out)[a])outMin[a]=(head->out)[a];
		}
		head=head->next;
	}
	head=data->data;
	while(head){
		for(a=0;a<data->numIn;a++){
			(head->in)[a]=((head->in)[a]-inMin[a]+1.0)/(inMax[a]-inMin[a]+1.0);
		}
		for(a=0;a<data->numOut;a++){
			(head->out)[a]=((head->out)[a]-outMin[a]+1.0)/(outMax[a]-outMin[a]+1.0);
		}
		head=head->next;
	}
}

bpNet *bp_createBPnet(int numIn,int numHidden,int numOut,int numHiddenLayer){
	bpNet *bp=(bpNet*)malloc(sizeof(bpNet));
	bpLayer **layerHidden=(bpLayer**)malloc(numHiddenLayer*sizeof(bpLayer*));
	int a=0,b=0;
	for(a=0;a<numHiddenLayer;a++){
		bpLayer *ly=(bpLayer*)malloc(sizeof(bpLayer));
		double *wg=(double*)malloc(numHidden*numHidden*sizeof(double));
		double *dw=(double*)malloc(numHidden*numHidden*sizeof(double));
		double *vl=(double*)malloc(numHidden*sizeof(double));
		for(b=0;b<numHidden*numHidden;b++){
			wg[b]=bp_getRand();
			dw[b]=0.0;
		}
		ly->weight=wg;
		ly->dWeight=dw;
		ly->value=vl;
		layerHidden[a]=ly;
	}
	bpLayer *layIn=(bpLayer*)malloc(sizeof(bpLayer));
	layIn->weight=(double*)malloc(numIn*numHidden*sizeof(double));
	layIn->dWeight=(double*)malloc(numIn*numHidden*sizeof(double));
	layIn->value=(double*)malloc(numIn*sizeof(double));
	for(a=0;a<numIn*numHidden;a++){
		(layIn->weight)[a]=bp_getRand();
		(layIn->dWeight)[a]=0.0;
	}
	bpLayer *layOut=(bpLayer*)malloc(sizeof(bpLayer));
	layOut->weight=(double*)malloc(numHidden*numOut*sizeof(double));
	layOut->dWeight=(double*)malloc(numHidden*numOut*sizeof(double));
	layOut->value=(double*)malloc(numOut*sizeof(double));
	for(a=0;a<numHidden*numOut;a++){
		(layOut->weight)[a]=bp_getRand();
		(layOut->dWeight)[a]=0.0;
	}
	bp->numIn=numIn;
	bp->numHidden=numHidden;
	bp->numOut=numOut;
	bp->numHiddenLayer=numHiddenLayer;
	bp->layerIn=layIn;
	bp->layerHidden=layerHidden;
	bp->layerOut=layOut;
	bp->inMax=(double*)malloc(numIn*sizeof(double));
	bp->inMin=(double*)malloc(numIn*sizeof(double));
	bp->outMax=(double*)malloc(numOut*sizeof(double));
	bp->outMin=(double*)malloc(numOut*sizeof(double));
	return bp;
}

void bp_forward(bpNet *bp,double *in){
	int a=0,b=0,c=0;
	double value=0.0;
	double *weight=NULL;
	double *b4=NULL;
	weight=(bp->layerIn)->weight;
	for(a=0;a<bp->numHidden;a++){
		value=0.0;
		for(b=0;b<bp->numIn;b++){
			value+=weight[a*bp->numIn+b]*in[b];
		}
		(((bp->layerHidden)[0])->value)[a]=1.0/(1.0+exp(-1.0*value));
	}
	for(a=1;a<bp->numHiddenLayer;a++){
		weight=((bp->layerHidden)[a])->weight;
		b4=((bp->layerHidden)[a-1])->value;
		for(b=0;b<bp->numHidden;b++){
			value=0.0;
			for(c=0;c<bp->numHidden;c++){
				value+=weight[b*bp->numHidden+c]*b4[c];
			}
			(((bp->layerHidden)[a])->value)[b]=1.0/(1.0+exp(-1.0*value));
		}
	}
	b4=((bp->layerHidden)[bp->numHiddenLayer-1])->value;
	weight=(bp->layerOut)->weight;
	for(a=0;a<bp->numOut;a++){
		value=0.0;
		for(b=0;b<bp->numHidden;b++){
			value+=weight[a*bp->numHidden+b]*b4[b];
		}
		((bp->layerOut)->value)[a]=value;
	}
	memcpy((bp->layerIn)->value,in,bp->numIn*sizeof(double));
}

void bp_backward(bpNet *bp,double *out,double *rate){
	double deviation=0.0,_deviation=0.0,dev=0.0;
	int a=0,b=0,c=0,d=0;
	double *weight=NULL;
	double *dWeight=NULL;
	double *value=NULL;
	double *b4=NULL;
	for(a=0;a<bp->numHidden;a++){
		b4=(((bp->layerHidden)[bp->numHiddenLayer-1]))->value;
		deviation=0.0;
		weight=(bp->layerOut)->weight;
		dWeight=(bp->layerOut)->dWeight;
		value=(bp->layerOut)->value;
		for(b=0;b<bp->numOut;b++){
			c=a+b*bp->numHidden;
			deviation+=(value[b]-out[b])*weight[c];
			dWeight[c]=rate[0]*dWeight[c]+rate[1]*(value[b]-out[b])*b4[a];
			weight[c]-=dWeight[c];
		}
		for(b=bp->numHiddenLayer-1;b>0;b--){
			_deviation=0.0;
			b4=(((bp->layerHidden)[b-1]))->value;//ATTENTION
			weight=((bp->layerHidden)[b])->weight;
			dWeight=((bp->layerHidden)[b])->dWeight;
			value=((bp->layerHidden)[b])->value;
			for(c=0;c<bp->numHidden;c++){
				d=a*bp->numHidden+c;//ATTENTION
				_deviation+=(dev=value[c]*(1.0-value[c])*deviation);
				dWeight[d]=rate[2]*dWeight[c]+rate[3]*dev*b4[c];
				weight[d]-=dWeight[d];
			}
			deviation=_deviation;
		}
		b4=(((bp->layerHidden)[0]))->value;
		weight=(bp->layerIn)->weight;
		dWeight=(bp->layerIn)->dWeight;
		value=(bp->layerIn)->value;
		for(b=0;b<bp->numIn;b++){
			c=a*bp->numIn+b;
			dWeight[c]=rate[2]*dWeight[c]+rate[3]*deviation*b4[a]*(1.0-b4[a])*value[b];
			weight[c]-=dWeight[c];
		}
	}
}

double bp_getDeviation(bpNet *bp,double *out){
	double dev=0.0;
	int a=0;
	double *value=(bp->layerOut)->value;
	for(a=0;a<bp->numOut;a++){
		dev+=fabs(value[a]-out[a])/out[a];
	}
	return dev;
}

void bp_train(bpNet *bp,bpData *data,bpRate getRate,double *_rate,int times,double _dev,bpPro showPro){
	double *rate=(double*)malloc(4*sizeof(double));
	if(getRate==NULL)memcpy(rate,_rate,4*sizeof(double));
	bpNode *head=data->data,*tmp=NULL;
	double dev=0.0;
	int a=0;
	do{
		dev=0.0;
		tmp=head;
		double ss=0.0;
		if(getRate)getRate(rate,a);
		while(tmp){
			bp_forward(bp,tmp->in);
			bp_backward(bp,tmp->out,rate);
			dev+=bp_getDeviation(bp,tmp->out);
			tmp=tmp->next;
			ss+=1.0;
		}
		a++;
		dev/=ss;
		if(showPro)showPro(a,dev);
	}while(times>a && dev>_dev);
	free(rate);
}

void bp_deleteBPnet(bpNet **_bp){
	bpNet *bp=*_bp;
	int a=0;
	for(a=0;a<bp->numHiddenLayer;a++){
		free((bp->layerHidden)[a]->weight);
		free((bp->layerHidden)[a]->dWeight);
		free((bp->layerHidden)[a]->value);
		free((bp->layerHidden)[a]);
	}
	free(bp->layerHidden);
	free((bp->layerIn)->weight);
	free((bp->layerIn)->dWeight);
	free((bp->layerIn)->value);
	free(bp->layerIn);
	free((bp->layerOut)->weight);
	free((bp->layerOut)->dWeight);
	free((bp->layerOut)->value);
	free(bp->layerOut);
	free(bp->inMax);
	free(bp->inMin);
	free(bp->outMax);
	free(bp->outMin);
	free(bp);
	*_bp=NULL;
}

void bp_compute(bpNet *bp,const double *_in,double *out){
	double *inMax=bp->inMax;
	double *inMin=bp->inMin;
	double *outMax=bp->outMax;
	double *outMin=bp->outMin;
	double *in=(double*)malloc(bp->numIn*sizeof(double));
	memcpy(in,_in,bp->numIn*sizeof(double));
	int a=0;
	for(a=0;a<bp->numIn;a++){
		in[a]=(in[a]-inMin[a]+1.0)/(inMax[a]-inMin[a]+1.0);
	}
	bp_forward(bp,in);
	memcpy(out,(bp->layerOut)->value,bp->numOut*sizeof(double));
	for(a=0;a<bp->numOut;a++){
		out[a]=out[a]*(outMax[a]-outMin[a]+1.0)+outMin[a]-1.0;
	}
	free(in);
}

void bp_clearData(bpData **_head){
	bpNode *head=(*_head)->data,*tmp=NULL;
	while(head){
		tmp=head;
		head=head->next;
		free(tmp->in);
		free(tmp->out);
		free(tmp);
	}
	free(*_head);
	*_head=NULL;
}

#endif
