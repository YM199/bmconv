#ifndef __BMCONV_H__
#define __BMCONV_H__

#pragma pack(push,1)

//位图文件头数据
typedef struct _BITMAP_FILE_HEADER{
	unsigned char signature[2];			//00~01:文件头签名字节,仅检测'BM'序
	unsigned long file_size;			//02~05:整个文件的大小
	unsigned long _reserved1;			//06~09:保留4字节,必须为0
	unsigned long data_offset;			//0A~0D:位图数据距文件开始的偏移
}BITMAP_FILE_HEADER;

//图像信息头数据
typedef struct _BITMAP_INFO_HEADER{
	unsigned long header_size;			//0E~11:位图信息头的长度,根据文件判断
	unsigned long width;				//12~15:位图宽度,像素为单位
	unsigned long height;				//16~19:位图高度,像素为单位
	unsigned short planes;				//1A~1B:位图的面数,该值总是1
	unsigned short bpp;					//1C~1D:位深:1,4,8,16,24,32
	unsigned long compress;				//1E~21:压缩说明
	unsigned long data_size;			//22~25:数据大小
	unsigned long hresolution;			//26~29:水平分辨率
	unsigned long vresolution;			//2A~2D:垂直分辨率
	unsigned long colors;				//2E:31:使用的颜色数
	unsigned long colors_important;		//重要的颜色数
}BITMAP_INFO_HEADER;

//颜色表
typedef struct _RGBQUAD{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char _reserved;
}BITMAP_RGB_QUAD;

//由图像信息头+颜色表组成的图像信息
typedef struct _BITMAP_INFO{
	BITMAP_INFO_HEADER bmih;
	BITMAP_RGB_QUAD bmcolor;
}BITMAP_INFO;

#pragma pack(pop)

#endif//!__BMCONV_H__
