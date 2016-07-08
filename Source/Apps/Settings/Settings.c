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
#include "sdcard.h"
#include "sdcardff.h"

#include "UI_Menu.h"
#include "UI_Dialogue.h"
#include "UI_Adjust.h"
#include "UI_Utilities.h"
#include "MultiLanguageStrings.h"

#include "Settings.h"
#include "EBProtocolConfig.h"
#include "About.h"

Settings_Struct* CurrentSettings = (Settings_Struct*)0x0801c000;

Settings_Struct SettingsBkp;

const Settings_Struct DefaultSettings = { 0 , 2 };

void SetLanguage(void);

void TimeSettings(void);

void ModelSettings(void);

void MountOrUnMountDisk(void);

/**
  * @brief  Settings

  * @param  None

  */
void Settings()
{
	UI_Menu_Param_Struct menuParams;
	u8 selection;
	if (SDCardMountStatus)
		menuParams.ItemString = (char *)Settings_Str[CurrentSettings->Language];
	else
		menuParams.ItemString = (char *)SettingsNoDisk_Str[CurrentSettings->Language];
	menuParams.DefaultPos = 0;
	menuParams.ItemNum = 6;
	menuParams.FastSpeed = 10;
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);

	UI_Menu_Init(&menuParams);

	xQueueReceive(UI_MenuMsg, &selection, portMAX_DELAY);
	UI_Menu_DeInit();

	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
	switch (selection)
	{
	case 0:MountOrUnMountDisk(); break;
	case 1:TimeSettings(); break;
	case 3:SetLanguage(); break;
	case 4:ModelSettings(); break;
	case 5:About();
	}
}

/**
  * @brief  Mount or UnMount disk

  * @param  None

  */
void MountOrUnMountDisk()
{
	FRESULT res;
	u32 capp;
	char tempString[20];
	if (SDCardMountStatus)
	{
		res = f_mount(NULL, "0:/", 1);
		if (res == FR_OK)
		{
			ShowSmallDialogue(SettingsUnmounted_Str[CurrentSettings->Language], 1000, true);
			SDCardMountStatus = false;
		}
		else
		{
			ShowSmallDialogue(SettingsUnmountFailed_Str[CurrentSettings->Language], 1000, true);
		}
	}
	else
	{
		capp = sdcard_Init(false);
		if (capp)
		{
			sprintf(tempString, SettingsMounted_Str[CurrentSettings->Language], capp, true);
			CheckEBDDirectories(false);
			ShowSmallDialogue(tempString, 1000, true);
			SDCardMountStatus = true;
		}
		else
		{
			ShowSmallDialogue(SettingsMountFailed_Str[CurrentSettings->Language], 1000, true);
		}
	}
}

/**
  * @brief  Save settings

  * @param  None

  */
void SaveSettings()
{
	int i = sizeof(Settings_Struct);
	u16 m = 0;
	u16 *p = (u16 *)CurrentSettings;
	FLASH_Unlock();
	FLASH_SetLatency(FLASH_Latency_2);
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(FLASH_SETTINGS_BLOCK);
	do
	{
		p = (u16*)(&SettingsBkp);
		FLASH_ProgramHalfWord(m + FLASH_SETTINGS_ADDR, *p);
		i -= 2;
		m += 2;
		p += 2;
	} while (i >= 0);
	FLASH_Lock();
}

/**
  * @brief  Set language

  * @param  None

  */
void SetLanguage()
{
	UI_Menu_Param_Struct menuParams;
	u8 selection;
	menuParams.ItemString = "English%日本語%日本语(华式)%台灣語";
	menuParams.DefaultPos = 0;
	menuParams.ItemNum = 4;
	menuParams.FastSpeed = 5;
	memcpy(&SettingsBkp, CurrentSettings, sizeof(Settings_Struct));
	//ShowSmallDialogue("Language", 1000, true);

	UI_Menu_Init(&menuParams);

	xQueueReceive(UI_MenuMsg, &selection, portMAX_DELAY);
	UI_Menu_DeInit();

	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
	switch (selection)
	{
	case 0:SettingsBkp.Language = 0; break;
	case 1:SettingsBkp.Language = 3; break;
	case 2:SettingsBkp.Language = 2; break;
	case 3:SettingsBkp.Language = 1;
	}
	SaveSettings();
	ShowSmallDialogue((char *)Saved_Str[CurrentSettings->Language], 1000, true);
}

/**
  * @brief  Check settings

  * @param  None

  */
bool CheckSettings()
{
	if (CurrentSettings->Language > LanguageNum - 1) return false;
	if (CurrentSettings->EBD_Model > EBD_MODEL_NUM - 1) return false;
	return true;
}

/**
  * @brief  Init Settings(check if settings data is OK)

  * @param  None

  */
void Settings_Init()
{
	if (!CheckSettings())
	{
		memcpy(&SettingsBkp, &DefaultSettings, sizeof(Settings_Struct));
		SaveSettings();
		ShowSmallDialogue("Settings Error", 1000, true);
		ShowSmallDialogue("Reset Done", 1000, true);
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		OLED_Refresh_Gram();
		xSemaphoreGive(OLEDRelatedMutex);
		GetNecessarySettings();
	}
}

/**
  * @brief  Get a param from user to set the time

  * @param  None

  */
u16 GetTimeParam(const char *askString, const char *unitString, u16 min, u16 max, u16 defaultValue)
{
	u16 tmp;
	UI_Adjust_Param_Struct timeAdjustParams;
	timeAdjustParams.AskString = askString;
	timeAdjustParams.Min = min;
	timeAdjustParams.Max = max;
	timeAdjustParams.Step = 1;
	timeAdjustParams.DefaultValue = defaultValue;
	timeAdjustParams.UnitString = unitString;
	timeAdjustParams.Pos_y = 33;
	timeAdjustParams.FastSpeed = 20;
	UI_Adjust_Init(&timeAdjustParams);
	xQueueReceive(UI_AdjustMsg, &tmp, portMAX_DELAY);
	UI_Adjust_DeInit();
	vTaskDelay(20 / portTICK_RATE_MS);
	OLED_Clear();
	return tmp;
}

/**
  * @brief  Time settings

  * @param  None

  */
void TimeSettings()
{
	struct Data_Time newTime;
	newTime.w_year = GetTimeParam(SetYear_Str[CurrentSettings->Language],
		SetYearUnit_Str[CurrentSettings->Language],
		2016, 2099, RTCTime.w_year);
	newTime.w_month = GetTimeParam(SetMonth_Str[CurrentSettings->Language],
		SetMonthUnit_Str[CurrentSettings->Language],
		1, 12, RTCTime.w_month);
	if (newTime.w_year % 4 == 0 && newTime.w_month == 2)
	{
		newTime.w_date = GetTimeParam(SetDay_Str[CurrentSettings->Language],
			SetDayUnit_Str[CurrentSettings->Language],
			1, 29, RTCTime.w_date > 29 ? 29 : RTCTime.w_date);
	}
	else
		newTime.w_date = GetTimeParam(SetDay_Str[CurrentSettings->Language],
			SetDayUnit_Str[CurrentSettings->Language],
			1, mon_table[newTime.w_month - 1], RTCTime.w_date > mon_table[newTime.w_month - 1] ?
			mon_table[newTime.w_month - 1] : RTCTime.w_date);
	newTime.hour = GetTimeParam(SetHour_Str[CurrentSettings->Language],
		SetHourUnit_Str[CurrentSettings->Language],
		0, 23, RTCTime.hour);
	newTime.min = GetTimeParam(SetMin_Str[CurrentSettings->Language],
		SetMinUnit_Str[CurrentSettings->Language],
		0, 59, RTCTime.min);
	newTime.sec = GetTimeParam(SetSec_Str[CurrentSettings->Language],
		SetSecUnit_Str[CurrentSettings->Language],
		0, 59, RTCTime.sec);
	Time_Update(newTime.w_year, newTime.w_month, newTime.w_date,
		newTime.hour, newTime.min, newTime.sec);
	ShowSmallDialogue(TimeSetting_Str[CurrentSettings->Language], 1000, true);
}

void ModelSettings()
{
	u16 addr = 0;
	u8 i;
	UI_Menu_Param_Struct menuParams;
	u8 selection;
	char tempString[100];
	for (i = 0; i < EBD_MODEL_NUM; i++)
	{
		strcpy(tempString + addr, EBD_Protocol_Descriptors[i]);
		addr += GetStringLengthInBytes(EBD_Protocol_Descriptors[i]);
		if (i < EBD_MODEL_NUM - 1)
			tempString[addr - 1] = '%';
	}
	menuParams.ItemString = tempString;
	menuParams.DefaultPos = 0;
	menuParams.ItemNum = EBD_MODEL_NUM;
	menuParams.FastSpeed = 5;
	memcpy(&SettingsBkp, CurrentSettings, sizeof(Settings_Struct));

	UI_Menu_Init(&menuParams);

	xQueueReceive(UI_MenuMsg, &selection, portMAX_DELAY);
	UI_Menu_DeInit();
	SettingsBkp.EBD_Model = selection;
	SaveSettings();
	ShowSmallDialogue((char *)Saved_Str[CurrentSettings->Language], 1000, true);
}

/**
  * @brief  Get necessary settings when current settings is not right

  * @param  None

  */
void GetNecessarySettings()
{
	ShowSmallDialogue("Set Language", 1000, true);
	SetLanguage();
	ShowSmallDialogue(SetTimeHint_Str[CurrentSettings->Language], 1000, true);
	TimeSettings();
	ShowSmallDialogue(SetModelHint_Str[CurrentSettings->Language], 1000, true);
	ModelSettings();
	ShowSmallDialogue((char *)Saved_Str[CurrentSettings->Language], 1000, true);
}
