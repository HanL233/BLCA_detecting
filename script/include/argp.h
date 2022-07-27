#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARGV_STR_MAX 0x20

int arg(const char *flag,int argc,const char **argv){
	int ag=-1;
	int a=-1;
	char b=0;
	while(++a<argc){
		if(argv[a][0]=='-'){
			if(strcmp(argv[a]+1,flag)==0){
				b=1;
				if(ag<0)ag=0;
			}else b=0;
			continue;
		}
		if(b)ag=a;
	}
	return ag;
}

int arg_(const char *flag,int argc,const char **argv){
	int a=-1;
	while(++a<argc){
		if(argv[a][0]=='-'){
			if(strcmp(argv[a]+1,flag)==0){
				break;
			}
		}
	}
	if(a==argc)return -1;
	return a;
}

/*
int argvHelp=arg("-help",argc,argv);
context=argv[argvHelp];
when argvHelp<0, no such a item
//
int argvLeft=arg_("l",argc,argv);
int argvRight=arg_("r",argc,argv);
if(argvRight>argvLeft){...}
only point out whether has such a flag, and the position of the flag if any
*/

//char *arr=getItemValue("item0","#item0=val0,item1=\"val1\"",0)
//free(arr);

typedef struct ARGV_PRA{
	char item_name[ARGV_STR_MAX+1];
	int *position;//return the position of item, when >=0 exists such a item
	int *value_position;//NULL=not require value;
}argv_pra;

int getArgvValue(argv_pra *arr,int len,int *default_val,int *length_val,int available_length,int argc,const char **argv){
	//return 0=normal,0<undefined item,0>no value and to get the correct index please -1
	int a=0,b=0,c=0,d=0;
	for(a=0;a<len;a++){
		if(arr[a].position)*(arr[a].position)=-1;
		if(arr[a].value_position)*(arr[a].value_position)=-1;
	}
	char minus=0;
	int item=-1;//0>no item
	for(a=1;a<argc;a++){
		if(strcmp(argv[a],"--")==0 && minus==0){
			minus=1;
			continue;
		}
		if(item>=0){
			if(argv[a][0]=='-' && minus==0)return (item+1);//note
			*(arr[item].value_position)=a;
			minus=0;
			item=-1;
			continue;
		}
		if(argv[a][0]!='-' || minus){
			if(default_val && c<available_length)default_val[c++]=a;
			minus=0;
			continue;
		}
		if(argv[a][1]==0)return -1*a;//note
		for(b=0;b<len;b++){
			if(strcmp(argv[a]+1,arr[b].item_name)==0){
				if(arr[b].position)*(arr[b].position)=a;
				if(arr[b].value_position)item=b;
				break;
			}
		}
		if(b!=len)continue;
		for(b=1;b<strlen(argv[a]);b++){
			char unknown=1;
			for(d=0;d<len;d++){
				if(arr[d].value_position==NULL && arr[d].item_name[1]==0){
					if(argv[a][b]==arr[d].item_name[0]){
						if(arr[d].position)*(arr[d].position)=a;
						unknown=0;
					}
				}
			}
			if(unknown)return -1*a;
		}
	}
	if(item>=0)return (item+1);
	if(length_val)*length_val=c;
	return 0;
}

/*
// when define a item which name has two or more words, it would be better using prefix "-"
// do not define a item which name can be formed by other items, for example, one item named ok will conflict with items like "o" and "k", to resolve this, use -ok instead of
#include <argp.h>

int main(int argc,const char **argv){
        argv_pra pra[2];
        int length_value=0;
        int default_value[0x20];
        int item0=-1;
        int pos0=-1,pos1=-1;
        memcpy(pra[0].item_name,"item0",ARGV_STR_MAX);
        pra[0].position=&pos0;
        pra[0].value_position=&item0;
        memcpy(pra[1].item_name,"item1",ARGV_STR_MAX);
        pra[1].position=&pos1;
        pra[1].value_position=NULL;

        int result=getArgvValue(pra,2,default_value,&length_value,0x20,argc,argv);
        if(result<0){
                printf("unkown item: %s\n",argv[-1*result]);
                exit(0);
        }else if(result){
                printf("%s requires value\n",pra[result-1].item_name);//note
                exit(0);
        }
        printf("default_value:\n");
        int a=0;
        for(a=0;a<length_value;a++){
                printf("%d. %s\n",a,argv[default_value[a]]);
        }
        printf("item_value:\n");
        printf("%s: ",pra[0].item_name);
        if(pos0<0){
                printf("no such item\n");
        }else{
                printf("%s\n",argv[item0]);
        }
        printf("%s: ",pra[1].item_name);
        if(pos1<0){
                printf("no such item\n");
        }else{
                printf("get\n");
        }
}

*/

char *getItemValue(const char *item,const char *str,char *err){
	char quote=0,equal=0,sprit=0,match=0;
	char *it=(char*)malloc((strlen(str)+1)*sizeof(char));
	char *va=(char*)malloc((strlen(str)+1)*sizeof(char));
	int a=-1;
	int its=0,vas=0;
	while(1){
		++a;
		if(str[a]=='#'){
			if(a && str[a-1]!=str[a]){
				if(equal){
					if(quote){
						sprit=0;
						va[vas++]=str[a];
					}else{
						va[vas]=0;
						free(it);
						if(match==0){
							free(va);
							va=NULL;
						}
						return va;
					}
				}else{
					free(it);
					free(va);
					if(err)*err=1;
					return NULL;
				}
			}else{
				return NULL;
			}
		}else if(str[a]=='"'){
			if(equal){
				if(quote){
					if(sprit){
						sprit=0;
						va[vas++]=str[a];
					}else if(match){
						free(it);
						va[vas]=0;
						return va;
					}else{
						quote=0;
					}
				}else{
					if(vas==0){
						quote=1;
					}else{
						free(it);
						free(va);
						if(err)*err=1;
						return NULL;
					}
				}
			}else{
				free(it);
				free(va);
				if(err)*err=1;
				return NULL;
			}
		}else if(str[a]=='='){
			if(equal){
				if(quote){
					sprit=0;
					va[vas++]=str[a];
				}else{
					free(it);
					free(va);
					if(err)*err=1;
					return NULL;
				}
			}else if(its){
				equal=1;
				it[its]=0;
				if(strcmp(it,item)==0){
					match=1;
				}
			}else{
				free(it);
				free(va);
				if(err)*err=1;
				return NULL;
			}
		}else if(str[a]=='\\'){
			if(equal && quote){
				if(sprit){
					sprit=0;
					va[vas++]=str[a];
				}else{
					sprit=1;
				}
			}else{
				free(it);
				free(va);
				if(err)*err=1;
				return NULL;
			}
		}else if(str[a]==' '){
			if(equal){
				if(quote){
					sprit=0;
					va[vas++]=str[a];
				}else{
					if(vas && match){
						free(it);
						va[vas]=0;
						return va;
					}
				}
			}
		}else if(str[a]==','){
			if(equal){
				if(quote){
					sprit=0;
					va[vas++]=str[a];
				}else if(vas){
					if(match){
						free(it);
						va[vas]=0;
						return va;
					}else{
						vas=0;
						its=0;
						equal=0;
					}
				}else{
					free(it);
					free(va);
					if(err)*err=1;
					return NULL;
				}
			}else{
				free(it);
				free(va);
				if(err)*err=1;
				return NULL;
			}
		}else if(str[a]==0){
			free(it);
			if(sprit || quote || equal==0){
				free(va);
				if(err)*err=1;
				return NULL;
			}
			va[vas]=0;
			if(match==0){
				free(va);
				va=NULL;
			}
			return va;
		}else{
			sprit=0;
			if(((its && equal==0) || (vas && quote==0)) && str[a-1]==' '){
				free(it);
				free(va);
				if(err)*err=1;
				return NULL;
			}
			if(equal){
				va[vas++]=str[a];
			}else{
				it[its++]=str[a];
			}
		}
	}
	free(it);
	free(va);
	return NULL;
}
