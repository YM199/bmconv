#ifndef __BMCONV_H__
#define __BMCONV_H__

#pragma pack(push,1)

//λͼ�ļ�ͷ����
typedef struct _BITMAP_FILE_HEADER{
	unsigned char signature[2];			//00~01:�ļ�ͷǩ���ֽ�,�����'BM'��
	unsigned long file_size;			//02~05:�����ļ��Ĵ�С
	unsigned long _reserved1;			//06~09:����4�ֽ�,����Ϊ0
	unsigned long data_offset;			//0A~0D:λͼ���ݾ��ļ���ʼ��ƫ��
}BITMAP_FILE_HEADER;

//ͼ����Ϣͷ����
typedef struct _BITMAP_INFO_HEADER{
	unsigned long header_size;			//0E~11:λͼ��Ϣͷ�ĳ���,�����ļ��ж�
	unsigned long width;				//12~15:λͼ���,����Ϊ��λ
	unsigned long height;				//16~19:λͼ�߶�,����Ϊ��λ
	unsigned short planes;				//1A~1B:λͼ������,��ֵ����1
	unsigned short bpp;					//1C~1D:λ��:1,4,8,16,24,32
	unsigned long compress;				//1E~21:ѹ��˵��
	unsigned long data_size;			//22~25:���ݴ�С
	unsigned long hresolution;			//26~29:ˮƽ�ֱ���
	unsigned long vresolution;			//2A~2D:��ֱ�ֱ���
	unsigned long colors;				//2E:31:ʹ�õ���ɫ��
	unsigned long colors_important;		//��Ҫ����ɫ��
}BITMAP_INFO_HEADER;

//��ɫ��
typedef struct _RGBQUAD{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char _reserved;
}BITMAP_RGB_QUAD;

//��ͼ����Ϣͷ+��ɫ����ɵ�ͼ����Ϣ
typedef struct _BITMAP_INFO{
	BITMAP_INFO_HEADER bmih;
	BITMAP_RGB_QUAD bmcolor;
}BITMAP_INFO;

#pragma pack(pop)

#endif//!__BMCONV_H__
