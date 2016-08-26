//File Name   LegacyTest.c
//Description LegacyTest

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#include "FreeRTOS_Standard_Include.h"

#include "MCP3421.h"

#include "Digital_Load.h"
#include "LED.h"
#include "SSD1306.h"
#include "sdcardff.h"
#include "W25Q64ff.h"
#include "LED_Animate.h"
#include "Keys.h"
#include "Cooling_Fan.h"
#include "Temperature_Sensors.h"

#include "UI_Adjust.h"
#include "UI_Dialogue.h"
#include "UI_Confirmation.h"
#include "UI_Button.h"
#include "UI_Menu.h"
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

#define RECORD_HANDLER_PRIORITY tskIDLE_PRIORITY+6
//#define LEGACY_TEST_CURRENT_MAX 4000
//#define LEGACY_TEST_POWER_MAX POWER_MAX 40000

FIL RecordFile;

u32 RecordFileWriteCount;

bool IsRecording;

xTaskHandle RecordHandle;

volatile SumupStruct FastUpdateCurrentSumUpData;


/**
  * @brief  Record task,update summary data and write
current data into sdcard

	  @Hint   This task can be killed by StopRecord()
  */
void Record_Handler(void *pvParameters)
{
	u8 lastSec = RTCCurrent.Sec;
  u8 lstSecDiv10;
	u8 stringLength;
	char tempString[50];
	SecondNum = 0;
	for (;;)
	{
		/*Write current meter data into sdcard*/
		if (IsRecording)
		{
			if (CurrentTemperatureSensor == Internal)
				sprintf(tempString, "%ld,%6.3f,%6.3f\r\n", SecondNum, FilteredMeterData.Current,
					FilteredMeterData.Voltage);
			else
				sprintf(tempString, "%ld,%6.3f,%6.3f,%5.1f\r\n", SecondNum, FilteredMeterData.Current,
					FilteredMeterData.Voltage, ExternalTemperature);
			stringLength = GetStringGraphicalLength(tempString);
			f_write(&RecordFile, tempString, stringLength, &RecordFileWriteCount);
		}
		/*Update summary data*/
		CurrentSumUpData.Capacity += (FilteredMeterData.Current / 3600);
		CurrentSumUpData.Work += (FilteredMeterData.Power / 3600);
		CurrentSumUpData.PlatformVolt = FilteredMeterData.Power / FilteredMeterData.Current;
    FastUpdateCurrentSumUpData.Capacity=CurrentSumUpData.Capacity;
    FastUpdateCurrentSumUpData.Work=CurrentSumUpData.Work;
    FastUpdateCurrentSumUpData.PlatformVolt=CurrentSumUpData.PlatformVolt;
		/*Keep blocked until a new second reached*/
		for (;;)
		{
      if(lstSecDiv10!=RTCCurrent.SecDiv10)
      {
		FastUpdateCurrentSumUpData.Capacity += (FilteredMeterData.Current / 36000);
		FastUpdateCurrentSumUpData.Work += (FilteredMeterData.Power / 36000);
		FastUpdateCurrentSumUpData.PlatformVolt = FilteredMeterData.Power / FilteredMeterData.Current;
            lstSecDiv10=RTCCurrent.SecDiv10;
      }
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
bool CreateRecordFile(char storageNo)
{
	FRESULT res;
	char tempString[60];
	sprintf(tempString, "x:/Kanade/Records/%04d_%02d_%02d %02d-%02d-%02d.csv", RTCTime.w_year,
		RTCTime.w_month, RTCTime.w_date, RTCTime.hour, RTCTime.min, RTCTime.sec);
	tempString[0] = storageNo;
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
	const char *modeStrings[2];

	modeStrings[0] = SelectLeagcyTestModeButtonCC_Str[CurrentSettings->Language];
	modeStrings[1] = SelectLeagcyTestModeButtonCP_Str[CurrentSettings->Language];

	selectModeButtonParams.ButtonStrings = modeStrings;
	selectModeButtonParams.ButtonNum = 2;
	selectModeButtonParams.DefaultValue = 0;
	selectModeButtonParams.Positions = SelectLegacyTestModePositions[CurrentSettings->Language];

	OLED_Clear_With_Mutex_TakeGive();

	ShowDialogue(SelectLeagcyTestMode_Str[CurrentSettings->Language],
		SelectLeagcyTestModeSubString_Str[CurrentSettings->Language], "", false, false);

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
  * @brief Start or recover load
  */
void StartOrRecoverLoad(Legacy_Test_Param_Struct* test_Params, bool *protectedFlag)
{
	if (test_Params->TestMode == ConstantCurrent)
		Send_Digital_Load_Command(test_Params->Current, Load_Start);
	else
		Send_Digital_Load_Command((float)test_Params->Power / CurrentMeterData.Voltage, Load_Start);
  Fan_Send_Command(Turn_On);
	for (;;)
	{
		/*If protection triggered,undo the legacy test*/
		if (FilteredMeterData.Voltage < (float)(test_Params->ProtectVolt) / 1000
			|| FilteredMeterData.Voltage < 0.5)
		{
			*protectedFlag = true;
			Send_Digital_Load_Command(0, Load_Stop);
			break;
		}

		/*Break if the load command effects*/
		if (FilteredMeterData.Current > (float)(test_Params->Current) / 1050
			&& FilteredMeterData.Current < (float)(test_Params->Current) / 950
			&& test_Params->TestMode == ConstantCurrent)
		{
			*protectedFlag = false;
			break;
		}

		if (CurrentMeterData.Power > (float)(test_Params->Power) / 1050
			&& CurrentMeterData.Power < (float)(test_Params->Power) / 950
			&& test_Params->TestMode == ConstantPower)
		{
			*protectedFlag = false;
			break;
		}

		vTaskDelay(1000 / portTICK_RATE_MS);
		if (test_Params->TestMode == ConstantCurrent)
			Send_Digital_Load_Command(test_Params->Current, Load_Start);
		else
			Send_Digital_Load_Command((float)test_Params->Power / CurrentMeterData.Voltage, Load_Keep);
	}

}

char SelectStorage()
{
	u8 cnt = 0;
	u8 selection;
	UI_Menu_Param_Struct menuParams;
	const char* storageTab[3];
	if (SDCardMountStatus)
	{
		storageTab[cnt] = SettingsItemFormatSD_Str[CurrentSettings->Language];
		cnt++;
	}
	if (SPIFlashMountStatus)
	{
		storageTab[cnt] = SettingsItemFormatInternal_Str[CurrentSettings->Language];
		cnt++;
	}
	storageTab[cnt] = DontSave_Str[CurrentSettings->Language];
	menuParams.ItemStrings = storageTab;
	menuParams.DefaultPos = 0;
	menuParams.ItemNum = ++cnt;
	menuParams.FastSpeed = 10;

	OLED_Clear_With_Mutex_TakeGive();

	UI_Menu_Init(&menuParams);

	xQueueReceive(UI_MenuMsg, &selection, portMAX_DELAY);
	UI_Menu_DeInit();

	if (storageTab[selection] == SettingsItemFormatSD_Str[CurrentSettings->Language])
	{
		return '0';
	}
	else if (storageTab[selection] == SettingsItemFormatInternal_Str[CurrentSettings->Language])
	{
		return '1';
	}
	else return 'N';
}

#define DIGITAL_LOAD_CURRENT_MAX  Digital_Load_Params[CurrentSettings->Digital_Load_Params_Mode]->CurrentMax
#define DIGITAL_LOAD_POWER_MAX  Digital_Load_Params[CurrentSettings->Digital_Load_Params_Mode]->PowerMax

/**
  * @brief  Run a legacy test with params from user
  */
void RunLegacyTest(u8* status, Legacy_Test_Param_Struct* test_Params)
{
	bool protectedFlag;
	char storage;
	*status = LEGACY_TEST;

	test_Params->TestMode = SelectLegacyTestMode();

	/*Get parameters of the legacy test*/
	if (test_Params->TestMode == ConstantCurrent)
	{
		test_Params->Current = GetTestParam(LegacyTestSetCurrent_Str[CurrentSettings->Language], 100,
			DIGITAL_LOAD_CURRENT_MAX,
			1000, 100, "mA", 20);
		if (test_Params->Current < 0)
		{
			*status = USBMETER_ONLY;
			OLED_Clear_With_Mutex_TakeGive();
			return;
		}
	}
	else
	{
		test_Params->Power =
			GetTestParam(LegacyTestSetPower_Str[CurrentSettings->Language], 500,
				DIGITAL_LOAD_POWER_MAX,
				10000, 500, "mW", 20);
		if (test_Params->Power < 0)
		{
			*status = USBMETER_ONLY;
			OLED_Clear_With_Mutex_TakeGive();
			return;
		}
	}
	test_Params->ProtectVolt =
		GetTestParam(ProtVoltageGet_Str[CurrentSettings->Language], 0,
		(int)(100 * CurrentMeterData.Voltage) * 10 > 0 ? (int)(100 * CurrentMeterData.Voltage) * 10 : 100,
			(int)(90 * CurrentMeterData.Voltage) * 10 > 0 ? (int)(90 * CurrentMeterData.Voltage) * 10 : 100, 10, "mV", 25);
	if (test_Params->ProtectVolt < 0)
	{
		*status = USBMETER_ONLY;
		OLED_Clear_With_Mutex_TakeGive();
		return;
	}
	/*Warn user of the unmounted sdcard*/
	if (!SDCardMountStatus && !SPIFlashMountStatus)
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
		storage = SelectStorage();
		if (storage != 'N')
		{
			/*Try to create a record file,show hint if failed*/
			if (!CreateRecordFile(storage))
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
		else IsRecording = false;
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

	StartOrRecoverLoad(test_Params, &protectedFlag);

	/*Delete logo and initStatus tasks*/
	LogoWithInitStatus_DeInit();

	//UpdateOLEDJustNow = false;
	//xSemaphoreGive(OLEDRelatedMutex);
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
    Fan_Send_Command(Auto);
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
	char storage;
	if (!SDCardMountStatus && !SPIFlashMountStatus)
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
		storage = SelectStorage();
		if (storage != 'N')
		{
			if (!CreateRecordFile(storage))
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
		else IsRecording = false;
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
	ShowDialogue(Summary_Str[CurrentSettings->Language], "", "", false, false);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);

	/*If the stop of the record is caused by protection,play sound to notice the
	user*/
	if (reason)
	{
		SoundStart(Tori_No_Uta);
		LED_Animate_Init(LEDSummaryAnimate);
	}
	/*Show item strings*/
	OLED_ShowAnyString(3, 14, SummaryCapacity_Str[CurrentSettings->Language], NotOnSelect, 12);
	OLED_ShowAnyString(3, 26, SummaryWork_Str[CurrentSettings->Language], NotOnSelect, 12);
	OLED_ShowAnyString(3, 38, SummaryTime_Str[CurrentSettings->Language], NotOnSelect, 12);
	OLED_ShowAnyString(3, 51, SummaryPlatVolt_Str[CurrentSettings->Language], NotOnSelect, 12);

	/*Print summary*/
	if (CurrentSumUpData.Capacity >= 100)
		sprintf(tempString, "%05.2fAh", CurrentSumUpData.Capacity);
	else
		sprintf(tempString, "%05.0fmAh", CurrentSumUpData.Capacity * 1000);
	OLED_ShowAnyString(75, 14, tempString, NotOnSelect, 12);
	if (CurrentSumUpData.Work >= 100)
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

	/*If it is a record with load,stop load*/
	if (*status == LEGACY_TEST) 
     {
       Send_Digital_Load_Command(0, Load_Stop);
       Fan_Send_Command(Auto);
     }

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
	OLED_Clear_With_Mutex_TakeGive();

	/*Reset status flag*/
	*status = USBMETER_ONLY;
}
