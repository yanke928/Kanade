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
#include "UI_Dialogue.h"
#include "Settings.h"

#include "About.h"
#include "VersionInfo.h"

void About()
{
 u8 productNameLength,productVersionLength;
 u8 prodectInfoAddr;
 u8 tmp;
 Key_Message_Struct keyMessage;
 SoundStart((SingleToneStruct*)ProductSound);
 productNameLength=GetStringLength(ProductName_Str[CurrentSettings->Language]);
 productVersionLength=GetStringLength(ProductVersion_Str[CurrentSettings->Language]);
 prodectInfoAddr=63-(productNameLength+productVersionLength+1)*3;
 ShowDialogue("","","");
 xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
 OLED_ShowAnyString(prodectInfoAddr,1,ProductName_Str[CurrentSettings->Language],NotOnSelect,12);
 OLED_ShowAnyString(prodectInfoAddr+(productNameLength+1)*6,1,ProductVersion_Str[CurrentSettings->Language],NotOnSelect,12);	
 OLED_InvertRect(1,1,126,13);
 tmp=GetStringLength(ProductDesigner_Str[CurrentSettings->Language]);
 tmp=GetCentralPosition(0,127,tmp);
 OLED_ShowAnyString(tmp,15,ProductDesigner_Str[CurrentSettings->Language],NotOnSelect,12);	
 DrawHorizonalDashedGrid(29,true,HighDensity);	
 tmp=GetStringLength(ProductVendor_Str[CurrentSettings->Language]);
 tmp=GetCentralPosition(0,127,tmp);
 OLED_ShowAnyString(tmp,32,ProductVendor_Str[CurrentSettings->Language],NotOnSelect,12);		
 DrawHorizonalDashedGrid(46,true,HighDensity);	
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

