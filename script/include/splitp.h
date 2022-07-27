#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __HAN_SPLITP__
#define __HAN_SPLITP__

typedef struct SPLITP{
	char *str;
	struct SPLITP *next;
}splitp;

int splitP(char ***arr,const char *str,const char *split){
	int a=-1;
	int sum=0;
	int beg=0;
	splitp *head=NULL,*tmp=NULL;
	while(str[++a]){
		int b=0,c=1;
		while(str[a+b] && split[b]){
			if(str[a+b]!=split[b]){
				c=0;
				break;
			}
			if(c==0)break;
			b++;
		}
		if(c && split[b]==0){
			char *s=(char*)malloc((a-beg+1)*sizeof(char));
			memcpy(s,str+beg,a-beg);
			s[a-beg]=0;
			splitp *sp=(splitp*)malloc(sizeof(splitp));
			sp->str=s;
			sp->next=head;
			head=sp;
			a+=strlen(split)-1;
			beg=a+1;
			sum++;
		}
	}
	char *s=(char*)malloc((a-beg+1)*sizeof(char));
	memcpy(s,str+beg,a-beg);
	s[a-beg]=0;
	int ss=sum+1;
	(*arr)=(char**)malloc(ss*sizeof(char**));
	if(sum){
		while(head && --sum>=0){
			(*arr)[sum]=head->str;
			tmp=head;
			head=head->next;
			free(tmp);
		}
	}
	(*arr)[ss-1]=s;
	return ss;
}

#endif

/*
remember to free each element of arr, and arr itself
*/
