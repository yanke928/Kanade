//File Name   LegacyTest.c
//Description LegacyTest

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "SSD1306.h"
#include "EBProtocol.h"
#include "sdcard.h"
#include "sdcardff.h"
#include "LED.h"
#include "Keys.h"
#include "TempSensors.h"

#include "UI_Adjust.h"
#include "UI_Dialogue.h"
#include "UI_Confirmation.h"
#include "UI_Utilities.h"

#include "MultiLanguageStrings.h"

#include "Settings.h"

#include "StepUpTest.h"
#include "USBMeter.h"
#include "RTC.h"
#include "StartUp.h"
#include "VirtualRTC.h"
#include "Music.h"

#include "LegacyTest.h"

#define RECORD_HANDLER_PRIORITY tskIDLE_PRIORITY+7
#define LEGACY_TEST_CURRENT_MAX 5000

FIL RecordFile;

u32 RecordFileWriteCount;

bool IsRecording;

xTaskHandle RecordHandle;

void Record_Handler(void *pvParameters)
{
	u8 lastSec = RTCCurrent.Sec;
	u8 stringLength;
	char tempString[50];
	SecondNum=0;
	for (;;)
	{
		if (IsRecording)
		{
			if (CurrentTemperatureSensor == Internal)
				sprintf(tempString, "%ld,%6.3f,%6.3f\r\n", SecondNum, CurrentMeterData.Current,
					CurrentMeterData.Voltage);
			else
				sprintf(tempString, "%ld,%6.3f,%6.3f,%5.1f\r\n", SecondNum, CurrentMeterData.Current,
					CurrentMeterData.Voltage, ExternalTemperature);
			stringLength = GetStringLength(tempString);
			f_write(&RecordFile, tempString, stringLength, &RecordFileWriteCount);
		}
		CurrentSumUpData.Capacity += (CurrentMeterData.Current / 3600);
		CurrentSumUpData.Work += (CurrentMeterData.Power / 3600);
		CurrentSumUpData.PlatformVolt = CurrentMeterData.Power / CurrentMeterData.Current;
		for (;;)
		{
			if (lastSec != RTCCurrent.Sec)
			{
				lastSec = RTCCurrent.Sec;
				break;
			}
			vTaskDelay(50 / portTICK_RATE_MS);
		}
	}
}

void ClearRecordData()
{
	CurrentSumUpData.Capacity = 0;
	CurrentSumUpData.Work = 0;
	CurrentSumUpData.PlatformVolt = 0;
}

bool CreateRecordFile()
{
	FRESULT res;
	char tempString[60];
	sprintf(tempString, "0:/EBD/Records/%04d_%02d_%02d %02d-%02d-%02d.csv", RTCTime.w_year,
		RTCTime.w_month, RTCTime.w_date, RTCTime.hour, RTCTime.min, RTCTime.sec);
	res = f_open(&RecordFile, tempString, FA_CREATE_ALWAYS | FA_WRITE);
	if (res == FR_OK) return true;
	return false;
}

void CreateRecordFileHeader()
{
	f_write(&RecordFile, "Time(s),Curt(A),Volt(V),Temp(C)\r\n",
		sizeof("Time(s),Curt(A),Volt(V),Temp(C)\r\n"), &RecordFileWriteCount);
}

void RunLegacyTest(u8* status, Legacy_Test_Param_Struct* test_Params)
{
	xTaskHandle logoHandle;
	xTaskHandle initStatusUpdateHandle;
	bool protectedFlag;
	*status = LEGACY_TEST;
	test_Params->Current =
		GetTestParam(LegacyTestSetCurrent_Str[CurrentSettings->Language], 100, LEGACY_TEST_CURRENT_MAX,
			1000, 100, "mA", 20);
	test_Params->ProtectVolt =
		GetTestParam(ProtVoltageGet_Str[CurrentSettings->Language], 0,
		(int)(1000 * CurrentMeterData.Voltage) / 10 * 10 > 0 ? (1000 * CurrentMeterData.Voltage) / 10 * 10 : 100,
			(int)(900 * CurrentMeterData.Voltage) / 10 * 10 > 0 ? (900 * CurrentMeterData.Voltage) / 10 * 10 : 100, 10, "mV", 25);
	if (!SDCardMountStatus)
	{
		IsRecording = false;
		if (!GetConfirmation(NoDiskConfirm_Str[CurrentSettings->Language], ""))
		{
			*status = USBMETER_ONLY;
			return;
		}
	}
	else
	{

		if (!CreateRecordFile())
		{
			ShowSmallDialogue(FileCreateFailed_Str[CurrentSettings->Language], 1000, true);
		}
		else
		{
			RecordFileWriteCount = 0;
			CreateRecordFileHeader();
			ShowSmallDialogue(FileCreated_Str[CurrentSettings->Language], 1000, true);
			IsRecording = true;
		}
	}
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	OLED_Refresh_Gram();
	xSemaphoreGive(OLEDRelatedMutex);
	logoHandle = Logo_Init();
	initStatusUpdateHandle = InitStatusHandler_Init();
	xQueueSend(InitStatusMsg, PleaseWait_Str[CurrentSettings->Language], 0);
	EBD_Sync();
	vTaskDelay(200);
	EBDSendLoadCommand(test_Params->Current, StartTest);
	for (;;)
	{
		EBD_Sync();
		if (CurrentMeterData.Voltage < (float)(test_Params->ProtectVolt) / 1000||
			CurrentMeterData.Voltage<0.5)
		{
			protectedFlag = true;
			vTaskDelay(200 / portTICK_RATE_MS);
			EBDSendLoadCommand(0, StopTest);
			break;
		}
		if (CurrentMeterData.Current > (float)(test_Params->Current) / 1010 &&
			CurrentMeterData.Current < (float)(test_Params->Current) / 990)
		{
			protectedFlag = false;
			break;
		}
		vTaskDelay(200 / portTICK_RATE_MS);
		EBDSendLoadCommand(test_Params->Current, StartTest);
	}
	if (logoHandle != NULL)
		vTaskDelete(logoHandle);
	if (initStatusUpdateHandle != NULL)
		vTaskDelete(initStatusUpdateHandle);
	UpdateOLEDJustNow=false;
	xSemaphoreGive(OLEDRelatedMutex);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	OLED_Refresh_Gram();
	xSemaphoreGive(OLEDRelatedMutex);
	if (protectedFlag == false)
	{
		ClearRecordData();
		VirtualRTC_Init();
		xTaskCreate(Record_Handler, "Record Handler",
			256, NULL, RECORD_HANDLER_PRIORITY, &RecordHandle);
		ShowSmallDialogue(LegacyTestStarted_Str[CurrentSettings->Language], 1000, true);
	}
	else
	{
		ShowSmallDialogue(LegacyTestVoltLow_Str[CurrentSettings->Language], 1000, true);
		if(IsRecording) f_close(&RecordFile);
		*status = USBMETER_ONLY;
	}
}

void StartRecord(u8* status)
{
	if (!SDCardMountStatus)
	{
		IsRecording = false;
		if (!GetConfirmation(NoDiskConfirm_Str[CurrentSettings->Language], ""))
		{
			*status = USBMETER_ONLY;
			return;
		}
	}
	else
	{
		if (!CreateRecordFile())
		{
			ShowSmallDialogue(FileCreateFailed_Str[CurrentSettings->Language], 1000, true);
		}
		else
		{
			RecordFileWriteCount = 0;
			CreateRecordFileHeader();
			ShowSmallDialogue(FileCreated_Str[CurrentSettings->Language], 1000, true);
			IsRecording = true;
		}
	}
	ClearRecordData();
	VirtualRTC_Init();
	xTaskCreate(Record_Handler, "Record Handler",
		256, NULL, RECORD_HANDLER_PRIORITY, &RecordHandle);
	ShowSmallDialogue(RecordStarted_Str[CurrentSettings->Language], 1000, true);
	*status = USBMETER_RECORD;
}

const LEDAnimateSliceStruct LEDSummaryAnimate[] =
{
 {White,50,255,false},
 {White,50,0,false},
 {0,0,0,END}//End of the animation
};

/**
  * @brief  Show summary for a record

  * @retval None
  */
void ShowSummary(u8 reason)
{
	Key_Message_Struct key_Message;
	char tempString[10];
	float platVolt;
	ShowDialogue(Summary_Str[CurrentSettings->Language],"","");
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_InvertRect(1, 1, 126, 14);
	if(reason)
	{
	 SoundStart(Ichiban_no_takaramono);
	 LED_Animate_Init(LEDSummaryAnimate);
	}
	OLED_ShowAnyString(3, 14, SummaryCapacity_Str[CurrentSettings->Language], NotOnSelect, 12);
	OLED_ShowAnyString(3, 26, SummaryWork_Str[CurrentSettings->Language], NotOnSelect, 12);
	OLED_ShowAnyString(3, 38, SummaryTime_Str[CurrentSettings->Language], NotOnSelect, 12);
	OLED_ShowAnyString(3, 51, SummaryPlatVolt_Str[CurrentSettings->Language], NotOnSelect, 12);
	if (CurrentSumUpData.Capacity >= 10000)
		sprintf(tempString, "%05.2fAh", CurrentSumUpData.Capacity);
	else
		sprintf(tempString, "%05.0fmAh", CurrentSumUpData.Capacity * 1000);
	OLED_ShowAnyString(75, 14, tempString, NotOnSelect, 12);
	if (CurrentSumUpData.Work >= 10000)
		sprintf(tempString, "%05.2fWh", CurrentSumUpData.Work);
	else
		sprintf(tempString, "%05.0fmWh", CurrentSumUpData.Work * 1000);
	OLED_ShowAnyString(75, 26, tempString, NotOnSelect, 12);
	GenerateVirtualRTCString(tempString);
	OLED_ShowAnyString(75, 38, tempString, NotOnSelect, 12);
	if (RTCCurrent.Day)
	{
		sprintf(tempString, "%dDays", RTCCurrent.Day);
		OLED_ShowAnyString(39, 38, tempString, NotOnSelect, 12);
	}
	if (CurrentSumUpData.Capacity * 1000 < 20 || CurrentSumUpData.Work * 1000 < 20)
	{
		strcpy(tempString, "--N/A--");
		OLED_ShowAnyString(79, 50, tempString, NotOnSelect, 12);
	}
	else
	{
		platVolt = CurrentSumUpData.Work / CurrentSumUpData.Capacity;
		if (platVolt >= 10)
		{
			sprintf(tempString, "%5.2fV", platVolt);
		}
		else
		{
			sprintf(tempString, "%5.3fV", platVolt);
		}
		OLED_ShowAnyString(81, 50, tempString, NotOnSelect, 12);
	}
	xSemaphoreGive(OLEDRelatedMutex);
	for(;;)
	{
   xQueueReceive(Key_Message, &key_Message, portMAX_DELAY);
	 SoundStop();
	 LED_Animate_DeInit();
	 if(key_Message.KeyEvent==MidDouble) 
   {
		 return;
	 }
	}
}

void StopRecord(u8* status,u8 reason)
{
	FRESULT res;
	if (RecordHandle != NULL)
		vTaskDelete(RecordHandle);
	if(IsRecording)
	{
	 res=f_close(&RecordFile);
	 if(res==FR_OK)
		 ShowSmallDialogue(Saved_Str[CurrentSettings->Language], 1000, true);
	 else
		 ShowSmallDialogue(SaveFailed_Str[CurrentSettings->Language], 1000, true);
	}
	if(*status==LEGACY_TEST) EBDSendLoadCommand(0, StopTest);
	VirtualRTC_DeInit();
	if(reason)
	{
	 ShowSmallDialogue(StepUpTestProtected_Str[CurrentSettings->Language], 1000, true);
	}
	ShowSummary(reason);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
	*status = USBMETER_ONLY;
}
