#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef __MONKEY_TEMPLET_SUM__
#define __MONKEY_TEMPLET_SUM__

typedef struct GSUM_LIST_NOD{
	void **p;
	struct GSUM_LIST_NOD *next;
}g_list_nod;

typedef struct GLIST_P{
	g_list_nod *head;
	int length;
	int here;
	int sum;
}g_list_p;

g_list_p *g_createListP(){
	g_list_p* p=(g_list_p *)malloc(sizeof(g_list_p));
	if(p==NULL)return NULL;
	p->length=1;
	p->here=0;
	p->sum=0;
	p->head=(g_list_nod*)malloc(sizeof(g_list_nod));
	p->head->p=(void **)malloc(sizeof(void *));
	p->head->next=NULL;
	return p;
}

char g_pushListP(g_list_p **p,void * val){
	if(*p==NULL)return 0;
	((*p)->head->p)[(*p)->here++]=val;
	(*p)->sum++;
	if((*p)->here>=(*p)->length){
		g_list_nod *tmp=(g_list_nod*)malloc(sizeof(g_list_nod));
		if(tmp==NULL){
			(*p)->here--;
			(*p)->sum--;
			return 0;
		}
		tmp->next=(*p)->head;
		(*p)->length<<=1;
		tmp->p=(void **)malloc(((*p)->length)*sizeof(void *));
		(*p)->here=0;
		(*p)->head=tmp;
	}
	return 1;
}

int g_getListArr(g_list_p *p,void ***a){
	if(p==NULL)return 0;
	void **arr=(void **)malloc((p->sum+1)*sizeof(void *));
	if(arr==NULL)return 0;
	arr[p->sum]=0;
	int i=1,j=0;
	while(j<p->sum){j+=i;i<<=1;}
	i>>=1;
	g_list_nod *head=p->head;
	if(j==p->sum && head)head=head->next;
	while(head){
		int a=0;
		for(a=j-i;a<j && a<p->sum;a++){
			arr[a]=(head->p)[a-j+i];
		}
		j-=i;
		i>>=1;
		head=head->next;
	}
	*a=arr;
	return p->sum;
}

void g_deleteListP(g_list_p **p){
	if(*p==NULL)return;
	g_list_nod *head=(*p)->head,*tmp=NULL;
	while(head){
		tmp=head;
		head=head->next;
		free(tmp->p);
		free(tmp);
	}
	free(*p);
	(*p)=NULL;
}

void g_clearListP(g_list_p *p,void (*f)(void*)){
	if(p==NULL)return;
	int i=1,j=0;
	while(j<p->sum){j+=i;i<<=1;}
	i>>=1;
	g_list_nod *head=p->head;
	if(j==p->sum && head)head=head->next;
	while(head){
		int a=0;
		for(a=j-i;a<j && a<p->sum;a++){
			f((head->p)[a-j+i]);
		}
		j-=i;
		i>>=1;
		head=head->next;
	}
}

#endif

/*
g_list_p *p=g_createListP();
sum pushListP(&p,'a');
void **arr=NULL;
int len=getListArr(p,&arr);
free(arr);
deleteListP(&p);
*/
