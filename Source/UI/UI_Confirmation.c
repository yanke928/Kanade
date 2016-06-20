//File Name    UI_Confirmation.c
//Description : Confirmation UI

#include <string.h>
#include <stdio.h>

#include "UI_Button.h" 
#include "UI_Dialogue.h" 
#include "MultiLanguageStrings.h" 

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "SSD1306.h"
#include "Keys.h"

#include "Settings.h"

#include "UI_Confirmation.h" 

bool GetConfirmation(const char subString0[],const char subString1[])
{
 u8 i;
 Key_Message_Struct keyMessage;
 UI_Button_Param_Struct buttonParams;
 buttonParams.ButtonString=ConfirmCancel_Str[CurrentSettings->Language];
 buttonParams.ButtonNum=2;
 buttonParams.DefaultValue=0;
 buttonParams.Positions=(OLED_PositionStruct *)ComfirmationPositions[CurrentSettings->Language];
 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
 OLED_Clear();
 xSemaphoreGive(OLEDRelatedMutex);
 ShowDialogue(Confirmation_Str[CurrentSettings->Language],subString0,subString1);
 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
 OLED_Refresh_Gram();
 xSemaphoreGive(OLEDRelatedMutex);
 UI_Button_Init(&buttonParams);
 xQueueReceive(UI_ButtonMsg, & i, portMAX_DELAY );
 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
 OLED_Clear();
 xSemaphoreGive(OLEDRelatedMutex);
 xQueueReceive(Key_Message, & keyMessage, portMAX_DELAY );
 if(i) return true;
 else return false;
}
