/*
*********************************************************************************************************
*	                                  
*	模块名称 : GUI界面主函数
*	文件名称 : MainTask.c
*	版    本 : V1.0
*	说    明 : GUI界面主函数
*	修改记录 :
*		版本号    日期          作者                 说明
*		v1.0    2013-03-26    Eric2013      ST固件库版本 V1.0.2版本。
*
*	Copyright (C), 2013-2014
*   QQ超级群：216681322
*   BLOG: http://blog.sina.com.cn/u/2565749395
*********************************************************************************************************
*/
#include "bsp.h"
#include "MainTask.h"



/*
************************************************************************
*						  用于SD卡
************************************************************************
*/
uint8_t data[1024];
FRESULT result;
FIL file;
FIL FileSave; //专门用于图片的保存
FILINFO finfo;
DIR DirInf;
UINT bw;
FATFS fs;


/*
*********************************************************************************************************
*	函 数 名: DispBMP
*	功能说明: 每次读取512字节，然后显示，如果使用内存管理还是很好的，这里没使用。
*	形    参：S_xpos    BMP图片显示X轴
*			  S_ypos    BMP图片显示Y轴
*             filename	文件名
*	返 回 值: 无
*********************************************************************************************************
*/
void GUI_DrawBMP(uint8_t S_xpos,uint16_t S_ypos,TCHAR *filename)
{	
	BITMAPINFO *pbmp;
	uint16_t  COLOR=0,tmp_color=0,countpix=0;
	uint8_t   color_byte,rgb=0;
	uint16_t  uiTemp,Xpos,Ypos;
	uint32_t  pointpos=0,count,DataSize;


	/* 打开文件 */		
	result = f_open(&file,filename, FA_OPEN_EXISTING | FA_READ|FA_OPEN_ALWAYS);
	if (result != FR_OK)
	{
		return;
	}

	/* 读数据 */
	result = f_read(&file, data, 512, &bw);
	if (result != FR_OK)
	{
		return;
	}

	pbmp=(BITMAPINFO*)data;												  
	count=pbmp->bmfHeader.bfOffBits;        							 
	color_byte=pbmp->bmiHeader.biBitCount/8;							  
	DataSize=pbmp->bmiHeader.biWidth*pbmp->bmiHeader.biHeight*color_byte;
	
	if((pbmp->bmiHeader.biWidth*color_byte)%4)
		uiTemp=((pbmp->bmiHeader.biWidth*color_byte)/4+1)*4;
	else
		uiTemp=pbmp->bmiHeader.biWidth*color_byte;

 	Xpos = S_xpos;
	Ypos = pbmp->bmiHeader.biHeight-1+S_ypos; 		
    
	RA8875_SetCursor(Xpos, Ypos);
	RA8875_REG = 0x02;
			
	while(1)
   {
     while(count<512)
	 {
	      if(color_byte==3)   
			{
				switch (rgb) 
				{
					case 0:
						tmp_color = data[count]>>3 ;
						COLOR |= tmp_color;
						break ;	   
					case 1: 
						tmp_color = data[count]>>2 ;
						tmp_color <<= 5 ;
						COLOR |= tmp_color ;
						break;	  
					case 2 : 
						tmp_color = data[count]>>3 ;
						tmp_color <<= 11 ;
						COLOR |= tmp_color ;
						break ;			
				}   
			}
          	if(color_byte==2)  
				{
					switch(rgb)
					{
						case 0 : 
							COLOR=data[count]&0x1f;
						    tmp_color=data[count]>>5;
							tmp_color<<=6;
							COLOR|=tmp_color;
							break ;   
						case 1 : 			  
							tmp_color=data[count];
							tmp_color<<=9 ;
							COLOR |= tmp_color ;
							break ;	 
					}		     
				}
			if(color_byte==4)
				{
					switch (rgb)
						{
							case 0 :  
								tmp_color=data[count];
								COLOR|=tmp_color>>3;
								break ;     
							case 1 :  
								tmp_color=data[count];
								tmp_color>>=2;
								COLOR|=tmp_color<<5;
								break ;	  
							case 2 :  
								tmp_color=data[count];
								tmp_color>>=3;
								COLOR|=tmp_color<<11;
								break ;	 
							case 3 :break ;   
						}		  	 
					}     
		    rgb++;	  
			count++;		  
			if(rgb==color_byte) 
			{							 				 	  	       		 
				RA8875_RAM = COLOR;    									    
			    COLOR=0x00; 
			    rgb=0;  		  
			}
			countpix++;
			pointpos++;
			
			if(pointpos>=DataSize)
			{
				f_close(&file);
				return;
			}
			if(countpix>=uiTemp)
			{		 
				RA8875_SetCursor(S_xpos, --Ypos);
				RA8875_REG = 0x02;
				countpix=0; 
			}
		 }
		  result = f_read(&file, data, 512, &bw);
		  if (result != FR_OK)
		 {
			return;
	     }
		  count=0;
	   }

}

/*
*********************************************************************************************************
*	函 数 名: GUI_Copy_ScreenRect()
*	功能说明: 屏幕截图，然后以bmp图片格式(为24位高真彩位图)保存到指定的路径下
*	形    参：x:X坐标   y:y坐标   sizex:长度   sizey:宽度
*             *Save_Path:保存路径,其中文件名后缀必须为bmp
*             例如:"0:/Picture/abcd.bmp" 注意!!:路径"0:/0:/Picture"必须存在
*             否则该函数调用无效。            	
*	返 回 值: 无
*********************************************************************************************************
*/
void GUI_SaveBMP(uint16_t startx,uint16_t starty,uint16_t sizex,uint16_t sizey,void *Save_Path)
{
	uint32_t	size = (sizex*sizey)*3;//-- 由于是24为BMP位图，一个像素占3个字节,所以要乘以3
	uint16_t	Header_num = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
	int16_t 	i = 0,j = 0,temp = 0,count = 0;
	uint16_t 	Buffer_num = 510;


	BITMAPFILEHEADER 	 BmpFileHeader;
	BITMAPINFOHEADER	 BmpInfoHeader;
	

	/*------------------------------- 建立文件头数据 -----------------------------------------*/
	BmpFileHeader.bfType      = 0x4D42;//--文件标志.只对'BM',用来识别BMP位图类型
	BmpFileHeader.bfSize 	  = size + Header_num;//--文件大小,占四个字节
	BmpFileHeader.bfReserved1 = 0;				  //--保留字段1
	BmpFileHeader.bfReserved2 = 0;				  //--保留字段2
	BmpFileHeader.bfOffBits   = Header_num;//--从文件开始到位图数据(bitmap data)开始之间的的偏移量
	/*------------------------------- 建立文件信息数据 ---------------------------------------*/
	BmpInfoHeader.biSize   = sizeof(BITMAPINFOHEADER);//--说明BITMAPINFOHEADER结构所需要的字数
	BmpInfoHeader.biWidth  = sizex;   //--说明图象的宽度，以象素为单位
	BmpInfoHeader.biHeight = sizey;   //--说明图象的高度，以象素为单位 
	BmpInfoHeader.biPlanes = 1;	      //--为目标设备说明位面数，其值将总是被设为1
	BmpInfoHeader.biBitCount = 24;    //--说明比特数/象素，其值为1、4、8、16、24、或32
	BmpInfoHeader.biCompression = 0;  //--说明图象数据压缩的类型,无压缩
	BmpInfoHeader.biSizeImage = size; //--说明图象的大小(必须是4的倍数)，以字节为单位。
										//--当用BI_RGB格式时，可设置为0 
	BmpInfoHeader.biXPelsPerMeter = 0;//--说明水平分辨率，用象素/米表示
	BmpInfoHeader.biYPelsPerMeter = 0;//--说明垂直分辨率，用象素/米表示
	BmpInfoHeader.biClrUsed		  = 0;//--说明位图实际使用的彩色表中的颜色索引数
	BmpInfoHeader.biClrImportant  = 0;//--说明对图象显示有重要影响的颜色索引的数目,如果是0,表示都重要
	/*-------------------------- 创建保存截图数据的文件  ------------------------------------*/
	if (f_open(&FileSave,Save_Path, FA_WRITE|FA_CREATE_ALWAYS) == FR_OK ) 
	{	
			//--先写图像头数据和图像信息数据
			result = f_write (&FileSave,&BmpFileHeader,sizeof(BmpFileHeader),&bw);
			if (result != FR_OK)
			{
				return;
			}
			
			result = f_write (&FileSave,&BmpInfoHeader,sizeof(BmpInfoHeader),&bw);
			if (result != FR_OK)
			{
				return;
			}
			for(j = sizey-1; j >= 0; j--)
			{		 	
					 for(i = 0; i < sizex; i++)
					 {
							temp = LCD_GetPixel(startx+i,starty+j);
							data[count+2] = (u8)((temp&0xf800)>>8);
							data[count+1] = (u8)((temp&0x7e0)>>3);
							data[count]   = (u8)((temp&0x1f)<<3);
							count += 3;
							if(count == Buffer_num)
							{
									count = 0;
									result = f_write (&FileSave,data,Buffer_num,&bw);
									if (result != FR_OK)
									{
										return;
									}		
							}
					 }
			}
		if(count > 0)	f_write (&FileSave,data,count,&bw);
		f_close(&FileSave);			
     }
}
