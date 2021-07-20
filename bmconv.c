#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmconv.h"

/*************************************************************************
�ļ�����:bmconv.c/bmconv.h
�ļ�����:BMP��ȡ������RGB��ʾ
����ʱ��:2013-04-20
�ļ�����:Ů������
�ļ��汾:1.0
�ļ�˵��:ר��
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

	printf("����BMP�ļ���·��:");
	fgets(file,sizeof(file),stdin);
	pstr = strrchr(file,'\n');
	if(pstr) *pstr = 0;
	printf("����BMP�ļ�����·��:");
	fgets(file_out,sizeof(file_out),stdin);
	pstr = strrchr(file_out,'\n');
	if(pstr) *pstr = 0;

	fpin = fopen(file,"rb");
	if(fpin==NULL){
		fprintf(stderr,"�������ļ�����!\n");
		return 1;
	}
	fpout = fopen(file_out,"wb");
	if(fpout==NULL){
		fprintf(stderr,"������ļ�����!\n");
		fclose(fpin);
		return 1;
	}
	fseek(fpin,0,SEEK_END);
	file_size = ftell(fpin);
	fseek(fpin,0,SEEK_SET);
	size = fread(&bmfh,1,sizeof(bmfh),fpin);
	if(size != sizeof(bmfh)){
		fclose(fpin);
		fprintf(stderr,"��ȡ�ļ�ͷʱ����!\n");
		return 1;
	}
	if(bmfh.signature[0]!='B' || bmfh.signature[1]!='M'){
		fclose(fpin);
		fprintf(stderr,"����BMP�ļ�!\n");
		return 1;
	}
	printf(
		"�ļ�ͷ�ṹ���С:%d\n"
		"�ļ�ͷǩ��:\'BM\'\n"
		"�ļ���С:%u bytes\n"
		"����4�ֽ�ֵ:%u\n"
		"λͼ���ݾ��ļ���ʼ��ƫ��:%u\n\n",(long)sizeof(bmfh),
		bmfh.file_size,bmfh._reserved1,bmfh.data_offset);
	size = fread(&bmih,1,sizeof(bmih),fpin);
	if(size != sizeof(bmih)){
		fclose(fpin);
		fprintf(stderr,"��ȡͼ����Ϣͷ����!\n");
		return 1;
	}
	printf(
		"ͼ����Ϣͷ�ṹ���С:%d\n"
		"ͼ����Ϣͷ��С:%d bytes\n"
		"λͼ���: %u ����\n"
		"λͼ�߶�: %u ����\n"
		"λͼ����: %d ��\n"
		"λ��: %d bpp\n"
		"ѹ��˵��: %d\n"
		"λͼ���ݴ�С: %u bytes\n"
		"ˮƽ�ֱ���: %u ����/��\n"
		"��ֱ�ֱ���: %u ����/��\n"
		"ʹ�õ���ɫ��: %u\n"
		"��Ҫ����ɫ��: %u\n\n",(long)sizeof(bmih),
		bmih.header_size,bmih.width,bmih.height,bmih.planes,bmih.bpp,bmih.compress,
		bmih.data_size,bmih.hresolution,bmih.vresolution,bmih.colors,bmih.colors_important);
	//����ļ����ݴ�С�Ƿ�����ļ�ͷ
	size = bmih.width*3;
	size += size%4==0?0:4-size%4;
	size *= bmih.height;
	if(size != file_size-bmfh.data_offset){
		fprintf(stderr,"λͼ�����������ļ�ͷ����ʶ���������ĳ��Ȳ�һ��!\n");
		fclose(fpin);
		return 1;
	}
	if(bmih.bpp != 24){
		fprintf(stderr,"����24λλ���BMPλͼ,����֧��!\n");
		fclose(fpin);
		return 1;
	}
	buffer=(unsigned char*)malloc(size);
	//printf("buffer��С:%d\n",size);
	if(buffer==NULL){
		fprintf(stderr,"�����ļ�������ʱ����!\n");
		fclose(fpin);
		return 1;
	}
	if(size != fread(buffer,1,size,fpin)){
		fprintf(stderr,"��ȡλͼ����ʱ��ȡ�ĳ��Ȳ�һ��!\n");
		free(buffer);
		fclose(fpin);
		return 1;
	}
	fclose(fpin);
	//���������ڴ�
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
	//�����ص����(һ������3���ֽ�),3���ֽ�ת����2���ֽ�,2���ֽ�ת���ַ�����Ҫ7���ֽڿռ�(0x0000,)
	//ÿ�к���1�����з�(ÿ��16��"0x0000,",2���ֽ�),�ַ�����ʼ�Ľ������ַ���("unsigned char image[] = {" + ���� + "};"
	for(;;){
		size_t pixels = bmih.width*bmih.height;
		size_t str_length = pixels*7;
		size_t crlf_count = pixels/16 + (pixels%16==0?0:1);
		size_t prefix_length = sizeof("unsigned short image[] = {\r\n")-1 + sizeof("};\r\n")-1;
		size_t total = str_length+crlf_count*2+prefix_length+1;
		outout_data = (char*)malloc(total);
		//printf("str:%d,crlf:%d,prefix:%d,total::%d\n",str_length,crlf_count*2,prefix_length,total);
		if(outout_data == NULL){
			fprintf(stderr,"���뱣�������õĻ�����ʧ��!\n");
			free(buffer);
			return 1;
		}
		printf("������������...\n");
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
			printf("���ɳɹ�!\n");
			//printf("��βָ��:pbuf-buffer=%d,ptr-outout_data=%d\n",pbuf-buffer,ptr-outout_data);
			len=fwrite(outout_data,1,(unsigned long)ptr-(unsigned long)outout_data,fpout);
			if(len != (unsigned long)ptr-(unsigned long)outout_data){
				fprintf(stderr,"д�����ݵĳ��Ȳ����ڴ�д���Ҫ��!\n");
			}else{
				printf("��ȫ�����浽�ļ�!\n");
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
