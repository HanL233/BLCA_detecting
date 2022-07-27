#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __HAN_UNIQ__
#define __HAN_UNIQ__

typedef struct UNIQ_ITEM{
	char *string;
	void *value;
	struct UNIQ_ITEM *previous;
	struct UNIQ_ITEM *next;
}uniq_item;

typedef void (*UNIQ_CLEAN)(void *);

void **getUniqID(uniq_item **_list,const char *string,long *_sum){
	long sum=0;
	uniq_item *list=*_list,*tmp=NULL;
	void **pointer=NULL;
	if(_sum)sum=*_sum;
	else{
		tmp=list;
		while(tmp){
			sum++;
			tmp=tmp->next;
		}
	}
	if(sum){
		tmp=list;
		long len=sum,dir=0;
		while(len && tmp && (dir=strcmp(tmp->string,string))){
			if(len!=1)len+=len%2;
			len>>=1;
			long l=0;
			while(len && tmp && ++l<=len){
				if(dir<0){
					if(tmp->next)tmp=tmp->next;
				}else{
					if(tmp->previous)tmp=tmp->previous;
				}
			}
		}
		if(dir){
			uniq_item *t=(uniq_item*)malloc(sizeof(uniq_item));
			t->string=(char*)malloc((strlen(string)+1)*sizeof(char));
			strcpy(t->string,string);
			t->value=NULL;
			if(dir<0){
				t->previous=tmp;
				t->next=tmp->next;
				if(tmp->next)tmp->next->previous=t;
				tmp->next=t;
			}else{
				t->previous=tmp->previous;
				t->next=tmp;
				if(tmp->previous)tmp->previous->next=t;
				tmp->previous=t;
				if(tmp==list)*_list=t;
			}
			if(_sum)*_sum=++sum;
			pointer=&(t->value);
		}else{
			pointer=&(tmp->value);
		}
	}else{
		*_list=(uniq_item*)malloc(sizeof(uniq_item));
		(*_list)->value=NULL;
		(*_list)->previous=NULL;
		(*_list)->next=NULL;
		(*_list)->string=(char*)malloc((strlen(string)+1)*sizeof(char));
		strcpy((*_list)->string,string);
		if(_sum)*_sum=1;
		pointer=&((*_list)->value);
	}
	return pointer;
}

char **getUniqKeys(uniq_item *list,long *_sum){
	long sum=0,a=0;
	uniq_item *tmp=list;
	if(_sum)sum=*_sum;
	else{
		while(tmp){
			tmp=tmp->next;
			sum++;
		}
	}
	char **arr=(char**)malloc(sum*sizeof(char*));
	while(list){
		arr[a++]=list->string;
		list=list->next;
	}
	if(_sum)*_sum=sum;
	return arr;
}

void **getUniqValues(uniq_item *list,long *_sum){
	long sum=0,a=0;
	uniq_item *tmp=list;
	if(_sum)sum=*_sum;
	else{
		while(tmp){
			tmp=tmp->next;
			sum++;
		}
	}
	void **arr=(void**)malloc(sum*sizeof(void*));
	while(list){
		arr[a++]=list->value;
		list=list->next;
	}
	if(_sum)*_sum=sum;
	return arr;
}

void clearUniqList(uniq_item **_list,UNIQ_CLEAN fun){
	uniq_item *list=*_list,*tmp=NULL;
	while(list){
		tmp=list;
		list=list->next;
		if(fun)fun(tmp->value);
		free(tmp->string);
		free(tmp);
	}
	*_list=NULL;
}

#endif

/*
   uniq_item *list=NULL;
   long sum=0;
   getUniqID(&list,str,&sum);
   clearUniqList(&list,NULL);
 */
