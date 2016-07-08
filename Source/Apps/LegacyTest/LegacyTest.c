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
#include "UI_Button.h"
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

#include "EBProtocolConfig.h"

#define RECORD_HANDLER_PRIORITY tskIDLE_PRIORITY+7
#define LEGACY_TEST_CURRENT_MAX CURRENT_MAX
#define LEGACY_TEST_POWER_MAX POWER_MAX

FIL RecordFile;

u32 RecordFileWriteCount;

bool IsRecording;

xTaskHandle RecordHandle;


/**
  * @brief  Record task,update summary data and write
current data into sdcard

	  @Hint   This task can be killed by StopRecord()
  */
void Record_Handler(void *pvParameters)
{
	u8 lastSec = RTCCurrent.Sec;
	u8 stringLength;
	char tempString[50];
	SecondNum = 0;
	for (;;)
	{
		/*Write current meter data into sdcard*/
		if (IsRecording)
		{
			if (CurrentTemperatureSensor == Internal)
				sprintf(tempString, "%ld,%6.3f,%6.3f\r\n", SecondNum, CurrentMeterData.Current,
					CurrentMeterData.Voltage);
			else
				sprintf(tempString, "%ld,%6.3f,%6.3f,%5.1f\r\n", SecondNum, CurrentMeterData.Current,
					CurrentMeterData.Voltage, ExternalTemperature);
			stringLength = GetStringGraphicalLength(tempString);
			f_write(&RecordFile, tempString, stringLength, &RecordFileWriteCount);
		}
		/*Update summary data*/
		CurrentSumUpData.Capacity += (CurrentMeterData.Current / 3600);
		CurrentSumUpData.Work += (CurrentMeterData.Power / 3600);
		CurrentSumUpData.PlatformVolt = CurrentMeterData.Power / CurrentMeterData.Current;
		/*Keep blocked until a new second reached*/
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

/**
  * @brief  Clear summary data
  */
void ClearRecordData()
{
	CurrentSumUpData.Capacity = 0;
	CurrentSumUpData.Work = 0;
	CurrentSumUpData.PlatformVolt = 0;
}

/**
  * @brief  Create a record file(with name of RTC time)

	@rtval: true(success) false(failed)
  */
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

/**
  * @brief  Create the header of a record
  */
void CreateRecordFileHeader()
{
	f_write(&RecordFile, "Time(s),Curt(A),Volt(V),Temp(C)\r\n",
		sizeof("Time(s),Curt(A),Volt(V),Temp(C)\r\n"), &RecordFileWriteCount);
}


/**
  * @brief  Select the mode of legacy test(CC or CP)

    @rtval mode
  */
u8 SelectLegacyTestMode()
{
 u8 mode;
 UI_Button_Param_Struct selectModeButtonParams;
	
 selectModeButtonParams.ButtonString=SelectLeagcyTestModeButtons_Str[CurrentSettings->Language];
 selectModeButtonParams.ButtonNum=2;
 selectModeButtonParams.DefaultValue=0;
 selectModeButtonParams.Positions=SelectLegacyTestModePositions[CurrentSettings->Language];
 
 xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
 OLED_Clear();
 xSemaphoreGive(OLEDRelatedMutex);
 
 ShowDialogue(SelectLeagcyTestMode_Str[CurrentSettings->Language],
	SelectLeagcyTestModeSubString_Str[CurrentSettings->Language],"");
 
 UI_Button_Init(&selectModeButtonParams);
 
 xQueueReceive(UI_ButtonMsg, &mode, portMAX_DELAY);
 UI_Button_DeInit();
 IgnoreNextKeyEvent();
 
 xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
 OLED_Clear();
 xSemaphoreGive(OLEDRelatedMutex);
 
 return mode;
}

/**
  * @brief  Run a legacy test with params from user
  */
void RunLegacyTest(u8* status, Legacy_Test_Param_Struct* test_Params)
{
	bool protectedFlag;
	*status = LEGACY_TEST;
	
	test_Params->TestMode=SelectLegacyTestMode();
	
	/*Get parameters of the legacy test*/
	if(test_Params->TestMode==ConstantCurrent)test_Params->Current =
		GetTestParam(LegacyTestSetCurrent_Str[CurrentSettings->Language], 100, 
	  EBD_Protocol_Config[CurrentSettings->EBD_Model]->CurrentMax,
			1000, 100, "mA", 20);
	else test_Params->Power=
			GetTestParam(LegacyTestSetPower_Str[CurrentSettings->Language], 500, 
	EBD_Protocol_Config[CurrentSettings->EBD_Model]->PowerMax,
			10000, 500, "mW", 20);
	test_Params->ProtectVolt =
		GetTestParam(ProtVoltageGet_Str[CurrentSettings->Language], 0,
		(int)(1000 * CurrentMeterData.Voltage) / 10 * 10 > 0 ? (1000 * CurrentMeterData.Voltage) / 10 * 10 : 100,
			(int)(900 * CurrentMeterData.Voltage) / 10 * 10 > 0 ? (900 * CurrentMeterData.Voltage) / 10 * 10 : 100, 10, "mV", 25);

	/*Warn user of the unmounted sdcard*/
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
		/*Try to create a record file,show hint if failed*/
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
	/*Clear the screen*/
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	OLED_Refresh_Gram();
	xSemaphoreGive(OLEDRelatedMutex);

	/*Init logo*/
	LogoWithInitStatus_Init();

	/*Show waiting string*/
	xQueueSend(InitStatusMsg, PleaseWait_Str[CurrentSettings->Language], 0);

	/*Sync EBD to meet the chance to send load command*/
	EBD_Sync();
	vTaskDelay(200);
	
	if(test_Params->TestMode==ConstantCurrent)
	EBDSendLoadCommand(test_Params->Current, StartTest);
	else
	EBDSendLoadCommand((float)test_Params->Power/CurrentMeterData.Voltage, StartTest);	

	/*Wait for load command effects*/
	for (;;)
	{
		EBD_Sync();

		/*If protection triggered,undo the legacy test*/
		if (CurrentMeterData.Voltage < (float)(test_Params->ProtectVolt) / 1000 ||
			CurrentMeterData.Voltage < 0.5)
		{
			protectedFlag = true;
			vTaskDelay(200 / portTICK_RATE_MS);
			EBDSendLoadCommand(0, StopTest);
			break;
		}

		/*Break if the load command effects*/
		if (CurrentMeterData.Current > (float)(test_Params->Current) / 1010 &&
			CurrentMeterData.Current < (float)(test_Params->Current) / 990&&
		test_Params->TestMode==ConstantCurrent)
		{
			protectedFlag = false;
			break;
		}
		
		if (CurrentMeterData.Power > (float)(test_Params->Power) / 1050 &&
			CurrentMeterData.Power < (float)(test_Params->Power) / 950&&
		test_Params->TestMode==ConstantPower)		
		{
			protectedFlag = false;
			break;
		}
		
		vTaskDelay(200 / portTICK_RATE_MS);
	  if(test_Params->TestMode==ConstantCurrent)
   	EBDSendLoadCommand(test_Params->Current, StartTest);
	  else
	  EBDSendLoadCommand((float)test_Params->Power/CurrentMeterData.Voltage, KeepTest);	
	}

	/*Delete logo and initStatus tasks*/
	LogoWithInitStatus_DeInit();

	UpdateOLEDJustNow = false;
	xSemaphoreGive(OLEDRelatedMutex);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	OLED_Refresh_Gram();
	xSemaphoreGive(OLEDRelatedMutex);
	if (protectedFlag == false)
	{
		ClearRecordData();
		VirtualRTC_Init();
		CreateTaskWithExceptionControl(Record_Handler, "Record Handler",
			256, NULL, RECORD_HANDLER_PRIORITY, &RecordHandle);
		ShowSmallDialogue(LegacyTestStarted_Str[CurrentSettings->Language], 1000, true);
	}
	else
	{
		ShowSmallDialogue(LegacyTestVoltLow_Str[CurrentSettings->Language], 1000, true);
		if (IsRecording) f_close(&RecordFile);
		*status = USBMETER_ONLY;
	}
}

/**
  * @brief  Start a record(without load)
  */
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
	ShowDialogue(Summary_Str[CurrentSettings->Language], "", "");
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_InvertRect(1, 1, 126, 14);

	/*If the stop of the record is caused by protection,play sound to notice the
	user*/
	if (reason)
	{
		SoundStart(Ichiban_no_takaramono);
		LED_Animate_Init(LEDSummaryAnimate);
	}

	/*Show item strings*/
	OLED_ShowAnyString(3, 14, SummaryCapacity_Str[CurrentSettings->Language], NotOnSelect, 12);
	OLED_ShowAnyString(3, 26, SummaryWork_Str[CurrentSettings->Language], NotOnSelect, 12);
	OLED_ShowAnyString(3, 38, SummaryTime_Str[CurrentSettings->Language], NotOnSelect, 12);
	OLED_ShowAnyString(3, 51, SummaryPlatVolt_Str[CurrentSettings->Language], NotOnSelect, 12);

	/*Print summary*/
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

	/*Keep in blocked until middle key double clicked*/
	for (;;)
	{
		xQueueReceive(Key_Message, &key_Message, portMAX_DELAY);
		/*Stop sound when any key event occurred*/
		SoundStop();
		LED_Animate_DeInit();
		if (key_Message.KeyEvent == MidDouble)
		{
			return;
		}
	}
}

/**
  * @brief  Stop a record
  */
void StopRecord(u8* status, u8 reason)
{
	FRESULT res;

	/*Delete record task*/
	if (RecordHandle != NULL)
		vTaskDelete(RecordHandle);

	/*If it is a record with file recording,close record file
	and show success or not*/
	if (IsRecording)
	{
		res = f_close(&RecordFile);
		if (res == FR_OK)
			ShowSmallDialogue(Saved_Str[CurrentSettings->Language], 1000, true);
		else
			ShowSmallDialogue(SaveFailed_Str[CurrentSettings->Language], 1000, true);
	}

	/*If it is a record with load,stop EBD*/
	if (*status == LEGACY_TEST) EBDSendLoadCommand(0, StopTest);

	/*DeInit virtual RTC*/
	VirtualRTC_DeInit();

	/*If the stop operation is caused by protection,show "protected"*/
	if (reason)
	{
		ShowSmallDialogue(StepUpTestProtected_Str[CurrentSettings->Language], 1000, true);
	}

	/*Show summary on the screen*/
	ShowSummary(reason);

	/*Wipe your ass*/
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);

	/*Reset status flag*/
	*status = USBMETER_ONLY;
}
