//File Name   £ºInternalTempSensor.c
//Description £ºGet temperature from the NTC inside the chip

#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>
#include "math.h"

#include "TempSensors.h"
#include "startup.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define TEMP_UPDATE_PRIORITY tskIDLE_PRIORITY

//see .h for details
float InternalTemperature;

float ExternalTemperature;

volatile uint16_t FilteredADCValue[ADC_FILTER_ITEM_NUM];

//see .h for details
volatile uint16_t  ADCConvertedValue[3];

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
void TempADC_AND_DMA_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;//Set DMA memory to ADCConvertedValue
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 3;
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
	ADC_InitStructure.ADC_NbrOfChannel = 3;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 3, ADC_SampleTime_239Cycles5);

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
void TemperatureHandler(void *pvParameters)
{
	u8 m, n, updateTempCount;
	u32 p;
	bool firstStart=true;
	volatile uint16_t FirstLevelADCFilterTank[ADC_FILTER_ITEM_NUM][ADC_FILTER_TANK_SIZE];
	volatile uint16_t SecondLevelADCFilterTank[ADC_FILTER_ITEM_NUM][ADC_FILTER_TANK_SIZE];
	updateTempCount = 0;
	p = 0;
	while (1)
	{
		SecondLevelFilterCount++;
		updateTempCount++;
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
		if (SecondLevelADCFilterTank[2][0] >= 200)
		{
			CurrentTemperatureSensor = External;
		}
		else
		{
			CurrentTemperatureSensor = Internal;
		}
		if (updateTempCount >= 40)
		{
			updateTempCount = 0;
			PowerSourceVoltage = (1.2 / (float)FilteredADCValue[1]) * 4096;
			InternalTemperature = (1.43 - (float)FilteredADCValue[0] *
				(PowerSourceVoltage / 4096)) * 1000 / 4.35 + 25;
			if (CurrentTemperatureSensor == External)
			{
				CalculateExtTemp();
			}
		}
		vTaskDelay(5 / portTICK_RATE_MS);
	}
}

/**
  * @brief   Config the INTTempSensor
  * @retval : None
  */
void TemperatureSensors_Init(void)
{
	TempADC_AND_DMA_Init();
	CurrentTemperatureSensor = Internal;
	xTaskCreate(TemperatureHandler, "Temperature Handler",
		128, NULL, TEMP_UPDATE_PRIORITY, NULL);
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

///**
//  * @brief    Detect the statur of external temperature sensor(if exist)

//	@param  : init: If the function is called while system initialization

//  * @retval : If external teperature sensor lost or replugged
//  */
//bool TemperatureSensorDetect(bool init)
//{
//	u8 logoTimerNo;
//	if (init)
//	{
//		logoTimerNo = LogoInit();
//	}
//	if (ADCConvertedValue[2] <= 200)
//	{
//		if (CurrentTemperatureSensor == External)
//		{
//			CurrentTemperatureSensor = Internal;
//			if (!init)
//				SendLogString("External_Temperature_Sensor Lost\n");
//			SendLogString("Internal_Temperature_Sensor is present\n\n");
//			return(true);
//		}
//		if (init)
//		{
//			ShowInitString("IntTemp is present", logoTimerNo);
//			SendLogString("Interrnal_Temperature_Sensor is present\n\n");
//			Delayxms(500);
//			SystemSubTimerDeInit(logoTimerNo);
//		}
//		return(false);
//	}
//	else
//	{
//		if (CurrentTemperatureSensor == Internal)
//		{
//			CurrentTemperatureSensor = External;
//			CalculateExtTemp();
//			if (init)
//			{
//				ShowInitString("ExtTemp is present", logoTimerNo);
//				SendLogString("External_Temperature_Sensor is present\n\n");
//				Delayxms(500);
//				SystemSubTimerDeInit(logoTimerNo);
//			}
//			else
//			{
//				SendLogString("External_Temperature_Sensor Plugged\n");
//				SendLogString("External_Temperature_Sensor is present\n\n");
//			}
//			return(true);
//		}
//		CalculateExtTemp();
//		return(false);
//	}
//}

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
	Vtemp = 3.3 - ((float)FilteredADCValue[2] / 4096)*3.3;
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
