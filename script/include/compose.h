#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __HAN_COMPOSE__
#define __HAN_COMPOSE__

typedef struct COMPOSE_ITEM{
	void *data;
	struct COMPOSE_ITEM *next;
}compose_item;

typedef struct COMPOSE_LINK{
	float adapt;
	compose_item *item;
	struct COMPOSE_LINK *shift;
	struct COMPOSE_LINK *previous;
	struct COMPOSE_LINK *next;
}compose_link;

typedef float (*ADAPT)(compose_item*);

float rand_max=RAND_MAX;

compose_link *compose_addItem(compose_link **_head,compose_link **_tail,compose_item *item,ADAPT fun){
	compose_link *head=*_head;
	compose_link *tail=*_tail;
	compose_link *tmp=(compose_link*)malloc(sizeof(compose_link));
	tmp->previous=tail;
	tmp->next=NULL;
	tmp->item=item;
	tmp->adapt=fun(item);
	tmp->shift=NULL;
	if(head==NULL){
		head=tmp;
	}else{
		tail->next=tmp;
	}
	tail=tmp;
	*_head=head;
	*_tail=tail;
	return tmp;
}

compose_link *compose_addData(compose_link **_head,compose_link **_tail,void *data,ADAPT fun){
	compose_item *item=(compose_item*)malloc(sizeof(compose_item));
	item->data=data;
	item->next=NULL;
	return compose_addItem(_head,_tail,item,fun);
}

void compose_composeItem(compose_link **_head,compose_link **_tail,compose_link *itema,compose_link *itemb,ADAPT fun){
	compose_link *head=*_head;
	compose_link *tail=*_tail;
	compose_item *t=itemb->item,*b4=NULL;
	while(t){
		b4=t;
		t=t->next;
	}
	b4->next=itema->item;
	itema->item=itemb->item;
	itema->adapt=fun(itema->item);
	if(itemb->next){
		itemb->next->previous=itemb->previous;
	}else{
		tail=itemb->previous;
	}
	if(itemb->previous){
		itemb->previous->next=itemb->next;
	}else{
		head=head->next;
	}
	free(itemb);
	*_head=head;
	*_tail=tail;
}

void compose_decomposeItem(compose_link **_head,compose_link **_tail,compose_link *link,float *_length,ADAPT fun){
	compose_item *head=NULL,*tail=NULL,*tmp=NULL,*tt=NULL,*ta=NULL,*tb=NULL,*tc=NULL,*l=NULL,*m=NULL;
	float max=0.0f,ada=0.0f,peak=0.0f;
	char st=0;
	tmp=head=link->item;
	tail=NULL;
	while(tmp){
		tail=tmp;
		tmp=tmp->next;
	}
	if(tail==NULL || tail==head){
		return;
	}
	tail->next=head;//circle
	while(tail!=head){
		ta=head;
		tb=ta->next;
		tc=tb->next;
		tt=NULL;
		st=1;
		do{
			ta->next=NULL;
			ada=fun(tc);
			if(st || max<ada){
				max=ada;
				tt=ta;
				st=0;
			}
			ta->next=tb;
			ta=ta->next;
			tb=tb->next;
			tc=tc->next;
			
		}while(ta!=head);
		head=tt->next->next;
		tt->next->next=l;
		l=tt->next;
		tail=tt;
		tail->next=head;
		if(peak<max){
			peak=max;
			m=l;
		}
	}
	head->next=l;
	if(peak){
		tt=m;
		m=m->next;
		tt->next=NULL;
		link->item=head;
		link->adapt=fun(link->item);
		if(m){
			compose_addItem(_head,_tail,m,fun);
			*_length+=1.0f;
		}
	}else{
		link->item=head;
	}
}

void compose_rand(compose_link **_head,compose_link **_tail,float *_length){
	compose_link *head=*_head;
	compose_link *tail=*_tail;
	compose_link *link=head,*tmp=NULL,*t=NULL;
	int dist=0,a=0;
	float length=*_length;
	compose_link **arr=(compose_link**)malloc(((int)(length))*sizeof(compose_link*));//+1
	link=head;
	while(link){
		dist=(int)((0.5f-(float)(rand())/rand_max)*length+0.5f);
		tmp=link;
		while(dist && tmp){
			if(dist>0){
				tmp=tmp->previous;
				dist--;
			}else{
				if(tmp->next)tmp=tmp->next;//|->
				dist++;
			}
		}
		link->shift=tmp;
		arr[a++]=link;
		link=link->next;
	}
	while(a--){
		tmp=arr[a];
		if(tmp==tmp->shift)continue;
		if(tmp->previous){
			tmp->previous->next=tmp->next;
		}else{
			head=tmp->next;
		}
		if(tmp->next){
			tmp->next->previous=tmp->previous;
		}else{
			tail=tmp->previous;
		}
		if(tmp->shift){
			if(tmp->shift->next){
				tmp->shift->next->previous=tmp;
			}else{
				tail=tmp;
			}
			tmp->next=tmp->shift->next;
			tmp->shift->next=tmp;
			tmp->previous=tmp->shift;
		}else{
			head->previous=tmp;
			tmp->next=head;
			tmp->previous=NULL;
			head=tmp;
		}
	}
	free(arr);
	*_head=head;
	*_tail=tail;
}

void compose(compose_link **_head,compose_link **_tail,float *_length,ADAPT fun){
	compose_rand(_head,_tail,_length);
	compose_link *head=*_head;
	compose_link *tail=*_tail;
	compose_link *link=head,*tmp=NULL,*t=NULL;
	compose_item *h=NULL,*b4=NULL;
	int dist=0,d=0,a=0;
	float max=0.0f,adapt=0.0f,length=*_length;
	while(link && link->next){
		length-=1.0f;
		compose_composeItem(&head,&tail,link,link->next,fun);
		compose_decomposeItem(&head,&tail,link,&length,fun);
		link=link->next;
	}
	*_head=head;
	*_tail=tail;
	*_length=length;
}

void compose_clearComposeLink(compose_link **_head,compose_link **_tail,float *length){
	compose_link *head=*_head,*tmp=NULL;
	compose_item *item=NULL,*t=NULL;
	while(head){
		tmp=head;
		head=head->next;
		item=tmp->item;
		while(item){
			t=item;
			item=item->next;
			free(t);
		}
		free(tmp);
	}
	*length=0.0f;
	*_head=NULL;
	*_tail=NULL;
}

#endif


/*
int ann(rec *area,int generation,ADAPT apt,ADAPT fun){
        compose_link *head=NULL,*tail=NULL,*tmp=NULL;
        compose_item *t=NULL;
        float length=0.0f;
        int a=0;
        while(area){
                compose_addData(&head,&tail,(void*)area,apt);
                area=area->next;
                length+=1.0f;
        }
        int gen=0;
        while(gen++<generation){
                fprintf(stderr,"%02d -> %g\n",gen,length);
                compose(&head,&tail,&length,apt);
        }
        tmp=head;
        while(tmp){
                fun(tmp->item);
                tmp=tmp->next;
        }
        compose_clearComposeLink(&head,&tail,&length);
        return 0;
}

int greedy(rec *area,ADAPT apt,ADAPT fun){
        compose_link *head=NULL,*tail=NULL,*tmp=NULL;
        compose_item *t=NULL;
        float length=1.0f;
        int a=0;
        compose_item *item=NULL;
        while(area){
                compose_item *it=(compose_item*)malloc(sizeof(compose_item));
                it->data=(void*)area;
                it->next=item;
                item=it;
                area=area->next;
        }
        compose_addItem(&head,&tail,item,apt);
        compose_decomposeItem(&head,&tail,head,&length,apt);
        tmp=head;
        while(tmp){
                fun(tmp->item);
                tmp=tmp->next;
        }
        compose_clearComposeLink(&head,&tail,&length);
        return 0;
}
*/
