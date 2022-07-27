#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <png.h>
#include <zlib.h>
#include "drawSVG.h"

char writeRGB2PNG(const char *fileName,int width,int height,unsigned char *R,unsigned char *G,unsigned char *B){
	png_structp png_ptr=NULL;
	png_infop   info_ptr=NULL;
	int iRetVal;
	png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	if (!png_ptr)return -1;
	info_ptr=png_create_info_struct(png_ptr);
	if (!info_ptr){
		png_destroy_write_struct(&png_ptr,NULL);
		return -1;
	}
	iRetVal= setjmp (png_jmpbuf(png_ptr));
	if (iRetVal){
		fprintf (stderr, "Error: %d\n" ,iRetVal);
		return -1;
	}
	FILE *fp= fopen (fileName,"wb");
	if (!fp){
		fprintf (stderr, "Error: cannot open %s for writing\n" ,fileName);
		return -1;
	}
	png_init_io(png_ptr,fp);
	png_set_IHDR(png_ptr,info_ptr,width,height,8,PNG_COLOR_TYPE_RGB,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_BASE,PNG_FILTER_TYPE_BASE);
	png_set_packing(png_ptr);
	png_write_info(png_ptr,info_ptr);
	int i=0,j=0;
	png_byte *row=(png_byte*)malloc(width*3*sizeof(png_byte));
	for(i=0;i<height;i++){
		for(j=0;j<width;j++){
			row[j*3+0]=(png_byte)R[i*width+j];
			row[j*3+1]=(png_byte)G[i*width+j];
			row[j*3+2]=(png_byte)B[i*width+j];
		}
		png_write_row(png_ptr,row);
	}
	free(row);
	png_write_end(png_ptr,info_ptr);
	png_destroy_write_struct(&png_ptr,&info_ptr);
	fclose(fp);
	return 0;
}

char readRGBfromPNG(const char *fileName,unsigned int *width,unsigned int *height,unsigned char **R,unsigned char **G,unsigned char **B){

	png_structp png_ptr=NULL;
	png_infop   info_ptr=NULL;
	int iRetVal;
	png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	if(!png_ptr)
		return -1;
	info_ptr=png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		png_destroy_read_struct(&png_ptr,NULL,NULL);
		return -1;
	}
	iRetVal= setjmp(png_jmpbuf(png_ptr));
	if(iRetVal){
		fprintf(stderr, "Error: %d\n" ,iRetVal);
		return -1;
	}

	FILE *fp= fopen(fileName, "rb" );
	if(!fp){
		fprintf(stderr, "Error: cannot open %s for reading\n" ,fileName);
		return -1;
	}

	png_init_io(png_ptr,fp);
	png_read_info(png_ptr, info_ptr);

	int bit_depth=0,color_type=0;

	png_get_IHDR(png_ptr,info_ptr,width,height,&bit_depth,&color_type,NULL,NULL,NULL);
	png_color_16p pBackground;
	png_get_bKGD(png_ptr,info_ptr,&pBackground);
	//if(color_type==PNG_COLOR_TYPE_RGB)
	//	png_set_rgb_to_rgba(png_ptr);
	if(color_type==PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);
	if(color_type==PNG_COLOR_TYPE_GRAY && bit_depth<8)
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	if(bit_depth==16)
		png_set_strip_16(png_ptr);
	if(png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);
	if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);
	png_read_update_info(png_ptr, info_ptr);
	png_byte **ppLinePtrs=(png_byte**)malloc((*height)*sizeof(png_byte*));
	int i,j;
	*R=(unsigned char*)malloc((*width)*(*height)*sizeof(unsigned char));
	*G=(unsigned char*)malloc((*width)*(*height)*sizeof(unsigned char));
	*B=(unsigned char*)malloc((*width)*(*height)*sizeof(unsigned char));
	unsigned char A=0;
	if(!ppLinePtrs || !R || !G || !B){
		fprintf(stderr,"Out of memory\n");
		free(ppLinePtrs);
		free(R);
		free(G);
		free(B);
		png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
		return -1;
	}
	if(color_type == PNG_COLOR_TYPE_RGBA || color_type == PNG_COLOR_TYPE_GA){
		for(i=0;i<(*height);i++){
			ppLinePtrs[i]=(png_byte*)malloc((*width)*4*sizeof(png_byte));
		}
		png_read_image(png_ptr,ppLinePtrs);
		for(j=0;j<*width;j++){
			for(i=0;i<*height;i++){
				A=(unsigned char)(ppLinePtrs[i][j*3+3]);
				(*R)[i*(*width)+j]=(unsigned char)(ppLinePtrs[i][j*3+0]*A/255);
				(*G)[i*(*width)+j]=(unsigned char)(ppLinePtrs[i][j*3+1]*A/255);
				(*B)[i*(*width)+j]=(unsigned char)(ppLinePtrs[i][j*3+2]*A/255);
			}
		}
	}else{
		for(i=0;i<(*height);i++){
			ppLinePtrs[i]=(png_byte*)malloc((*width)*3*sizeof(png_byte));
		}
		png_read_image(png_ptr,ppLinePtrs);
		for(j=0;j<*width;j++){
			for(i=0;i<*height;i++){
				(*R)[i*(*width)+j]=(unsigned char)(ppLinePtrs[i][j*3+0]);
				(*G)[i*(*width)+j]=(unsigned char)(ppLinePtrs[i][j*3+1]);
				(*B)[i*(*width)+j]=(unsigned char)(ppLinePtrs[i][j*3+2]);
			}
		}
	}
	for(i=0;i<(*height);i++){
		free(ppLinePtrs[i]);
	}
	free(ppLinePtrs);
	png_read_end(png_ptr,info_ptr);
	png_destroy_read_struct(&png_ptr,&info_ptr,NULL);
	fclose(fp);
	return 0;
}

char getColorEx(const char ** (*f)(int*),int len,unsigned char **R,unsigned char **G,unsigned char **B){
	*R=(unsigned char*)malloc(len*sizeof(unsigned char));
	*G=(unsigned char*)malloc(len*sizeof(unsigned char));
	*B=(unsigned char*)malloc(len*sizeof(unsigned char));
	int i=0,colorLen=0;
	const char **colorArr=f(&colorLen);
	char **colorArrEx=extendColor(colorArr,colorLen,len);
	for(i=0;i<len;i++){
		sepColor(colorArrEx[i],*R+i,*G+i,*B+i);
		free(colorArrEx[i]);
	}
	free(colorArrEx);
	return 0;
}

int float2PNG(const char *fileName,float *data,int width,int height,unsigned char *_R,unsigned char *_G,unsigned char *_B,int len){
	int msize=width*height;
	unsigned char *R=(unsigned char*)calloc(msize,sizeof(unsigned char));
        unsigned char *G=(unsigned char*)calloc(msize,sizeof(unsigned char));
        unsigned char *B=(unsigned char*)calloc(msize,sizeof(unsigned char));
	float maxValue=0.0f,minValue=0.0f;
	int i=0;
	for(i=0;i<msize;i++){
		if(i==0 || maxValue<data[i]){
			maxValue=data[i];
		}
		if(i==0 || minValue>data[i]){
			minValue=data[i];
		}
	}
	float rangeValue=maxValue-minValue;
	if(rangeValue==0.0f){
		for(i=0;i<msize;i++){
			R[i]=_R[0];
			G[i]=_G[0];
			B[i]=_B[0];
		}
	}else{
		for(i=0;i<msize;i++){
			int idx=((data[i]-minValue)/rangeValue)*(len-1);
			R[i]=_R[idx];
			G[i]=_G[idx];
			B[i]=_B[idx];
		}
	}
	int re=writeRGB2PNG(fileName,width,height,R,G,B);
	free(R);
	free(G);
	free(B);
	return re;

}

int int2PNG(const char *fileName,int *data,int width,int height,unsigned char *_R,unsigned char *_G,unsigned char *_B,int len){
	int msize=width*height;
	int i=0,j=0;
	unsigned char *R=(unsigned char*)calloc(msize,sizeof(unsigned char));
        unsigned char *G=(unsigned char*)calloc(msize,sizeof(unsigned char));
        unsigned char *B=(unsigned char*)calloc(msize,sizeof(unsigned char));
	for(i=0;i<msize;i++){
		j=data[i];
		if(j>=0 && j<len){
			R[i]=_R[j];
			G[i]=_G[j];
			B[i]=_B[j];
		}
	}
	int re=writeRGB2PNG(fileName,width,height,R,G,B);
	free(R);
	free(G);
	free(B);
	return re;
}
