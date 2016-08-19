//File Name     Calibrate.c
//Description : Calibrate

#include <string.h>
#include <stdio.h>

#include "FreeRTOS_Standard_Include.h"
#include "MCP3421.h"
#include "SSD1306.h"
#include "Keys.h"
#include "Digital_Load.h"
#include "MultiLanguageStrings.h"
#include "UI_Dialogue.h"
#include "UI_Menu.h"
#include "UI_Confirmation.h"
#include "Settings.h"
#include "PWM_Ref.h"

#include "Calibrate.h"

Calibration_t* const Calibration_Data = (Calibration_t*)(CALIBRATION_DATA_ADDRESS);

Calibration_t Calibration_Backup;

const Calibration_t Calibration_Default = { 0.72,0,1,1 };

/**
  * @brief  Save calibration

  * @param  None
  */
void SaveCalibration()
{
	int i = sizeof(Calibration_t);
	u16 m = 0;
	u16 *p = (u16 *)Calibration_Data;
	FLASH_Unlock();
	FLASH_SetLatency(FLASH_Latency_2);
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(CALIBRATION_DATA_ADDRESS);
	p = (u16*)(&Calibration_Backup);
	do
	{
		FLASH_ProgramHalfWord(m + CALIBRATION_DATA_ADDRESS, *p);
		i -= 2;
		m += 2;
		p++;
	} while (i >= 0);
	FLASH_Lock();
}

/**
  * @brief  Check calibration data,reset if illegal

  * @param  None

	  @retval None
  */
void CheckCalibrationData()
{
	if (
		Calibration_Data->Refk > Calibration_Default.Refk*1.5 ||
		Calibration_Data->Refk < Calibration_Default.Refk*0.66 ||
		Calibration_Data->Refb<-0.5 ||
		Calibration_Data->Refb>0.5 ||
		Calibration_Data->VoltageCoeficient>1.1 ||
		Calibration_Data->VoltageCoeficient<0.9 ||
		Calibration_Data->CurrentCoeficient>1.1 ||
		Calibration_Data->CurrentCoeficient < 0.9)
	{
		memcpy(&Calibration_Backup, &Calibration_Default, sizeof(Calibration_t));
		SaveCalibration();
		ShowSmallDialogue("Cali Error", 1000, true);
		ShowSmallDialogue("Reset Done", 1000, true);
	}
}

/**
  * @brief  Digital_Load_Calibrate programme

  * @param  None
  */
void Digital_Load_Calibrate()
{
	float k, b;
	float vRef0A5 = 0;
	float vRef2A0 = 0;
	u8 t;
	char tempString[20];
	ShowSmallDialogue(AmpfilierSelfCalibrationRunning_Str[CurrentSettings->Language], 1000, false);
	Send_Digital_Load_Command(500, Load_Start);
	vTaskDelay(6000 / portTICK_RATE_MS);
	for (t = 0; t < 20; t++)
	{
		if (CurrentMeterData.Current<0.498 || CurrentMeterData.Current>0.502)
		{
			Send_Digital_Load_Command(0, Load_Stop);
			ShowSmallDialogue(AmpfilierSelfCalibrationFailed_Str[CurrentSettings->Language], 1000, true);
			return;
		}
		vTaskDelay(250 / portTICK_RATE_MS);
		vRef0A5 += CurrentRefVoltage*0.05;
	}
	Send_Digital_Load_Command(2000, Load_Start);
	vTaskDelay(6000 / portTICK_RATE_MS);
	for (t = 0; t < 20; t++)
	{
		if (CurrentMeterData.Current<1.998 || CurrentMeterData.Current>2.002)
		{
			Send_Digital_Load_Command(0, Load_Stop);
			ShowSmallDialogue(AmpfilierSelfCalibrationFailed_Str[CurrentSettings->Language], 1000, true);
			return;
		}
		vTaskDelay(250 / portTICK_RATE_MS);
		vRef2A0 += CurrentRefVoltage*0.05;
	}
	Send_Digital_Load_Command(0, Load_Stop);
	k = (vRef2A0 - vRef0A5) / 1.5;
	b = vRef0A5 - k*0.5;
	memcpy(&Calibration_Backup, Calibration_Data, sizeof(Calibration_t));
	Calibration_Backup.Refk = k;
	Calibration_Backup.Refb = b;
	SaveCalibration();
	sprintf(tempString, "k=%6.4f", k);
	ShowSmallDialogue(tempString, 1000, true);
	sprintf(tempString, "b=%6.4f", b);
	ShowSmallDialogue(tempString, 1000, true);
	ShowSmallDialogue(AmpfilierSelfCalibrationSuccess_Str[CurrentSettings->Language], 1000, true);
}


/**
  * @brief  Voltage calibrate programme

  * @param  None
  */
void Voltage_Calibrate()
{
	Key_Message_Struct message;
	char tempString[20];
	double coeficient = 1;
  reDraw:
	ShowDialogue(CalibrationItemVoltageCalibration_Str[CurrentSettings->Language], "", "<--         -->", false, false);
	OLED_ShowAnyString(4, 16, "Volt:", NotOnSelect, 16);
	SetKeyBeatRate(20);
	for (;;)
	{
		sprintf(tempString, "%7.4fV", FilteredMeterData.Voltage*coeficient);
		xSemaphoreTake(USBMeterState_Mutex, portMAX_DELAY);
		OLED_ShowAnyString(60, 16, tempString, NotOnSelect, 16);
		OLED_Refresh_Gram();
		xSemaphoreGive(USBMeterState_Mutex);
		if (xQueueReceive(Key_Message, &message, 270 / portTICK_RATE_MS) == pdPASS)
		{
			if (message.AdvancedKeyEvent == LeftContinous ||
				message.KeyEvent == LeftClick)
			{
				if ((coeficient - 0.000001)*Calibration_Data->VoltageCoeficient > 0.9)
					coeficient = coeficient - 0.00001;
			}
			if (message.AdvancedKeyEvent == RightContinous ||
				message.KeyEvent == RightClick)
			{
				if ((coeficient + 0.000001)*Calibration_Data->VoltageCoeficient < 1.1)
					coeficient = coeficient + 0.00001;
			}
			if (message.KeyEvent == MidClick)
			{
				goto Done;
			}
      if(message.KeyEvent==MidDouble)
      {
        if(GetConfirmation(CalibrationAbort_Str[CurrentSettings->Language],""))
         {
          return;
         }
        goto reDraw;
      }
		}
	}
Done:
	coeficient = coeficient*Calibration_Data->VoltageCoeficient;
	memcpy(&Calibration_Backup, Calibration_Data, sizeof(Calibration_t));
	Calibration_Backup.VoltageCoeficient = coeficient;
	SaveCalibration();
	sprintf(tempString, "k=%.7f", Calibration_Data->VoltageCoeficient);
	ShowSmallDialogue(tempString, 1000, true);
	ShowSmallDialogue(VoltageCalibrationSuccess_Str[CurrentSettings->Language], 1000, true);
}

/**
  * @brief  Current calibrate programme

  * @param  None
  */
void Current_Calibrate()
{
	Key_Message_Struct message;
	char tempString[20];
	double coeficient = 1;
  reDraw:
	ShowDialogue(CalibrationItemCurrentCalibration_Str[CurrentSettings->Language], "", "<--         -->", false, false);
	OLED_ShowAnyString(4, 16, "Curt:", NotOnSelect, 16);
	SetKeyBeatRate(20);
  Digital_Load_Unlock();
	Set_RefVoltageTo(2.16);
	for (;;)
	{
		sprintf(tempString, "%7.4fA", FilteredMeterData.Current*coeficient);
		xSemaphoreTake(USBMeterState_Mutex, portMAX_DELAY);
		OLED_ShowAnyString(60, 16, tempString, NotOnSelect, 16);
		OLED_Refresh_Gram();
		xSemaphoreGive(USBMeterState_Mutex);
		if (xQueueReceive(Key_Message, &message, 270 / portTICK_RATE_MS) == pdPASS)
		{
			if (message.AdvancedKeyEvent == LeftContinous ||
				message.KeyEvent == LeftClick)
			{
				if ((coeficient - 0.000001)*Calibration_Data->CurrentCoeficient > 0.9)
					coeficient = coeficient - 0.00001;
			}
			if (message.AdvancedKeyEvent == RightContinous ||
				message.KeyEvent == RightClick)
			{
				if ((coeficient + 0.000001)*Calibration_Data->CurrentCoeficient < 1.1)
					coeficient = coeficient + 0.00001;
			}
			if (message.KeyEvent == MidClick)
			{
				goto Done;
			}
      if(message.KeyEvent==MidDouble)
      {
        if(GetConfirmation(CalibrationAbort_Str[CurrentSettings->Language],""))
         {
         	Set_RefVoltageTo(0);
          Digital_Load_Lock();
          return;
         }
        goto reDraw;
      }
		}
	}
Done:
	Set_RefVoltageTo(0);
  Digital_Load_Lock();
	coeficient = coeficient*Calibration_Data->CurrentCoeficient;
	memcpy(&Calibration_Backup, Calibration_Data, sizeof(Calibration_t));
	Calibration_Backup.CurrentCoeficient = coeficient;
	SaveCalibration();
	sprintf(tempString, "k=%.7f", Calibration_Data->CurrentCoeficient);
	ShowSmallDialogue(tempString, 1000, true);
	ShowSmallDialogue(VoltageCalibrationSuccess_Str[CurrentSettings->Language], 1000, true);
}

/**
  * @brief  Select calibrate item

  * @param  None
  */
void CalibrateSelect()
{
	UI_Menu_Param_Struct menuParams;
	u8 selection;
	const char *calobrateItems[4];

	calobrateItems[0] = CalibrationItemAmpfilierSelfCalibration_Str[CurrentSettings->Language];
	calobrateItems[1] = CalibrationItemVoltageCalibration_Str[CurrentSettings->Language];
	calobrateItems[2] = CalibrationItemCurrentCalibration_Str[CurrentSettings->Language];

	menuParams.ItemStrings = calobrateItems;
	menuParams.DefaultPos = 0;
	menuParams.ItemNum = 3;
	menuParams.FastSpeed = 5;
	UI_Menu_Init(&menuParams);

	xQueueReceive(UI_MenuMsg, &selection, portMAX_DELAY);
	UI_Menu_DeInit();

	OLED_Clear_With_Mutex_TakeGive();

	switch (selection)
	{
	case 0:Digital_Load_Calibrate(); break;
	case 1:Voltage_Calibrate(); break;
	case 2:Current_Calibrate();
	}
}
