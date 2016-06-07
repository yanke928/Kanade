//File Name   £ºUI_Dialogue.c
//Description : UI Dialogue

#include <string.h>
#include <stdio.h>

#include "SSD1306.h"
#include "UI_Button.h" 
#include "Keys.h" 

#include "UI_Dialogue.h" 

void ShowDialogue(char titleString[],char subString0[],char subString1[])
{
 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
 OLED_DrawRect(0, 0, 127, 63, DRAW);
 OLED_DrawHorizonalLine(13, 0, 127, 1);
 OLED_ShowAnyString(3, 1, titleString,NotOnSelect, 12);
 OLED_ShowString(5, 18, subString0);
 OLED_ShowString(5, 42, subString1);
 xSemaphoreGive(OLEDRelatedMutex);
}

