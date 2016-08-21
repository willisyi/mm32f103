#ifndef __TSENSOR_H
#define __TSENSOR_H	
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//开发板
//ADC 驱动代码			   
////////////////////////////////////////////////////////////////////////////////// 	  
 		 
#define ADC_CH_TEMP  	ADC_Channel_16 //温度传感器通道
   
u16  T_Get_Temp(void);  //取得温度值
void T_Adc_Init(void); //ADC通道初始化
void ADC1_SingleChannel_Get(uint8_t ADC_Channel_x,unsigned int *puiADData);//得到某个通道10次采样的平均值 
void  T_Get_Adc_Average(uint8_t ADC_Channel_x,unsigned int *puiADData);

#endif 
