#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef __T_TEST__
#define __T_TEST__

float sum(float *arr,int len){
	if(arr==NULL || len==0){
		return 0.0f;
	}
	float s=0.0f;
	int i=0;
	for(i=0;i<len;i++){
		s+=arr[i];
	}
	return s;
}

float mean(float *arr,int len){
	if(arr==NULL || len==0){
		return 0.0f;
	}
	float s=sum(arr,len);
	return s/((float)len);
}

float var(float *arr,int len){
	if(arr==NULL || len<2){
		return 0.0f;
	}
	float m=mean(arr,len);
	float s=0.0f,t=0.0f;
	int i=0;
	for(i=0;i<len;i++){
		t=arr[i]-m;
		s+=t*t;
	}
	s/=(float)(len-1);
	return s;
}

float sd(float *arr,int len){
	if(arr==NULL || len<2){
		return 0.0f;
	}
	return sqrt(var(arr,len));
}

float var_(float *arr,int len,float m){
	if(arr==NULL || len<2){
		return 0.0f;
	}
	float s=0.0f,t=0.0f;
	int i=0;
	for(i=0;i<len;i++){
		t=arr[i]-m;
		s+=t*t;
	}
	s/=(float)(len-1);
	//printf("mean=%g\n",m);
	//printf("var=%g\n",s);
	return s;
}

float sd_(float *arr,int len,float m){
	if(arr==NULL || len<2){
		return 0.0f;
	}
	return sqrt(var_(arr,len,m));
}

float tValue(float *arrA,int lenA,float *arrB,int lenB){
	if(arrA==NULL || lenA<2 || arrB==NULL || lenB<2){
		return 0.0f;
	}
	float mA=mean(arrA,lenA),mB=mean(arrB,lenB);
	float vA=var_(arrA,lenA,mA),vB=var_(arrB,lenB,mB);
	if(vA==0 || vB==0)return 0.0f;
	//float sA=sd_(arrA,lenA,mA),sB=sd_(arrB,lenB,mB);
	float lA=(float)lenA,lB=(float)lenB;
	//return (mA-mB)/sqrt(((lA-1.0f)*vA+(lB-1.0f)*vB)/(lA+lB-2.0f)*(1.0f/lA+1.0f/lB));
	return (mA-mB)/sqrt(vA/lA+vB/lB);
}

void scale(float *arr,int len){
	float m=mean(arr,len);
	float s=sd_(arr,len,m);
	int i=0;
	if(s>0){
		for(i=0;i<len;i++){
			arr[i]=(arr[i]-m)/s;
		}
	}
}

#endif
