//File Name   StepUpTest.c
//Description StepupTest

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "SSD1306.h"
#include "EBProtocol.h"

#include "UI_Dialogue.h"

#include "StepUpTest.h"

/*Step-up test memory mapping*/
#define FLASH_CACHE_START_ADDR 0x0801dc00
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
	u16 currentTime = 0;
	u16 currentAddr = 0;
	u16 currentCurrent;
	u16 lastCurrent = test_Params->StartCurrent;
	u8 i;
	EBD_Sync();
	EraseCacheBlocks();
	vTaskDelay(200 / portTICK_RATE_MS);
	EBDSendLoadCommand(0, StartTest);
	vTaskDelay(200 / portTICK_RATE_MS);
	EBDSendLoadCommand(test_Params->StartCurrent, KeepTest);
	EBD_Sync();
	for (;;)
	{
		xQueueReceive(EBDRxDataMsg, &i, portMAX_DELAY);
		Flash_ProgramFloat(FLASH_VOLTAGE_ADDR + currentAddr * sizeof(float), CurrentMeterData.Voltage);
		Flash_ProgramFloat(FLASH_CURRENT_ADDR + currentAddr * sizeof(float), CurrentMeterData.Current);
		xQueueSend(StepUpTest_UI_UpdateMsg, &currentTime, 0);
		currentTime = currentTime + 2;
		currentAddr++;
		currentCurrent = (currentTime / test_Params->TimeInterval)*test_Params->Step + test_Params->StartCurrent;
		if ((CurrentMeterData.Voltage < (float)test_Params->ProtectVolt / 1000) ||
			(CurrentMeterData.Voltage < 0.5) ||
			(currentCurrent > test_Params->StopCurrent))
		{
			vTaskDelay(200 / portTICK_RATE_MS);
			EBDSendLoadCommand(0, StopTest);
			if (currentCurrent > test_Params->StopCurrent)
				currentTime = 65535;
			else
				currentTime = 65534;
			xQueueSend(StepUpTest_UI_UpdateMsg, &currentTime, 0);
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
	u16 currentTime = 0;
	ShowDialogue("Test Running...", "", "");
	for (;;)
	{
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		StepUpTestOnTimeUI(currentTime);
		xSemaphoreGive(OLEDRelatedMutex);
		if (currentTime != 0)
		{
			vTaskDelay(1000 / portTICK_RATE_MS);
			xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
			StepUpTestOnTimeUI(currentTime + 1);
			xSemaphoreGive(OLEDRelatedMutex);
		}
		xQueueReceive(StepUpTest_UI_UpdateMsg, &currentTime, portMAX_DELAY);
		if (currentTime == 65535 || currentTime == 65534)
		{
			xQueueSend(StepUpTest_UI_Test_DoneMsg, &currentTime, 0);
			vTaskDelete(NULL);
		}
	}
}

void StepUpTestOnTimeUI(u16 timeNow)
{
	char tempString[10];
	OLED_ShowAnyString(5, 15, "RunTime:", NotOnSelect, 16);
	sprintf(tempString, "%3ds", timeNow);
	OLED_ShowAnyString(88, 15, tempString, NotOnSelect, 16);
	OLED_ShowAnyString(5, 31, "Voltage:", NotOnSelect, 16);
	if (CurrentMeterData.Voltage < 10)
		sprintf(tempString, "%5.3fV", CurrentMeterData.Voltage);
	else
		sprintf(tempString, "%5.2fV", CurrentMeterData.Voltage);
	OLED_ShowAnyString(72, 31, tempString, NotOnSelect, 16);
	OLED_ShowAnyString(5, 47, "Current:", NotOnSelect, 16);
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
	StepUpTest_UI_UpdateMsg = xQueueCreate(2, sizeof(u16));
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
	StepUpTest_UI_Test_DoneMsg = xQueueCreate(1, sizeof(u16));
	xTaskCreate(StepUpTest_UI_Handler, "Stepup test UI Handler",
		128, NULL, STEPUPTEST_HANDLER_PRIORITY, NULL);
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

void RunAStepUpTest()
{
	StepUpTestParamsStruct test_Params;
	u16 i;
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
	test_Params.StartCurrent = 100;
	test_Params.StopCurrent = 1000;
	test_Params.Step = 100;
	test_Params.TimeInterval = 10;
	test_Params.ProtectVolt = 1000;
	StepUpTest_Init(&test_Params);
	StepUpTest_UI_Init();
	xQueueReceive(StepUpTest_UI_Test_DoneMsg, &i, portMAX_DELAY);
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
	if (i == 65535)
		ShowSmallDialogue("Test Done!", 1000, true);
	else
		ShowSmallDialogue("Protected!", 1000, true);
}

