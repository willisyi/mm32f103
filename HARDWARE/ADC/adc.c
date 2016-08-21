 #include "adc.h"
 #include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//开发板
//ADC 代码	   
////////////////////////////////////////////////////////////////////////////////// 
	   
		   
//初始化ADC
//这里我们仅以规则通道为例
//我们默认将开启通道0~3																	   
void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1	, ENABLE );	  //使能ADC1通道时钟
 
	//PA1 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

	/* Initialize the ADC_PRESCARE values */
  ADC_InitStructure.ADC_PRESCARE = ADC_PCLK2_PRESCARE_16;//16分频
  /* Initialize the ADC_Mode member */
  ADC_InitStructure.ADC_Mode = ADC_Mode_Single;//单次转换模式
	/* Initialize the ADC_ContinuousConvMode member */
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//失能连续转换模式
  /* Initialize the ADC_DataAlign member */
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//数据右对齐
  /* Initialize the ADC_ExternalTrigConv member */
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;//ADC1 通道1，即PA1
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   


	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1

}				  
//获得ADC值
//ch:通道值 0~3
u16 Get_Adc(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 0, 0 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
	
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);//清除转换结束标志位
	ADC_SoftwareStartConvCmd(ADC1, DISABLE); 

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

//获取平均值
u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 	 



























