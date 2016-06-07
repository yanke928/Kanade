//File Name   £ºSettings.c
//Description £ºSettings UI

#include "stm32f10x.h"
#include <stdio.h>

#include "stm32f10x_rtc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "SSD1306.h"

#include "UI_Menu.h"

#include "settings.h"

void Settings()
{
 UI_Menu_Param_Struct menuParams;
 u8 selection;
 menuParams.ItemString="Unmount Disk%Clock Settings%Buzzer Settings%Language%System Scan%System Info";
 menuParams.DefaultPos=0;
 menuParams.ItemNum=6;
 menuParams.FastSpeed=5;
 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
 OLED_Clear();
 xSemaphoreGive(OLEDRelatedMutex);
 UI_Menu_Init(&menuParams);
 xQueueReceive(UI_MenuMsg, & selection, portMAX_DELAY );
 xSemaphoreTake( OLEDRelatedMutex, portMAX_DELAY );
 OLED_Clear();
 xSemaphoreGive(OLEDRelatedMutex);
}

