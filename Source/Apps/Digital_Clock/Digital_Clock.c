//File Name   Digital_Clock.c
//Description Digital_Clock

#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_exti.h"
#include "misc.h"

#include "SSD1306.h"
#include "Temperature_Sensors.h"
#include "RTC.h"
#include "Keys.h"
#include "ADC.h"

#include "FreeRTOS_Standard_Include.h"

#include "UI_Clock.h"
#include "UI_Adjust.h"
#include "UI_Confirmation.h"
#include "UI_Dialogue.h"
#include "UI_Menu.h"

#include "MultiLanguageStrings.h"

#include "Digital_Clock.h"

#include "Settings.h"

//Update rate of the temperature
#define TEMPERATURE_UPDATE_RATE_SEC_PER_UPDATE 10

static void ShowTime(void);
static void ShowDate(void);
static void ShowTemperature(void);
static void ShowWeek(void);
static void Refresh_Gram(void);
static void Set_STOP_MODE_With_Second_Wake(void);
static void Keys_WakeUp_Config(bool enable);

#define Keys_Wakeup_Init() Keys_WakeUp_Config(true)
#define Keys_Wakeup_DeInit() Keys_WakeUp_Config(false)

/**
  * @brief  Digital clock app

	  @retval None
  */
void Digital_Clock()
{
	u8 updateTempCnt = 0;
	OLED_Clear_With_Mutex_TakeGive();
	Keys_Wakeup_Init();
	for (;;)
	{
		Time_Get();
    //TakeAScreenShot();
		if (updateTempCnt == 0)
		{
			Restart_ADC_And_DMA();
			QuickGet_Enviroment_Temperature();
			updateTempCnt = TEMPERATURE_UPDATE_RATE_SEC_PER_UPDATE;
		}
		updateTempCnt--;
		ShowTime();
		ShowDate();
		ShowTemperature();
    ShowWeek();
		Refresh_Gram();
		Set_STOP_MODE_With_Second_Wake();
		STM32_Init();
		if (ANY_KEY_PRESSED)
		{
			Restart_ADC_And_DMA();
			Keys_Wakeup_DeInit();
			IgnoreNextKeyEvent();
			return;
		}
	}
}

/**
  * @brief  Set a wake alarm of 1 second and enter stop mode

	  @retval None
  */
static void Set_STOP_MODE_With_Second_Wake(void)
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

void Keys_WakeUp_Config(bool enable)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource4);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);

	/* Register EXTI Line3,4,5 to NVIC*/
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = enable ? ENABLE : DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure EXTI Line3,4,5 to generate an interrupt on falling edge */
	EXTI_ClearITPendingBit(EXTI_Line3);
	EXTI_InitStructure.EXTI_Line = EXTI_Line3;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = enable ? ENABLE : DISABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_ClearITPendingBit(EXTI_Line4);
	EXTI_InitStructure.EXTI_Line = EXTI_Line4;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_ClearITPendingBit(EXTI_Line5);
	EXTI_InitStructure.EXTI_Line = EXTI_Line5;
	EXTI_Init(&EXTI_InitStructure);
}

int Get_Idle_Time()
{
	int t;
	UI_Adjust_Param_Struct adjust_params;
retry:
	OLED_Clear_With_Mutex_TakeGive();
	adjust_params.AskString = IdleClockSettingsIdleClockTime_Str[CurrentSettings->Language];
	adjust_params.Min = 15;
	adjust_params.Max = 300;
	adjust_params.Step = 15;
	adjust_params.DefaultValue = SettingsBkp.Idle_Clock_Settings.IdleTime;
	adjust_params.UnitString = SetSecUnit_Str[CurrentSettings->Language];
	adjust_params.FastSpeed = 10;
	adjust_params.Pos_y = 33;
	UI_Adjust_Init(&adjust_params);
	xQueueReceive(UI_AdjustMsg, &t, portMAX_DELAY);
	UI_Adjust_DeInit();
	if (t < 15)
	{
		if (GetConfirmation(AbortConfirmation_Str[CurrentSettings->Language], ""))
		{
			return -1;
		}
		goto retry;
	}
	return t;
}

void Idle_Clock_Settings()
{
	UI_Menu_Param_Struct clock;
	u8 selection;
	const char *selections[2];
	int idleTime;
retry:
	if (CurrentSettings->Idle_Clock_Settings.ClockEnable)
		selections[0] = IdleClockSettingsDisableIdleClock_Str[CurrentSettings->Language];
	else
		selections[0] = IdleClockSettingsEnableIdleClock_Str[CurrentSettings->Language];
	selections[1] = IdleClockSettingsIdleClockTime_Str[CurrentSettings->Language];

	clock.ItemStrings = selections;
	clock.DefaultPos = 0;
	clock.ItemNum = 2;
	clock.FastSpeed = 5;

	UI_Menu_Init(&clock);

	memcpy(&SettingsBkp, CurrentSettings, sizeof(Settings_Struct));
	xQueueReceive(UI_MenuMsg, &selection, portMAX_DELAY);
	UI_Menu_DeInit();

	if (selection == 255)
	{
		goto done;
	}

	if (selection == 0)
	{
		if (CurrentSettings->Idle_Clock_Settings.ClockEnable == true)
		{
			SettingsBkp.Idle_Clock_Settings.ClockEnable = false;
			SaveSettings();
			ShowSmallDialogue(Disabled_Str[CurrentSettings->Language], 1000, true);
			goto done;
		}
		else
		{
			SettingsBkp.Idle_Clock_Settings.ClockEnable = true;
			SettingsBkp.Idle_Clock_Settings.IdleTime =
				SettingsBkp.Idle_Clock_Settings.IdleTime < 15 ? 15 : SettingsBkp.Idle_Clock_Settings.IdleTime;
			SettingsBkp.Idle_Clock_Settings.IdleTime =
				SettingsBkp.Idle_Clock_Settings.IdleTime > 300 ? 300 : SettingsBkp.Idle_Clock_Settings.IdleTime;
			SaveSettings();
			ShowSmallDialogue(Enabled_Str[CurrentSettings->Language], 1000, true);
			goto retry;
		}
	}
	else
	{
		if (CurrentSettings->Idle_Clock_Settings.ClockEnable == true)
		{
			idleTime = Get_Idle_Time();
			if (idleTime == -1) goto done;
			else
			{
				SettingsBkp.Idle_Clock_Settings.IdleTime = idleTime;
				SaveSettings();
				ShowSmallDialogue(Saved_Str[CurrentSettings->Language], 1000, true);
				goto done;
			}
		}
		else
		{
			ShowSmallDialogue(IdleClockSettingsPleaseEnable_Str_Str[CurrentSettings->Language], 1000, true);
			goto retry;
		}
	}
done:
	OLED_Clear();
}

/**
  * @brief  Keys wakeup ISR

	  @retval None
  */
void EXTI3_IRQHandler()
{
	EXTI_ClearITPendingBit(EXTI_Line3);
}

/**
  * @brief  Keys wakeup ISR

	  @retval None
  */
void EXTI4_IRQHandler()
{
	EXTI_ClearITPendingBit(EXTI_Line4);
}

/**
  * @brief  Keys wakeup ISR

	  @retval None
  */
void EXTI9_5_IRQHandler()
{
	EXTI_ClearITPendingBit(EXTI_Line5);
}

/**
  * @brief  RTCAlarm ISR

	  @retval None
  */
void RTCAlarm_IRQHandler(void)
{
	EXTI_ClearITPendingBit(EXTI_Line17);
}

/**
  * @brief  Show clock time

	  @retval None
  */
static void ShowTime()
{
	char timeString[20];
	GenerateRTCTimeString(timeString);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Show1624String(0, 0, timeString);
	xSemaphoreGive(OLEDRelatedMutex);
}

/**
  * @brief  Show clock date

	  @retval None
  */
static void ShowDate()
{
	char dateString[20];
	GenerateRTCDateString(dateString);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Show1216String(4, 28, dateString);
	xSemaphoreGive(OLEDRelatedMutex);
}


/**
  * @brief  Show clock temperature

	  @retval None
  */
static void ShowTemperature()
{
	char tempString[20];
	GenerateTempString(tempString, MOS);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Show1216String(64, 48, tempString + 1);
	xSemaphoreGive(OLEDRelatedMutex);
}

/**
  * @brief  Show clock week

	  @retval None
  */
static void ShowWeek()
{
	char tempString[20];
	GenerateRTCWeekString(tempString);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Show1216String(4, 48, tempString);
	xSemaphoreGive(OLEDRelatedMutex);
}

/**
  * @brief  Refresh GRAM

	  @retval None
  */
static void Refresh_Gram()
{
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Refresh_Gram();
	xSemaphoreGive(OLEDRelatedMutex);
}
