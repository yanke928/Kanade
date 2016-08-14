//File Name   Virtual	RTC.c
//Description Simulate a RTC Clock with SystemBeats    

#include <stdio.h>  

#include "stm32f10x.h"
#include "stm32f10x_rtc.h"
#include "misc.h"

#include "FreeRTOS_Standard_Include.h"

#include "VirtualRTC.h"

#define VIRTUAL_CLOCK_UPDATE_PRIORITY tskIDLE_PRIORITY+5

//see .h for details about variables

volatile RTCStructTypedef RTCCurrent;

volatile unsigned long SecondNum;

volatile unsigned long SecondNumNow;

unsigned long StartSecondNum;

unsigned long SecondBkp;

xTaskHandle VirtualRTCHandle = NULL;

/**
  * @brief   Update RTCCurrent struct according to SecondNum
  * @retval : None
  */
static void UpdateRTCStruct(void)
{
  static u8 lstSec=0;
	RTCCurrent.Sec = SecondNum % 60;
	RTCCurrent.Day = SecondNum / 86400;
	RTCCurrent.Hour = SecondNum / 3600 - RTCCurrent.Day * 24;
	RTCCurrent.Min = SecondNum / 60 - RTCCurrent.Hour * 60 - RTCCurrent.Day * 1440;
  if(RTCCurrent.SecDiv10+1<10) RTCCurrent.SecDiv10++;
  if(lstSec!=RTCCurrent.Sec) 
  {
   lstSec=RTCCurrent.Sec;
   RTCCurrent.SecDiv10=0;
  }
}

/**
* @brief   Write time to string in format dd hh:mm:ss
  * @retval : None
  */
void GenerateVirtualRTCString(char string[])
{
	sprintf(string, "%02d:%02d:%02d", RTCCurrent.Hour,
		RTCCurrent.Min, RTCCurrent.Sec);
}

/**
* @brief  This task update the virtual RTC

  * @retval : None
  */
static void VirtualRTC_Handler(void *pvParameters)
{
   
	while (1)
	{
		SecondNumNow = RTC_GetCounter();
		SecondNum = SecondNumNow - StartSecondNum;
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
	VirtualRTCHandle = NULL;
}


/**
  * @brief   Init virtualRTC
  * @retval : None
  */
void VirtualRTC_Init(void)
{
	StartSecondNum = RTC_GetCounter();
	if (VirtualRTCHandle != NULL)
		vTaskDelete(VirtualRTCHandle);
	xTaskCreate(VirtualRTC_Handler, "VirtualClock",
		128, NULL, VIRTUAL_CLOCK_UPDATE_PRIORITY, &VirtualRTCHandle);
}

/**
  * @brief   Pause virtual RTC
  * @retval : None
  */
void VirtualRTC_Pause(void)
{
	SecondBkp = SecondNum;
	if (VirtualRTCHandle != NULL)
		vTaskSuspend(VirtualRTCHandle);
}

/**
  * @brief   Resume virtual RTC
  * @retval : None
  */
void VirtualRTC_Resume(void)
{
	SecondNumNow = RTC_GetCounter();
	StartSecondNum = SecondNumNow - SecondBkp;
	if (VirtualRTCHandle != NULL)
		vTaskResume(VirtualRTCHandle);
}

