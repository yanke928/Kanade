//File Name   £ºSettings.c
//Description £ºSettings UI

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>

#include "stm32f10x_rtc.h"
#include "stm32f10x_flash.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "SSD1306.h"

#include "UI_Menu.h"
#include "UI_Dialogue.h"
#include "MultiLanguageStrings.h"

#include "Settings.h"

Settings_Struct* CurrentSettings=(Settings_Struct*)0x0801d800;

Settings_Struct SettingsBkp;

const Settings_Struct DefaultSettings={0};

void SetLanguage(void);

void Settings()
{
 UI_Menu_Param_Struct menuParams;
 u8 selection;
 menuParams.ItemString=(char *)Settings_Str[CurrentSettings->Language];
 menuParams.DefaultPos=0;
 menuParams.ItemNum=6;
 menuParams.FastSpeed=10;
 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
 OLED_Clear();
 xSemaphoreGive(OLEDRelatedMutex);
 UI_Menu_Init(&menuParams);
 xQueueReceive(UI_MenuMsg, & selection, portMAX_DELAY );
 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
 OLED_Clear();
 xSemaphoreGive(OLEDRelatedMutex);
 switch(selection)
 {
	 case 3:SetLanguage();
 }
}

void SaveSettings()
{
  int i=sizeof(Settings_Struct);
	u16 m=0;
	u16 *p=(u16 *)CurrentSettings;
	FLASH_Unlock();
	FLASH_SetLatency(FLASH_Latency_2);
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
	FLASH_ErasePage(FLASH_SETTINGS_BLOCK);
	do
	{
	 p = (u16*)(&SettingsBkp);
	 FLASH_ProgramHalfWord(m+FLASH_SETTINGS_ADDR, *p);
	 i-=2;
	 m+=2;
	 p+=2;
	}
	while(i>=0);
	FLASH_Lock();
}

void SetLanguage()
{
 UI_Menu_Param_Struct menuParams;
 u8 selection;
 menuParams.ItemString="English%Japanese%Japanese(CN)%Taiwanese";
 menuParams.DefaultPos=0;
 menuParams.ItemNum=4;
 menuParams.FastSpeed=5;
 memcpy(&SettingsBkp,CurrentSettings,sizeof(Settings_Struct));
 ShowSmallDialogue("Language",1000);
 UI_Menu_Init(&menuParams);
 xQueueReceive(UI_MenuMsg, & selection, portMAX_DELAY );
 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
 OLED_Clear();
 xSemaphoreGive(OLEDRelatedMutex); 
 switch(selection)
 {
	 case 0:SettingsBkp.Language=0;break;
	 case 1:SettingsBkp.Language=3;break;
	 case 2:SettingsBkp.Language=2;break;
	 case 3:SettingsBkp.Language=1;
 }
 SaveSettings();
 ShowSmallDialogue((char *)Saved_Str[CurrentSettings->Language],1000);	
}

bool CheckSettings()
{
 if(CurrentSettings->Language>LanguageNum-1) return false;
 return true;
}

void Settings_Init()
{
 if(!CheckSettings())
 {
  memcpy(&SettingsBkp,&DefaultSettings,sizeof(Settings_Struct));
	SaveSettings();
  ShowSmallDialogue("Settings Error",1000);	
	ShowSmallDialogue("Reset Done",1000);	
	xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
  OLED_Refresh_Gram();
  xSemaphoreGive(OLEDRelatedMutex); 
 }
}

