//File Name    ADC.c
//Description  ADC hardware driver

#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>
#include <math.h>

#include "ADC.h"
#include "startup.h"
#include "MCP3421.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define ADC_FILTER_PRIORITY tskIDLE_PRIORITY

volatile float FilteredADCValue[ADC_FILTER_ITEM_NUM];

volatile uint16_t  ADCConvertedValue[ADC_FILTER_ITEM_NUM];

float PowerSourceVoltage;

/**
  * @brief   Initialize temperatureADC and its corresponding DMA
  * @retval : None
  */
void ADC_And_DMA_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;//Set DMA memory to ADCConvertedValue
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 5;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;

	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;

	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	DMA_Cmd(DMA1_Channel1, ENABLE);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;

	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 5;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 4, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 5, ADC_SampleTime_239Cycles5);

	ADC_TempSensorVrefintCmd(ENABLE);

	ADC_DMACmd(ADC1, ENABLE);

	ADC_Cmd(ADC1, ENABLE);

	ADC_ResetCalibration(ADC1);

	while (ADC_GetResetCalibrationStatus(ADC1));

	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1));

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/**
  * @brief   ADC filter
  * @retval : None
  */
void ADC_Filter(void *pvParameters)
{
	u8 m=0;
  float delta;
	bool currentTrend;
  bool lstTrend[ADC_FILTER_ITEM_NUM];
  float lstCoeficient[ADC_FILTER_ITEM_NUM];
	while (1)
	{
		for (m = 0; m < ADC_FILTER_ITEM_NUM; m++)
		{
		 delta=(float)ADCConvertedValue[m]-FilteredADCValue[m];
		 if(delta<0) currentTrend=false;
		 else currentTrend=true;

		 if(currentTrend==lstTrend[m]) lstCoeficient[m]=lstCoeficient[m]*2;
		 else lstCoeficient[m]=0.001;
		 lstTrend[m]=currentTrend;
			
		 lstCoeficient[m]=lstCoeficient[m]<=0?0.01:lstCoeficient[m];
		 lstCoeficient[m]=lstCoeficient[m]>0.99?0.99:lstCoeficient[m];
			
		 FilteredADCValue[m]=delta*lstCoeficient[m]+FilteredADCValue[m];
		}
		vTaskDelay(2 / portTICK_RATE_MS);
	}
}

void ADC_Hardware_Init()
{
 ADC_And_DMA_Init(); 
 xTaskCreate(ADC_Filter,"ADC_Filter",128,NULL,ADC_FILTER_PRIORITY,NULL);
}
