#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef __MONKEY_TEMPLET__
#define __MONKEY_TEMPLET__

typedef struct LIST_NOD{
	char *p;
	struct LIST_NOD *next;
}list_nod;

typedef struct LIST_P{
	list_nod *head;
	int length;
	int here;
	int sum;
}list_p;

list_p *createListP(){
	list_p* p=(list_p *)malloc(sizeof(list_p));
	if(p==NULL)return NULL;
	p->length=1;
	p->here=0;
	p->sum=0;
	p->head=(list_nod*)malloc(sizeof(list_nod));
	p->head->p=(char*)malloc(sizeof(char));
	p->head->next=NULL;
	return p;
}

char pushListP(list_p **p,char val){
	if(*p==NULL)return 0;
	((*p)->head->p)[(*p)->here++]=val;
	(*p)->sum++;
	if((*p)->here>=(*p)->length){
		list_nod *tmp=(list_nod*)malloc(sizeof(list_nod));
		if(tmp==NULL){
			(*p)->here--;
			(*p)->sum--;
			return 0;
		}
		tmp->next=(*p)->head;
		(*p)->length<<=1;
		tmp->p=(char*)malloc(((*p)->length)*sizeof(char));
		(*p)->here=0;
		(*p)->head=tmp;
	}
	return 1;
}

int pushListPP(list_p **p,const char *str){
	int sum=0,a=-1;
	while(str[++a] && pushListP(p,str[a])){
		sum++;
	}
	return sum;
}

char popListP(list_p **p){
	if(*p==NULL)return -1;
	if((*p)->sum==0)return -1;
	(*p)->here--;
	(*p)->sum--;
	if((*p)->here<0){
		list_nod *ln=(*p)->head;
		(*p)->head=ln->next;
		free(ln->p);
		(*p)->length>>=1;
		(*p)->here=(*p)->length-1;
		free(ln);
	}
	return ((*p)->head->p)[(*p)->here];
}

int getListArr(list_p *p,char **a){
	if(p==NULL)return 0;
	char *arr=(char*)malloc((p->sum+1)*sizeof(char));
	if(arr==NULL)return 0;
	arr[p->sum]=0;
	int i=1,j=0;
	while(j<p->sum){j+=i;i<<=1;}
	i>>=1;
	list_nod *head=p->head;
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
	if(*a)free(*a);
	*a=arr;
	return p->sum;
}

char getListP(list_p *p,int pos){
	if(p==NULL)return -1;
	if(pos>=p->sum)return -1;
	int i=1,j=0;
	while(j<p->sum){j+=i;i<<=1;}
	i>>=1;
	list_nod *head=p->head;
	if(j==p->sum && head)head=head->next;
	while(head){
		if(pos>=j-i && pos<j)return (head->p)[pos-j+i];
		j-=i;
		i>>=1;
		head=head->next;
	}
	return -1;
}

char compareListP(list_p *p0,list_p *p1){
	if(p0==NULL || p1==NULL)return 0;
	if(p0==p1)return 1;
	if(p0->sum != p1->sum)return 0;
	int s=0;
	int i=1,j=0;
	while(j<p0->sum){j+=i;i<<=1;}
	i>>=1;
	list_nod *head0=p0->head;
	list_nod *head1=p1->head;
	if(j==p0->sum && head0){
		head0=head0->next;
		head1=head1->next;
	}
	while(head0){
		if(head1==NULL)return 0;
		int a=0;
		for(a=j-i;a<j && a<p0->sum;a++){
			if((head0->p)[a-j+i] != (head1->p)[a-j+i]){
				return 0;
			}
		}
		j-=i;
		i>>=1;
		head0=head0->next;
		head1=head1->next;
	}
	return 1;
}

int getListSum(list_p *p){
	if(p)return p->sum;
	else return 0;
}

void deleteListP(list_p **p){
	if(*p==NULL)return;
	list_nod *head=(*p)->head,*tmp=NULL;
	while(head){
		tmp=head;
		head=head->next;
		free(tmp->p);
		free(tmp);
	}
	free(*p);
	(*p)=NULL;
}

char fuseListP(list_p **p,list_p *src){
	if((*p)==NULL || src==NULL)return 0;
	char *arr=NULL;
	int sum=getListArr(src,&arr);
	if(arr==NULL)return 0;
	int s=pushListPP(p,arr);
	free(arr);
	if(s==sum)return 1;
	return 0;
}

list_p *copyListP(list_p *p){
	if(p==NULL)return NULL;
	list_p *lp=createListP();
	lp->sum=p->sum;
	lp->length=p->length;
	lp->here=p->here;
	list_nod **b4=NULL;
	while(p->head){
		char *str=(char*)malloc(p->length*sizeof(char));
		memcpy(str,p->head->p,p->length);
		list_nod *ln=(list_nod*)malloc(sizeof(list_nod));
		ln->p=str;
		ln->next=NULL;
		if(b4)(*b4)->next=ln;
		else lp->head=ln;
		b4=&ln;
		p->head=p->head->next;
		p->length>>=1;
	}
	return lp;
}

#endif

/*
list_p *p=createListP();
pushListP(&p,'a');
char *arr=NULL;
int len=getListArr(p,&arr);
free(arr);
deleteListP(&p);
*/
