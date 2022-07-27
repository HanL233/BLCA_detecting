#include <readp.h>
#include <gListp.h>
#include <splitp.h>

int getDataFromFile(const char *fileName,float ***matrix,int *width,int *height){
	FILE *fp=fopen(fileName,"r");
	if(fp==NULL)return -1;
	*height=0;
	*width=0;
	g_list_p *gp=g_createListP();
	int i=0;
	while(!feof(fp)){
		char *str=NULL;
		readP(fp,&str);
		if(str && str[0] && str[0]!='#'){
			char **arr=NULL;
			int len=splitP(&arr,str,",");
			float *d=(float*)malloc(len*sizeof(float));
			g_pushListP(&gp,(void*)d);
			for(i=0;i<len;i++){
				d[i]=atof(arr[i]);
				free(arr[i]);
			}
			free(arr);
			if(*width){
				if(*width!=len){
					free(str);
					fclose(fp);
					g_clearListP(gp,free);
					g_deleteListP(&gp);
					return -2;
				}
			}else{
				*width=len;
			}
			
		}
		free(str);
	}
	fclose(fp);
	*height=g_getListArr(gp,(void***)matrix);
	g_deleteListP(&gp);
	return 0;
}

void showMatrix(float **matrix,int width,int height){
	int i=0,j=0;
	for(i=0;i<height;i++){
		for(j=0;j<width;j++){
			if(j)printf(",");
			printf("%g",(matrix[i])[j]);
		}
		printf("\n"); 
	}
}

void clearMatrix(float ***matrix,int *width,int *height){
	int i=0;
	for(i=0;i<*height;i++){
		free((*matrix)[i]);
	}
	free(*matrix);
	*matrix=NULL;
	*width=0;
	*height=0;
}
