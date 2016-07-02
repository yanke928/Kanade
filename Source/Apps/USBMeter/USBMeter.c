//File Name   USBMeterUI.c
//Description Main UI

#include "stm32f10x.h"
#include "stm32f10x_it.h"

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "EBProtocol.h"
#include "TempSensors.h"
#include "SSD1306.h"
#include "RTC.h"
#include "VirtualRTC.h"
#include "UI_Confirmation.h"
#include "MultiLanguageStrings.h"
#include "Keys.h"
#include "sdcard.h"
#include "sdcardff.h"

#include "mass_mal.h"

#include "Settings.h"
#include "StepUpTest.h"
#include "FastCharge_Trigger.h"
#include "LegacyTest.h"
#include "MassStorage.h"

#include "UI_Dialogue.h"

#include "USBMeter.h"

#define USB_METER_PRIORITY tskIDLE_PRIORITY+3

/**
  * @brief  USBMeter

  * @retval None
  */
void USBMeter(void *pvParameters)
{
	char tempString[20];
	Key_Message_Struct keyMessage;
	u8 status = *(u8*)pvParameters;
	Legacy_Test_Param_Struct legacy_Test_Params;
	ClearKeyEvent(keyMessage);
	while (1)
	{
	Refresh:
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		DisplayBasicData(tempString, status);
		if (status == USBMETER_RECORD || status == LEGACY_TEST)
		{
			DisplayRecordData(tempString);
		}
		xSemaphoreGive(OLEDRelatedMutex);
		if (status == LEGACY_TEST)
		{
			if (CurrentMeterData.Voltage * 1000 < legacy_Test_Params.ProtectVolt ||
				CurrentMeterData.Voltage < 0.5)
			{
				StopRecord(&status, 1);
				goto Refresh;
			}
		}
		if (xQueueReceive(Key_Message, &keyMessage, 1000 / portTICK_RATE_MS) == pdPASS)
		{
			if (status == USBMETER_ONLY)
			{
				switch (keyMessage.KeyEvent)
				{
				case MidDouble:if (GetConfirmation(RecordConfirm_Str[CurrentSettings->Language], ""))
					StartRecord(&status); break;
				case MidLong:Settings(); break;
				case LeftClick:if (GetConfirmation(StepUpConfirm_Str[CurrentSettings->Language], ""))
					RunAStepUpTest(); break;
				case RightClick:if (GetConfirmation(LegacyTestConfirm_Str[CurrentSettings->Language], ""))
					RunLegacyTest(&status, &legacy_Test_Params); break;
				}
				switch (keyMessage.AdvancedKeyEvent)
				{
				case LeftContinous:if (GetConfirmation(QCMTKConfirm_Str[CurrentSettings->Language], ""))
					FastChargeTriggerUI(); break;
				case RightContinous:if
					(GetConfirmation(MountUSBMassStorageConfirm_Str[CurrentSettings->Language], ""))
					MassStorage_App(); break;
				}
				goto Refresh;
			}
			else
			{
				xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
				OLED_Clear();
				xSemaphoreGive(OLEDRelatedMutex);
				if (keyMessage.KeyEvent == MidDouble)
				{
					if (GetConfirmation(RecordStopConfirm_Str[CurrentSettings->Language], ""))
					{
						StopRecord(&status, 0);
						goto Refresh;
					}
				}
				else
				{
					ShowDialogue(Hint_Str[CurrentSettings->Language],
						RecordIsRunningHint1_Str[CurrentSettings->Language],
						RecordIsRunningHint2_Str[CurrentSettings->Language]);
					vTaskDelay(1000 / portTICK_RATE_MS);
					xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
					OLED_Clear();
					xSemaphoreGive(OLEDRelatedMutex);
				}
				goto Refresh;
			}
		}
	}
}

/**
  * @brief  Display Volt,Cureent,Power and Temperature
	on the screen
  * @param  a tempString which includes at lease 7 elements
	By sharing the tempString,more sources can be saved
	@retval None
  */
void DisplayBasicData(char tempString[], u8 currentStatus)
{
	if (CurrentMeterData.Voltage >= 10)
	{
		sprintf(tempString, "%5.2fV", CurrentMeterData.Voltage);
	}
	else
	{
		sprintf(tempString, "%5.3fV", CurrentMeterData.Voltage);
	}
	OLED_ShowString(0, 0, tempString);
	if (CurrentMeterData.Current >= 0.1)
		sprintf(tempString, "%5.3fA", CurrentMeterData.Current);
	else
		sprintf(tempString, "%04.1fmA", CurrentMeterData.Current * 1000);
	OLED_ShowString(80, 0, tempString);
	if (CurrentMeterData.Power >= 10)
	{
		sprintf(tempString, "%5.2fW", CurrentMeterData.Power);
	}
	else
	{
		sprintf(tempString, "%5.3fW", CurrentMeterData.Power);
	}
	OLED_ShowString(0, 16, tempString);
	if (CurrentTemperatureSensor == Internal)
	{
		GenerateTempString(tempString, Internal);
		if (tempString[0] != '1')tempString[0] = '0';
		OLED_ShowString(80, 16, tempString);
	}
	else
	{
		GenerateTempString(tempString, External);
		OLED_ShowAnyString(80, 16, "Ex", NotOnSelect, 8);
		OLED_ShowAnyString(92, 16, tempString, NotOnSelect, 8);
		GenerateTempString(tempString, Internal);
		OLED_ShowAnyString(80, 24, "In", NotOnSelect, 8);
		OLED_ShowAnyString(92, 24, tempString, NotOnSelect, 8);
	}
	if (currentStatus == USBMETER_ONLY)
	{
		GenerateRTCDateString(tempString);
		OLED_ShowString(0, 32, tempString);
		GenerateRTCTimeString(tempString);
		OLED_ShowString(0, 48, tempString);
		GenerateRTCWeekString(tempString);
		OLED_ShowString(104, 32, tempString);
		sprintf(tempString, "%5.2fV", CurrentMeterData.VoltageDP);
		OLED_ShowAnyString(92, 48, tempString, NotOnSelect, 8);
		sprintf(tempString, "%5.2fV", CurrentMeterData.VoltageDM);
		OLED_ShowAnyString(92, 56, tempString, NotOnSelect, 8);
		OLED_ShowAnyString(80, 48, "D+", NotOnSelect, 8);
		OLED_ShowAnyString(80, 56, "D-", NotOnSelect, 8);
	}
}

/**
  * @brief  Display Capacity,Work,LastingTime
  * @param  a tempString which includes at lease 9 elements
	By sharing the tempString,more sources can be saved
	@retval None
  */
void DisplayRecordData(char tempString[])
{
	if (CurrentSumUpData.Capacity >= 10000)
		sprintf(tempString, "%05.2fAh", CurrentSumUpData.Capacity);
	else
		sprintf(tempString, "%05.0fmAh", CurrentSumUpData.Capacity * 1000);
	OLED_ShowString(0, 32, tempString);
	if (CurrentSumUpData.Work >= 10000)
		sprintf(tempString, "%05.2fWh", CurrentSumUpData.Work);
	else
		sprintf(tempString, "%05.0fmWh", CurrentSumUpData.Work * 1000);
	OLED_ShowString(0, 48, tempString);
	GenerateVirtualRTCString(tempString);
	OLED_ShowAnyString(82, 51, tempString, NotOnSelect, 12);
	sprintf(tempString, "%d Day(s)", RTCCurrent.Day);
	OLED_ShowAnyString(82, 35, tempString, NotOnSelect, 12);
}

void USBMeter_Init(u8 status)
{
	CreateTaskWithExceptionControl(USBMeter, "USBMeter",
		384, &status, USB_METER_PRIORITY, NULL);
	vTaskDelay(50 / portTICK_RATE_MS);
}
