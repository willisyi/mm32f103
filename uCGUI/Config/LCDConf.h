#ifndef LCDCONF_H
#define LCDCONF_H


#define LCD_XSIZE          (240)	
#define LCD_YSIZE          (320)		
#define LCD_CONTROLLER     (9341)	 //LCD控制器的型号
#define LCD_BITSPERPIXEL   (16)
#define LCD_FIXEDPALETTE   (565) //调色板格式
#define LCD_SWAP_RB        (1)   //红蓝反色交换
#define LCD_INIT_CONTROLLER()    LCD_Driver_Init()         //TFT初始化函数
#endif /* LCDCONF_H */

