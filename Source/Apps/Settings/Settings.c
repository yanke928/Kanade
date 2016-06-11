//File Name   Settings.c
//Description Settings UI

#pragma diag_suppress 870

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>

#include "stm32f10x_rtc.h"
#include "stm32f10x_flash.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "SSD1306.h"
#include "RTC.h"

#include "UI_Menu.h"
#include "UI_Dialogue.h"
#include "UI_Adjust.h"
#include "MultiLanguageStrings.h"

#include "Settings.h"

Settings_Struct* CurrentSettings=(Settings_Struct*)0x0801d800;

Settings_Struct SettingsBkp;

const Settings_Struct DefaultSettings={0};

void SetLanguage(void);

void TimeSettings(void);

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
	 case 1:TimeSettings();break;
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
 menuParams.ItemString="English%日本語%日本语(华式)%台灣語";
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

u16 GetTimeParam(char *askString,char *unitString,u16 min,u16 max,u16 defaultValue)
{
 u16 tmp; 
 UI_Adjust_Param_Struct timeAdjustParams;
 timeAdjustParams.AskString=askString;
 timeAdjustParams.Min=min;
 timeAdjustParams.Max=max;
 timeAdjustParams.Step=1;
 timeAdjustParams.DefaultValue=defaultValue;
 timeAdjustParams.UnitString=unitString;
 timeAdjustParams.Pos_y=33;
 timeAdjustParams.FastSpeed=20;
 UI_Adjust_Init(&timeAdjustParams);
 xQueueReceive(UI_AdjustMsg, & tmp, portMAX_DELAY );
 OLED_Clear();
 return tmp;
}

void TimeSettings()
{
 struct Data_Time newTime;
 newTime.w_year=GetTimeParam((char*)SetYear_Str[CurrentSettings->Language],
	 (char*)SetYearUnit_Str[CurrentSettings->Language],
	 2016,2099,2016);
 newTime.w_month=GetTimeParam((char*)SetMonth_Str[CurrentSettings->Language],
	 (char*)SetMonthUnit_Str[CurrentSettings->Language],
	 1,12,1);
}

