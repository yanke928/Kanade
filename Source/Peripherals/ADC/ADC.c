//File Name    ADC.c
//Description  ADC hardware driver

#include <stdio.h>
#include <math.h>

#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#include "FreeRTOS_Standard_Include.h"

#include "startup.h"

#include "ADC.h"

//Priority of ADC_Filter task 
#define ADC_FILTER_PRIORITY tskIDLE_PRIORITY

//Priority of ADC_Filter task when a quick filter command is triggered
#define QUICK_ADC_FILTER_PRIORITY tskIDLE_PRIORITY+8

//Count of quick collect when a quick filter command is triggered
#define QUICK_ADC_FILTER_COLLECT_CYCLE 20

volatile float FilteredADCValue[ADC_FILTER_ITEM_NUM];

volatile uint16_t  ADCConvertedValue[ADC_FILTER_ITEM_NUM];

float PowerSourceVoltage;

xTaskHandle ADC_Filter_Handle;

xQueueHandle ADC_Quick_Collect_Msg;

/**
  * @brief   Initialize temperatureADC and its corresponding DMA
  * @retval : None
  */
static void ADC_And_DMA_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	DMA_DeInit(DMA1_Channel1);
  ADC_DeInit(ADC1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;//Set DMA memory to ADCConvertedValue
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 6;
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
	ADC_InitStructure.ADC_NbrOfChannel = 6;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 3, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 4, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 5, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 6, ADC_SampleTime_239Cycles5);

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
static void ADC_Filter(void *pvParameters)
{
	u8 m = 0;
	u8 t;
	u8 quickCollectCnt = 0;
	float delta;
	bool currentTrend;
	volatile bool lstTrend[ADC_FILTER_ITEM_NUM];
	volatile float lstCoeficient[ADC_FILTER_ITEM_NUM];
	for (;;)
	{
		/*Keep in loop for QUICK_ADC_FILTER_COLLECT_CYCLE times to collect ADC data quickly*/
		do
		{
			for (m = 0; m < ADC_FILTER_ITEM_NUM; m++)
			{
				delta = (float)ADCConvertedValue[m] - FilteredADCValue[m];
				if (delta < 0) currentTrend = false;
				else currentTrend = true;

				if (currentTrend == lstTrend[m]) lstCoeficient[m] = lstCoeficient[m] * 2;
				else lstCoeficient[m] = 0.001;
				lstTrend[m] = currentTrend;

				lstCoeficient[m] = lstCoeficient[m] <= 0 ? 0.01 : lstCoeficient[m];
				lstCoeficient[m] = lstCoeficient[m] > 0.99 ? 0.99 : lstCoeficient[m];

				FilteredADCValue[m] = delta*lstCoeficient[m] + FilteredADCValue[m];
			}
			if (quickCollectCnt > 0)
			{
				quickCollectCnt--;
				/*Reset priority to lowest after quick colleck command*/
				if (quickCollectCnt == 0) vTaskPrioritySet(NULL, ADC_FILTER_PRIORITY);
			}
		} while (quickCollectCnt > 0);

    if(quickCollectCnt>QUICK_ADC_FILTER_COLLECT_CYCLE) quickCollectCnt=0;

		if (xQueueReceive(ADC_Quick_Collect_Msg, &t, 2 / portTICK_RATE_MS) == pdPASS)
		{
			quickCollectCnt = QUICK_ADC_FILTER_COLLECT_CYCLE;
		}
	}
}

/**
  * @brief  Trigger a quick ADC collect command

  * @param  None

  * @retval None
  */
void Quick_ADC_Collect()
{
	u8 msg;
	/*Set priorty of the filter higher temperarily*/
	vTaskPrioritySet(ADC_Filter_Handle, QUICK_ADC_FILTER_PRIORITY);
	/*Send a Quick Collect Command*/
	xQueueSend(ADC_Quick_Collect_Msg, &msg, 100 / portTICK_RATE_MS);
}

/**
  * @brief  Init ADC hardware

  * @param  None

  * @retval None
  */
void ADC_Hardware_Init()
{
	ADC_And_DMA_Init();
	ADC_Quick_Collect_Msg = xQueueCreate(1, sizeof(u8));
	xTaskCreate(ADC_Filter, "ADC_Filter", 128, NULL, ADC_FILTER_PRIORITY, &ADC_Filter_Handle);
}

void Restart_ADC_And_DMA()
{
 u32 cnt=10000;
 vPortEnterCritical();
 ADC_And_DMA_Init();
 while(cnt--);
 vPortExitCritical();
}
