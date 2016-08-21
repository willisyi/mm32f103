/*
*********************************************************************************************************
*                                                µC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2000, SEGGER Microcontroller Systeme GmbH          
*
*              µC/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed 
*              in any way. We appreciate your understanding and fairness.
*
* File        : MainTask.c
* Purpose     : Application program in windows simulator
*********************************************************************************************************
*/


#include "GUI.h"
#include "led.h"
#include "lcd.h"
#include "LCDConf.h"
#include "FreeRTOS.h"
#include "task.h"

extern const GUI_BITMAP bmMicriumLogo;
extern const GUI_BITMAP bmMicriumLogo_1bpp;


/*
  *******************************************************************
  *
  *              main()
  *
  *******************************************************************
*/
void GUIDEMO_ShowColorBar(void) {
  int nBars = 13;
  int NumColors = LCD_GetDevCap(LCD_DEVCAP_NUMCOLORS);
  int XSize = LCD_XSIZE;
  int i, yStep, y0, x0;
  #if GUIDEMO_LARGE
    y0 = 70;
    x0 = 60;
    GUIDEMO_ShowIntro("Color bar",
                      "emWin features "
                      "\nan integrated color"
                      "\nmanagement"
                      "\nwill always find the best available color"
                      "\nfor any logical color");
  #else
    #if !(GUIDEMO_TINY)
      y0 = 55;
      x0 = 55;
    #else
      y0 = 16;
      x0 = 40;
      GUIDEMO_ShowIntro("Color bar",
                        "emWin features "
                        "\nan integrated color"
                        "\nmanagement");
    #endif
  #endif
	XSize -= x0;
  #if (LCD_YSIZE < 320)
    yStep = (LCD_YSIZE - y0) / nBars;
  #else
    yStep = (LCD_YSIZE - y0 - 60) / nBars;
  #endif
  GUI_SetColor(GUI_WHITE);
  GUI_SetBkColor(GUI_BLACK); 
  GUI_Clear();
	GUI_SetFont(&GUI_Font8x16);
	GUI_DispString("Color bars\n");
  #if !(GUIDEMO_TINY)
    GUI_SetFont(&GUI_Font8_ASCII);
    #ifdef LCD_CONTROLLER
      GUI_DispString("\nLCD_CONTROLLER: ");
      GUI_DispDecMin(LCD_CONTROLLER);
      GUI_DispString("\n");
    #endif
    GUI_DispDecMin(LCD_BITSPERPIXEL);
    GUI_DispString(" bpp");
    #ifdef LCD_BUSWIDTH
      GUI_DispString(", ");
      GUI_DispDecMin(LCD_BUSWIDTH);
      GUI_DispString(" bit bus");
    #endif
    GUI_DispString(", ");
    GUI_DispDecMin(NumColors);
    GUI_DispString(" colors\n");
    #if (LCD_FIXEDPALETTE) 
      GUI_DispString("Fixed palette: ");
      GUI_DispDecMin(LCD_FIXEDPALETTE);
    #else
      GUI_DispString("(Custom)");
    #endif
  #endif
  #if !GUIDEMO_LARGE
    #if (LCD_YSIZE < 100)
      GUI_SetFont(&GUI_Font4x6);
    #elif (LCD_YSIZE < 160)
      GUI_SetFont(&GUI_Font6x8);
    #else
      GUI_SetFont(&GUI_Font8x8);
    #endif
  #else
    GUI_SetFont(&GUI_Font8x8);
  #endif
  GUI_SetColor(GUI_WHITE);
  GUI_DispStringAt("Red",     0, y0 +      yStep);
  GUI_DispStringAt("Green",   0, y0 + 3  * yStep);
  GUI_DispStringAt("Blue",    0, y0 + 5  * yStep);
  GUI_DispStringAt("Grey",    0, y0 + 6  * yStep);
  GUI_DispStringAt("Yellow",  0, y0 + 8  * yStep);
  GUI_DispStringAt("Cyan",    0, y0 + 10 * yStep);
  GUI_DispStringAt("Magenta", 0, y0 + 12 * yStep);
  for (i = 0; (i < XSize); i++) {
    U16 cs = (255 * (U32)i) / XSize;
    U16 x = x0 + i;;
/* Red */
    GUI_SetColor(cs);
    GUI_DrawVLine(x, y0, y0 + yStep - 1);
    GUI_SetColor(0x0000ff + (255 - cs) * 0x10100L);
    GUI_DrawVLine(x, y0 + yStep, y0 + 2 * yStep - 1);
/* Green */
    GUI_SetColor(cs<<8);
    GUI_DrawVLine(x, y0 + 2 * yStep, y0 + 3 * yStep - 1);
    GUI_SetColor(0x00ff00 + (255 - cs) * 0x10001L);
    GUI_DrawVLine(x, y0 + 3 * yStep, y0 + 4 * yStep - 1);
/* Blue */
    GUI_SetColor(cs * 0x10000L);
    GUI_DrawVLine(x, y0 + 4 * yStep, y0 + 5 * yStep - 1);
    GUI_SetColor(0xff0000 + (255 - cs) * 0x00101L);
    GUI_DrawVLine(x, y0 + 5 * yStep, y0 + 6 * yStep - 1);
/* Gray */
    GUI_SetColor(cs * 0x10101L);
    GUI_DrawVLine(x, y0 + 6 * yStep, y0 + 7 * yStep - 1);
/* Yellow */
    GUI_SetColor(cs * 0x00101L);
    GUI_DrawVLine(x, y0 + 7 * yStep, y0 + 8 * yStep - 1);
    GUI_SetColor(0x00ffff + (255 - cs) * 0x10000L);
    GUI_DrawVLine(x, y0 + 8 * yStep, y0 + 9 * yStep - 1);
/* Cyan */
    GUI_SetColor(cs * 0x10100L);
    GUI_DrawVLine(x, y0 + 9 * yStep, y0 + 10 * yStep - 1);
    GUI_SetColor(0xffff00 + (255 - cs) * 0x00001L);
    GUI_DrawVLine(x, y0 + 10 * yStep, y0 + 11 * yStep - 1);
/* Magenta */
    GUI_SetColor(cs * 0x10001L);
    GUI_DrawVLine(x, y0 + 11 * yStep, y0 + 12 * yStep - 1);
    GUI_SetColor(0xff00ff + (255 - cs) * 0x00100L);
    GUI_DrawVLine(x, y0 + 12 * yStep, y0 + 13 * yStep - 1);
  }
  GUI_Delay(1000);
}

void MainTask(void *pvParameters) {
	GUI_PID_STATE STATE;
  int i,YPos;
  int LCDXSize = LCD_GET_XSIZE();
  int LCDYSize = LCD_GET_YSIZE();
  const GUI_BITMAP *pBitmap;
	LCD_Driver_Init();
  GUI_Init();
  GUI_SetBkColor(GUI_RED); GUI_Clear();
  GUI_Delay(1000);
  GUI_SetBkColor(GUI_BLUE); GUI_Clear();
  GUI_Delay(1000);
  GUI_SetColor(GUI_WHITE);
  for (i=0; i<1000; i+=10) {
    GUI_DrawHLine(i,0,100);
    GUI_DispStringAt("Line ",0,i);
    GUI_DispDecMin(i);
  }
  GUI_Delay(1000);
  GUI_SetColor(0x0);
  GUI_SetBkColor(0xffffff);
  for (i=0; i<160; i++) {
    int len = (i<80) ? i : 160-i;
    GUI_DrawHLine(i,20,len+20);
  }
  GUI_Delay(1000);
  GUI_Clear();
  if (LCD_GET_YSIZE()>(100+bmMicriumLogo_1bpp.YSize)) {
    pBitmap=&bmMicriumLogo;
  } else {
    GUI_SetColor(GUI_BLUE);
    pBitmap=&bmMicriumLogo_1bpp;
  }
  GUI_DrawBitmap(pBitmap,(LCDXSize-pBitmap->XSize)/2,10);
  YPos=20+pBitmap->YSize;
  GUI_SetFont(&GUI_FontComic24B_1);
  GUI_DispStringHCenterAt("www.micrium.com",LCDXSize/2,YPos);
  GUI_Delay(1000);
  GUI_SetColor(GUI_RED);
  GUI_DispStringHCenterAt("© 2004\n", LCDXSize/2,YPos+30);
  GUI_SetFont(&GUI_Font10S_1);
  GUI_DispStringHCenterAt("Micriµm Inc.",LCDXSize/2,YPos+60);;
  GUI_Delay(2000);
	
	GUIDEMO_ShowColorBar();
	
	while(1)
	{
				GUI_TOUCH_Exec();
        GUI_TOUCH_GetState(&STATE);
        if (STATE.Pressed == 1)
        {
            LCD_ShowString(50,50,200,16,16,"x:");
            LCD_ShowNum(74,50,STATE.x,5,16);        //????
            LCD_ShowNum(74,70,GUI_TOUCH_GetxPhys(),5,16);
           
            LCD_ShowString(124,50,200,16,16,"y:");
            LCD_ShowNum(160,50,STATE.y,5,16);    //????
            LCD_ShowNum(160,70,GUI_TOUCH_GetyPhys(),5,16);

            GUI_SetColor(GUI_RED);
					  GUI_SetPenSize(4);
            //GUI_DrawPoint( STATE.x , STATE.y );
					TP_Draw_Big_Point(STATE.x, STATE.y, GUI_RED);
        }else
				{
					vTaskDelay(10);
				}
	}
}
