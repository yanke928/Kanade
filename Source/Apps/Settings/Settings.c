//File Name   Settings.c
//Description Settings UI

#pragma diag_suppress 870

#include <stdio.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_flash.h"

#include "FreeRTOS_Standard_Include.h"

#include "SSD1306.h"
#include "RTC.h"
#include "sdcard.h"
#include "sdcardff.h"
#include "W25Q64.h"
#include "W25Q64ff.h"
#include "ff.h"

#include "UI_Menu.h"
#include "UI_Dialogue.h"
#include "UI_Adjust.h"
#include "UI_Utilities.h"
#include "UI_Confirmation.h"
#include "MultiLanguageStrings.h"

#include "About.h"
#include "SelfTest.h"

#include "Settings.h"

Settings_Struct* CurrentSettings = (Settings_Struct*)0x0803b800;

Settings_Struct SettingsBkp;

const Settings_Struct DefaultSettings = { 0 , 2 ,75 ,120 ,5};

typedef  void(*pFunction)(void);

uint32_t JumpAddress;
pFunction Jump_To_Application;

void SetLanguage(void);

void TimeSettings(void);

void OverHeatSettings(void);

void MountOrUnMountDisk(void);

void FormatDisks(void);

void FirmwareUpdate(void);

/**
  * @brief  Settings

  * @param  None

  */
void Settings()
{
	UI_Menu_Param_Struct menuParams;
	u8 selection;
	const char* stringTab[10];
	
	if (SDCardMountStatus)
  stringTab[0]=SettingsItemUnmountDisk_Str[CurrentSettings->Language];
	else
	stringTab[0]=SettingsItemMountDisk_Str[CurrentSettings->Language];
	
	stringTab[1]=SettingsItemClockSettings_Str[CurrentSettings->Language];
	stringTab[2]=SettingsItemOverHeatControl_Str[CurrentSettings->Language];
	stringTab[3]=SettingsItemLanguage_Str[CurrentSettings->Language];
	stringTab[4]=SettingsItemFormatDisks_Str[CurrentSettings->Language];
	stringTab[5]=SettingsItemFirmwareUpdate_Str[CurrentSettings->Language];
	stringTab[6]=SettingsItemSystemInfo_Str[CurrentSettings->Language];
	stringTab[7]=SettingsItemSystemScan_Str[CurrentSettings->Language];
	
	menuParams.ItemStrings=stringTab;
	menuParams.DefaultPos = 0;
	menuParams.ItemNum = 8;
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
	case 2:OverHeatSettings(); break;
	case 3:SetLanguage(); break;
	case 4:FormatDisks();break;
	case 5:FirmwareUpdate();break;
	case 6:About();break;
	case 7:SelfTest();break;
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
			SDCardUnMountedManually = true;
			SDCardMountStatus = false;
		}
		else
		{
			ShowSmallDialogue(SettingsUnmountFailed_Str[CurrentSettings->Language], 1000, true);
		}
	}
	else
	{
		capp = SDCard_Init(false);

		if (capp)
		{
			sprintf(tempString, SettingsMounted_Str[CurrentSettings->Language], capp);
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
	p = (u16*)(&SettingsBkp);
	do
	{
		FLASH_ProgramHalfWord(m + FLASH_SETTINGS_ADDR, *p);
		i -= 2;
		m += 2;
		p++;
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
	const char *languageTab[4];
	
	languageTab[0]="English";
	languageTab[1]="日本語";
	languageTab[2]="日本语(华式)";
	languageTab[3]="台灣語";	
	
	menuParams.ItemStrings =languageTab;
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
	if (CurrentSettings->InternalTemperature_Max > 130||
		  CurrentSettings->InternalTemperature_Max < 55) return false;

	if (CurrentSettings->ExternalTemperature_Max > 200||
		  CurrentSettings->ExternalTemperature_Max < 40) return false;
	
	if (CurrentSettings->Protection_Resume_Gap > 20||
		  CurrentSettings->Protection_Resume_Gap < 5) return false;
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

/**
  * @brief  OverHeat settings

  * @param  None

  */
void OverHeatSettings(void)
{
 memcpy(&SettingsBkp, CurrentSettings, sizeof(Settings_Struct));
 SettingsBkp.InternalTemperature_Max = GetTimeParam(SetInternalTemp_Max_Str[CurrentSettings->Language],
		Temperature_Unit_Str[CurrentSettings->Language],
		55, 130, CurrentSettings->InternalTemperature_Max);
 SettingsBkp.ExternalTemperature_Max = GetTimeParam(SetExternalTemp_Max_Str[CurrentSettings->Language],
		Temperature_Unit_Str[CurrentSettings->Language],
		40, 200, CurrentSettings->ExternalTemperature_Max);
 SettingsBkp.Protection_Resume_Gap = GetTimeParam(SetTempProtectResumeGap_Str[CurrentSettings->Language],
		Temperature_Unit_Str[CurrentSettings->Language],
		5, 20, CurrentSettings->Protection_Resume_Gap);
	SaveSettings();
	ShowSmallDialogue(Saved_Str[CurrentSettings->Language], 1000, true);
}

/**
  * @brief  FormatDisk

  * @param  None

  */
void FormatDisks(void)
{
 UI_Menu_Param_Struct menuParams;
 const char* diskTab[2];
 FRESULT res;
	
 u8 selection;
 u8 cnt=0;
 if(1)
 {
  diskTab[cnt]=SettingsItemFormatInternal_Str[CurrentSettings->Language];
	cnt++;
 }
 if(1)
 {
  diskTab[cnt]=SettingsItemFormatSD_Str[CurrentSettings->Language];
	cnt++;		
 }
 if(cnt==0)
 {
  ShowSmallDialogue(SettingsItemNoDisk_Str[CurrentSettings->Language], 1000, true);
	return;
 }
	menuParams.ItemStrings = diskTab;
	menuParams.DefaultPos = 0;
	menuParams.ItemNum = cnt;
	menuParams.FastSpeed = 5;
 
	UI_Menu_Init(&menuParams);
 	xQueueReceive(UI_MenuMsg, &selection, portMAX_DELAY);
	UI_Menu_DeInit();
  if(selection==255) goto Done;
  if(diskTab[selection]==SettingsItemFormatInternal_Str[CurrentSettings->Language])
	{
	 if (GetConfirmation(FormatInternalConfirm_Str[CurrentSettings->Language], ""))
	 {
	  ShowSmallDialogue(Formatting_Str[CurrentSettings->Language], 1000, false);
		//res=f_mkfs("1:/",1,4096,SPI_FLASH_fatfs.win,_MAX_SS);
		 res=f_mkfs("1:/",1,4096);
	 }
	 else goto Done;
	}
	else if(diskTab[selection]==SettingsItemFormatSD_Str[CurrentSettings->Language])
	{
	 if (GetConfirmation(FormatSDConfirm_Str[CurrentSettings->Language], ""))
	 {
	  ShowSmallDialogue(Formatting_Str[CurrentSettings->Language], 1000, false);
	  //res=f_mkfs("0:/",1,512,SD_fatfs.win,_MAX_SS);
		 res=f_mkfs("0:/",0,0);
	 }
	 else goto Done;	 
	}
 if(res==FR_OK)  ShowSmallDialogue(FormatSuccess_Str[CurrentSettings->Language], 1000, true);
 else ShowDiskIOStatus(res);
	Done:
	OLED_Clear();
}

/**
  * @brief  Set firmware update flag

  * @param  None

  */
void SetFirmwareUpdateFlag()
{
 BKP_WriteBackupRegister(BKP_DR2, 0x0001);	
}

__asm void SystemReset(void)
{
 MOV R0, #1           //; 
 MSR FAULTMASK, R0    //; 清除FAULTMASK 禁止一切中断产生
 LDR R0, =0xE000ED0C  //;
 LDR R1, =0x05FA0004  //; 
 STR R1, [R0]         //; 系统软件复位   
 
deadloop
    B deadloop        //; 死循环使程序运行不到下面的代码
}

void Jump2Addr(u32 addr)
{
	__set_PRIMASK(1);
	if (((*(__IO uint32_t *) addr) & 0x2FFE0000) == 0x20000000)
	{
		JumpAddress = *(__IO uint32_t *) (addr + 4);
		Jump_To_Application = (pFunction)JumpAddress;

		__set_MSP(*(__IO uint32_t *) addr);
		Jump_To_Application();
	}
	return;
}


/**
  * @brief  Set firmware update flag and jump to IAP

  * @param  None

  */
void FirmwareUpdate()
{
 if (GetConfirmation(FirmwareUpdateConfirm_Str[CurrentSettings->Language], ""))
 {
	portENTER_CRITICAL();
  SetFirmwareUpdateFlag();
  SystemReset();
 }
 OLED_Clear();
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
	ShowSmallDialogue((char *)Saved_Str[CurrentSettings->Language], 1000, true);
}

