//File Name    UI_ProgressBar.c
//Description : ProgressBar UI

#include <string.h>
#include <stdio.h>

#include "SSD1306.h"

#include "UI_Utilities.h" 

#include "UI_Dialgram.h"

#include "UI_ProgressBar.h"

#include "ExceptionHandle.h"

#define UI_PROGRESSBAR_HANDLER_PRIORITY tskIDLE_PRIORITY+2

xQueueHandle UI_ProgressBarMsg;

xTaskHandle UI_ProgressBarHandle=NULL;

void UI_ProgressBar_Handler(void *pvParameters)
{
	ProgressBar_Param_Struct* progressBar_Params = (ProgressBar_Param_Struct*)pvParameters;
	int lastPos = progressBar_Params->Pos1.x - 1;
	int progressBarLength = progressBar_Params->Pos2.x - progressBar_Params->Pos1.x;
	int currentPos = progressBar_Params->Pos1.x - 1;
	float currentValue = progressBar_Params->MinValue;
	for (;;)
	{
		if (xQueueReceive(UI_ProgressBarMsg, &currentValue, portMAX_DELAY) == pdPASS)
		{
			if (currentValue < 0)
			{
				for(;;) vTaskDelay(portMAX_DELAY);
			}
			currentPos = (progressBar_Params->Pos1.x - 1) + progressBarLength*((currentValue - progressBar_Params->MinValue) /
				(progressBar_Params->MaxValue - progressBar_Params->MinValue)) + 1;
			xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
			if (lastPos != currentPos&&currentPos > progressBar_Params->Pos1.x - 1 && currentPos < progressBar_Params->Pos2.x + 1)
			{
				if (currentPos >= lastPos)
					OLED_InvertRect(lastPos + 1, progressBar_Params->Pos1.y, currentPos, progressBar_Params->Pos2.y);
				else
					OLED_InvertRect(currentPos, progressBar_Params->Pos1.y, lastPos - 1, progressBar_Params->Pos2.y);
			}
			xSemaphoreGive(OLEDRelatedMutex);
			lastPos = currentPos;
		}
	}
}

void UI_ProgressBar_Init(ProgressBar_Param_Struct * progressBarParams)
{
	UI_ProgressBarMsg = xQueueCreate(1, sizeof(float));
	CreateTaskWithExceptionControl(UI_ProgressBar_Handler, "UI_ProgressBar Handler",
		128, progressBarParams, UI_PROGRESSBAR_HANDLER_PRIORITY, &UI_ProgressBarHandle);
}

void UI_ProgressBar_DeInit()
{
 if(UI_ProgressBarHandle!=NULL)
	 vTaskDelete(UI_ProgressBarHandle);
 vQueueDelete(UI_ProgressBarMsg);
 UI_ProgressBarHandle=NULL;
}
