#include "tsensor.h"
#include "delay.h"
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//开发板
//ADC 驱动代码			   
////////////////////////////////////////////////////////////////////////////////// 	  
 
		   
//初始化ADC
//这里我们仅以规则通道为例
//我们默认将开启通道0~3	
void T_Adc_Init(void)  //ADC通道初始化
{
	ADC_InitTypeDef ADC_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1	, ENABLE );	  //使能GPIOA,ADC1通道时钟
  
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //分频因子6时钟为72M/6=12MHz

   	ADC_DeInit(ADC1);  //将外设 ADC1 的全部寄存器重设为缺省值
 
	/* Initialize the ADC_PRESCARE values */
  ADC_InitStructure.ADC_PRESCARE = ADC_PCLK2_PRESCARE_16;
  /* Initialize the ADC_Mode member */
  ADC_InitStructure.ADC_Mode = ADC_Mode_Single;
	/* Initialize the ADC_ContinuousConvMode member */
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  /* Initialize the ADC_DataAlign member */
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	 /* Initialize the ADC_ExternalTrigConv member */
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器
	ADC1->ADCHS&=0xffffffe00;	/*屏蔽所有通道*/


	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 0, 0); 

	ADC_TempSensorVrefintCmd(ENABLE); //开启内部温度传感器
	ADC1->ADCFG|=0x04;

 
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1


}
void  T_Get_Adc_Average(uint8_t ADC_Channel_x,unsigned int *puiADData)
{
	unsigned int  uiCnt=0;
	/*ADCR寄存器的ADST位使能，软件启动转换*/
	ADC_SoftwareStartConvCmd(ADC1, ENABLE); 
	
	do
	{
		/*读当前数据寄存器值*/
		*puiADData=ADC1->ADDATA;
		uiCnt++;
	}
	while( (((*puiADData>>21)&0x01)==0) &&  (uiCnt<100)); 
	/*1.判断是否超时 2.判断是否有有效数据 3.判断是否为当前通道号*/
	if((uiCnt<100)&&(((*puiADData>>21)&0x01)==1)&&(((*puiADData>>16)&0x0f)==ADC_Channel_x))
	{	
		*puiADData&=0xfff;
	}
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	/*ADCR寄存器的ADST位失能，软件转换结束*/
	ADC_SoftwareStartConvCmd(ADC1, DISABLE); 
	
} 	   

