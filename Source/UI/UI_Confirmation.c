//File Name    UI_Confirmation.c
//Description : Confirmation UI

#include <string.h>
#include <stdio.h>

#include "FreeRTOS_Standard_Include.h"

#include "SSD1306.h"
#include "Keys.h"

#include "UI_Button.h" 
#include "UI_Dialogue.h" 

#include "MultiLanguageStrings.h" 

#include "Settings.h"

#include "UI_Confirmation.h" 

bool GetConfirmation(const char subString0[], const char subString1[])
{
	u8 i;
	const char* yesOrNoStrings[2];
	UI_Button_Param_Struct buttonParams;
	
	yesOrNoStrings[0]=ConfirmationCancel_Str[CurrentSettings->Language];
  yesOrNoStrings[1]=ConfirmationConfirm_Str[CurrentSettings->Language];
	
	//Key_Message_Struct keyMessage;
	buttonParams.ButtonStrings = yesOrNoStrings;
	buttonParams.ButtonNum = 2;
	buttonParams.DefaultValue = 0;
	buttonParams.Positions = (OLED_PositionStruct *)ComfirmationPositions[CurrentSettings->Language];
	
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
	ShowDialogue(Confirmation_Str[CurrentSettings->Language], subString0, subString1);
	
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Refresh_Gram();
	xSemaphoreGive(OLEDRelatedMutex);
	
	UI_Button_Init(&buttonParams);
	
	xQueueReceive(UI_ButtonMsg, &i, portMAX_DELAY);
	UI_Button_DeInit();
	
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
	IgnoreNextKeyEvent();
	//xQueueReceive(Key_Message, &keyMessage, portMAX_DELAY);
	if (i) return true;
	else return false;
}
