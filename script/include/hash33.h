#include <stdlib.h>
#include <string.h>

#ifndef __HAN_HASH__
#define __HAN_HASH__

#define HASH_INIT 1024

typedef struct S_HASH{
	void *value;
	char *string;
	struct S_HASH *next;
}_HASH;

typedef struct SHASH{
	unsigned long LEN;
	unsigned long SUM;
	_HASH *hash;
}HASH;

typedef struct SHPOINTER{
	int n;
	_HASH *p;
}HPOINTER;

void moveToBegining(HPOINTER *hp){
	hp->p=NULL;
	hp->n=0;
}

const char *HASH_KV(HASH *hash,HPOINTER *hp,void **value){
	if(hp->p){
		if(hp->p->next){
			hp->p=hp->p->next;
			if(value)*value=hp->p->value;
			return hp->p->string;
		}else{
			if(hp->n<hash->LEN)hp->n++;
		}
	}
	while(hp->n<hash->LEN){
		if((hash->hash)[hp->n].next){
			hp->p=(hash->hash)[hp->n].next;
                        if(value)*value=hp->p->value;
                        return hp->p->string;
		}
		(hp->n)++;
	}
	if(value)*value=NULL;
	return NULL;
}

unsigned long time33(const char *str) { 
	unsigned long hash = 5381;
	int len=strlen(str); 
	/* variant with the hash unrolled eight times */ 
	for (; len >= 8; len -= 8) { 
		hash = ((hash << 5) + hash) + *str++; 
		hash = ((hash << 5) + hash) + *str++; 
		hash = ((hash << 5) + hash) + *str++; 
		hash = ((hash << 5) + hash) + *str++; 
		hash = ((hash << 5) + hash) + *str++; 
		hash = ((hash << 5) + hash) + *str++; 
		hash = ((hash << 5) + hash) + *str++; 
		hash = ((hash << 5) + hash) + *str++; 
	} 
	switch (len) { 
		case 7: hash = ((hash << 5) + hash) + *str++; /* fallthrough... */ 
		case 6: hash = ((hash << 5) + hash) + *str++; /* fallthrough... */ 
		case 5: hash = ((hash << 5) + hash) + *str++; /* fallthrough... */ 
		case 4: hash = ((hash << 5) + hash) + *str++; /* fallthrough... */ 
		case 3: hash = ((hash << 5) + hash) + *str++; /* fallthrough... */ 
		case 2: hash = ((hash << 5) + hash) + *str++; /* fallthrough... */ 
		case 1: hash = ((hash << 5) + hash) + *str++; break; 
		case 0: break; 
	} 
	return hash; 
}

HASH *createHASH(){
	HASH *hash=(HASH*)malloc(sizeof(HASH));
	hash->LEN=HASH_INIT;
	hash->SUM=0;
	hash->hash=(_HASH*)calloc(HASH_INIT,sizeof(_HASH));
	return hash;
}

void clearHASH(HASH *hash,void (*fun_clr)(void*)){
	_HASH *target,*tmp;
	while((hash->LEN)--){
		target=(hash->hash)[hash->LEN].next;
		while(target){
			target=(tmp=target)->next;
			if(fun_clr)fun_clr(tmp->value);
			free(tmp->string);
			free(tmp);
		}
	}
	free(hash->hash);
	free(hash);
}

void **HASH_V(HASH *hash,const char *str,char insert,void (*fun_clr)(void*)){
	if(hash->SUM >= hash->LEN+((hash->LEN)>>1)){
		unsigned long LEN=((hash->LEN)<<1),i,p;
		_HASH *_hash=(_HASH*)calloc(LEN,sizeof(_HASH)),*tmp,*_tmp,*__tmp,*b4;
		for(i=0;i<hash->LEN;i++){
			tmp=(hash->hash)[i].next;
			while(tmp){
				p=time33(tmp->string)%LEN;
				_tmp=_hash[p].next;
				b4=NULL;
				while(_tmp && strcmp(_tmp->string,tmp->string)>0)_tmp=(b4=_tmp)->next;
				tmp=(__tmp=tmp)->next;
				if(b4){ 
					__tmp->next=b4->next;
					b4->next=__tmp;
				}else{  
					_hash[p].next=__tmp;
					__tmp->next=NULL;
				}
			}
		}
		free(hash->hash);
		hash->hash=_hash;
		hash->LEN=LEN;
	}

	unsigned long index=time33(str)%(hash->LEN);
	int cmp;
	_HASH *target=(hash->hash)[index].next,*b4=NULL;
	while(target){
		cmp=strcmp(target->string,str);
		if(cmp==0){
			if(insert>=0){
				return &(target->value);
			}else{
				if(b4)b4->next=target->next;
				else (hash->hash)[index].next=target->next;
				if(fun_clr)fun_clr(target->value);
				free(target->string);
				free(target);
				hash->SUM--;
				return NULL;
			}
		}
		if(cmp<0){
			if(insert>0){
				_HASH *n=(_HASH*)malloc(sizeof(_HASH));
				n->value=NULL;
				n->string=(char*)malloc((strlen(str)+1)*sizeof(char));
				strcpy(n->string,str);
				if(b4){
					n->next=b4->next;
					b4->next=n;
				}else{
					(hash->hash)[index].next=n;
					n->next=target;
				}
				hash->SUM++;
				return &(n->value);
			}else{
				return NULL;
			}
		}
		target=(b4=target)->next;
	}
	if(insert>0){
		_HASH *n=(_HASH*)malloc(sizeof(_HASH));
		n->value=NULL;
		n->string=(char*)malloc((strlen(str)+1)*sizeof(char));
		strcpy(n->string,str);
		if(b4){
			n->next=b4->next;
			b4->next=n;
		}else{
			(hash->hash)[index].next=n;
			n->next=NULL;
		}
		hash->SUM++;
		return &(n->value);
	}
	return NULL;
}

#endif
