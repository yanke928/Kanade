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

#include "LegacyTest.h"

#define RECORD_HANDLER_PRIORITY tskIDLE_PRIORITY+7
#define LEGACY_TEST_CURRENT_MAX 5000

FIL RecordFile;

u32 RecordFileWriteCount;

bool IsRecording;

xTaskHandle RecordHandle;

void Record_Handler(void *pvParameters)
{
 u8 lastSec=RTCCurrent.Sec;
 u8 stringLength;
 char tempString[50];
 for(;;)
	{
	 if(IsRecording)
	 {
		if (CurrentTemperatureSensor == Internal)
		sprintf(tempString, "%ld,%6.3f,%6.3f\r\n", SecondNum, CurrentMeterData.Current,
			CurrentMeterData.Voltage);
	  else
		sprintf(tempString, "%ld,%6.3f,%6.3f,%5.1f\r\n", SecondNum, CurrentMeterData.Current,
			CurrentMeterData.Voltage, ExternalTemperature);  
		stringLength = GetStringLength(tempString);
			f_write(&RecordFile, tempString,stringLength,&RecordFileWriteCount);	  
	 }
	 	CurrentSumUpData.Capacity += (CurrentMeterData.Current / 3600);
	  CurrentSumUpData.Work += (CurrentMeterData.Power / 3600);
	  CurrentSumUpData.PlatformVolt = CurrentMeterData.Power / CurrentMeterData.Current;
	 for(;;) 
  {
	 if(lastSec!=RTCCurrent.Sec)
	 {
	  lastSec=RTCCurrent.Sec;
		break;
	 }
	 vTaskDelay(50/portTICK_RATE_MS);
	}
	}
}

void ClearRecordData()
{
 CurrentSumUpData.Capacity=0;
 CurrentSumUpData.Work=0;	
 CurrentSumUpData.PlatformVolt=0;
}

void RunLegacyTest(u8* status,Legacy_Test_Param_Struct* test_Params)
{
 FRESULT res;
 xTaskHandle logoHandle;
 xTaskHandle initStatusUpdateHandle;
 bool protectedFlag;
 *status=LEGACY_TEST;
 test_Params->Current=
 GetTestParam(LegacyTestSetCurrent_Str[CurrentSettings->Language], 100, LEGACY_TEST_CURRENT_MAX,
 1000, 100, "mA", 20);
 test_Params->ProtectVolt=
 GetTestParam(ProtVoltageGet_Str[CurrentSettings->Language],0,
 (int)(1000 * CurrentMeterData.Voltage)/10*10>0?(1000 * CurrentMeterData.Voltage)/10*10:100,
 (int)(900 * CurrentMeterData.Voltage)/10*10>0?(900 * CurrentMeterData.Voltage) / 10 * 10:100, 10, "mV", 25);
 if(!SDCardMountStatus)
 {
	IsRecording=false;
  if(!GetConfirmation(NoDiskConfirm_Str[CurrentSettings->Language],""))
	{
	 *status=USBMETER_ONLY;
	 return;
	}
 }
 else
 {
  char tempString[60];
	sprintf(tempString, "0:/EBD/Records/%04d_%02d_%02d %02d-%02d-%02d.csv", RTCTime.w_year,
			RTCTime.w_month, RTCTime.w_date, RTCTime.hour, RTCTime.min, RTCTime.sec);
	res = f_open(&RecordFile, tempString, FA_CREATE_ALWAYS | FA_WRITE);
	if (res != FR_OK)
		{
			ShowSmallDialogue(FileCreateFailed_Str[CurrentSettings->Language],1000,true);
		}
	else
	{
	 RecordFileWriteCount=0;
	 ShowSmallDialogue(FileCreated_Str[CurrentSettings->Language],1000,true);
			f_write(&RecordFile, "Time(s),Curt(A),Volt(V),Temp(C)\r\n",
			sizeof("Time(s),Curt(A),Volt(V),Temp(C)\r\n"), &RecordFileWriteCount);
	 IsRecording=true;
	}
 }
 xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
 OLED_Clear();
 OLED_Refresh_Gram();
 xSemaphoreGive(OLEDRelatedMutex);
	logoHandle = Logo_Init();
	initStatusUpdateHandle = InitStatusHandler_Init();
 	xQueueSend(InitStatusMsg, "Waiting...", 0);
 EBD_Sync();
 vTaskDelay(200);
 EBDSendLoadCommand(test_Params->Current,StartTest);
 for(;;)
 {
	EBD_Sync();
	if(CurrentMeterData.Voltage<(float)(test_Params->ProtectVolt)/1000)
	{
	 protectedFlag=true;
	vTaskDelay(200/ portTICK_RATE_MS); 
	EBDSendLoadCommand(0,StopTest);
		break;
	}
	if(CurrentMeterData.Current>(float)(test_Params->Current)/1010&&
		 CurrentMeterData.Current<(float)(test_Params->Current)/990)
		 {
		  protectedFlag=false;
			break;
		 }
	 vTaskDelay(200/ portTICK_RATE_MS); 
	 EBDSendLoadCommand(test_Params->Current,StartTest);
 }
 if(logoHandle!=NULL)
 vTaskDelete(logoHandle);
 if(initStatusUpdateHandle!=NULL)
 vTaskDelete(initStatusUpdateHandle);
 xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
 OLED_Clear();
 OLED_Refresh_Gram();
 xSemaphoreGive(OLEDRelatedMutex);
 if(protectedFlag==false)
 {
 ClearRecordData();
	 VirtualRTC_Init();
   xTaskCreate(Record_Handler, "Record Handler",
		128, NULL, RECORD_HANDLER_PRIORITY, &RecordHandle);
		 ShowSmallDialogue("Success!!",1000,true);
 }
 else
 {
  ShowSmallDialogue("Protected!!",1000,true);
	 *status=USBMETER_ONLY;
 }
}
