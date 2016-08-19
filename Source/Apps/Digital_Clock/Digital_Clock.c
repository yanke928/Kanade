//File Name   Digital_Clock.c
//Description Digital_Clock

#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "misc.h"

#include "SSD1306.h"
#include "Temperature_Sensors.h"
#include "RTC.h"
#include "Keys.h"

#include "FreeRTOS_Standard_Include.h"

#include "UI_Clock.h"

#include "Digital_Clock.h"

#define TEMPERATURE_UPDATE_RATE_SEC_PER_UPDATE 10

static void ShowTime(void);
static void ShowDate(void);
static void ShowTemperature(void);
static void Refresh_Gram(void);
static void Set_STOP_MODE_With_Second_Wake(void);

void Digital_Clock()
{
	u8 updateTempCnt = 0;
	OLED_Clear_With_Mutex_TakeGive();
	for (;;)
	{
		Time_Get();
		if (updateTempCnt == 0)
		{
			QuickGet_Enviroment_Temperature();
			updateTempCnt = TEMPERATURE_UPDATE_RATE_SEC_PER_UPDATE;
		}
		updateTempCnt--;
		ShowTime();
		ShowDate();
		ShowTemperature();
		Refresh_Gram();
		Set_STOP_MODE_With_Second_Wake();
		if (ANY_KEY_PRESSED)
		{
			IgnoreNextKeyEvent(); return;
		}
	}
}

void Set_STOP_MODE_With_Second_Wake(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	EXTI_ClearITPendingBit(EXTI_Line17);
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RTC_SetAlarm(RTC_GetCounter());
	RTC_WaitForLastTask();
	RTC_ITConfig(RTC_FLAG_ALR, ENABLE);
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
}

void RTCAlarm_IRQHandler(void)
{
	EXTI_ClearITPendingBit(EXTI_Line17);
	STM32_Init();
}

static void ShowTime()
{
	char timeString[20];
	GenerateRTCTimeString(timeString);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Show1624String(0, 0, timeString);
	xSemaphoreGive(OLEDRelatedMutex);
}

static void ShowDate()
{
	char dateString[20];
	GenerateRTCDateString(dateString);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Show1216String(4, 28, dateString);
	xSemaphoreGive(OLEDRelatedMutex);
}

static void ShowTemperature()
{
	char tempString[20];
	GenerateTempString(tempString, MOS);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Show1216String(4, 48, tempString + 1);
	xSemaphoreGive(OLEDRelatedMutex);
}

static void Refresh_Gram()
{
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Refresh_Gram();
	xSemaphoreGive(OLEDRelatedMutex);
}
