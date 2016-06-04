//File Name   £ºVirtual	RTC.c
//Description £ºSimulate a RTC Clock with SystemBeats      

#include "misc.h"
#include "VirtualRTC.h"
#include "stm32f10x.h"
#include <stdio.h>

#include "stm32f10x_rtc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define VIRTUAL_CLOCK_UPDATE_PRIORITY tskIDLE_PRIORITY+2

//see .h for details about variables

volatile RTCStructTypedef RTCCurrent;

volatile unsigned long SecondNum;

volatile unsigned long SecondNumNow;

unsigned long StartSecondNum;

xTaskHandle VirtualRTCHandle;

/**
  * @brief   Update RTCCurrent struct according to SecondNum
  * @retval : None
  */
void UpdateRTCStruct(void)
{
	RTCCurrent.Sec = SecondNum % 60;
	RTCCurrent.Day = SecondNum / 86400;
	RTCCurrent.Hour = SecondNum / 3600 - RTCCurrent.Day * 24;
	RTCCurrent.Min = SecondNum / 60 - RTCCurrent.Hour * 60 - RTCCurrent.Day * 1440;
}

/**
* @brief   Write time to string in format dd hh:mm:ss
  * @retval : None
  */
void GenerateVirtualRTCString(unsigned char string[])
{
	sprintf((char *)string, "%02d:%02d:%02d", RTCCurrent.Hour,
		RTCCurrent.Min, RTCCurrent.Sec);
}

/**
* @brief  This task update the virtual RTC

  * @retval : None
  */
void VirtualRTC_Handler(void *pvParameters)
{
 while(1)
 {
  SecondNumNow=RTC_GetCounter();
	SecondNum=SecondNumNow-StartSecondNum;
	UpdateRTCStruct();
	vTaskDelay(100 / portTICK_RATE_MS);
 }
}

/**
* @brief   DeInit virtualRTC
  * @retval : None
  */
void VirtualRTC_DeInit(void)
{
 vTaskDelete(VirtualRTCHandle);
}


/**
  * @brief   Init virtualRTC
  * @retval : None
  */
void VirtualRTC_Init(void)
{
  StartSecondNum = RTC_GetCounter(); 
	if(VirtualRTCHandle!=NULL)
  vTaskDelete(VirtualRTCHandle);
  xTaskCreate(VirtualRTC_Handler,"VirtualClock",
	32,NULL,VIRTUAL_CLOCK_UPDATE_PRIORITY,&VirtualRTCHandle);
}
