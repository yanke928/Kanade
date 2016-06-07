//File Name   £ºUI_Dialogue.c
//Description : Dialogue UI

#include <string.h>
#include <stdio.h>

#include "SSD1306.h"
#include "Keys.h" 

#include "UI_Dialogue.h" 
#include "UI_Utilities.h" 

void ShowDialogue(char titleString[],char subString0[],char subString1[])
{
 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
 OLED_DrawRect(0, 0, 127, 63, DRAW);
 OLED_DrawHorizonalLine(13, 0, 127, 1);
 OLED_ShowAnyString(3, 1, titleString,NotOnSelect, 12);
 OLED_ShowAnyString(4, 16, subString0,NotOnSelect, 16);
 OLED_ShowAnyString(4, 42, subString1,NotOnSelect, 16);
 xSemaphoreGive(OLEDRelatedMutex);
}

void ShowSmallDialogue(char string[],u16 time)
{
	u8 startAddr, endAddr;
	u8 stringLength;
	stringLength = GetStringLength(string);
	startAddr = 63 - stringLength * 4;
	endAddr = startAddr + stringLength * 8; 
	xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
	OLED_Clear();
  OLED_DrawRect(startAddr - 4, 20, endAddr + 4, 44, DRAW);
	OLED_ShowAnyString(startAddr, 24, string,NotOnSelect,16);
  xSemaphoreGive(OLEDRelatedMutex);
	vTaskDelay(time/portTICK_RATE_MS);
 	xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
	OLED_Clear();
  xSemaphoreGive(OLEDRelatedMutex);	
}

