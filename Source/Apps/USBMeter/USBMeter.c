////File Name   USBMeterUI.c
//Description Main UI

#include <stdio.h>
#include <string.h>

#include "stm32f10x.h"

#include "FreeRTOS_Standard_Include.h"

#include "Digital_Load.h"
#include "Temperature_Sensors.h"
#include "SSD1306.h"
#include "RTC.h"
#include "ADC.h"
#include "MCP3421.h"
#include "VirtualRTC.h"
#include "UI_Confirmation.h"
#include "MultiLanguageStrings.h"
#include "Keys.h"
#include "sdcard.h"
#include "sdcardff.h"
#include "DataPin2Protocol.h"

#include "mass_mal.h"

#include "Settings.h"
#include "StepUpTest.h"
#include "FastCharge_Trigger.h"
#include "LegacyTest.h"
#include "MassStorage.h"
#include "ExceptionHandle.h"
#include "Digital_Clock.h"

#include "UI_Dialogue.h"
#include "UI_Utilities.h"

#include "USBMeter.h"

#define USB_METER_PRIORITY tskIDLE_PRIORITY+3

#define EFFECT_DATA_NUM_PER_SCREEN 127

float CurveBuff0[EFFECT_DATA_NUM_PER_SCREEN];
float CurveBuff1[EFFECT_DATA_NUM_PER_SCREEN];

enum { VoltDMDP, VoltCurt };

u16 Data_p = 0;

bool firstCycle = true;

static void DisplayBasicData(char tempString[], u8 currentStatus, u8 firstEnter);

static void DisplayRecordData(char tempString[]);

static void ScrollDialgram_Routine(void);

#define IDLE_TIME_SEC 5

#define DIGITAL_LOAD_POWER_MAX  Digital_Load_Params[CurrentSettings->Digital_Load_Params_Mode]->PowerMax

/**
  * @brief  USBMeter

  * @retval None
  */
static void USBMeter(void *pvParameters)
{
	char tempString[20];
	bool fastChargeStatus = false;
	bool overPowerExceptionHandledFlag = false;
	bool exceptionHandleRtval;
	u8 firstEnter = 1;
	Key_Message_Struct keyMessage;
	u8 status = USBMETER_ONLY;
	u8 reSendLoadCommandCnt = 0;
	u8 updateBasicDataCnt = 5;
	u32 lastWakeTime = xTaskGetTickCount();
	Legacy_Test_Param_Struct legacy_Test_Params;
	ClearKeyEvent();
	while (1)
	{
	Refresh:
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		xSemaphoreTake(USBMeterState_Mutex, portMAX_DELAY);
		if (updateBasicDataCnt == 5)
		{
			updateBasicDataCnt = 0;
			DisplayBasicData(tempString, status, firstEnter);
		}
		xSemaphoreGive(USBMeterState_Mutex);
		if (status == USBMETER_RECORD || status == LEGACY_TEST)
		{
			DisplayRecordData(tempString);
		}
		xSemaphoreGive(OLEDRelatedMutex);
		if ((MOSTemperature > CurrentSettings->Protect_Settings.InternalTemperature_Max ||
			ExternalTemperature > CurrentSettings->Protect_Settings.ExternalTemperature_Max) &&
			firstEnter == 0)
		{
			System_OverHeat_Exception_Handler(status, &legacy_Test_Params);
			lastWakeTime = xTaskGetTickCount();
			updateBasicDataCnt = 5; goto Refresh;
		}
		if (status == LEGACY_TEST
			&& ((FilteredMeterData.Power*1000 > DIGITAL_LOAD_POWER_MAX + 500 && overPowerExceptionHandledFlag == false)
				|| (FilteredMeterData.Power*1000 > DIGITAL_LOAD_POWER_MAX + 5000 && overPowerExceptionHandledFlag == true)))
		{
			if (overPowerExceptionHandledFlag == true)
			{
				System_OverPower_Exception_Handler(status, &legacy_Test_Params);
				ClearKeyEvent();
        StopRecord(&status, 0);
				lastWakeTime = xTaskGetTickCount();
				updateBasicDataCnt = 5;
				overPowerExceptionHandledFlag = false;
				goto Refresh;
			}
			exceptionHandleRtval = System_OverPower_Exception_Handler(status, &legacy_Test_Params);
			overPowerExceptionHandledFlag = true;
			if (exceptionHandleRtval)
			{
				ClearKeyEvent();
				lastWakeTime = xTaskGetTickCount();
				updateBasicDataCnt = 5;
				goto Refresh;
			}
			else
			{
				StopRecord(&status, 0);
        overPowerExceptionHandledFlag = false;
				ClearKeyEvent();
				lastWakeTime = xTaskGetTickCount();
				updateBasicDataCnt = 5;
				goto Refresh;
			}
		}
		if (status == LEGACY_TEST)
		{
			if (CurrentMeterData.Voltage * 1000 < legacy_Test_Params.ProtectVolt ||
				CurrentMeterData.Voltage < 0.5)
			{
				StopRecord(&status, 1);
				updateBasicDataCnt = 5;
				lastWakeTime = xTaskGetTickCount();
				goto Refresh;
			}
			if (legacy_Test_Params.TestMode == ConstantPower)
			{
				reSendLoadCommandCnt++;
				if (reSendLoadCommandCnt == 50)
				{
					if (CurrentMeterData.Current < 0.1)
						Send_Digital_Load_Command((float)legacy_Test_Params.Power / CurrentMeterData.Voltage, Load_Start);
					else
						Send_Digital_Load_Command((float)legacy_Test_Params.Power / CurrentMeterData.Voltage, Load_Keep);
					reSendLoadCommandCnt = 0;
				}
			}
		}
		if (firstEnter) firstEnter--;
		//    sprintf(tempString,"%d",ADCConvertedValue[3]);
		//    OLED_ShowAnyString(0,0,tempString,NotOnSelect,16);
		if (xQueueReceive(Key_Message, &keyMessage, 100 / portTICK_RATE_MS) == pdPASS)
		{
			if (status == USBMETER_ONLY)
			{
				switch (keyMessage.KeyEvent)
				{
				case MidClick: ScrollDialgram_Routine(); break;
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
				case LeftContinous:
					if (!fastChargeStatus)
					{
						if (GetConfirmation(QCMTKConfirm_Str[CurrentSettings->Language], ""))
							FastChargeTriggerUI(&fastChargeStatus);
					}
					else
					{
						if (GetConfirmation(ReleaseFastCharge_Str[CurrentSettings->Language], ""))
							ReleaseFastCharge(&fastChargeStatus);
					} break;
				case RightContinous:if
					(GetConfirmation(MountUSBMassStorageConfirm_Str[CurrentSettings->Language], ""))
					MassStorage_App(); break;
				}
				ClearKeyEvent();
				lastWakeTime = xTaskGetTickCount();
				updateBasicDataCnt = 5;
				goto Refresh;
			}
			else
			{
				OLED_Clear_With_Mutex_TakeGive();
				if (keyMessage.KeyEvent == MidDouble)
				{

					if (GetConfirmation(RecordStopConfirm_Str[CurrentSettings->Language], ""))
					{
						StopRecord(&status, 0);
					}
				}
				//				else if (keyMessage.KeyEvent == MidClick)
				//				{
				//					ScrollDialgram_Routine();
				//				}
				else
				{
					ShowDialogue(Hint_Str[CurrentSettings->Language],
						RecordIsRunningHint1_Str[CurrentSettings->Language],
						RecordIsRunningHint2_Str[CurrentSettings->Language], false, false);
					vTaskDelay(1000 / portTICK_RATE_MS);
					OLED_Clear_With_Mutex_TakeGive();
				}
				ClearKeyEvent();
				lastWakeTime = xTaskGetTickCount();
				updateBasicDataCnt = 5;
				goto Refresh;
			}
		}
		if (status == USBMETER_RECORD || status == LEGACY_TEST || FilteredMeterData.Current > 0.01)
		{
			lastWakeTime = xTaskGetTickCount();
		}
		if ((xTaskGetTickCount() - lastWakeTime)*portTICK_RATE_MS / 1000 > CurrentSettings->Idle_Clock_Settings.IdleTime
			&&CurrentSettings->Idle_Clock_Settings.ClockEnable == true)
		{
			Digital_Clock();
			OLED_Clear_With_Mutex_TakeGive();
			updateBasicDataCnt = 4;
			lastWakeTime = xTaskGetTickCount();
		}
		updateBasicDataCnt++;
	}
}


/**
  * @brief  Write a new dataset to curve buffer

  * @retval None
  */
void WriteCurrentMeterData2CurveBuff(u8 mode)
{
	/*Increase data pos*/
	Data_p++;

	/*Reset data pos to 0 and clear first cycle flag*/
	if (Data_p == EFFECT_DATA_NUM_PER_SCREEN)
	{
		firstCycle = false;
		Data_p = 0;
	}

	/*Especially,the first data of the forst cycle should also apply to pos 0*/
	if (Data_p == 1 && firstCycle)
	{
		switch (mode)
		{
		case VoltCurt:
			CurveBuff0[0] = FilteredMeterData.Voltage;
			CurveBuff1[0] = FilteredMeterData.Current; break;
		case VoltDMDP:
			CurveBuff0[0] = CurrentMeterData.VoltageDP;
			CurveBuff1[0] = CurrentMeterData.VoltageDM;
		}

	}
	switch (mode)
	{
	case VoltCurt:
		CurveBuff0[Data_p] = FilteredMeterData.Voltage;
		CurveBuff1[Data_p] = FilteredMeterData.Current; break;
	case VoltDMDP:
		CurveBuff0[Data_p] = CurrentMeterData.VoltageDP;
		CurveBuff1[Data_p] = CurrentMeterData.VoltageDM;
	}
}

/**
  * @brief  Find the num th data in dataset according to the pos of Data_p

  * @retval None
  */
static float FindxthData(float* data, u16 num)
{
	u16 p;
	p = num + Data_p + 1;
	if (p >= EFFECT_DATA_NUM_PER_SCREEN)
		p = p - EFFECT_DATA_NUM_PER_SCREEN;
	if (firstCycle) p = num;
	return data[p];
}

/**
  * @brief  Draw a single curve in curve buffer

  * @retval None
  */
static void DrawSingleCurveInCurveBuff(float* data, float min, float max)
{
	u8 pixelsPerData = 128 / EFFECT_DATA_NUM_PER_SCREEN;
	u8 currentAddr = 0;
	float currentPosData, nextPosData;
	u8 y1, y2;
	u16 i;

	/*Especially for the first cycle,the effective data length in curve buffers
  should be assigned to Data_p*/
	int numMax = (firstCycle == true) ? (Data_p) : EFFECT_DATA_NUM_PER_SCREEN - 1;

	for (i = 0; i < numMax; i++)
	{
		currentPosData = FindxthData(data, i);
		nextPosData = FindxthData(data, i + 1);
		y1 = 3 + (u8)
			((float)55 * ((max - currentPosData) /
			(float)(max - min)));
		y2 = 3 + (u8)
			((float)55 * ((max - nextPosData) /
			(float)(max - min)));
		OLED_DrawAnyLine(currentAddr, y1, currentAddr + pixelsPerData, y2, DRAW);
		currentAddr = currentAddr + pixelsPerData;
	}

}

/**
  * @brief  Draw max and min indicator and grids

  * @retval None
  */
static void DrawDialgramMinAndMaxAndGrids(float voltMin, float voltMax, float curtMin, float curtMax, bool drawMinAndMax, u8 mode)
{
	char tempString[10];
	u8 length;
	u8 i;

	/*Draw horizonal grids*/
	for (i = 0; i < 6; i++)
	{
		DrawHorizonalDashedGrid(3 + i * 11, DRAW, LowDensity);
	}

	if (drawMinAndMax)
	{
		switch (mode)
		{
		case VoltCurt:
			sprintf(tempString, "%.1fV", voltMax);
			OLED_ShowAnyString(0, 0, tempString, NotOnSelect, 8);
			sprintf(tempString, "%.1fV", voltMin);
			OLED_ShowAnyString(0, 56, tempString, NotOnSelect, 8);

			sprintf(tempString, "%.1fA", curtMax);
			length = GetStringGraphicalLength(tempString);
			OLED_ShowAnyString(127 - 6 * length, 0, tempString, NotOnSelect, 8);

			sprintf(tempString, "%.1fA", curtMin);
			length = GetStringGraphicalLength(tempString);
			OLED_ShowAnyString(127 - 6 * length, 56, tempString, NotOnSelect, 8);
			break;
		case VoltDMDP:
			sprintf(tempString, "%.1fV", voltMax);
			OLED_ShowAnyString(0, 0, tempString, NotOnSelect, 8);
			sprintf(tempString, "%.1fV", voltMin);
			OLED_ShowAnyString(0, 56, tempString, NotOnSelect, 8);
		}
	}
}

/**
  * @brief  Refresh dynamic diagram

  * @retval None
  */
static void RefreshDialgram(u8 mode)
{
	u16 temp;
	float currentMin, currentMax, voltageMin, voltageMax;
	u16 effectiveDataNum;
	if (firstCycle == true) effectiveDataNum = Data_p;
	else effectiveDataNum = EFFECT_DATA_NUM_PER_SCREEN;

	/*Find max and min in current and voltage buffer*/
	currentMin = FindMin(CurveBuff1, effectiveDataNum);
	voltageMin = FindMin(CurveBuff0, effectiveDataNum);
	currentMax = FindMax(CurveBuff1, effectiveDataNum);
	voltageMax = FindMax(CurveBuff0, effectiveDataNum);

	/*Make max and min into n*0.1 for a better display*/
	temp = (currentMax - currentMin) * 10;
	temp = temp + 2;
	currentMin = (float)((u16)(currentMin * 10)) / 10;
	currentMax = currentMin + (float)temp*0.1;
	temp = (voltageMax - voltageMin) * 10;
	temp = temp + 2;
	voltageMin = (float)((u16)(voltageMin * 10)) / 10;
	voltageMax = voltageMin + (float)temp*0.1;

	switch (mode)
	{
	case VoltDMDP:
		voltageMin = 0;
		currentMin = 0;
		voltageMax = 3.3;
		currentMax = 3.3;
		break;
	case VoltCurt:
		/*Fixed max and min code,remove for better dynamic display*/
		voltageMin = 0;
		currentMin = 0;
		voltageMax = voltageMax > 12 ? voltageMax : 12;
		currentMax = currentMax > 2 ? currentMax : 2;
	}

	xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
	OLED_Clear();
	DrawSingleCurveInCurveBuff(CurveBuff0, voltageMin, voltageMax);
	DrawSingleCurveInCurveBuff(CurveBuff1, currentMin, currentMax);
	DrawDialgramMinAndMaxAndGrids(voltageMin, voltageMax, currentMin, currentMax, true, mode);
	OLED_Refresh_Gram();
	xSemaphoreGive(OLEDRelatedMutex);
}

/**
  * @brief  Scroll dialgram mode

  * @retval None
  */
static void ScrollDialgram_Routine()
{
	Key_Message_Struct keyMessage;
	u8 mode = VoltCurt;

	/*Initialize Data_p and make firstcycle flag*/
	Data_p = 0;
	firstCycle = true;

	for (;;)
	{
		WriteCurrentMeterData2CurveBuff(mode);
		RefreshDialgram(mode);
		if (xQueueReceive(Key_Message, &keyMessage, 100 / portTICK_RATE_MS) == pdPASS)
		{
			if (keyMessage.KeyEvent == MidClick)
			{
				OLED_Clear_With_Mutex_TakeGive();
				return;
			}
			else if (keyMessage.KeyEvent == MidLong)
			{
				if (mode == VoltCurt)
				{
					Data_p = 0;
					firstCycle = true;
					mode = VoltDMDP;
				}
				else
				{
					Data_p = 0;
					firstCycle = true;
					mode = VoltCurt;
				}
			}
		}
	}

}

#define PROTOCOL_DISPLAY_TIME 3
#define PROTOCOL_DISPLAY_COUNT_MAX 6

/**
  * @brief  Display Volt,Cureent,Power and Temperature
	on the screen
  * @param  a tempString which includes at lease 7 elements
	By sharing the tempString,more sources can be saved
	@retval None
  */
static void DisplayBasicData(char tempString[], u8 currentStatus, u8 firstEnter)
{
	static u8 displayProtocolCnt = PROTOCOL_DISPLAY_COUNT_MAX;
	u8 displayedProtocolNum;
	u8 length;
	u8 pos;

	if (FilteredMeterData.Voltage >= 9.9999)
	{
		sprintf(tempString, "%.3fV", FilteredMeterData.Voltage);
	}
	else
	{
		sprintf(tempString, "%.4fV", FilteredMeterData.Voltage);
	}
	OLED_ShowString(0, 0, tempString);
	if (FilteredMeterData.Current >= 0.9999)
		sprintf(tempString, "%.4fA", FilteredMeterData.Current);
	else
		sprintf(tempString, "%05.1fmA", FilteredMeterData.Current * 1000);
	OLED_ShowString(72, 0, tempString);
	if (FilteredMeterData.Power >= 9.9999)
	{
		sprintf(tempString, "%.3fW ", FilteredMeterData.Power);
	}
	else
	{
		sprintf(tempString, "%.4fW", FilteredMeterData.Power);
	}
	OLED_ShowString(0, 16, tempString);
	if (CurrentTemperatureSensor == Internal)
	{
		OLED_FillRect(72, 16, 127, 32, 0);
		GenerateTempString(tempString, MOS);
		if (firstEnter)
			OLED_ShowString(80, 16, "-----C");
		else
		{
			if (tempString[0] != '1')tempString[0] = '0';
			OLED_ShowString(80, 16, tempString);
		}
	}
	else
	{
		OLED_FillRect(72, 16, 127, 32, 0);
		GenerateTempString(tempString, External);
		OLED_ShowAnyString(72, 16, "Ext", NotOnSelect, 8);
		if (firstEnter)
			OLED_ShowAnyString(92, 16, "-----C", NotOnSelect, 8);
		else
			OLED_ShowAnyString(92, 16, tempString, NotOnSelect, 8);
		GenerateTempString(tempString, MOS);
		OLED_ShowAnyString(72, 24, "Int", NotOnSelect, 8);
		if (firstEnter)
			OLED_ShowAnyString(92, 24, "-----C", NotOnSelect, 8);
		else
			OLED_ShowAnyString(92, 24, tempString, NotOnSelect, 8);
	}
	if (currentStatus == USBMETER_ONLY)
	{
		displayProtocolCnt--;
		if (displayProtocolCnt == 0) displayProtocolCnt = PROTOCOL_DISPLAY_COUNT_MAX;
		GenerateRTCDateString(tempString);
		OLED_ShowString(0, 32, tempString);
		GenerateRTCTimeString(tempString);
		OLED_ShowString(0, 48, tempString);
		GenerateRTCWeekString(tempString);
		OLED_ShowString(104, 32, tempString);
		displayedProtocolNum = PossibleProtocolNum > 2 ? 2 : PossibleProtocolNum;
		if (displayProtocolCnt < PROTOCOL_DISPLAY_TIME&&PossibleProtocolNum>0)
		{
			OLED_FillRect(72, 48, 127, 63, 0);
			length = GetStringGraphicalLength(ProtocolTab[PossibleProtocolTab[0]].DeviceName);
			pos = 104 - length * 3;
			if (displayedProtocolNum == 1)
			{
				OLED_ShowAnyString(pos, 50, ProtocolTab[PossibleProtocolTab[0]].DeviceName, NotOnSelect, 12);
			}
			else
			{
				OLED_ShowAnyString(pos, 48, ProtocolTab[PossibleProtocolTab[0]].DeviceName, NotOnSelect, 8);
				length = GetStringGraphicalLength(ProtocolTab[PossibleProtocolTab[1]].DeviceName);
				pos = 104 - length * 3;
				OLED_ShowAnyString(pos, 56, ProtocolTab[PossibleProtocolTab[1]].DeviceName, NotOnSelect, 8);
			}
		}
		else
		{
			OLED_FillRect(72, 48, 127, 63, 0);
			sprintf(tempString, "%5.2fV", CurrentMeterData.VoltageDP);
			OLED_ShowAnyString(92, 48, tempString, NotOnSelect, 8);
			sprintf(tempString, "%5.2fV", CurrentMeterData.VoltageDM);
			OLED_ShowAnyString(92, 56, tempString, NotOnSelect, 8);
			OLED_ShowAnyString(80, 48, "D+", NotOnSelect, 8);
			OLED_ShowAnyString(80, 56, "D-", NotOnSelect, 8);
		}
	}
}

/**
  * @brief  Display Capacity,Work,LastingTime
  * @param  a tempString which includes at lease 9 elements
	By sharing the tempString,more sources can be saved
	@retval None
  */
static void DisplayRecordData(char tempString[])
{
	if (CurrentSumUpData.Capacity >= 10000)
		sprintf(tempString, "%05.2fAh", FastUpdateCurrentSumUpData.Capacity);
	else
		sprintf(tempString, "%05.0fmAh", FastUpdateCurrentSumUpData.Capacity * 1000);
	OLED_ShowString(0, 32, tempString);
	if (CurrentSumUpData.Work >= 10000)
		sprintf(tempString, "%05.2fWh", FastUpdateCurrentSumUpData.Work);
	else
		sprintf(tempString, "%05.0fmWh", FastUpdateCurrentSumUpData.Work * 1000);
	OLED_ShowString(0, 48, tempString);
	GenerateVirtualRTCString(tempString);
	OLED_ShowAnyString(81, 51, tempString, NotOnSelect, 12);
	sprintf(tempString, "%d Day(s)", RTCCurrent.Day);
	OLED_ShowAnyString(81, 35, tempString, NotOnSelect, 12);
}

void USBMeter_Init(u8 status)
{
	CreateTaskWithExceptionControl(USBMeter, "USBMeter",
		384, &status, USB_METER_PRIORITY, NULL);
}
