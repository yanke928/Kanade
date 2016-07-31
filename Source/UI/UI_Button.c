//File Name     UI_Button.c
//Description : UI button

#include <string.h>
#include <stdio.h>

#include "FreeRTOS_Standard_Include.h"

#include "SSD1306.h"
#include "Keys.h" 

#include "UI_Button.h" 
#include "UI_Utilities.h" 

#include "ExceptionHandle.h" 

#define UI_BUTTON_HANDLER_PRIORITY tskIDLE_PRIORITY+2

xQueueHandle UI_ButtonMsg;

xTaskHandle UI_ButtonHandle = NULL;

/**
  * @brief  Show buttons on screen

  * @param  ButtonString:A string which contains the strings
	  of the buttons,use''%''to seperate two strings
		e.g."string1\nstring2\nstring3"

						ButtonNum:The quantity of the buttons that displayed

						DefaultValue:The default selection of the buttons

						Positions:A struct array contains the positions
	  of the buttons

	@retval The handler will send the result to UI_ButtonMsg when confirmation got
  */
void UI_Button_Handler(void *pvParameters)
{
	Key_Message_Struct keyMessage;
	UI_Button_Param_Struct* buttonParams = (UI_Button_Param_Struct*)pvParameters;
	int selection = buttonParams->DefaultValue;
	u16 i, p;
	bool firstIn = true;

	/*Print the strings with respective positions to screen*/
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	//SetUpdateOLEDJustNow();
	for (i = 0; i < buttonParams->ButtonNum; i++)
	{
		OLED_ShowAnyString(buttonParams->Positions[i].x, buttonParams->Positions[i].y,
			buttonParams->ButtonStrings[i], NotOnSelect, 12);
	}
	ClearKeyEvent(keyMessage);
	//ResetUpdateOLEDJustNow();
	xSemaphoreGive(OLEDRelatedMutex);
	for (;;)
	{
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		//SetUpdateOLEDJustNow();
		/*Print the selection box to strings*/
		for (i = 0; i < buttonParams->ButtonNum; i++)
		{
			p = GetStringGraphicalLength(buttonParams->ButtonStrings[i]);
			//Draw the box if button[i] is selected,undraw if not 
			if (selection == i)
				OLED_DrawRect(buttonParams->Positions[i].x - 2, buttonParams->Positions[i].y - 2,
					buttonParams->Positions[i].x + p * 6 + 1, buttonParams->Positions[i].y + 13, DRAW);
			else
				OLED_DrawRect(buttonParams->Positions[i].x - 2, buttonParams->Positions[i].y - 2,
					buttonParams->Positions[i].x + p * 6 + 1, buttonParams->Positions[i].y + 13, UNDRAW);
		}
		//ResetUpdateOLEDJustNow();
		xSemaphoreGive(OLEDRelatedMutex);
		if (firstIn)
		{
			for (;;)
			{
				vTaskDelay(10 / portTICK_RATE_MS);
				if (MIDDLE_KEY == KEY_OFF)
				{
					firstIn = false;
					break;
				}
			}
		}
		for (;;)
		{
			if (MIDDLE_KEY == KEY_ON)
			{
				xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
				//SetUpdateOLEDJustNow();
				OLED_ShowAnyString(buttonParams->Positions[selection].x, buttonParams->Positions[selection].y,
					buttonParams->ButtonStrings[selection], OnSelect, 12);
				xSemaphoreGive(OLEDRelatedMutex);
				while (MIDDLE_KEY == KEY_ON)
				{
					vTaskDelay(10 / portTICK_RATE_MS);
				}
				xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
				OLED_ShowAnyString(buttonParams->Positions[selection].x, buttonParams->Positions[selection].y,
					buttonParams->ButtonStrings[selection], NotOnSelect, 12);
				xSemaphoreGive(OLEDRelatedMutex);
				vTaskDelay(50 / portTICK_RATE_MS);
				//ResetUpdateOLEDJustNow();
				xQueueSend(UI_ButtonMsg, &selection, 100 / portTICK_RATE_MS);
				for (;;) vTaskDelay(portMAX_DELAY);
			}
			else
			{
				if (xQueueReceive(Key_Message, &keyMessage, 0) == pdPASS)
				{
					switch (keyMessage.KeyEvent)
					{
					case LeftClick:selection--;
						if (selection < 0) selection = buttonParams->ButtonNum - 1; break;
					case RightClick:selection++;
						if (selection > buttonParams->ButtonNum - 1) selection = 0;
					}
					break;
				}
				vTaskDelay(10 / portTICK_RATE_MS);
			}
		}
	}
}

/**
  * @brief  See UI_Button_Handler for details

  * @param  Button param struct pointer

  */
void UI_Button_Init(UI_Button_Param_Struct * buttonParams)
{
	UI_ButtonMsg = xQueueCreate(1, sizeof(u8));
	CreateTaskWithExceptionControl(UI_Button_Handler, "UI_Button Handler",
		300, buttonParams, UI_BUTTON_HANDLER_PRIORITY, &UI_ButtonHandle);
}

/**
  * @brief DeInit button UI
  */
void UI_Button_DeInit()
{
	if (UI_ButtonHandle != NULL)
		vTaskDelete(UI_ButtonHandle);
	vQueueDelete(UI_ButtonMsg);
	UI_ButtonHandle = NULL;
}
