//File Name   StepUpTest.c
//Description StepupTest

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#include "FreeRTOS_Standard_Include.h"

#include "SSD1306.h"

#include "MCP3421.h"

#include "UI_Dialogue.h"
#include "UI_Adjust.h"
#include "UI_ListView.h"
#include "UI_Dialgram.h"
#include "UI_Confirmation.h"
#include "UI_ProgressBar.h"

#include "Cooling_Fan.h"

#include "MultiLanguageStrings.h"

#include "Settings.h"

#include "Digital_Load.h"

#include "StepUpTest.h"

/*Step-up test memory mapping*/
#define FLASH_CACHE_START_ADDR 0x0807a000
#define FLASH_VOLTAGE_ADDR  FLASH_CACHE_START_ADDR
#define FLASH_CURRENT_ADDR  FLASH_CACHE_START_ADDR+6144     
#define FLASH_DATAEND_ADDR  FLASH_CACHE_START_ADDR+12288
#define UPDATE_RATE_TIME_PER_RECORD 1

#define CURRENT_MAX 4000

/*Step-up test params*/
//#define STEP_UP_TEST_INTERVAL_MAX 30
//#define STEP_UP_TEST_CURRENT_MAX CURRENT_MAX

#define STEPUPTEST_HANDLER_PRIORITY tskIDLE_PRIORITY+6

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

	vPortEnterCritical();
	EraseCacheBlocks();
	vPortExitCritical();
	Send_Digital_Load_Command(test_Params->StartCurrent, Load_Start);
	vTaskDelay(2000 / portTICK_RATE_MS);
	currentState.TestOverFlag = 0;
	currentState.CurrentTime = 0;
	for (;;)
	{
    vTaskDelay(1000*UPDATE_RATE_TIME_PER_RECORD/portTICK_RATE_MS);
    xSemaphoreTake(USBMeterState_Mutex, portMAX_DELAY);
		xQueueSend(StepUpTest_UI_UpdateMsg, &currentState, 0);
		Flash_ProgramFloat(FLASH_VOLTAGE_ADDR + currentAddr * sizeof(float), CurrentMeterData.Voltage);
		Flash_ProgramFloat(FLASH_CURRENT_ADDR + currentAddr * sizeof(float), CurrentMeterData.Current);
    xSemaphoreGive(USBMeterState_Mutex);
    currentState.CurrentTime=currentState.CurrentTime+UPDATE_RATE_TIME_PER_RECORD;
		currentAddr++;
		currentCurrent = (currentState.CurrentTime / test_Params->TimeInterval)*test_Params->Step + test_Params->StartCurrent;
		if ((CurrentMeterData.Voltage < (float)test_Params->ProtectVolt / 1000) ||
			(CurrentMeterData.Voltage < 0.5) ||
			(currentCurrent > test_Params->StopCurrent))
		{
			Send_Digital_Load_Command(0, Load_Stop);
			if (currentCurrent > test_Params->StopCurrent)
				currentState.TestOverFlag = 1;
			else
				currentState.TestOverFlag = 2;
			xQueueSend(StepUpTest_UI_UpdateMsg, &currentState, 0);
			vTaskDelete(NULL);
		}
		if (currentCurrent != lastCurrent)
		{
			Send_Digital_Load_Command(currentCurrent, Load_Keep);
			lastCurrent = currentCurrent;
		}
	}
}

void StepUpTestOnTimeUI(u16 timeNow);

void StepUpTest_UI_Handler(void *pvParameters)
{
	StepUpTestStateStruct currentState;
	u16 testTime = *(u16*)pvParameters;
	float progressTime;
	ProgressBar_Param_Struct progressBar_Params;
	progressBar_Params.Pos1.x = 1;
	progressBar_Params.Pos1.y = 1;
	progressBar_Params.Pos2.x = 126;
	progressBar_Params.Pos2.y = 13;
	progressBar_Params.MinValue = 0;
	progressBar_Params.MaxValue = testTime;
	currentState.CurrentTime = 0;
	ShowDialogue(StepUpTestRunning_Str[CurrentSettings->Language], "", "",false,false);
	UI_ProgressBar_Init(&progressBar_Params);
	StepUpTestOnTimeUI(0);
	for (;;)
	{
		xQueueReceive(StepUpTest_UI_UpdateMsg, &currentState, portMAX_DELAY);
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		StepUpTestOnTimeUI(currentState.CurrentTime);
		xSemaphoreGive(OLEDRelatedMutex);
		progressTime = currentState.CurrentTime;
		if (currentState.CurrentTime != 0)
		{
			xQueueSend(UI_ProgressBarMsg, &progressTime, portMAX_DELAY);
			xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
			StepUpTestOnTimeUI(currentState.CurrentTime + 1);
			xSemaphoreGive(OLEDRelatedMutex);
			progressTime++;
			xQueueSend(UI_ProgressBarMsg, &progressTime, portMAX_DELAY);
		}
		if (currentState.TestOverFlag > 0)
		{
			progressTime = -100;
			xQueueSend(UI_ProgressBarMsg, &progressTime, portMAX_DELAY);
			UI_ProgressBar_DeInit();
			xQueueSend(StepUpTest_UI_Test_DoneMsg, &currentState, portMAX_DELAY);
			vTaskDelete(NULL);
		}
	}
}

void StepUpTestOnTimeUI(u16 timeNow)
{
	char tempString[10];
	OLED_ShowAnyString(5, 15, RunTime_Str[CurrentSettings->Language], NotOnSelect, 16);
	sprintf(tempString, "%3ds", timeNow);
	OLED_ShowAnyString(88, 15, tempString, NotOnSelect, 16);
	OLED_ShowAnyString(5, 31, CurrentVoltage_Str[CurrentSettings->Language], NotOnSelect, 16);
	if (CurrentMeterData.Voltage < 10)
		sprintf(tempString, "%5.3fV", CurrentMeterData.Voltage);
	else
		sprintf(tempString, "%5.2fV", CurrentMeterData.Voltage);
	OLED_ShowAnyString(72, 31, tempString, NotOnSelect, 16);
	OLED_ShowAnyString(5, 47, CurrentCurrent_Str[CurrentSettings->Language], NotOnSelect, 16);
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
	CreateTaskWithExceptionControl(StepUpTest_Handler, "Stepup test Handler",
		256, test_Params, STEPUPTEST_HANDLER_PRIORITY, NULL);
}

/**
  * @brief  Init user interface for stepup test

	@param  test_Params:Step up test parameters

  * @retval None
  */
void StepUpTest_UI_Init(u16* testTime)
{
	StepUpTest_UI_Test_DoneMsg = xQueueCreate(1, sizeof(StepUpTestStateStruct));
	CreateTaskWithExceptionControl(StepUpTest_UI_Handler, "Stepup test UI Handler",
		160, testTime, STEPUPTEST_HANDLER_PRIORITY, NULL);
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


int GetTestParam(const char askString[], int min, int max, int defaultValue, int step, char unitString[], u8 fastSpeed)
{
	int t;
	UI_Adjust_Param_Struct adjust_params;
  retry:
	OLED_Clear_With_Mutex_TakeGive();
	adjust_params.AskString = (char *)askString;
	adjust_params.Min = min;
	adjust_params.Max = max;
	adjust_params.Step = step;
	adjust_params.DefaultValue = defaultValue;
	adjust_params.UnitString = unitString;
	adjust_params.FastSpeed = fastSpeed;
	adjust_params.Pos_y = 33;
	UI_Adjust_Init(&adjust_params);
	xQueueReceive(UI_AdjustMsg, &t, portMAX_DELAY);
	UI_Adjust_DeInit();
  if(t<min)
  {
   if(GetConfirmation(AbortConfirmation_Str[CurrentSettings->Language],""))
    {
     return -1;
    }
   goto retry;
  }
	return t;
}

/**
  * @brief  Show Step-up test result in listView
  */
bool ShowStepUpTestResultInListView(u16 time)
{
	ListView_Param_Struct listView_Params;
	u16 i;
	char *ItemNameTime = (char *)StepUpTestDialgramTime_Str[CurrentSettings->Language];
	char *ItemNameCurt = (char *)StepUpTestDialgramCurrent_Str[CurrentSettings->Language];
	char *ItemNameVolt = (char *)StepUpTestDialgramVoltage_Str[CurrentSettings->Language];
	char sprintfCommandCurt[] = "%0.3f";
	char sprintfCommandVolt[] = "%0.3f";
	listView_Params.ItemNames[0] = ItemNameTime;
	listView_Params.ItemNames[1] = ItemNameCurt;
	listView_Params.ItemNames[2] = ItemNameVolt;
	listView_Params.DataPointers[0] = (float*)(FLASH_CURRENT_ADDR);
	listView_Params.DataPointers[1] = (float*)(FLASH_VOLTAGE_ADDR);
	listView_Params.sprintfCommandStrings[0] = sprintfCommandCurt;
	listView_Params.sprintfCommandStrings[1] = sprintfCommandVolt;
	listView_Params.ItemNum = 3;
	listView_Params.ItemPositions[0] = 2;
	listView_Params.ItemPositions[1] = 30;
	listView_Params.ItemPositions[2] = 81;
	listView_Params.ItemPositions[3] = 127;
	listView_Params.DefaultPos = 0;
	listView_Params.FastSpeed = 25;
	listView_Params.Item1AutoNum = true;
	listView_Params.Item1AutoNumStart = 0;
	listView_Params.Item1AutoNumStep = UPDATE_RATE_TIME_PER_RECORD;
	listView_Params.ListLength = time / UPDATE_RATE_TIME_PER_RECORD;
	UI_ListView_Init(&listView_Params);
	xQueueReceive(UI_ListViewMsg, &i, portMAX_DELAY);
	UI_ListView_DeInit();
	if (i == 32767) return true;
	else return false;
}

/**
  * @brief  Find maximun value in given dataSet

	@params dataSet:Target dataSet

			dataLength:Search-length in dataSet

  * @retval The max value in given dataSet
  */
float FindMax(float *dataSet, u16 dataLength)
{
	u16 i;
	float max;
	max = dataSet[0];
	for (i = 1; i < dataLength; i++)
	{
		if (dataSet[i] > max) max = dataSet[i];
	}
	return(max);
}

/**
  * @brief  Find minimum value in given dataSet

	@params dataSet:Target dataSet

			dataLength:Search-length in dataSet

  * @retval The min value in given dataSet
  */
float FindMin(float *dataSet, u16 dataLength)
{
	u16 i;
	float min;
	min = dataSet[0];
	for (i = 1; i < dataLength; i++)
	{
		if (dataSet[i] < min) min = dataSet[i];
	}
	return(min);
}

/**
  * @brief  Show Step-up test result in dialgram
  */
bool ShowStepUpTestResultInDialgram(u16 time)
{
	Dialgram_Param_Struct dialgram_Params;
	u8 i, temp;

	/*Set dialgram*/
	dialgram_Params.DataPointers[0] = (float*)(FLASH_VOLTAGE_ADDR);
	dialgram_Params.DataPointers[1] = (float*)(FLASH_CURRENT_ADDR);
	dialgram_Params.MaxAndMinSprintfCommandStrings[0] = "%0.1fV";
	dialgram_Params.MaxAndMinSprintfCommandStrings[1] = "%0.1fA";
	dialgram_Params.DataNumSprintfCommandString = "t=%03ds";
	dialgram_Params.DataSprintfCommandStrings[0] = "%0.3fV";
	dialgram_Params.DataSprintfCommandStrings[1] = "%0.3fA";
	dialgram_Params.RecordLength = time / 1;
	dialgram_Params.MaxValues[0] = FindMax((float*)(FLASH_VOLTAGE_ADDR), 
		time / 1);
	dialgram_Params.MinValues[0] = FindMin((float*)(FLASH_VOLTAGE_ADDR), 
		time / 1);
	dialgram_Params.MaxValues[1] = FindMax((float*)(FLASH_CURRENT_ADDR),
		time / 1);
	dialgram_Params.MinValues[1] = FindMin((float*)(FLASH_CURRENT_ADDR), 
		time / 1);
	dialgram_Params.Item1AutoNumStart = 0;
	dialgram_Params.Item1AutoNumStep = 1;

	/*Calculate the premium max/min values for display*/
	for (i = 0; i < 2; i++)
	{
		temp = (dialgram_Params.MaxValues[i] - dialgram_Params.MinValues[i]) * 10;
		temp = temp + 2;
		dialgram_Params.MinValues[i] = (float)((u16)(dialgram_Params.MinValues[i] * 10)) / 10;
		dialgram_Params.MaxValues[i] = dialgram_Params.MinValues[i] + (float)temp*0.1;
	}

	/*Init dialgram*/
	UI_Dialgram_Init(&dialgram_Params);

	/*Keep blocked until exit*/
	xQueueReceive(UI_DialogueMsg, &i, portMAX_DELAY);
	UI_Dialgram_DeInit();

	if (i) return true;
	else return false;
}

void ShowStepUpTestResult(u16 time)
{
	bool exit;
	for (;;)
	{
	ListView:
		exit = ShowStepUpTestResultInListView(time - 1);
		if (exit == true)
			if (GetConfirmation(StepUpTestExitBroswer_Str[CurrentSettings->Language], "")) return;
			else goto ListView;
			ShowSmallDialogue(StepUpTestDialgram_Str[CurrentSettings->Language], 800, true);
		Dialgram:
			exit = ShowStepUpTestResultInDialgram(time - 1);
			if (exit == true)
				if (GetConfirmation(StepUpTestExitBroswer_Str[CurrentSettings->Language], "")) return;
				else goto Dialgram;
				ShowSmallDialogue(StepUpTestList_Str[CurrentSettings->Language], 800, true);
	}
}

void RunAStepUpTest()
{
	StepUpTestParamsStruct test_Params;
	StepUpTestStateStruct testInfo;
	u16 testTime;

	/*Get neccesary params*/
	test_Params.StartCurrent =
		GetTestParam(StartCurrentGet_Str[CurrentSettings->Language], 100, 
	  CURRENT_MAX-100,
			1000, 100, "mA", 20);
  if(test_Params.StartCurrent<0) goto clear;
	test_Params.Step = GetTestParam(StepCurrentGet_Str[CurrentSettings->Language], 100,
		CURRENT_MAX-test_Params.StartCurrent>500?500:CURRENT_MAX-test_Params.StartCurrent,
		100, 100, "mA", 10);
  if(test_Params.Step<0) goto clear;
	test_Params.StopCurrent =
		GetTestParam(EndCurrentGet_Str[CurrentSettings->Language], test_Params.StartCurrent + test_Params.Step , 
	  CURRENT_MAX,
		test_Params.StartCurrent + test_Params.Step, test_Params.Step, "mA", 20);
  if(test_Params.StopCurrent<0) goto clear;
	test_Params.TimeInterval =
		GetTestParam(TimeIntervalGet_Str[CurrentSettings->Language], 2, 30,
			4, 2, "s", 10);
  if(test_Params.TimeInterval<0) goto clear;
	test_Params.ProtectVolt =
		GetTestParam(ProtVoltageGet_Str[CurrentSettings->Language], 0,
		(int)(100 * CurrentMeterData.Voltage) * 10 > 0 ? (int)(100 * CurrentMeterData.Voltage) * 10 : 100,
			(int)(90 * CurrentMeterData.Voltage) * 10 > 0 ? (int)(90 * CurrentMeterData.Voltage) * 10 : 100, 10, "mV", 25);
  if(test_Params.ProtectVolt<0) goto clear;
	testTime = ((test_Params.StopCurrent - test_Params.StartCurrent) / test_Params.Step + 1)*test_Params.TimeInterval;

	/*Clear the screen*/
	OLED_Clear_With_Mutex_TakeGive();

  Fan_Send_Command(Turn_On);

	/*Init StepUpTest Tasks*/
	StepUpTest_Init(&test_Params);
	StepUpTest_UI_Init(&testTime);

	/*Keep blocked until test done*/
	xQueueReceive(StepUpTest_UI_Test_DoneMsg, &testInfo, portMAX_DELAY);

	/*DeInit StepUpTest queues*/
	vQueueDelete(StepUpTest_UI_UpdateMsg);
	vQueueDelete(StepUpTest_UI_Test_DoneMsg);

	/*Clear the screen*/
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);

  Fan_Send_Command(Auto);

	/*Show the reason for the end of the test*/
	if (testInfo.TestOverFlag == 1)
		ShowSmallDialogue(StepUpTestDone_Str[CurrentSettings->Language], 1000, true);
	else
		ShowSmallDialogue(StepUpTestProtected_Str[CurrentSettings->Language], 1000, true);

	/*Show test result*/
	ShowStepUpTestResult(testInfo.CurrentTime);

  clear:
	/*Clear the screen*/
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
}

