//File Name   £ºUI_Confirmation.c
//Description : Confirmation UI

#include <string.h>
#include <stdio.h>

#include "SSD1306.h"
#include "UI_Button.h" 
#include "UI_Dialogue.h" 
#include "MultiLanguageStrings.h" 

#include "UI_Confirmation.h" 

bool GetConfirmation(char subString0[],char subString1[])
{
 u8 i;
 UI_Button_Param_Struct buttonParams;
 buttonParams.ButtonString=(char *)ConfirmCancel_Str[Language];
 buttonParams.ButtonNum=2;
 buttonParams.DefaultValue=0;
 buttonParams.Positions=(OLED_PositionStruct *)ComfirmationPositions[Language];
 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
 OLED_Clear();
 xSemaphoreGive(OLEDRelatedMutex);
 ShowDialogue((char *)Confirmation_Str[Language],subString0,subString1);
 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
 OLED_Refresh_Gram();
 xSemaphoreGive(OLEDRelatedMutex);
 UI_Button_Init(&buttonParams);
 xQueueReceive(UI_ButtonMsg, & i, portMAX_DELAY );
 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
 OLED_Clear();
 xSemaphoreGive(OLEDRelatedMutex);
 if(i) return true;
 else return false;
}
