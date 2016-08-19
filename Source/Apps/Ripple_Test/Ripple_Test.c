//File Name   Ripple_Test.c
//Description Ripple test

#include <stdio.h>

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#include "MCP3421.h"

#include "SSD1306.h"

#include "FreeRTOS_Standard_Include.h"

#include "UI_Dialogue.h"

#include "Digital_Load.h"

#include "StepUpTest.h"
#include "Ripple_Test.h"

#define VOLT_BUFF_LENGTH 40

float VoltBuff[VOLT_BUFF_LENGTH];

bool FirstCyclePassed = false;

u16 VoltData_p = 0;

void WriteCurrentData2VoltBuff()
{
	VoltBuff[VoltData_p] = CurrentMeterData.Voltage;
	/*Increase data pos*/
	VoltData_p++;

	/*Reset data pos to 0 and clear first cycle flag*/
	if (VoltData_p == VOLT_BUFF_LENGTH)
	{
		FirstCyclePassed = true;
		VoltData_p = 0;
	}
}

float GetRipple()
{
  float min=FindMin(VoltBuff, VOLT_BUFF_LENGTH);
  float max=FindMax(VoltBuff, VOLT_BUFF_LENGTH);
  float ripple=(max-min)/2;
	return ripple;
}

void Run_Ripple_Test()
{
	double ripple;
	char tempString[20];
	ShowDialogue("Ripple Test", "Waiting...", "", false, false);
	Send_Digital_Load_Command(2000, Load_Start);
	vTaskDelay(2000 / portTICK_RATE_MS);
	for (;;)
	{
		WriteCurrentData2VoltBuff();
		vTaskDelay(250 / portTICK_RATE_MS);
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		sprintf(tempString, "%.4fA", FilteredMeterData.Current);
		OLED_ShowAnyString(4, 42, tempString, NotOnSelect, 16);
		sprintf(tempString, "%6.4fV", FilteredMeterData.Voltage);
		OLED_ShowAnyString(63, 42, tempString, NotOnSelect, 16);
		xSemaphoreGive(OLEDRelatedMutex);
		if (FirstCyclePassed == true)
		{
			ripple = GetRipple();
			xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
			sprintf(tempString, "%05.1fmV", ripple * 1000);
			OLED_ShowAnyString(4, 16, tempString, NotOnSelect, 16);
			xSemaphoreGive(OLEDRelatedMutex);
		}
	}
}
