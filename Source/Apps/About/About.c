//File Name   About.c
//Description About UI

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "SSD1306.h"
#include "Music.h"
#include "Keys.h"

#include "UI_Utilities.h"
#include "Settings.h"

#include "About.h"
#include "VersionInfo.h"

void About()
{
 u8 productNameLength,productVersionLength;
 u8 prodectInfoAddr;
 Key_Message_Struct keyMessage;
 SoundStart((SingleToneStruct*)ProductSound);
 productNameLength=GetStringLength(ProductName_Str[CurrentSettings->Language]);
 productVersionLength=GetStringLength(ProductVersion_Str[CurrentSettings->Language]);
 prodectInfoAddr=63-(productNameLength+productVersionLength+1)*3;
 xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
 OLED_ShowAnyString(prodectInfoAddr,0,ProductName_Str[CurrentSettings->Language],NotOnSelect,12);
 OLED_ShowAnyString(prodectInfoAddr+(productNameLength+1)*6,0,ProductVersion_Str[CurrentSettings->Language],NotOnSelect,12);	
 OLED_InvertRect(0,0,127,11);
 xSemaphoreGive(OLEDRelatedMutex);
 xQueueReceive(Key_Message, & keyMessage, portMAX_DELAY );
 SoundStop();
 while(1)
	{
		xQueueReceive(Key_Message, & keyMessage, portMAX_DELAY );
		if(keyMessage.KeyEvent==MidDouble) break;
	}
 OLED_Clear();
}

