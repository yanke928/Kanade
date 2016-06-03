//File Name   £ºUSBMeterUI.c
//Description £ºMain UI

#include "stm32f10x.h"
#include "stm32f10x_it.h"

#include <stdio.h>
#include <string.h>

#include "USBMeter.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "EBProtocol.h"
#include "TempSensors.h"
#include "SSD1306.h"
#include "RTC.h"
#include "VirtualRTC.h"

#define USB_METER_PRIORITY tskIDLE_PRIORITY+3

/**
  * @brief  USBMeter

  * @retval None
  */
void USBMeter(void *pvParameters)
{
 char tempString[20];
 portTickType xLastWakeTime;
 u8 status=*(u8*)pvParameters;
 xLastWakeTime=xTaskGetTickCount();
	while (1) 
	{
	 xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY );
	 DisplayBasicData(tempString,status);
	 if(status==USBMETER_RECORD)
	 {
	  DisplayRecordData(tempString);
	 }
	 xSemaphoreGive(OLEDRelatedMutex);
	 vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);
	}
}

/**
  * @brief  Display Volt,Cureent,Power and Temperature
	on the screen
  * @param  a tempString which includes at lease 7 elements
	By sharing the tempString,more sources can be saved
	@retval None
  */
void DisplayBasicData(char tempString[],u8 currentStatus)
{
	if (CurrentMeterData.Voltage >= 10)
	{
		sprintf(tempString, "%5.2fV", CurrentMeterData.Voltage);
	}
	else
	{
		sprintf(tempString, "%5.3fV", CurrentMeterData.Voltage);
	}
	OLED_ShowString(0, 0, (unsigned char*)tempString);
	if (CurrentMeterData.Current >= 0.1)
		sprintf(tempString, "%5.3fA", CurrentMeterData.Current);
	else
		sprintf(tempString, "%04.1fmA", CurrentMeterData.Current * 1000);
	OLED_ShowString(80, 0, (unsigned char*)tempString);
	if (CurrentMeterData.Power >= 10)
	{
		sprintf(tempString, "%5.2fW", CurrentMeterData.Power);
	}
	else
	{
		sprintf(tempString, "%5.3fW", CurrentMeterData.Power);
	}
	OLED_ShowString(0, 16, (unsigned char*)tempString);
	if (CurrentTemperatureSensor == Internal)
	{
		GenerateTempString(tempString, Internal);
		if (tempString[0] != '1')tempString[0] = '0';
		OLED_ShowString(80, 16, (unsigned char*)tempString);
	}
	else
	{
		GenerateTempString(tempString, External);
		OLED_ShowSelectionString(80, 16, (unsigned char *)"Ex", NotOnSelect, 8);
		OLED_ShowSelectionString(92, 16, (unsigned char *)tempString, NotOnSelect, 8);
		GenerateTempString(tempString, Internal);
		OLED_ShowSelectionString(80, 24, (unsigned char *)"In", NotOnSelect, 8);
		OLED_ShowSelectionString(92, 24, (unsigned char *)tempString, NotOnSelect, 8);
	}
	if (currentStatus==USBMETER_ONLY)
	{
		GenerateRTCDateString(tempString);
		OLED_ShowString(0, 32, (unsigned char*)tempString);
		GenerateRTCTimeString(tempString);
		OLED_ShowString(0, 48, (unsigned char*)tempString);
		GenerateRTCWeekString(tempString);
		OLED_ShowString(104, 32, (unsigned char*)tempString);
		sprintf(tempString, "%5.2fV", CurrentMeterData.VoltageDP);
		OLED_ShowSelectionString(92, 48, (unsigned char*)tempString, NotOnSelect, 8);
		sprintf(tempString, "%5.2fV", CurrentMeterData.VoltageDM);
		OLED_ShowSelectionString(92, 56, (unsigned char*)tempString, NotOnSelect, 8);
		OLED_ShowSelectionString(80, 48, (unsigned char*)"D+", NotOnSelect, 8);
		OLED_ShowSelectionString(80, 56, (unsigned char*)"D-", NotOnSelect, 8);
	}
}

/**
  * @brief  Display Capacity,Work,LastingTime
  * @param  a tempString which includes at lease 9 elements
	By sharing the tempString,more sources can be saved
	@retval None
  */
void DisplayRecordData(char tempString[])
{
	if (CurrentSumUpData.Capacity >= 10000)
		sprintf(tempString, "%05.2fAh", CurrentSumUpData.Capacity);
	else
		sprintf(tempString, "%05.0fmAh", CurrentSumUpData.Capacity * 1000);
	OLED_ShowString(0, 32, (unsigned char*)tempString);
	if (CurrentSumUpData.Work >= 10000)
		sprintf(tempString, "%05.2fWh", CurrentSumUpData.Work);
	else
		sprintf(tempString, "%05.0fmWh", CurrentSumUpData.Work * 1000);
	OLED_ShowString(0, 48, (unsigned char*)tempString);
	GenerateVirtualRTCString((unsigned char *)tempString);
	OLED_ShowSelectionString(82, 51, (unsigned char *)tempString, NotOnSelect, 12);
	sprintf(tempString, "%d Day(s)", RTCCurrent.Day);
	OLED_ShowSelectionString(82, 35, (unsigned char *)tempString, NotOnSelect, 12);
}

void USBMeter_Init(u8 status)
{
 xTaskCreate(USBMeter,"USBMeter",
	256,&status,USB_METER_PRIORITY,NULL);
}
