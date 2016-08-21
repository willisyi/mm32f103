/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              礐/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : GUITouch.Conf.h
Purpose     : Configures touch screen module
----------------------------------------------------------------------
*/


#ifndef __GUITOUCH_CONF_H
#define __GUITOUCH_CONF_H

#define GUI_TOUCH_SWAP_XY       0  //xy轴交换
#define GUI_TOUCH_MIRROR_X      0
#define GUI_TOUCH_MIRROR_Y      0


#define GUI_TOUCH_AD_LEFT 	 0   //AD转换的x最小值
#define GUI_TOUCH_AD_RIGHT 	 240  //AD转换的x最大值
#define GUI_TOUCH_AD_TOP 	 0
#define GUI_TOUCH_AD_BOTTOM  320		//AD转换y最大值

#define GUI_TOUCH_XSIZE     LCD_XSIZE
#define GUI_TOUCH_YSIZE     LCD_YSIZE
		
#endif /* GUITOUCH_CONF_H */
