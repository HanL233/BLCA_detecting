#include <stdlib.h>

typedef struct GUNIQ_ITEM{
	void *key;
	void *value;
	struct GUNIQ_ITEM *previous;
	struct GUNIQ_ITEM *next;
}guniq_item;

typedef void (*GUNIQ_CLEAN)(void *);
typedef char (*GUNIQ_CMP)(void*,void*);

void **g_getUniqID(guniq_item **_list,void *key,GUNIQ_CMP cmp,char insert,int *_sum){
	int sum=0;
	guniq_item *list=*_list,*tmp=NULL;
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
		int len=sum,dir=0;
		while(len && tmp && (dir=cmp(tmp->key,key))){
			if(len!=1)len+=len%2;
			len>>=1;
			int l=0;
			while(len && tmp && ++l<=len){
				if(dir<0){
					if(tmp->next)tmp=tmp->next;
				}else{
					if(tmp->previous)tmp=tmp->previous;
				}
			}
		}
		if(dir && insert){
			guniq_item *t=(guniq_item*)malloc(sizeof(guniq_item));
			t->key=key;
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
	}else if(insert){
		*_list=(guniq_item*)malloc(sizeof(guniq_item));
		(*_list)->value=NULL;
		(*_list)->previous=NULL;
		(*_list)->next=NULL;
		(*_list)->key=key;
		if(_sum)*_sum=1;
		pointer=&((*_list)->value);
	}
	return pointer;
}

void **g_getUniqKeys(guniq_item *list,int *_sum){
	int sum=0,a=0;
	guniq_item *tmp=list;
	if(_sum)sum=*_sum;
	else{
		while(tmp){
			tmp=tmp->next;
			sum++;
		}
	}
	void **arr=(void**)malloc(sum*sizeof(void*));
	while(list){
		arr[a++]=list->key;
		list=list->next;
	}
	if(_sum)*_sum=sum;
	return arr;
}

void **g_getUniqValues(guniq_item *list,int *_sum){
	int sum=0,a=0;
	guniq_item *tmp=list;
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

void g_clearUniqList(guniq_item **_list,GUNIQ_CLEAN clearKey,GUNIQ_CLEAN clearValue){
	guniq_item *list=*_list,*tmp=NULL;
	int a=0;
	while(list){
		tmp=list;
		list=list->next;
		if(clearValue)clearValue(tmp->value);
		if(clearKey)clearKey(tmp->key);
		free(tmp);
	}
	*_list=NULL;
}

char g_cmp(void *a,void *b){
	unsigned long i=(unsigned long)a;
	unsigned long j=(unsigned long)b;
	if(a>b)return 1;
	if(a<b)return -1;
	return 0;
}

int g_uniq(void ***_arr,int length){
	void **arr=*_arr;
	guniq_item *list=NULL;
	int a=0,sum=0;
	for(a=0;a<length;a++){
		g_getUniqID(&list,arr[a],g_cmp,1,&sum);
	}
	free(arr);
	*_arr=g_getUniqKeys(list,&sum);
	g_clearUniqList(&list,NULL,NULL);
	return sum;
}

/*
guniq_item *list=NULL;
int sum=0;
void *ID=g_getUniqID(&list,(void*)key,cmp,1,&sum);
if(ID!=NULL)free(key);
g_clearUniqList(&list,clr,clr);
*/
