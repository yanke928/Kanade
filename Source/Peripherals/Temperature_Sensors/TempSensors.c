//File Name    ADC.c
//Description  All about internal ADC

#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>
#include <math.h>

#include "TempSensors.h"
#include "startup.h"
#include "MCP3421.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define ADC_FILTER_PRIORITY tskIDLE_PRIORITY

//see .h for details
float InternalTemperature;

float ExternalTemperature;

volatile uint16_t FilteredADCValue[ADC_FILTER_ITEM_NUM];

//see .h for details
volatile uint16_t  ADCConvertedValue[ADC_FILTER_ITEM_NUM];

volatile u8 SecondLevelFilterCount = 0;

float PowerSourceVoltage;

volatile u8 CurrentTemperatureSensor = Internal;

const u8 TempTab[TEMPTAB_LENGTH] = { 0,10,20,30,40,50,60,70,80,90,100,110,120
,130,140,150,160,170,180,190,200 };

const float ResistTab[TEMPTAB_LENGTH] = { 325,200,125,80,52,35.7,24.6,17.29,12.35,8.927,6.55,
4.888,3.702,2.812,2.146,1.665,1.314,1.051,0.842,0.676,0.552 };

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
  * @brief   Temperature task
  * @retval : None
  */
void ADC_Filter(void *pvParameters)
{
	u8 m, n;
	u32 p;
	bool firstStart=true;
	volatile uint16_t FirstLevelADCFilterTank[ADC_FILTER_ITEM_NUM][ADC_FILTER_TANK_SIZE];
	volatile uint16_t SecondLevelADCFilterTank[ADC_FILTER_ITEM_NUM][ADC_FILTER_TANK_SIZE];
	p = 0;
	while (1)
	{
		SecondLevelFilterCount++;
		/*If the sub-filter tank is full(10 effective records got)*/
		if (SecondLevelFilterCount == ADC_FILTER_TANK_SIZE)
		{
			SecondLevelFilterCount = 0;
			for (m = 0; m < ADC_FILTER_ITEM_NUM; m++)
			{
				p = 0;
				for (n = 0; n < ADC_FILTER_TANK_SIZE; n++)
				{
					p = p + FirstLevelADCFilterTank[m][n];
				}
				for (n = 1; n < ADC_FILTER_TANK_SIZE; n++)
				{
					SecondLevelADCFilterTank[m][ADC_FILTER_TANK_SIZE - n] =
						SecondLevelADCFilterTank[m][ADC_FILTER_TANK_SIZE - n - 1];
				}
				SecondLevelADCFilterTank[m][0] = p / ADC_FILTER_TANK_SIZE;
				if(firstStart) 
				{
				 for(n = 1; n < ADC_FILTER_TANK_SIZE; n++)
					{
					 SecondLevelADCFilterTank[m][n] = p / ADC_FILTER_TANK_SIZE;
					}
				}
				p = 0;
				for (n = 0; n < ADC_FILTER_TANK_SIZE; n++)
				{
					p = p + SecondLevelADCFilterTank[m][n];
				}
				FilteredADCValue[m] = p / ADC_FILTER_TANK_SIZE;
			}
			firstStart=false;
		}
		/*Else add a new record to sub-filter tank*/
		else
		{
			for (m = 0; m < ADC_FILTER_ITEM_NUM; m++)
			{
				for (n = 1; n < ADC_FILTER_TANK_SIZE; n++)
				{
					FirstLevelADCFilterTank[m][ADC_FILTER_TANK_SIZE - n] =
						FirstLevelADCFilterTank[m][ADC_FILTER_TANK_SIZE - n - 1];
				}
				FirstLevelADCFilterTank[m][0] =
					ADCConvertedValue[m];
			}
		}
		vTaskDelay(2 / portTICK_RATE_MS);
	}
}

void Temperature_Handler(void *pvParameters)
{
 for(;;)
	{
	 vTaskDelay( 100/ portTICK_RATE_MS);
	 PowerSourceVoltage = (1.2 / (float)FilteredADCValue[4]) * 4096;
	 InternalTemperature = (1.43 - (float)FilteredADCValue[3] *
	 (PowerSourceVoltage / 4096)) * 1000 / 4.35 + 25;
		if (FilteredADCValue[0] >= 200)
		{
			CurrentTemperatureSensor = External;
		}
		else
		{
			CurrentTemperatureSensor = Internal;
		}
	  if (CurrentTemperatureSensor == External)
			{
				CalculateExtTemp();
			}
	}
}

void DataPins_Voltage_Handler(void *pvParameters)
{
 for(;;)
	{
	 vTaskDelay( 100/ portTICK_RATE_MS);
	 CurrentMeterData.VoltageDP=(float)FilteredADCValue[2]/4096*PowerSourceVoltage;
	 CurrentMeterData.VoltageDM=(float)FilteredADCValue[1]/4096*PowerSourceVoltage;
	}
}

/**
  * @brief   Config the INTTempSensor
  * @retval : None
  */
void TemperatureSensors_Init(void)
{
	ADC_And_DMA_Init();
	CurrentTemperatureSensor = Internal;
	xTaskCreate(ADC_Filter, "ADC_Filter",
		160, NULL, ADC_FILTER_PRIORITY, NULL);
	xTaskCreate(Temperature_Handler, "Temperature_Handler",
		64, NULL, ADC_FILTER_PRIORITY, NULL);
	xTaskCreate(DataPins_Voltage_Handler, "DataPins_Voltage_Handler",
		64, NULL, ADC_FILTER_PRIORITY, NULL);
}


/**
  * @brief   Generate a string for INTTemperature in format xx.xC
  * @retval : None
  */
void GenerateTempString(char string[], u8 sensorNo)
{
	if (sensorNo == Internal)
		sprintf(string, "%5.1fC", InternalTemperature);
	else
		sprintf(string, "%5.1fC", ExternalTemperature);
}

/**
  * @brief    Calculate the external temperature according to
ADC result

  * @retval : None
  */
void CalculateExtTemp(void)
{
	float Vtemp, Rtemp;
	float k;
	u8 i;
	/*Calculate the voltage that temperature sensor divided*/
	Vtemp = 3.3 - ((float)FilteredADCValue[0] / 4096)*3.3;
	/*Calculate the resistance of temperature sensor*/
	Rtemp = (Vtemp / (3.3 - Vtemp))*TEMP_DIVIDER;
	if (Rtemp >= ResistTab[0])
	{
		ExternalTemperature = TempTab[0];
		return;
	}
	else if (Rtemp <= ResistTab[TEMPTAB_LENGTH - 1])
	{
		ExternalTemperature = TempTab[TEMPTAB_LENGTH - 1];
		return;
	}
	for (i = 0; i < TEMPTAB_LENGTH - 1; i++)
	{
		if (Rtemp > ResistTab[i + 1])
		{
			k = (float)(TempTab[i + 1] - TempTab[i]) / (ResistTab[i + 1] - ResistTab[i]);
			ExternalTemperature = TempTab[i] - (k*(ResistTab[i] - Rtemp));
			return;
		}
	}
}


/**
  * @brief    Show current selected temperature sensor

  * @retval : None
  */
void ShowCurrentTempSensor(void)
{
	if (CurrentTemperatureSensor == Internal)
	{
		xQueueSend(InitStatusMsg, "No Ext.Sensor", 0);
	}
	else
	{
		xQueueSend(InitStatusMsg, "Ext.Sensor Plugged", 0);
	}
}
