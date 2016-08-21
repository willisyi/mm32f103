#ifndef __APPLICATION_H__
#define __APPLICATION_H__
#include "sys.h"

//FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"

#define LED0_ON()   GPIO_ResetBits(GPIOA,GPIO_Pin_8);
#define LED0_OFF()  GPIO_SetBits(GPIOA,GPIO_Pin_8);
#define LED0B()     GPIOA->ODR ^= 0x0100; 

#define LED1_ON()   GPIO_ResetBits(GPIOD,GPIO_Pin_2);
#define LED1_OFF()  GPIO_SetBits(GPIOD,GPIO_Pin_2);
#define LED1B()     GPIOD->ODR ^= 0x04; 
///////////////////////////EventBits_t define////////////////////////////////////
#define E_KEY0 (1<<0)
#define E_KEY1 (1<<1)
#define E_KEYWKUP (1<<2)

#define LCD_SLEEP_COUNT 500
#define ICON_NUM (2)  // The number of main funtion.
/////////////////////////////////////////////////////////////
typedef struct _icon_info_t{
	const char title[10];
	u8 x;
	u8 y;
	u8 h;//水平长度
	u8 v;
}icon_info_t;

//////////////////////////////////////////////////////////////

void vSystemInitTask(void *pvParameters);

#endif
