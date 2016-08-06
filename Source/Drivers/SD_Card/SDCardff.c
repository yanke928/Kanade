//File Name   SDCardff.c
//Description SDCard fatfs

#include <stdbool.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "ff.h"
#include "SSD1306.h"
#include "UI_Dialogue.h"
#include "SDCardff.h"
#include "Settings.h"
#include "MultiLanguageStrings.h"

#define SDCARD_PAP_SERVICE_PRIORITY tskIDLE_PRIORITY+3

FATFS SD_fatfs;

bool SDCardMountStatus = false;

bool SDCardUnMountedManually = false;


/**
  * @brief  SDCard plug-and-play service

	  @retval None
  */
void SDCardPlugAndPlay_Service(void *pvParameter)
{
	bool LstSDExist = SDCard_Exist();
	u32 capp;
	char tempString[30];
	for (;;)
	{
		if (LstSDExist != SDCard_Exist())
		{
			if (LstSDExist)
			{
				if (SDCardUnMountedManually)
				{
					SDCardUnMountedManually = false;
				}
				else
				{
					xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
					OLED_BackupScreen();
					xSemaphoreGive(OLEDRelatedMutex);
					ShowSmallDialogue(SDCardRemovedAccidentally_Str[CurrentSettings->Language], 1000, true);
					SDCardMountStatus = false;
					f_mount(NULL, "0:/", 1);
					xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
					OLED_RestoreScreen();
					xSemaphoreGive(OLEDRelatedMutex);
				}
			}
			else
			{
				vTaskDelay(200 / portTICK_RATE_MS);
				capp = SDCard_Init(false);
				xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
				OLED_BackupScreen();
				xSemaphoreGive(OLEDRelatedMutex);
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
					SDCardMountStatus = false;
				}
				xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
				OLED_RestoreScreen();
				xSemaphoreGive(OLEDRelatedMutex);
			}
			LstSDExist = !LstSDExist;
		}
		vTaskDelay(100 / portTICK_RATE_MS);
	}
}

/**
  * @brief  Init SDCard plug-and-play service

	  @retval None
  */
void SDCardPlugAndPlay_Service_Init(void)
{
	xTaskCreate(SDCardPlugAndPlay_Service, "SDCard_PAP_Service",
		128, NULL, SDCARD_PAP_SERVICE_PRIORITY, NULL);
}

