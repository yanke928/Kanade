//File Name   Digital_Clock.c
//Description Digital_Clock

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#include "SSD1306.h"
#include "RTC.h"
#include "Keys.h"

#include "FreeRTOS_Standard_Include.h"

#include "UI_Clock.h"

#include "Digital_Clock.h"

void Digital_Clock()
{
 char timeString[20];
 u8 lstSec;
 xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
 OLED_Clear();
 xSemaphoreGive(OLEDRelatedMutex); 
 for(;;)
 {
  GenerateRTCTimeString(timeString);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Show1624String(0,0,timeString);
	xSemaphoreGive(OLEDRelatedMutex);
  GenerateRTCDateString(timeString);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Show1216String(4,32,timeString);
	xSemaphoreGive(OLEDRelatedMutex);
  lstSec=RTCTime.sec;
  for(;;)
  {
   if(RTCTime.sec!=lstSec) break;
   if(ANY_KEY_PRESSED)
    {
     IgnoreNextKeyEvent();return;
    }
   vTaskDelay(20/portTICK_RATE_MS);
  }
 }
}
