//File Name   Digital_Clock.c
//Description Digital_Clock

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#include "SSD1306.h"
#include "Temperature_Sensors.h"
#include "RTC.h"
#include "Keys.h"

#include "FreeRTOS_Standard_Include.h"

#include "UI_Clock.h"

#include "Digital_Clock.h"

void ShowTime(void);
void ShowDate(void);

void Digital_Clock()
{
 u8 lstSec;
 OLED_Clear_With_Mutex_TakeGive();
 for(;;)
 {
  ShowTime();
  ShowDate();
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

void ShowTime()
{
 char timeString[20];
  GenerateRTCTimeString(timeString);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Show1624String(0,0,timeString);
	xSemaphoreGive(OLEDRelatedMutex);
}

void ShowDate()
{
char timeString[20];
  GenerateRTCDateString(timeString);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Show1216String(4,32,timeString);
	xSemaphoreGive(OLEDRelatedMutex);
}
