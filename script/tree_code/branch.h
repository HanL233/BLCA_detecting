#include <string.h>
#include <gListp.h>

#define LEAF_MAX_DISTANCE 50
#define LEAF_MAX_SKIP 20

typedef struct SLEAF{
	char available;
	int support;
	int center;
	int from;
	int to;
	int length;
	int *position;
	struct SLEAF *previous;
	struct SLEAF *next;
}LEAF;

int getSupport(float **matrix,int width,int height,int *position,int length,int min_support){
	int support=0,_support=0,min=height-min_support;
	int i=0,j=0;
	for(i=0;i<height;i++){
		for(j=0;j<length-1;j++){
			if(matrix[i][position[j]]<=matrix[i][position[j+1]])break;
		}
		if(j==length-1){
			support++;
		}else{
			_support++;
			if(_support>min)return 0;
		}
	}
	return support;
}

LEAF *getHere(LEAF *head,LEAF *tail,LEAF *here,int from,int to){//TODO TEST
	if(here==NULL)here=head;
	int center=from+to;
	//while(here && (here->center<center || (here->center==center && here->to<to)))here=here->next;
	//while(here && (here->center>center || (here->center==center && here->from<from)))here=here->previous;
	while(here && here->center<center)here=here->next;
	while(here && here->center>center)here=here->previous;
	if(here==NULL)here=tail;
	return here;
}

int *getPosition(int *_pos0,int _len0,int *_pos1,int _len1,int *len){
	*len=0;
	int *pos0=_pos0,*pos1=_pos1,len0=_len0,len1=_len1;
	if(len0>len1){//keep the pos0 be the shorter one
		pos0=_pos1;pos1=_pos0;len0=_len1;len1=_len0;
	}
	//check whether there is any common element(s), and whether the orders are the same
	int *common=(int*)malloc(len0*sizeof(int));
	int i=0,j=0,k=0,l=0;
	for(i=0;i<len0;i++){
		common[i]=-1;
		for(j=0;j<len1;j++){
			if(pos0[i]==pos1[j]){
				for(k=0;k<i;k++){
					if(common[k]>j){//order conflict
						free(common);
						return NULL;
					}
				}
				common[i]=j;
				l++;
				break;
			}
		}
	}
	if(l==0 || l==len0){//no common element or one is included by another one
		free(common);
		return NULL;
	}
	//check whether no element or just element(s) of one side existing before the first common element
	int first=0;
	while(common[first]<0)first++;
	if(first!=0 && common[first]!=0){
		free(common);
		return NULL;
	}
	*len=len0+len1-l;
	int *pos=(int*)malloc((*len)*sizeof(int));
	int n=0;
	for(i=0;i<first;i++)pos[n++]=pos0[i];
	for(i=0;i<common[first];i++)pos[n++]=pos1[i];
	pos[n++]=pos0[first];
	//check whether no element or just element(s) of one side existing between two adjacent common elements
	for(i=first+1;i<len0;i++){
		if(common[i]>0){
			if(first+1!=i && common[first]+1!=common[i]){
				*len=0;
				free(common);
				free(pos);
				return NULL;
			}
			for(j=first+1;j<i;j++)pos[n++]=pos0[j];
			for(j=common[first]+1;j<common[i];j++)pos[n++]=pos1[j];
			first=i;
			pos[n++]=pos0[first];
		}
	}
	//check whether no element or just element(s) of one side existing after the last common element
	if(first!=len0-1 && common[first]!=len1-1){
		*len=0;
		free(common);
		free(pos);
		return NULL;
	}
	for(j=first+1;j<len0;j++)pos[n++]=pos0[j];
	for(j=common[first]+1;j<len1;j++)pos[n++]=pos1[j];
	free(common);
	return pos;
}

char isTheSame(int *arr0,int *arr1,int len){
	while(len--){
		if(arr0[len]!=arr1[len])return 0;
	}
	return 1;
}

char isIncluded(int *_arr0,int _len0,int *_arr1,int _len1){
	int *arr0=_arr0,*arr1=_arr1;
	int len0=_len0,len1=_len1;
	if(len0<len1){//len0 should be larger than len1
		arr0=_arr1;
		arr1=_arr0;
		len0=_len1;
		len1=_len0;
	}
	int i=0,j=-1;
	len0-=len1-1;
	for(i=0;i<len1;i++){
		for(j++;j<len0+i;j++){
			if(arr0[j]==arr1[i]){
				break;
			}
		}
		if(j==len0+i)return 0;
	}
	/*len0+=len1-1;
	for(i=0;i<len0;i++){
		if(i)fprintf(stderr,",");
		fprintf(stderr,"%d",arr0[i]);
	}
	fprintf(stderr," - ");
	for(i=0;i<len1;i++){
		if(i)fprintf(stderr,",");
		fprintf(stderr,"%d",arr1[i]);
	}
	fprintf(stderr,"\n");
	fprintf(stderr,"get\n");*/
	//exit(0);
	return 1;
}

LEAF *insertLEAF(LEAF **head,LEAF **tail,LEAF *here,int *position,int length,int support){
	int i=0;
	int min=position[0];
	int max=min;
	for(i=1;i<length;i++){
		if(min>position[i]){
			min=position[i];
		}else if(max<position[i]){
			max=position[i];
		}
	}
	LEAF *leaf=(LEAF*)malloc(sizeof(LEAF));
	leaf->available=1;
	leaf->support=support;
	leaf->center=max+min;
	leaf->from=min;
	leaf->to=max;
	leaf->length=length;
	leaf->position=position;
	here=getHere(*head,*tail,here,min,max);
	if(here){
		leaf->next=here->next;
		if(here->next){
			here->next->previous=leaf;
		}else{
			*tail=leaf;
		}
		here->next=leaf;
		leaf->previous=here;
	}else{
		leaf->next=*head;
		if(*head){
			(*head)->previous=leaf;
		}else{
			*tail=leaf;
		}
		leaf->previous=NULL;
		*head=leaf;
	}
	return leaf;
}

void deleteLEAF(LEAF **head,LEAF **tail,LEAF *here){
	if(*head!=*tail){
		if(here->previous){
			here->previous->next=here->next;
		}else{
			*head=(*head)->next;
			if(*head)(*head)->previous=NULL;
		}
		if(here->next){
			here->next->previous=here->previous;
		}else{
			*tail=(*tail)->previous;
			if(*tail)(*tail)->next=NULL;
		}
	}else{
		*head=NULL;
		*tail=NULL;
	}
	free(here->position);
	free(here);
}

void clearLEAF(LEAF **head,LEAF **tail){
	*tail=NULL;
	LEAF *tmp=NULL;
	while(*head){
		*head=(tmp=*head)->next;
		free(tmp->position);
		free(tmp);
	}
}

void showSingleLEAF(LEAF *here){
	int i=0;
	printf("center=%d,support=%d,position=",here->center,here->support);
	for(i=0;i<here->length;i++){
		if(i)printf(",");
		printf("%d",(here->position)[i]);
	}
	printf("\n");
}

void showLEAF(LEAF *head){
	while(head){
		showSingleLEAF(head);
		head=head->next;
	}
}

int *copyPosition(int *position,int length){
	int *p=(int*)malloc(length*sizeof(int));
	memcpy(p,position,length*sizeof(int));
	return p;
}


int initLEAF(float **matrix,int width,int height,LEAF **head,LEAF **tail,int cur_support,int min_support){
	LEAF *last=NULL;//,*_tmp=NULL;
	int position[2];
	int i=0,j=0,sum=0;//,flag=0;
	//int from=0,to=0;
	for(i=0;i<width;i++){
		for(j=1;j<=16;j++){
			if(i+(1<<j)==i || i+(1<<j)>=width)continue;
			position[0]=i;
			position[1]=i+(1<<j);
			int support=getSupport(matrix,width,height,position,2,min_support);
			if(support>=min_support){
				last=insertLEAF(head,tail,last,copyPosition(position,2),2,support);
				sum++;
			}
			position[1]=i;
			position[0]=i+(1<<j);
			support=getSupport(matrix,width,height,position,2,min_support);
			if(support>=min_support){
				last=insertLEAF(head,tail,last,copyPosition(position,2),2,support);
				sum++;
			}
		}
	}
	return sum;
}

void showMiniSingleLEAF(LEAF *here,int total){
	int i=0;
	for(i=0;i<here->length;i++){
		if(i)printf(",");
		printf("%d",(here->position)[i]);
	}
	printf("\t%d\t%d\n",here->support,total-here->support);
}

int _updateLEAF(float **matrix,int width,int height,LEAF **head,LEAF **tail,int cur_support){
	LEAF *first=*head,*second=*head,*third=*head,*tmp=NULL,*_tmp=NULL;
	int *pos=NULL;
	int len=0,sup=0,sum=0,_sum=0;
	while(first){
		//if(first->available>=0){
			while(second && second->center+LEAF_MAX_DISTANCE<first->center)second=second->next;
			while(third && third->center<LEAF_MAX_DISTANCE+first->center)third=third->next;
			tmp=second;
			while(tmp && tmp!=third){
				//if(tmp!=first && tmp->available>=0){
				if(tmp!=first){
					pos=getPosition(tmp->position,tmp->length,first->position,first->length,&len);
					/*if(pos){
						_tmp=tmp->previous;
						while(_tmp && (_tmp->from>=first->from || _tmp->from>=tmp->from)){
							if(_tmp!=first && len==_tmp->length && isTheSame(pos,_tmp->position,len)){
								free(pos);
								pos=NULL;
								break;
							}
							_tmp=_tmp->previous;
						}
					}
					if(pos){
						_tmp=tmp->next;
						while(_tmp && (_tmp->to<=first->to || _tmp->to<=tmp->to)){
							if(_tmp!=first && len==_tmp->length && isTheSame(pos,_tmp->position,len)){
								free(pos);
								pos=NULL;
								break;
							}
							_tmp=_tmp->next;
						}
					}*/
					if(pos){
						_tmp=second;
						while(_tmp && _tmp!=third){
							if(_tmp!=first && _tmp!=tmp && len<=_tmp->length && isIncluded(_tmp->position,_tmp->length,pos,len)){
							//if(_tmp!=first && _tmp!=tmp && len==_tmp->length && isTheSame(_tmp->position,pos,len)){
								free(pos);
								pos=NULL;
								break;
							}
							_tmp=_tmp->next;
						}
					}
					if(pos){
						sup=getSupport(matrix,width,height,pos,len,cur_support);

						if(sup>=cur_support){
							/*if(len==3 && pos[0]==19 && pos[1]==21 && pos[2]==25){
								fprintf(stderr,"--> ");
								int i=0;
								for(i=0;i<tmp->length;i++){
									if(i)fprintf(stderr,",");
									fprintf(stderr,"%d",(tmp->position)[i]);
								}
								fprintf(stderr," + ");
								for(i=0;i<first->length;i++){
									if(i)fprintf(stderr,",");
									fprintf(stderr,"%d",(first->position)[i]);
								}
								fprintf(stderr," = ");
								for(i=0;i<len;i++){
									if(i)fprintf(stderr,",");
									fprintf(stderr,"%d",pos[i]);
								}
								fprintf(stderr,"\tsupport=%d vs %d\n",sup,cur_support);
							}*/
							insertLEAF(head,tail,first,pos,len,sup);//NOTE
							first->available=0;
							tmp->available=0;
							sum++;
						}else{
							free(pos);
						}
					}
				}
				tmp=tmp->next;
			}
		//}
		first=first->next;
	}
	tmp=*head;
	/*while(tmp){
		if(tmp->available==0){
			tmp->available=-1;
			sum++;
		}
		tmp=tmp->next;
	}*/
	while(tmp){
		tmp=(_tmp=tmp)->next;
		if(_tmp->available==0){
			showMiniSingleLEAF(_tmp,height);
			deleteLEAF(head,tail,_tmp);
			_sum++;
		}
	}
	//fprintf(stderr,"+%d -%d\n",sum,_sum);
	return sum;
}

void updateLEAF(float **matrix,int width,int height,LEAF **head,LEAF **tail,int cur_support){
	while(_updateLEAF(matrix,width,height,head,tail,cur_support));
	/*LEAF *tmp=*head,*_tmp=NULL;
	int sum=0;
	while(tmp){
		tmp=(_tmp=tmp)->next;
		if(_tmp->available<=0){
			showMiniSingleLEAF(_tmp,height);
			deleteLEAF(head,tail,_tmp);
			sum++;
		}
	}
	fprintf(stderr,"- %d\n",sum);*/
}
