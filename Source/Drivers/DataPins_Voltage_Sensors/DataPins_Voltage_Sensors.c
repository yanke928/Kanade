//File Name    DataPins_Voltage_Sensors.c
//Description  DataPins_Voltage_Sensor driver

#include <stdio.h>

#include "stm32f10x.h"

#include "ADC.h"
#include "MCP3421.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "DataPins_Voltage_Sensors.h"

#define DATAPINS_VOLTAGE_UPDATE_HANDLER_PRIORITY tskIDLE_PRIORITY

void DataPins_Voltage_Handler(void *pvParameters)
{
 for(;;)
	{
	 vTaskDelay( 100/ portTICK_RATE_MS);
	 CurrentMeterData.VoltageDP=(float)FilteredADCValue[2]/4096*PowerSourceVoltage;
	 CurrentMeterData.VoltageDM=(float)FilteredADCValue[1]/4096*PowerSourceVoltage;
	}
}

void DataPins_Voltage_Sensor_Init()
{
 xTaskCreate(DataPins_Voltage_Handler,"DataPins_Voltage_Handler",64,NULL,
DATAPINS_VOLTAGE_UPDATE_HANDLER_PRIORITY,NULL);
}
