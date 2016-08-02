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

float ExternalTemperature;

volatile u8 CurrentTemperatureSensor = Internal;

const u8 TempTab[TEMPTAB_LENGTH] = { 0,10,20,30,40,50,60,70,80,90,100,110,120
,130,140,150,160,170,180,190,200 };

const float ResistTab[TEMPTAB_LENGTH] = { 325,200,125,80,52,35.7,24.6,17.29,12.35,8.927,6.55,
4.888,3.702,2.812,2.146,1.665,1.314,1.051,0.842,0.676,0.552 };

void CalculateExtTemp(void);

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

/**
  * @brief   Config the INTTempSensor

  * @retval : None
  */
void TemperatureSensors_Init(void)
{
	xTaskCreate(Temperature_Handler, "Temperature_Handler",
		160, NULL, TEMPERATURE_UPDATE_HANDLER_PRIORITY, NULL);
}

