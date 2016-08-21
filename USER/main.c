#include "stdio.h"
//开发板范例代码21
//触摸屏实验 
#include "application.h"

//FreeRTOS
#include "FreeRTOS.h"
#include "task.h"


///////////////////////////////////////////////////////


 int main(void)
 { 
	taskENTER_CRITICAL();	
	xTaskCreate( vSystemInitTask, (const  portCHAR * ) "Init", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, NULL );
	taskEXIT_CRITICAL();
  vTaskStartScheduler();
	return 0;
}

