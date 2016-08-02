//File Name   FastCharge_trigger.c
//Description FastCharge trigger

#include <stdio.h>
#include <string.h>

#include "stm32f10x.h"

#include "FreeRTOS_Standard_Include.h"

#include "SSD1306.h"
#include "Keys.h"
#include "MCP3421.h"

#include "UI_Dialogue.h"
#include "UI_Button.h"
#include "UI_Menu.h"

#include "FastCharge_Trigger_Circuit.h"

#include "FastCharge_Trigger.h"

typedef struct {
	u8 Plus;
	u8 Minus;
}FastChargeAdjustCommandSetStruct;

/**
  * @brief  Show current voltage and current

  * @retval None
  */
void ShowCurrentVoltCurt()
{
	char tempString[10];
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	//SetUpdateOLEDJustNow();
	xSemaphoreTake(USBMeterState_Mutex, portMAX_DELAY);
	if (CurrentMeterData.Voltage >= 10)
	{
		sprintf(tempString, "%5.2fV", CurrentMeterData.Voltage);
	}
	else
	{
		sprintf(tempString, "%5.3fV", CurrentMeterData.Voltage);
	}
	OLED_ShowAnyString(68, 20, tempString, NotOnSelect, 16);
	sprintf(tempString, "%5.3fA", CurrentMeterData.Current);
	xSemaphoreGive(USBMeterState_Mutex);
	OLED_ShowAnyString(68, 40, tempString, NotOnSelect, 16);
	//ResetUpdateOLEDJustNow();
	xSemaphoreGive(OLEDRelatedMutex);
}

/**
  * @brief  UserInterface of QC2.0 trigger

  * @retval None
  */
void QC2Trigger_Init(void)
{
	u8 i;
	u8 mode;
	
	Key_Message_Struct keyMsg;
	UI_Button_Param_Struct button_params;
	const char* voltStrings[4];
	OLED_PositionStruct voltagesPositions[4] = { {10,22},{36,22},{10,42},{36,42} };
	
	voltStrings[0]="5V";
	voltStrings[1]="9V";
	voltStrings[2]="12V";
	voltStrings[3]="20V";
	
	/*Set the positions for the voltage options*/	
	ShowDialogue("QC2.0 Mode", "", "");
	button_params.ButtonStrings = voltStrings;
	button_params.ButtonNum = 4;
	button_params.DefaultValue = 0;
	button_params.Positions = voltagesPositions;
	OLED_Refresh_Gram();
	for (;;)
	{
		UI_Button_Init(&button_params);
		for (;;)
		{
			ShowCurrentVoltCurt();
			if (xQueueReceive(UI_ButtonMsg, &i, 0) == pdPASS) break;
			vTaskDelay(50 / portTICK_RATE_MS);
		}
		UI_Button_DeInit();
		button_params.DefaultValue = i;
		switch (i)
		{
		case 0:mode=QC2_Normal;xQueueSend(FastCharge_Msg, &mode, 100 / portTICK_RATE_MS); break;
		case 1:mode=QC2_9V;xQueueSend(FastCharge_Msg, &mode, 100 / portTICK_RATE_MS); break;
		case 2:mode=QC2_12V;xQueueSend(FastCharge_Msg, &mode, 100 / portTICK_RATE_MS); break;
		case 3:mode=QC2_20V;xQueueSend(FastCharge_Msg, &mode, 100 / portTICK_RATE_MS); break;
		}
		vTaskDelay(200 / portTICK_RATE_MS);
		if (xQueueReceive(Key_Message, &keyMsg, 0) == pdPASS)
		{
			if (keyMsg.KeyEvent == MidDouble) return;
		}
	}
}

/**
  * @brief  USB trigger UI with only +/- Keys

  * @retval None
  */
void USBTriggerAdjustUI(char titleString[], FastChargeAdjustCommandSetStruct commandSet, OLED_PositionStruct positions[2])
{
	Key_Message_Struct keyMsg;
	//SetUpdateOLEDJustNow();
	ShowDialogue(titleString, "", "");
	/*Draw a button set with "LiveMode",get the timerNo allocated*/
	OLED_DrawRect(positions[0].x - 2, positions[0].y - 2, positions[0].x + 17, positions[0].y + 17, DRAW);
	OLED_DrawRect(positions[1].x - 2, positions[1].y - 2, positions[1].x + 17, positions[1].y + 17, DRAW);
	for (;;)
	{
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		OLED_ShowIcon(positions[0].x, positions[0].y, 0x00, DRAW);
		OLED_ShowIcon(positions[1].x, positions[1].y, 0x01, DRAW);
		xSemaphoreGive(OLEDRelatedMutex);
		for (;;)
		{
			ShowCurrentVoltCurt();
			if (LEFT_KEY == KEY_ON)
			{
				xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
				OLED_ShowIcon(positions[1].x, positions[1].y, 0x01, UNDRAW);
				xSemaphoreGive(OLEDRelatedMutex);
				while (LEFT_KEY == KEY_ON)
				{
				 vTaskDelay(20/portTICK_RATE_MS);
				 ShowCurrentVoltCurt();
				}
				xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
				OLED_ShowIcon(positions[1].x, positions[1].y, 0x01, DRAW);
				xSemaphoreGive(OLEDRelatedMutex);
				xQueueSend(FastCharge_Msg, &commandSet.Plus, 100 / portTICK_RATE_MS);
			}
			else if (RIGHT_KEY == KEY_ON)
			{
				xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
				OLED_ShowIcon(positions[0].x, positions[0].y, 0x00, UNDRAW);
				xSemaphoreGive(OLEDRelatedMutex);
				while (RIGHT_KEY == KEY_ON)
				{
				 vTaskDelay(20/portTICK_RATE_MS);
				 ShowCurrentVoltCurt();
				}
				xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
				OLED_ShowIcon(positions[0].x, positions[0].y, 0x00, DRAW);
				xSemaphoreGive(OLEDRelatedMutex);
				xQueueSend(FastCharge_Msg, &commandSet.Minus, 100 / portTICK_RATE_MS);
			}
			if (xQueueReceive(Key_Message, &keyMsg, 30/portTICK_RATE_MS) == pdPASS)
			{
				if (keyMsg.KeyEvent == MidDouble)
				{
					//ResetUpdateOLEDJustNow(); 
					return;
				}
			}
		}
	}
}

/**
  * @brief  UserInterface of MTK-PE trigger

  * @retval None
  */
void MTKTrigger_Init(void)
{
	OLED_PositionStruct positions[2] = { {40,28},{10,28} };
	FastChargeAdjustCommandSetStruct commandSet;
	commandSet.Minus = MTK_Decrease;
	commandSet.Plus = MTK_Increase;
	USBTriggerAdjustUI("MTK-PE Mode", commandSet, positions);
}

/**
  * @brief  UserInterface of QC3.0 trigger

  * @retval None
  */
void QC3Trigger_Init(void)
{
	OLED_PositionStruct positions[2] = { {40,28},{10,28} };
	FastChargeAdjustCommandSetStruct commandSet;
	commandSet.Minus = QC3_Decrease;
	commandSet.Plus = QC3_Increase;
	USBTriggerAdjustUI("QC3.0 Mode", commandSet, positions);
}


/**
  * @brief  UserInterface of fastCharge cheating

  * @retval None
  */
void FastChargeTriggerUI(void)
{
	u8 selection;
	const char* protocolTab[3];
	UI_Menu_Param_Struct menu_params;
	
	protocolTab[0]="QC2.0";
	protocolTab[1]="QC3.0";
	protocolTab[2]="MTK-PE";
	
	menu_params.ItemStrings = protocolTab;
	menu_params.DefaultPos = 0;
	menu_params.ItemNum = 3;
	menu_params.FastSpeed = 5;
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);

	UI_Menu_Init(&menu_params);

	xQueueReceive(UI_MenuMsg, &selection, portMAX_DELAY);
	UI_Menu_DeInit();

	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
	switch (selection)
	{
	case 0:QC2Trigger_Init(); break;
	case 1:QC3Trigger_Init(); break;
	case 2:MTKTrigger_Init();
	}
	UpdateOLEDJustNow = false;
	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	xSemaphoreGive(OLEDRelatedMutex);
}

