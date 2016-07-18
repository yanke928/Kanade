//File Name     UI_Adjust.c
//Description : Adjust UI

#include <string.h>
#include <stdio.h>

#include "SSD1306.h"
#include "Keys.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "UI_Adjust.h"
#include "UI_Utilities.h"

#include "ExceptionHandle.h"

#define UI_ADJUST_HANDLER_PRIORITY tskIDLE_PRIORITY+2

xQueueHandle UI_AdjustMsg;

xTaskHandle UI_Adjust_Handle=NULL;

void ReFormatNumString(char string[], u8 targetCharNum)
{
	u8 i, p;
	/*Get the sprinted string length*/
	for (i = 0; string[i] != 0; i++);
	/*if the length of sprinted string is shorter than digitNum*/
	if (targetCharNum > i)
	{
		/*Move the sprinted string back (digitNum-sprintedStringLength) bytes*/
		for (p = 0; p < i; p++)
		{
			string[targetCharNum - 1 - p] = string[i - 1 - p];
		}
	}
	/*Fill the empty digits with '0'*/
	for (p = 0; p < targetCharNum - i; p++)
	{
		string[p] = '0';
	}
	/*End the string*/
	string[targetCharNum] = 0;
}

void UI_Adjust_Handler(void *pvParameters)
{
	UI_Adjust_Param_Struct* adjustParams = (UI_Adjust_Param_Struct*)pvParameters;
	Key_Message_Struct keyMsg;
	u8 digitNum, unitCharNum, length;
	u8 i;
	u8 askStringLength;
	char value[7];
	OLED_PositionStruct positions[4];
	int currentValue = adjustParams->DefaultValue;
	/*Get length of maxValue*/
	if (adjustParams->Max / 10 >= 1)
	{
		digitNum = 2;
		if (adjustParams->Max / 100 >= 1)
		{
			digitNum = 3;
			if (adjustParams->Max / 1000 >= 1)
			{
				digitNum = 4;
				if (adjustParams->Max / 10000 >= 1)
				{
					digitNum = 5;
				}
			}
		}
	}
	/*Get length of unitString*/
	for (i = 0; adjustParams->UnitString[i] != 0; i++);
	unitCharNum = i;
	/*Get total length(pixels) of the adjust UI*/
	length = 40 + (digitNum + unitCharNum) * 8;
	/*Set positions of button(-),currentValue string,button(+) and unitString*/
	positions[0].x = 63 - (length / 2);
	positions[1].x = positions[0].x + 20;
	positions[2].x = positions[1].x + digitNum * 8 + 4;
	positions[3].x = positions[2].x + 20;
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	/*Draw button(-)*/
	OLED_ShowIcon(positions[0].x, adjustParams->Pos_y, 0x01, DRAW);
	/*Print currentValue(defaultValue) to string*/
	sprintf(value, "%d", currentValue);
	/*Resize the currentValueString(to the length of maxValue)*/
	ReFormatNumString(value, digitNum);
	/*show currentValue(defaultValue)*/
	OLED_ShowAnyString(positions[1].x, adjustParams->Pos_y, value, NotOnSelect, 16);
	/*Draw button(+)*/
	OLED_ShowIcon(positions[2].x, adjustParams->Pos_y, 0x00, DRAW);
	/*Draw unitString*/
	OLED_ShowAnyString(positions[3].x, adjustParams->Pos_y, adjustParams->UnitString, NotOnSelect, 16);
	/*Draw grids*/
	OLED_DrawRect(positions[0].x - 2, adjustParams->Pos_y - 2, positions[0].x + length -
		unitCharNum * 8 + 1, adjustParams->Pos_y + 17, DRAW);
	OLED_DrawVerticalLine(positions[0].x + 17, adjustParams->Pos_y - 2, adjustParams->Pos_y + 17, DRAW);
	OLED_DrawVerticalLine(positions[2].x - 2, adjustParams->Pos_y - 2, adjustParams->Pos_y + 17, DRAW);
	askStringLength = GetStringGraphicalLength(adjustParams->AskString);
	/*Show askString*/
	if (askStringLength * 8 + positions[0].x - 2 > 127)
		OLED_ShowAnyString(64 - askStringLength * 8 / 2, adjustParams->Pos_y - 22,
			adjustParams->AskString, NotOnSelect, 16);
	else
		OLED_ShowAnyString(positions[0].x - 2, adjustParams->Pos_y - 22,
			adjustParams->AskString, NotOnSelect, 16);
	SetKeyBeatRate(adjustParams->FastSpeed);
	SetUpdateOLEDJustNow();
	OLED_Refresh_Gram();
	xSemaphoreGive(OLEDRelatedMutex);
	ClearKeyEvent(keyMsg);
	for (;;)
	{
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		/*Print latest value to string*/
		sprintf(value, "%d", currentValue);
		/*Resize the currentValueString(to the length of maxValue)*/
		ReFormatNumString(value, digitNum);
		/*Refresh currentValue*/
		OLED_ShowAnyString(positions[1].x, adjustParams->Pos_y, value, NotOnSelect, 16);
		xSemaphoreGive(OLEDRelatedMutex);
		for (;;)
		{
			xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
			if (LEFT_KEY == KEY_ON)
			{
				OLED_ShowIcon(positions[0].x, adjustParams->Pos_y, 0x01, UNDRAW);
			}
			else
			{
				OLED_ShowIcon(positions[0].x, adjustParams->Pos_y, 0x01, DRAW);
			}
			if (RIGHT_KEY == KEY_ON)
			{
				OLED_ShowIcon(positions[2].x, adjustParams->Pos_y, 0x00, UNDRAW);
			}
			else
			{
				OLED_ShowIcon(positions[2].x, adjustParams->Pos_y, 0x00, DRAW);
			}
			xSemaphoreGive(OLEDRelatedMutex);
			if (xQueueReceive(Key_Message, &keyMsg, 20) == pdPASS)
			{
				if (keyMsg.KeyEvent == LeftClick || keyMsg.AdvancedKeyEvent == LeftContinous)
				{
					currentValue = currentValue - adjustParams->Step;
					if (currentValue < adjustParams->Min) currentValue = adjustParams->Min;
				}
				else if (keyMsg.KeyEvent == RightClick || keyMsg.AdvancedKeyEvent == RightContinous)
				{
					currentValue = currentValue + adjustParams->Step;
					if (currentValue > adjustParams->Max) currentValue = adjustParams->Max;
				}
				else if (keyMsg.KeyEvent == MidClick)
				{
					ResetUpdateOLEDJustNow();
					xQueueSend(UI_AdjustMsg, &currentValue, portMAX_DELAY);
					for(;;) vTaskDelay(portMAX_DELAY);
				}
				break;
			}
		}
	}
}

/**
  * @brief  See UI_Adjust_Handler for details

  * @param  Adjust param struct pointer

  */
void UI_Adjust_Init(UI_Adjust_Param_Struct * adjustParams)
{
	UI_AdjustMsg = xQueueCreate(1, sizeof(u32));
	CreateTaskWithExceptionControl(UI_Adjust_Handler, "UI_Adjust Handler",
		256, adjustParams, UI_ADJUST_HANDLER_PRIORITY, &UI_Adjust_Handle);
}

/**
  * @brief  DeInit UI_Adjust

  * @param  Adjust param struct pointer

  */
void UI_Adjust_DeInit(void)
{
	if(UI_Adjust_Handle!=NULL)
		vTaskDelete(UI_Adjust_Handle);
	vQueueDelete(UI_AdjustMsg);
  UI_Adjust_Handle=NULL;
}

