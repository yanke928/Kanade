//File Name    Temperature_Sensors.c
//Description  Temperature_Sensor driver

#include <stdio.h>

#include "stm32f10x.h"

#include "ADC.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "Startup.h"

#include "Temperature_Sensors.h"

#define TEMPERATURE_UPDATE_HANDLER_PRIORITY tskIDLE_PRIORITY

float InternalTemperature;

float MOSTemperature;

float ExternalTemperature;

volatile u8 CurrentTemperatureSensor = Internal;

const u8 TempTab[TEMPTAB_LENGTH] = { 0,10,20,30,40,50,60,70,80,90,100,110,120
,130,140,150,160,170,180,190,200 };

const float ResistTab[TEMPTAB_LENGTH] = { 325,200,125,80,52,35.7,24.6,17.29,12.35,8.927,6.55,
4.888,3.702,2.812,2.146,1.665,1.314,1.051,0.842,0.676,0.552 };

float CalculateNTCTemp(u8 adcIndex);

void Temperature_Handler(void *pvParameters)
{
	for (;;)
	{
		vTaskDelay(100 / portTICK_RATE_MS);
		PowerSourceVoltage = (1.2 / (float)FilteredADCValue[5]) * 4096;
		InternalTemperature = (1.43 - (float)FilteredADCValue[4] *
			(PowerSourceVoltage / 4096)) * 1000 / 4.35 + 25;
		if (FilteredADCValue[1] >= 200)
		{
			CurrentTemperatureSensor = External;
		}
		else
		{
			CurrentTemperatureSensor = Internal;
		}
		MOSTemperature = CalculateNTCTemp(0);
		if (CurrentTemperatureSensor == External)
		{
			ExternalTemperature = CalculateNTCTemp(1);
		}
	}
}

/**
  * @brief   Generate a string for INTTemperature in format xx.xC

  * @retval : None
  */
void GenerateTempString(char string[], u8 sensorNo)
{
	switch (sensorNo)
	{
	case Internal:sprintf(string, "%5.1fC", InternalTemperature); break;
	case External:sprintf(string, "%5.1fC", ExternalTemperature); break;
	case MOS:sprintf(string, "%5.1fC", MOSTemperature); break;
	}
}


/**
  * @brief    Calculate the external temperature according to
ADC result

  * @retval : None
  */
float CalculateNTCTemp(u8 adcIndex)
{
	float Vtemp, Rtemp;
	float temperature;
	float k;
	u8 i;
	/*Calculate the voltage that temperature sensor divided*/
	Vtemp = 3.3 - ((float)FilteredADCValue[adcIndex] / 4096)*3.3;
	/*Calculate the resistance of temperature sensor*/
	Rtemp = (Vtemp / (3.3 - Vtemp))*TEMP_DIVIDER;
	if (Rtemp >= ResistTab[0])
	{
		temperature = TempTab[0];
		return temperature;
	}
	else if (Rtemp <= ResistTab[TEMPTAB_LENGTH - 1])
	{
		temperature = TempTab[TEMPTAB_LENGTH - 1];
		return temperature;
	}
	for (i = 0; i < TEMPTAB_LENGTH - 1; i++)
	{
		if (Rtemp > ResistTab[i + 1])
		{
			k = (float)(TempTab[i + 1] - TempTab[i]) / (ResistTab[i + 1] - ResistTab[i]);
			temperature = TempTab[i] - (k*(ResistTab[i] - Rtemp));
			return temperature;
		}
	}
	return 0;
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

/**
  * @brief   Config the INTTempSensor

  * @retval : None
  */
void TemperatureSensors_Init(void)
{
	xTaskCreate(Temperature_Handler, "Temperature_Handler",
		160, NULL, TEMPERATURE_UPDATE_HANDLER_PRIORITY, NULL);
}

/**
  * @brief   Get current enviroment temperature quickly

  * @retval : None
  */
void QuickGet_Enviroment_Temperature(void)
{
	Quick_ADC_Collect();
	vTaskDelay(2 / portTICK_RATE_MS);
	MOSTemperature = CalculateNTCTemp(0)-1.5;
}

