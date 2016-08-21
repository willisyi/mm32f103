/*
*********************************************************************************************************
*	                                  
*	模块名称 : GUI界面主函数
*	文件名称 : MainTask.c
*	版    本 : V1.0
*	说    明 : GUI界面主函数
*	修改记录 :
*		版本号    日期          作者                 说明
*		v1.0    2012-12-07    Eric2013      ST固件库版本 V1.0.2版本。
*
*	Copyright (C), 2013-2014
*   QQ超级群：216681322
*   BLOG: http://blog.sina.com.cn/u/2565749395
*********************************************************************************************************
*/

#ifndef __MainTask_H
#define __MainTask_H

#include "stm32f4xx.h"

#include "GUI.h"
#include "DIALOG.h"
#include "WM.h"
#include "BUTTON.h"
#include "CHECKBOX.h"
#include "DROPDOWN.h"
#include "EDIT.h"
#include "FRAMEWIN.h"
#include "LISTBOX.h"
#include "MULTIEDIT.h"
#include "RADIO.h"
#include "SLIDER.h"
#include "TEXT.h"
#include "PROGBAR.h"
#include "SCROLLBAR.h"
#include "LISTVIEW.h"
#include "GRAPH.h"
#include "MENU.h"
#include "MULTIPAGE.h"
#include "ICONVIEW.h"
#include "TREEVIEW.h"

#include "ff.h"

/*************************BMP信息头 40字节***************************/
typedef  __packed struct
{
    DWORD biSize ;		   //说明BITMAPINFOHEADER结构所需要的字数。
    LONG  biWidth ;		   //说明图象的宽度，以象素为单位 
    LONG  biHeight ;	   //说明图象的高度，以象素为单位 
    WORD  biPlanes ;	   //为目标设备说明位面数，其值将总是被设为1 
    WORD  biBitCount ;	   //说明比特数/象素，其值为1、4、8、16、24、或32
    DWORD biCompression ;  //说明图象数据压缩的类型。其值可以是下述值之一：
						   //BI_RGB：没有压缩；
						   //BI_RLE8：每个象素8比特的RLE压缩编码，压缩格式由2字节组成(重复象素计数和颜色索引)；  
    					   //BI_RLE4：每个象素4比特的RLE压缩编码，压缩格式由2字节组成
  						   //BI_BITFIELDS：每个象素的比特由指定的掩码决定。
    DWORD biSizeImage ;	   //说明图象的大小，以字节为单位。当用BI_RGB格式时，可设置为0  
    LONG  biXPelsPerMeter ;//说明水平分辨率，用象素/米表示
    LONG  biYPelsPerMeter ;//说明垂直分辨率，用象素/米表示
    DWORD biClrUsed ;	   //说明位图实际使用的彩色表中的颜色索引数
    DWORD biClrImportant ; //说明对图象显示有重要影响的颜色索引的数目，如果是0，表示都重要。 
}BITMAPINFOHEADER ;

/*************************BMP头文件 14字节***************************/
typedef  __packed struct
{
    WORD  bfType ;        //文件标志.只对'BM',用来识别BMP位图类型
    DWORD bfSize ;	      //文件大小,占四个字节
    WORD  bfReserved1 ;   //保留
    WORD  bfReserved2 ;   //保留
    DWORD bfOffBits ;     //从文件开始到位图数据(bitmap data)开始之间的的偏移量
}BITMAPFILEHEADER ;

/*************************位图信息头 54字节*************************/
typedef  __packed struct
{ 
	BITMAPFILEHEADER bmfHeader;
	BITMAPINFOHEADER bmiHeader;    
}BITMAPINFO;

/*
************************************************************************
*						  FatFs
************************************************************************
*/
extern FRESULT result;
extern FIL file;
extern FILINFO finfo;
extern DIR DirInf;
extern UINT bw;
extern FATFS fs;

extern void GUI_DrawBMP(uint8_t S_xpos,uint16_t S_ypos,TCHAR *filename);
extern void GUI_SaveBMP(uint16_t startx,uint16_t starty,uint16_t sizex,uint16_t sizey,void *Save_Path);

/*
************************************************************************
*						emWin字体
************************************************************************
*/
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ_Song_12;
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ_FangSong_16;
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ_Song_16;
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ_Hei_24;
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ_Kai_24;
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ_Song_24;
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ_SimSun_1616;
extern GUI_CONST_STORAGE  GUI_FONT GUI_FontHZ_SimSun_2424;

#endif

/*****************************(END OF FILE) *********************************/
