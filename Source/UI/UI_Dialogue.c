//File Name    UI_Dialogue.c
//Description : Dialogue UI

#include <string.h>
#include <stdio.h>

#include "SSD1306.h"
#include "Keys.h" 

#include "UI_Dialogue.h" 
#include "UI_Utilities.h" 

/**
  * @brief  Show dialogue

  * @param titleString: A string which being shown as title of the dialogue

			 subString0:  First line of the passage

					 subString1:  Second Line of the passage

  */
void ShowDialogue(const char titleString[], const char subString0[], const char subString1[])
{
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_DrawRect(0, 0, 127, 63, DRAW);
	OLED_DrawHorizonalLine(13, 0, 127, 1);
	OLED_ShowAnyString(3, 1, titleString, NotOnSelect, 12);
	OLED_ShowAnyString(4, 16, subString0, NotOnSelect, 16);
	OLED_ShowAnyString(4, 42, subString1, NotOnSelect, 16);
	xSemaphoreGive(OLEDRelatedMutex);
}

/**
  * @brief  Show small dialogue

  * @param  string: Passage

			time:   Time of the dialoue that lasts

			occupyThread:Will ShowSmallDialogue() occupies thread
until time is up
  */
void ShowSmallDialogue(const char string[], u16 time, bool occupyThread)
{
	u8 startAddr, endAddr;
	u8 stringLength;
	stringLength = GetStringGraphicalLength(string);
	startAddr = 63 - stringLength * 4;
	endAddr = startAddr + stringLength * 8;
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	OLED_DrawRect(startAddr - 4, 20, endAddr + 4, 44, DRAW);
	OLED_ShowAnyString(startAddr, 24, string, NotOnSelect, 16);
	OLED_Refresh_Gram();
	if (occupyThread)
	{
		vTaskDelay(time / portTICK_RATE_MS);
	}
	xSemaphoreGive(OLEDRelatedMutex);
	if (occupyThread)
	{
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		OLED_Clear();
		xSemaphoreGive(OLEDRelatedMutex);
	}
}

