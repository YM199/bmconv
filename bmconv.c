#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmconv.h"

/*************************************************************************
文件名称:bmconv.c/bmconv.h
文件功能:BMP提取到点阵RGB显示
创建时间:2013-04-20
文件作者:女孩不哭
文件版本:1.0
文件说明:专用
*************************************************************************/

__inline unsigned int myrgb(int a)
{
	unsigned short b = (a&0xff)>>3;
	unsigned short g = ((a&0xff00)>>10);
	unsigned short r = ((a&0xff0000)>>19);
	unsigned int rgb = (b|(g<<5)|(r<<11));
	return rgb;
}

int main(void)
{
	char file[260];
	char file_out[260];
	char* pstr = NULL;
	unsigned char* buffer=NULL;
	char*  outout_data = NULL;

	BITMAP_FILE_HEADER bmfh={0};
	BITMAP_INFO_HEADER bmih={0};
	FILE* fpin = NULL;
	FILE* fpout = NULL;
	size_t size = 0;
	size_t file_size;

	printf("输入BMP文件打开路径:");
	fgets(file,sizeof(file),stdin);
	pstr = strrchr(file,'\n');
	if(pstr) *pstr = 0;
	printf("输入BMP文件保存路径:");
	fgets(file_out,sizeof(file_out),stdin);
	pstr = strrchr(file_out,'\n');
	if(pstr) *pstr = 0;

	fpin = fopen(file,"rb");
	if(fpin==NULL){
		fprintf(stderr,"打开输入文件错误!\n");
		return 1;
	}
	fpout = fopen(file_out,"wb");
	if(fpout==NULL){
		fprintf(stderr,"打开输出文件错误!\n");
		fclose(fpin);
		return 1;
	}
	fseek(fpin,0,SEEK_END);
	file_size = ftell(fpin);
	fseek(fpin,0,SEEK_SET);
	size = fread(&bmfh,1,sizeof(bmfh),fpin);
	if(size != sizeof(bmfh)){
		fclose(fpin);
		fprintf(stderr,"读取文件头时错误!\n");
		return 1;
	}
	if(bmfh.signature[0]!='B' || bmfh.signature[1]!='M'){
		fclose(fpin);
		fprintf(stderr,"不是BMP文件!\n");
		return 1;
	}
	printf(
		"文件头结构体大小:%d\n"
		"文件头签名:\'BM\'\n"
		"文件大小:%u bytes\n"
		"保留4字节值:%u\n"
		"位图数据距文件开始的偏移:%u\n\n",(long)sizeof(bmfh),
		bmfh.file_size,bmfh._reserved1,bmfh.data_offset);
	size = fread(&bmih,1,sizeof(bmih),fpin);
	if(size != sizeof(bmih)){
		fclose(fpin);
		fprintf(stderr,"读取图像信息头错误!\n");
		return 1;
	}
	printf(
		"图像信息头结构体大小:%d\n"
		"图像信息头大小:%d bytes\n"
		"位图宽度: %u 像素\n"
		"位图高度: %u 像素\n"
		"位图面数: %d 面\n"
		"位深: %d bpp\n"
		"压缩说明: %d\n"
		"位图数据大小: %u bytes\n"
		"水平分辨率: %u 像素/米\n"
		"垂直分辨率: %u 像素/米\n"
		"使用的颜色数: %u\n"
		"重要的颜色数: %u\n\n",(long)sizeof(bmih),
		bmih.header_size,bmih.width,bmih.height,bmih.planes,bmih.bpp,bmih.compress,
		bmih.data_size,bmih.hresolution,bmih.vresolution,bmih.colors,bmih.colors_important);
	//检测文件数据大小是否符合文件头
	size = bmih.width*3;
	size += size%4==0?0:4-size%4;
	size *= bmih.height;
	if(size != file_size-bmfh.data_offset){
		fprintf(stderr,"位图的数据量与文件头所标识的数据量的长度不一致!\n");
		fclose(fpin);
		return 1;
	}
	if(bmih.bpp != 24){
		fprintf(stderr,"不是24位位深的BMP位图,不被支持!\n");
		fclose(fpin);
		return 1;
	}
	buffer=(unsigned char*)malloc(size);
	//printf("buffer大小:%d\n",size);
	if(buffer==NULL){
		fprintf(stderr,"分配文件缓冲区时出错!\n");
		fclose(fpin);
		return 1;
	}
	if(size != fread(buffer,1,size,fpin)){
		fprintf(stderr,"读取位图数据时读取的长度不一致!\n");
		free(buffer);
		fclose(fpin);
		return 1;
	}
	fclose(fpin);
	//交换上下内存
	for(;;){
		int bytes_line = size/bmih.height;
		int rows = bmih.height/2;
		int it=0;
		unsigned char* tmp = (unsigned char*)malloc(bytes_line);
		while(it<rows){
			memcpy(tmp,buffer+bytes_line*it,bytes_line);
			memcpy(buffer+bytes_line*it,buffer+(bmih.height-1-it)*bytes_line,bytes_line);
			memcpy(buffer+(bmih.height-1-it)*bytes_line,tmp,bytes_line);
			it++;
		}
		free(tmp);
		break;
	}
	//总像素点个数(一个像素3个字节),3个字节转换成2个字节,2个字节转成字符串需要7个字节空间(0x0000,)
	//每行后面1个换行符(每行16个"0x0000,",2个字节),字符串开始的结束的字符串("unsigned char image[] = {" + 换行 + "};"
	for(;;){
		size_t pixels = bmih.width*bmih.height;
		size_t str_length = pixels*7;
		size_t crlf_count = pixels/16 + (pixels%16==0?0:1);
		size_t prefix_length = sizeof("unsigned short image[] = {\r\n")-1 + sizeof("};\r\n")-1;
		size_t total = str_length+crlf_count*2+prefix_length+1;
		outout_data = (char*)malloc(total);
		//printf("str:%d,crlf:%d,prefix:%d,total::%d\n",str_length,crlf_count*2,prefix_length,total);
		if(outout_data == NULL){
			fprintf(stderr,"申请保存数据用的缓冲区失败!\n");
			free(buffer);
			return 1;
		}
		printf("正在生成数据...\n");
		for(;;){
			char* ptr = outout_data;
			unsigned char* pbuf = buffer;
			int count = 0;
			unsigned long lines;
			int skip = (bmih.width*3%4)?(4-bmih.width*3%4):0;
			int len;
			len=sprintf(ptr,"unsigned short image[] = {\r\n");
			ptr += len;
			for(lines=0;lines<bmih.height;){
				len=sprintf(ptr,"0x%04X,",(unsigned short)myrgb(*(int*)pbuf));
				ptr += len;
				count++;
				if(count == 16){
					count = 0;
					len=sprintf(ptr,"\r\n");
					ptr += len;
				}
				pbuf += 3;
				if((unsigned long)pbuf-(unsigned long)buffer-lines*skip == bmih.width*3*(lines+1)){
					pbuf += skip;
					lines++;
				}
			}
			if(count==0){
				ptr -= 3;
				len=sprintf(ptr,"\r\n");
				
				ptr+=len;
			}else{
				--ptr;
				len=sprintf(ptr,"\r\n");
				ptr += len;
			}
			len=sprintf(ptr,"};\r\n");
			ptr += len;
			printf("生成成功!\n");
			//printf("结尾指针:pbuf-buffer=%d,ptr-outout_data=%d\n",pbuf-buffer,ptr-outout_data);
			len=fwrite(outout_data,1,(unsigned long)ptr-(unsigned long)outout_data,fpout);
			if(len != (unsigned long)ptr-(unsigned long)outout_data){
				fprintf(stderr,"写入数据的长度不等于待写入的要求!\n");
			}else{
				printf("已全部保存到文件!\n");
			}
			fclose(fpout);
			free(buffer);
			free(outout_data);
			break;
		}
		break;
	}
	return 0;
}
