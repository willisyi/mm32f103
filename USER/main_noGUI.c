#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "key.h"  
#include "24cxx.h" 
#include "myiic.h"
#include "touch.h" 
#include "rtc.h"
//ø™∑¢∞Â∑∂¿˝¥˙¬Î21
//¥•√˛∆¡ µ—È 
#include "calendar.h"

//FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"
#include "GUI.h"

///////////////////////////EventBits_t define////////////////////////////////////
#define E_KEY0 (1<<0)
#define E_KEY1 (1<<1)
#define E_KEYWKUP (1<<2)

static EventGroupHandle_t keyEvent = NULL; // Key event
/////////////////////////////////////////////////////////////////////////////////
#define LCD_SLEEP_COUNT 500
bool isInAdjust = FALSE;
bool isLCDIdle = FALSE;
static u32 lcdIdleCount = 0;
bool isSetTime = FALSE;
////////////////////////////////////////////////////////////////////////////////
// ±º‰…Ë÷√∞¥≈•
#define B1X1 10
#define B1Y1 260
#define B1X2 60
#define B1Y2 300
///////////////////////////////////////////////////////

#define LED0_ON()   GPIO_ResetBits(GPIOA,GPIO_Pin_8);
#define LED0_OFF()  GPIO_SetBits(GPIOA,GPIO_Pin_8);
#define LED0B()     GPIOA->ODR ^= 0x0100; 

#define LED1_ON()   GPIO_ResetBits(GPIOD,GPIO_Pin_2);
#define LED1_OFF()  GPIO_SetBits(GPIOD,GPIO_Pin_2);
#define LED1B()     GPIOD->ODR ^= 0x04; 
//extern void MainTask(void *pvParameters);   
extern void MainTaskWM(void *pvParameters);  
extern void MainTaskTouch(void *pvParameters);
//void MainTaskTouch(void *pvParameters)
//{
//	while(1)
//	{
//		LED1B();
//		//GUI_TOUCH_Exec();
//		vTaskDelay(500);
//	}
//}
void Load_Draw_Dialog(void)
{
	LCD_Clear(WHITE);//«Â∆¡   
 	POINT_COLOR=BLUE;//…Ë÷√◊÷ÃÂŒ™¿∂…´ 
	LCD_ShowString(lcddev.width-24,0,200,16,16,"RST");//œ‘ æ«Â∆¡«¯”Ú
  POINT_COLOR=RED;//…Ë÷√ª≠± ¿∂…´ 
	
	//œ‘ æ ±÷”…Ë÷√∞¥≈•
	LCD_DrawRectangle(B1X1,B1Y1,B1X2,B1Y2);
	LCD_ShowString(B1X1+8,B1Y1+8,40,16,16,"H");//œ‘ æ«Â∆¡«¯”Ú
	LCD_DrawRectangle(B1X1+50,B1Y1,B1X2+50,B1Y2);
	LCD_ShowString(B1X1+58,B1Y1+8,40,16,16,"M");//œ‘ æ«Â∆¡«¯”Ú
}
  
bool isTouched(u16 x1, u16 y1, u16 x2, u16 y2)
{
	 if( (tp_dev.x[0] >x1)&&(tp_dev.x[0] < x2) && (tp_dev.y[0]> y1) &&(tp_dev.y[0] <y2))
		 return TRUE;
	 else return FALSE;
}
////////////////////////////////////////////////////////////////////////////////
/* To make sure the task waiting for event run timely, this task need high priority.*/
void vKeyScanTask(void *pvParameters)
{
	 u8 key;
	 keyEvent = xEventGroupCreate();// create event group
	 while(1)
	 {
			key = KEY_Scan(0);
		 switch(key)
		 {
			 case KEY0_PRES:
				 lcdIdleCount = 0;
				 xEventGroupSetBits(keyEvent, E_KEY0);
			   printf("Key0 pressed!\n");
				 break;
			 case KEY1_PRES:
				 lcdIdleCount = 0;
				 xEventGroupSetBits(keyEvent, E_KEY1);
			   printf("Key 1 pressed!\n");
				 break;
			 case WKUP_PRES:
				 lcdIdleCount = 0;
				 xEventGroupSetBits(keyEvent, E_KEYWKUP);
				 break;
			 default:
				 //printf("Key not recogized!\n");
			   break;
		 }
		 vTaskDelay(100);
	 }
}
void vDisplayTask(void *pvParameters)
{
	  
	while(1)
	{ 	
		if (!isInAdjust)
		{
//LCD sleep	
		lcdIdleCount ++;
		if((lcdIdleCount > LCD_SLEEP_COUNT) && (isLCDIdle == FALSE))
		{
			 isLCDIdle = TRUE;
			 LCD_DisplayOff();
			 LCD_LED = 0; //LCD light turn off
		}
		else if(isLCDIdle && (lcdIdleCount < LCD_SLEEP_COUNT))
		{
			 LCD_LED=1;
			 LCD_DisplayOn();
			 isLCDIdle = FALSE;
		}
		else{}
//////			
			tp_dev.scan(0); 		 
			if(tp_dev.sta&TP_PRES_DOWN)			//¥•√˛∆¡±ª∞¥œ¬
			{	
				lcdIdleCount = 0;
				if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height)
				{	
					if(tp_dev.x[0]>(lcddev.width-24)&&tp_dev.y[0]<16)Load_Draw_Dialog();//«Â≥˝
					else TP_Draw_Big_Point(tp_dev.x[0],tp_dev.y[0],RED);		//ª≠Õº	  			   
				}
			}else delay_ms(10);	//√ª”–∞¥º¸∞¥œ¬µƒ ±∫Ú 
	  }
		else
		{
			printf("Still adjusting screen, couldnot use. Or lcd closed.\r\n");
			vTaskDelay(1000);
		}
	}
}

void vTouchAdjustTask(void *pvParameters)
{ 	
	EventBits_t event_bits;
	printf("Touch screen adjust task running...\r\n");
	while(1)
	{
    event_bits = xEventGroupWaitBits(keyEvent,    /* The event group handle. */
                                     E_KEY0,     /* The bit pattern the event group is waiting for. */
                                     pdTRUE,     /* will be cleared automatically. */
                                     pdFALSE,    /* Don't wait for both bits, either bit unblock task. */
                                     portMAX_DELAY); /* Block indefinitely to wait for the condition to be met. */
		if((event_bits & E_KEY0) == E_KEY0)	//KEY0∞¥œ¬,‘Ú÷¥–––£◊º≥Ã–Ú
		{	
			isInAdjust = TRUE; // Indictate we are adjusting touch screen.
			LCD_Clear(WHITE);//«Â∆¡
		  TP_Adjust();  //∆¡ƒª–£◊º, ¥¯”–≥¨ ±◊‘∂ØÕÀ≥ˆπ¶ƒ‹
			TP_Save_Adjdata();	 
			Load_Draw_Dialog();
			isInAdjust = FALSE;
		}
	}
}

void vLEDTask( void *pvParameters )
{
	for( ;; )
	{					
        LED0B();  
        vTaskDelay(1000);
	}
}

void MainTaskTouch(void *pv)
{
	int i;
	while(1)
	{
		GUI_TOUCH_Exec();
		if (i% 50 == 0)
				LED1 = !LED1;
		vTaskDelay(10);
		i++;
	}
}

void vRTCShowTask(void *pvParameters)
{
	u8 t;	
  _calendar_obj config;	

	while(1)
	{								    
		if(t!=calendar.sec)  // not setting time
		{
				//œ‘ æ ±º‰
			POINT_COLOR=BLUE;//…Ë÷√◊÷ÃÂŒ™¿∂…´					 
			LCD_ShowString(60,130,200,16,16,"    -  -     ");	   
			LCD_ShowString(60,162,200,16,16,"  :  :  ");
			
			t=calendar.sec;
			POINT_COLOR=LIGHTGREEN;//…Ë÷√◊÷ÃÂŒ™¿∂…´	
			LCD_ShowNum(60,130,calendar.w_year,4,16);									  
			LCD_ShowNum(100,130,calendar.w_month,2,16);									  
			LCD_ShowNum(124,130,calendar.w_date,2,16);	 
			switch(calendar.week)
			{
				case 0:
					LCD_ShowString(60,148,200,16,16,"Sunday   ");
					break;
				case 1:
					LCD_ShowString(60,148,200,16,16,"Monday   ");
					break;
				case 2:
					LCD_ShowString(60,148,200,16,16,"Tuesday  ");
					break;
				case 3:
					LCD_ShowString(60,148,200,16,16,"Wednesday");
					break;
				case 4:
					LCD_ShowString(60,148,200,16,16,"Thursday ");
					break;
				case 5:
					LCD_ShowString(60,148,200,16,16,"Friday   ");
					break;
				case 6:
					LCD_ShowString(60,148,200,16,16,"Saturday ");
					break;  
			}
			LCD_ShowNum(60,162,calendar.hour,2,16);									  
			LCD_ShowNum(84,162,calendar.min,2,16);									  
			LCD_ShowNum(108,162,calendar.sec,2,16);
		}
		
		{
			tp_dev.scan(0);
			config.w_year = calendar.w_year;
			config.w_month = calendar.w_month;
			config.w_date = calendar.w_date;
			if (isTouched(B1X1,B1Y1,B1X2,B1Y2)) // H pressed
			{
				 config.hour = calendar.hour + 1;
				 if (config.hour >=24)
					 config.hour = 0;
				 LCD_ShowNum(60,162,calendar.hour,2,16);
				 RTC_Set(config.w_year,config.w_month, config.w_date,config.hour,config.min,config.sec);
			}
			if (isTouched(B1X1+50,B1Y1,B1X2+50,B1Y2)) // H pressed
			{
				 config.min = calendar.min + 1;
				 if (config.min >=60)
					 config.min = 0;
				 LCD_ShowNum(84,162,calendar.min,2,16);
				 RTC_Set(config.w_year,config.w_month, config.w_date,config.hour,config.min,config.sec);
			}
				
		}			
		vTaskDelay(50);	
	}		
}
void vSystemInitTask(void *pvParameters)
{
	 //hardware init
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //…Ë÷√NVIC÷–∂œ∑÷◊È2:2Œª«¿’º”≈œ»º∂£¨2ŒªœÏ”¶”≈œ»º
	 delay_init();	    	 //—” ±∫Ø ˝≥ı ºªØ	
	 uart_init(115200);	 	 //¥Æø⁄≥ı ºªØ
	 LCD_Driver_Init();
	 tp_dev.init();			//¥•√˛∆¡≥ı ºªØ
	 LED_Init();		  		//≥ı ºªØ”ÎLED¡¨Ω”µƒ”≤º˛Ω”ø⁄	
	 KEY_Init();				//∞¥º¸≥ı ºªØ

	 if(tp_dev.touchtype!=0XFF)// hint for adjust.
	 {
		  LCD_ShowString(60,110,200,16,16,"Press KEY0 to Adjust");//µÁ◊Ë∆¡≤≈œ‘ æ
	 }
	 delay_ms(1500);
	 	 
	 while(RTC_Init())		//RTC≥ı ºªØ	£¨“ª∂®“™≥ı ºªØ≥…π¶
	 { 
			LCD_ShowString(60,130,200,16,16,"RTC ERROR!   ");	
			delay_ms(100);
			LCD_ShowString(60,130,200,16,16,"RTC Trying...");	
	 }
	 //¥¥Ω®»ŒŒÒ
	 Load_Draw_Dialog(); //show initial screen.
	// xTaskCreate( MainTaskWM, (const  portCHAR * ) "Init", configMINIMAL_STACK_SIZE*4, NULL, tskIDLE_PRIORITY+1, NULL );
	 xTaskCreate( MainTaskTouch, (const  portCHAR * ) "Touch", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, NULL );
//	 xTaskCreate( vTouchAdjustTask, (const  portCHAR * ) "TouchAdjust", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL );
//	 xTaskCreate( vRTCShowTask, (const  portCHAR * ) "RTC", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL );
   xTaskCreate( vLEDTask, (const  portCHAR * ) "LED", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL );
	 xTaskCreate( ShowCalendar, (const  portCHAR * ) "calendar", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL );//ShowCalendar
//	 xTaskCreate( vKeyScanTask, (const  portCHAR * ) "KeyScan", 256, NULL, configMAX_PRIORITIES-2, NULL );
//	 xTaskCreate( vDisplayTask, (const  portCHAR * ) "Display", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, NULL );

	 vTaskDelete(NULL);// delete this task.
}

 int main(void)
 { 
	taskENTER_CRITICAL();	
	xTaskCreate( vSystemInitTask, (const  portCHAR * ) "Init", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, NULL );
	taskEXIT_CRITICAL();
  vTaskStartScheduler();
	return 0;
}

