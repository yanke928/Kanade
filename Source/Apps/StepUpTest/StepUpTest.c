//File Name   StepUpTest.c
//Description StepupTest

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

#include "UI_Dialogue.h"
#include "UI_Adjust.h"
#include "UI_ListView.h"

#include "MultiLanguageStrings.h"

#include "Settings.h"

#include "StepUpTest.h"

/*Step-up test memory mapping*/
#define FLASH_CACHE_START_ADDR 0x0801d000
#define FLASH_VOLTAGE_ADDR  FLASH_CACHE_START_ADDR
#define FLASH_CURRENT_ADDR  FLASH_CACHE_START_ADDR+3072     
#define FLASH_DATAEND_ADDR  FLASH_CACHE_START_ADDR+6144

/*Step-up test params*/
#define STEP_UP_TEST_INTERVAL_MAX 30
#define STEP_UP_TEST_CURRENT_MAX 5000

#define STEPUPTEST_HANDLER_PRIORITY tskIDLE_PRIORITY+5

xQueueHandle StepUpTest_UI_UpdateMsg;
xQueueHandle StepUpTest_UI_Test_DoneMsg;

void EraseCacheBlocks(void);
void Flash_ProgramFloat(u32 addr, float data);

void StepUpTest_Handler(void *pvParameters)
{
	StepUpTestParamsStruct* test_Params = (StepUpTestParamsStruct*)pvParameters;
	StepUpTestStateStruct currentState;
	u16 currentAddr = 0;
	u16 currentCurrent;
	u16 lastCurrent = test_Params->StartCurrent;
	u8 i;
	EBD_Sync();
	vPortEnterCritical();
	EraseCacheBlocks();
	vPortExitCritical();
	vTaskDelay(200 / portTICK_RATE_MS);
	EBDSendLoadCommand(0, StartTest);
	vTaskDelay(200 / portTICK_RATE_MS);
	EBDSendLoadCommand(test_Params->StartCurrent, KeepTest);
	EBD_Sync();
	currentState.TestOverFlag=0;
	currentState.CurrentTime=0;
	for (;;)
	{
		xQueueReceive(EBDRxDataMsg, &i, portMAX_DELAY);
		Flash_ProgramFloat(FLASH_VOLTAGE_ADDR + currentAddr * sizeof(float), CurrentMeterData.Voltage);
		Flash_ProgramFloat(FLASH_CURRENT_ADDR + currentAddr * sizeof(float), CurrentMeterData.Current);
		xQueueSend(StepUpTest_UI_UpdateMsg, &currentState, 0);
		currentState.CurrentTime = currentState.CurrentTime + 2;
		currentAddr++;
		currentCurrent = (currentState.CurrentTime / test_Params->TimeInterval)*test_Params->Step + test_Params->StartCurrent;
		if ((CurrentMeterData.Voltage < (float)test_Params->ProtectVolt / 1000) ||
			(CurrentMeterData.Voltage < 0.5) ||
			(currentCurrent > test_Params->StopCurrent))
		{
			vTaskDelay(200 / portTICK_RATE_MS);
			EBDSendLoadCommand(0, StopTest);
			if (currentCurrent > test_Params->StopCurrent)
				currentState.TestOverFlag=1;
			else
				currentState.TestOverFlag=2;
			xQueueSend(StepUpTest_UI_UpdateMsg, &currentState, 0);
			vTaskDelete(NULL);
		}
		if (currentCurrent != lastCurrent)
		{
			vTaskDelay(200 / portTICK_RATE_MS);
			EBDSendLoadCommand(currentCurrent, KeepTest);
			lastCurrent = currentCurrent;
		}
	}
}

void StepUpTestOnTimeUI(u16 timeNow);

void StepUpTest_UI_Handler(void *pvParameters)
{
	StepUpTestStateStruct currentState;
	currentState.CurrentTime=0;
	ShowDialogue((char *)StepUpTestRunning_Str[CurrentSettings->Language], "", "");
	for (;;)
	{
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		StepUpTestOnTimeUI(currentState.CurrentTime);
		xSemaphoreGive(OLEDRelatedMutex);
		if (currentState.CurrentTime != 0)
		{
			vTaskDelay(1000 / portTICK_RATE_MS);
			xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
			StepUpTestOnTimeUI(currentState.CurrentTime + 1);
			xSemaphoreGive(OLEDRelatedMutex);
		}
		xQueueReceive(StepUpTest_UI_UpdateMsg, &currentState, portMAX_DELAY);
		if (currentState.TestOverFlag>0)
		{
			xQueueSend(StepUpTest_UI_Test_DoneMsg, &currentState, 0);
			vTaskDelete(NULL);
		}
	}
}

void StepUpTestOnTimeUI(u16 timeNow)
{
	char tempString[10];
	OLED_ShowAnyString(5, 15, (char *)RunTime_Str[CurrentSettings->Language], NotOnSelect, 16);
	sprintf(tempString, "%3ds", timeNow);
	OLED_ShowAnyString(88, 15, tempString, NotOnSelect, 16);
	OLED_ShowAnyString(5, 31, (char *)CurrentVoltage_Str[CurrentSettings->Language], NotOnSelect, 16);
	if (CurrentMeterData.Voltage < 10)
		sprintf(tempString, "%5.3fV", CurrentMeterData.Voltage);
	else
		sprintf(tempString, "%5.2fV", CurrentMeterData.Voltage);
	OLED_ShowAnyString(72, 31, tempString, NotOnSelect, 16);
	OLED_ShowAnyString(5, 47, (char *)CurrentCurrent_Str[CurrentSettings->Language], NotOnSelect, 16);
	sprintf(tempString, "%5.3fA", CurrentMeterData.Current);
	OLED_ShowAnyString(72, 47, tempString, NotOnSelect, 16);
}

/**
  * @brief  Start a StepUp test

	@param  test_Params:Step up test parameters

  * @retval None
  */
void StepUpTest_Init(StepUpTestParamsStruct* test_Params)
{
	StepUpTest_UI_UpdateMsg = xQueueCreate(2, sizeof(StepUpTestStateStruct));
	xTaskCreate(StepUpTest_Handler, "Stepup test Handler",
		256, test_Params, STEPUPTEST_HANDLER_PRIORITY, NULL);
}

/**
  * @brief  Init user interface for stepup test

	@param  test_Params:Step up test parameters

  * @retval None
  */
void StepUpTest_UI_Init(void)
{
	StepUpTest_UI_Test_DoneMsg = xQueueCreate(1, sizeof(StepUpTestStateStruct));
	xTaskCreate(StepUpTest_UI_Handler, "Stepup test UI Handler",
		160, NULL, STEPUPTEST_HANDLER_PRIORITY, NULL);
}


/**
  * @brief  Erase blocks that step-up test occupypies

  * @retval None
  */
void EraseCacheBlocks()
{
	u8 t;
	FLASH_Unlock();
	FLASH_SetLatency(FLASH_Latency_2);
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	for (t = 0;; t++)
	{
		if (FLASH_VOLTAGE_ADDR + t * 1024 > FLASH_DATAEND_ADDR)
		{
			break;
		}
		FLASH_ErasePage(FLASH_VOLTAGE_ADDR + t * 1024);
	}
}

/**
  * @brief  Programme a float_t data to flash(as cache)

	@param  addr:Address
			data:Data

  * @retval None
  */
void Flash_ProgramFloat(u32 addr, float data)
{
	u32 *p;
	p = (u32*)&data;
	FLASH_ProgramWord(addr, p[0]);
}


u16 GetTestParam(const char askString[],u16 min,u16 max,u16 defaultValue,u16 step,char unitString[],u8 fastSpeed)
{
 u16 t;
 UI_Adjust_Param_Struct adjust_params;
 xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
 OLED_Clear();
 xSemaphoreGive(OLEDRelatedMutex);
 adjust_params.AskString=(char *)askString;
 adjust_params.Min=min;
 adjust_params.Max=max;
 adjust_params.Step=step;
 adjust_params.DefaultValue=defaultValue;
 adjust_params.UnitString=unitString;
 adjust_params.FastSpeed=fastSpeed;
 adjust_params.Pos_y=33;
 UI_Adjust_Init(&adjust_params);
 xQueueReceive(UI_AdjustMsg, & t, portMAX_DELAY );
 return t;
}

void ShowStepUpTestResult(u16 time)
{
 ListView_Param_Struct listView_Params;
 u16 i;
 char ItemNameTime[]="Time";
 char ItemNameCurt[]="Curt(A)"; 
 char ItemNameVolt[]="Volt(V)"; 
 char sprintfCommandCurt[]="%0.3f"; 
 char sprintfCommandVolt[]="%0.3f"; 
 listView_Params.ItemNames[0]=ItemNameTime;
 listView_Params.ItemNames[1]=ItemNameCurt;
 listView_Params.ItemNames[2]=ItemNameVolt;
 listView_Params.DataPointers[0]=(float*)(FLASH_CURRENT_ADDR);
 listView_Params.DataPointers[1]=(float*)(FLASH_VOLTAGE_ADDR);
 listView_Params.sprintfCommandStrings[0]=sprintfCommandCurt;
 listView_Params.sprintfCommandStrings[1]=sprintfCommandVolt;	
 listView_Params.ItemNum=3;
 listView_Params.ItemPositions[0] = 2;
 listView_Params.ItemPositions[1] = 30;
 listView_Params.ItemPositions[2] = 81;
 listView_Params.ItemPositions[3] = 127;
 listView_Params.DefaultPos = 0;
 listView_Params.FastSpeed=25;
 listView_Params.Item1AutoNum=true;
 listView_Params.Item1AutoNumStart=0;
 listView_Params.Item1AutoNumStep=2;
 listView_Params.ListLength=time/2;
// sprintf(tempString,"%f",*(float*)(FLASH_CURRENT_ADDR));
// OLED_ShowString(0,0,tempString);
// while(1) vTaskDelay(100);
 UI_ListView_Init(&listView_Params);
 xQueueReceive(UI_ListViewMsg, &i, portMAX_DELAY);
}

void RunAStepUpTest()
{
	StepUpTestParamsStruct test_Params;
	StepUpTestStateStruct testInfo;
	test_Params.StartCurrent = 
	GetTestParam(StartCurrentGet_Str[CurrentSettings->Language],100,STEP_UP_TEST_CURRENT_MAX,
	1000,100,"mA",20);
	test_Params.StopCurrent = 
	GetTestParam(EndCurrentGet_Str[CurrentSettings->Language],100,STEP_UP_TEST_CURRENT_MAX,
	2000,100,"mA",20);
	test_Params.Step = GetTestParam(StepCurrentGet_Str[CurrentSettings->Language],100,500,
	100,100,"mA",10);
	test_Params.TimeInterval = 
	GetTestParam(TimeIntervalGet_Str[CurrentSettings->Language],2,30,
	4,2,"s",10);
	test_Params.ProtectVolt = 
	GetTestParam(ProtVoltageGet_Str[CurrentSettings->Language],0,20000,
	900*CurrentMeterData.Voltage/10*10,10,"mV",25);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
	StepUpTest_Init(&test_Params);
	StepUpTest_UI_Init();
	xQueueReceive(StepUpTest_UI_Test_DoneMsg, &testInfo, portMAX_DELAY);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
	if (testInfo.TestOverFlag==1)
		ShowSmallDialogue("Test Done!", 1000, true);
	else
		ShowSmallDialogue("Protected!", 1000, true);
	ShowStepUpTestResult(testInfo.CurrentTime);
}

