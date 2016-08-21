/*License LGPL v3. guo8113@126.com*/
#include "application.h"
#include "GUI.h"
#include "string.h"

#include "touch.h" 
#include "key.h"
#include "usart.h"
#include "lcd.h"
#include "led.h"
#include "delay.h"
#include "rtc.h"
#include "calendar.h"
#include "alarm.h"
#include "controller.h"


static EventGroupHandle_t keyEvent = NULL; // Key event
SemaphoreHandle_t xSemTPAdjust;
SemaphoreHandle_t xSemAlarm;
TaskHandle_t hAppTaskHandle=NULL;
/////////////////////////////////////////////////////////////////////////////////
bool isInAdjust = FALSE;
bool isLCDIdle = FALSE;
bool isSetTime = FALSE;
icon_info_t iconObj[ICON_NUM]={{"AdjustTP",60, 60, 128, 32},
												{"Center",60, 120, 128, 32}
											 };
//////////////////////////////////////////////////////////////////////////////

void vRunApplication(void *pvParameters);
////////////////////////////////////////////////////////////////////////////////
//ÅÐ¶Ï´¥µãÊÇ²»ÊÇÔÚÖ¸¶¨ÇøÓòÖ®ÄÚ
//(x,y):ÆðÊ¼×ø±ê
//xlen,ylen:ÔÚx,y·½ÏòÉÏµÄÆ«ÒÆ³¤¶È
//·µ»ØÖµ :
u8 Is_Sel_Area(GUI_PID_STATE State,u16 x,u16 y,u16 xend,u16 yend)
{
	if(State.x<=xend&&State.x>=x&&State.y<=yend&&State.y>=y)return 1;
	else return 0;
}


void InitDesktop(void)
{
	int i;
  GUI_SetBkColor(GUI_BLACK); 
  GUI_Clear(); 
  GUI_SetFont(&GUI_FontComic24B_ASCII);
	GUI_SetColor(GUI_WHITE);
	GUI_DispStringAt("SmartController",10,8);
	GUI_SetFont(&GUI_Font16_ASCII);
	for(i=0;i<ICON_NUM;i++)
	{
		GUI_DrawRect(iconObj[i].x, iconObj[i].y, iconObj[i].x+iconObj[i].h, iconObj[i].y+iconObj[i].v);
		GUI_DispStringAt(iconObj[i].title, iconObj[i].x+4,iconObj[i].y+4);
	}
}
void vLEDTask( void *pvParameters )
{
	for( ;; )
	{		  
		 LED0_OFF(); 			
     vTaskDelay(2000);
		 LED0_ON();
		 vTaskDelay(50);		
	}
}

/* To make sure the task waiting for event run timely, this task need high priority.*/
//    event_bits = xEventGroupWaitBits(keyEvent,    /* The event group handle. */
//                                     E_KEY0,     /* The bit pattern the event group is waiting for. */
//                                     pdTRUE,     /* will be cleared automatically. */
//                                     pdFALSE,    /* Don't wait for both bits, either bit unblock task. */
//                                     portMAX_DELAY); /* Block indefinitely to wait for the condition to be met. */
//		if((event_bits & E_KEY0) == E_KEY0)	//KEY0°´ÏÂ,ÔòÖ´ÐÐÐ£×¼³ÌÐò
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
				 xEventGroupSetBits(keyEvent, E_KEY0);
			   printf("Key0 pressed!\n");
			   SLOT0 =!SLOT0;
				 break;
			 case KEY1_PRES:
				 xEventGroupSetBits(keyEvent, E_KEY1);
			    vTaskResume(hAppTaskHandle);
			   //printf("Key1 pressed!\n");
				 break;
			 case WKUP_PRES:
				 xEventGroupSetBits(keyEvent, E_KEYWKUP);
				 SLOT1 =!SLOT1;
			   printf("Key WKUP pressed!\n"); 
				 break;
			 default:
				 //printf("Key not recogized!\n");
			   break;
		 }
		 vTaskDelay(100);
	 }
}


void TouchAdjust(void)
{ 	
	printf("Touch screen adjust running...\r\n");
	xSemaphoreTake(xSemTPAdjust, portMAX_DELAY);
	//calendarGuiDeinit();
	isInAdjust = TRUE; // Indictate we are adjusting touch screen.
	LCD_Clear(WHITE);//ÇåÆÁ
	TP_Adjust();  //ÆÁÄ»Ð£×¼, ´øÓÐ³¬Ê±×Ô¶¯ÍË³ö¹¦ÄÜ
	TP_Save_Adjdata();	 
	//Load_Draw_Dialog();
	isInAdjust = FALSE;
	//calendarGuiInit();
	xSemaphoreGive(xSemTPAdjust);
}

void vTouchScanTask(void *pv)
{
	while(1)
	{
		if(!isInAdjust)
		{
			GUI_TOUCH_Exec();
			vTaskDelay(10);
		}
		else
		{
			vTaskDelay(100);
		}
	}
}
void vAlarmTask(void *pv)
{
	u8 slots=kSlotNone;
	xSemAlarm=xSemaphoreCreateBinary();
	
	Alarm_LoadAlarmSetting();
	g_alarmObj.current = 0xFF;
	
	Alarm_SetOneAlarm(0);

	while(1)
	{
			if (xSemaphoreTake(xSemAlarm, portMAX_DELAY) == pdTRUE)	//ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			{
				//deal with alarm IRQ
				if(isAction) // Is alarm IRQ or Alarm Setting?
				{
					 slots = g_alarmObj.dest[g_alarmObj.current]; //Get which slot should be dealed with.
					 printf("SLOT %d will be controled\r\n", slots);
					 Controller_RelayAction(slots, g_alarmObj.action[g_alarmObj.current]);			 
				}
				 //LED1 =!LED1;
				 Alarm_SetOneAlarm(isAction);
				 isAction=0;
				 Alarm_StoreAlarmSetting();
			}
			else
			{
				 printf("Alarm task timeout\n");
			}	
	}
}

void OS_Init(void)
{
	xSemTPAdjust =  xSemaphoreCreateMutex(); 
	
}

void Test(void *pv)
{
	 u8 i;
	 while(1)
	 {
		 printf("-----------alarm settings--------\r\n");
		 printf("ID     timecount  action repeat hasSet dest\r\n");
		 for(i=0;i<ALARM_COUNT;i++)
		 {
			 printf("%1d     %8x,   %5d ,  %4d,  %4d  %d\r\n", i, g_alarmObj.seccount[i], g_alarmObj.action[i], g_alarmObj.repeat[i],g_alarmObj.hasSet[i], g_alarmObj.dest[i]);
		 }

		 vTaskDelay(60000);
	 }
}

void vSystemInitTask(void *pvParameters)
{
	 //hardware init
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //ÉèÖÃNVICÖÐ¶Ï·Ö×é2:2Î»ÇÀÕ¼ÓÅÏÈ¼¶£¬2Î»ÏìÓ¦ÓÅÏÈ¼
	 delay_init();	    	 //ÑÓÊ±º¯Êý³õÊ¼»¯	
	 uart_init(115200);	 	 //´®¿Ú³õÊ¼»¯
	 LCD_Driver_Init();
	 tp_dev.init();			//´¥ÃþÆÁ³õÊ¼»¯
	 LED_Init();		  		//³õÊ¼»¯ÓëLEDÁ¬½ÓµÄÓ²¼þ½Ó¿Ú	
	 KEY_Init();				//°´¼ü³õÊ¼»¯
	 Controller_Init();
	
	 memset(&g_alarmObj,0, sizeof(g_alarmObj));
		
	 while(RTC_Init())		//RTC³õÊ¼»¯	£¬Ò»¶¨Òª³õÊ¼»¯³É¹¦
	 { 
			LCD_ShowString(60,130,200,16,16,"RTC ERROR!   ");	
			delay_ms(100);
			LCD_ShowString(60,130,200,16,16,"RTC Trying...");	
	 }

	 if(tp_dev.touchtype!=0XFF)// hint for adjust.
	 {
		  LCD_ShowString(30,110,200,16,16,"Press KEY0 to Adjust");//µç×èÆÁ²ÅÏÔÊ¾
	 }
	 vTaskDelay(1500);
	 
	 GUI_Init();
	 GUI_SetBkColor(GUI_BLACK); 
	 GUI_Clear(); 
	 
   OS_Init();
	 InitDesktop();

	 xTaskCreate( vLEDTask, (const  portCHAR * ) "LED", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );
	 //´¥ÃþÆÁÐ£×¼
	 xTaskCreate( vKeyScanTask, (const  portCHAR * ) "Key", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL ); 
	 xTaskCreate( vTouchScanTask, (const  portCHAR * ) "Touch", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, NULL );
   xTaskCreate( vRunApplication, (const  portCHAR * ) "mainGUI", configMINIMAL_STACK_SIZE*4, NULL, tskIDLE_PRIORITY+2, &hAppTaskHandle );
	 xTaskCreate( vAlarmTask, (const  portCHAR * ) "alarm", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL );
	 
	 xTaskCreate( Test, (const  portCHAR * ) "t", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, NULL );
	 
	 //vTaskDelete(NULL);// delete this task.
	 vTaskSuspend(NULL); //Stop this task.
	 while(1) //will never get here.
	 {
		 vTaskDelay(1000);
	 }
}

void vRunApplication(void *pvParameters)
{
	GUI_PID_STATE State;
	u8 i=0;
	u8 sel_app=0;
	while(1)
	{
		GUI_TOUCH_GetState(&State);
		
		if(State.Pressed)
		{
			while(PEN==0)	//µÈ´ý´¥Ãþ°´¼üÊÍ·Å
			{
					vTaskDelay(10);
			}
			for(i=0;i<ICON_NUM;i++)
			{
				 if(Is_Sel_Area(State, iconObj[i].x, iconObj[i].y, iconObj[i].x+iconObj[i].h, iconObj[i].y+iconObj[i].v))
				 {
						sel_app = i;
						break;
				 }
			}
			
			switch(sel_app)
			{
				case 0:
					TouchAdjust();
				  InitDesktop();
					break;
				case 1:
					ShowCalendar();
				  InitDesktop();
				  break;
				default:
					printf("Not implemented app\n");
				  break;
			}
		}//end if(State.Pressed)
		vTaskDelay(100);
	}	
}
