//File Name   About.c
//Description About UI

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#include "FreeRTOS_Standard_Include.h"

#include "SSD1306.h"
#include "Music.h"
#include "Keys.h"

#include "UI_Utilities.h"
#include "UI_Dialogue.h"
#include "Settings.h"

#include "About.h"
#include "VersionInfo.h"

/**
  * @brief  About UI

	  @retval None
  */
void About()
{
 u8 productNameLength,productVersionLength;
 u8 prodectInfoAddr;
 u8 tmp;
 Key_Message_Struct keyMessage;
	
 /*Start music*/
 SoundStart(Only_My_Railgun);
	
 /*Prepare foe the display of product version and product name*/
 productNameLength=GetStringGraphicalLength(ProductName_Str[CurrentSettings->Language]);
 productVersionLength=GetStringGraphicalLength(ProductVersion_Str[CurrentSettings->Language]);
 prodectInfoAddr=63-(productNameLength+productVersionLength+1)*3;
	
 /*Draw dialogue*/
 ShowDialogue("","","",false,false);

 /*Display product info string and version string*/
 xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
 OLED_ShowAnyString(prodectInfoAddr,1,ProductName_Str[CurrentSettings->Language],OnSelect,12);
 OLED_ShowAnyString(prodectInfoAddr+(productNameLength+1)*6,1,ProductVersion_Str[CurrentSettings->Language],OnSelect,12);	
 
 /*Display designer,provider strings*/
 tmp=GetStringGraphicalLength(ProductDesigner_Str[CurrentSettings->Language]);
 tmp=GetCentralPosition(0,127,tmp);
 OLED_ShowAnyString(tmp,15,ProductDesigner_Str[CurrentSettings->Language],NotOnSelect,12);	
 DrawHorizonalDashedGrid(29,true,HighDensity);	
 tmp=GetStringGraphicalLength(ProductVendor_Str[CurrentSettings->Language]);
 tmp=GetCentralPosition(0,127,tmp);
 OLED_ShowAnyString(tmp,32,ProductVendor_Str[CurrentSettings->Language],NotOnSelect,12);		
 DrawHorizonalDashedGrid(46,true,HighDensity);	
 tmp=GetStringGraphicalLength(CustomString_Str[CurrentSettings->Language]);
 tmp=GetCentralPosition(0,127,tmp);
 OLED_ShowAnyString(tmp,49,CustomString_Str[CurrentSettings->Language],NotOnSelect,12);		
 xSemaphoreGive(OLEDRelatedMutex);
 
 /*Stop music when a key event takes place*/
 xQueueReceive(Key_Message, & keyMessage, portMAX_DELAY );
 SoundStop();
 
 /*Exit when middle key double clicked*/
 while(1)
	{
		xQueueReceive(Key_Message, & keyMessage, portMAX_DELAY );
		if(keyMessage.KeyEvent==MidDouble) break;
	}
 OLED_Clear();
}

