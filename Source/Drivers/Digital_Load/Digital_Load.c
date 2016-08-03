//File Name     Digital Load.c
//Description : Digital Load API

#include <string.h>

#include "stm32f10x_flash.h"

#include "FreeRTOS_Standard_Include.h"

#include "PWM_Ref.h"
#include "MCP3421.h"

#include "Digital_Load.h"

#include "UI_Dialogue.h"
#include "MultiLanguageStrings.h"
#include "Settings.h"

#define DIGITAL_LOAD_HANDLER_PRORITY tskIDLE_PRIORITY+7

#define CALIBRATION_DATA_ADDRESS 0x0803e800

#define _ABS(x) ((x) > 0 ? (x) : -(x))

xQueueHandle Digital_Load_Command;

float CurrentRefVoltage;

typedef struct
{
	float Refk;
	float Refb;
}Calibration_t;

Calibration_t* const Calibration_Data = (Calibration_t*)(CALIBRATION_DATA_ADDRESS);

Calibration_t Calibration_Backup;

const Calibration_t Calibration_Default = { 0.72,0 };

void SaveCalibration(void);

void Set_Constant_Current(float curt)
{
	float voltageREF = (float)SENSOR_RESISTANCE*(float)SENSOR_REFERENCE_GAIN*curt / 1000;
	Set_RefVoltageTo(voltageREF);
}

/**
  * @brief  Digital load handler

  * @param  None

	  @retval None
  */
void Digital_Load_Handler(void *pvParameters)
{
	float current = 0;
	double lstRefVoltage;
	double curtRefVoltage;
	double preferredRefVoltage;

	double coeficient = 0.05;

	bool  lstTrend;
	bool  curtTrend;
	for (;;)
	{
		while (xQueueReceive(Digital_Load_Command, &current, portMAX_DELAY) != pdPASS);
		if (current > 0 && current < 4.1)
		{
			preferredRefVoltage = current*Calibration_Data->Refk + Calibration_Data->Refb;
			lstRefVoltage = preferredRefVoltage;
			Set_RefVoltageTo(lstRefVoltage);
			if (current < 0.3)
				vTaskDelay(750 / portTICK_RATE_MS);
			else
				vTaskDelay(500 / portTICK_RATE_MS);
			lstTrend = true;
			coeficient = 0.01;
		}
		for (;;)
		{
			if (current < 0 || current> 4.1)
			{
				Set_RefVoltageTo(0); break;
			}
			else
			{
				xSemaphoreTake(USBMeterState_Mutex, portMAX_DELAY);
				curtRefVoltage = current / CurrentMeterData.Current*lstRefVoltage;
				xSemaphoreGive(USBMeterState_Mutex);
				if (current > CurrentMeterData.Current) curtTrend = true;
				else curtTrend = false;
				if (curtRefVoltage != lstRefVoltage)
				{
					if (lstTrend == curtTrend) coeficient = coeficient * 2;
					else coeficient = 0.02;
					coeficient = coeficient > 0.5 ? 0.5 : coeficient;
					coeficient = coeficient < 0.01 ? 0.01 : coeficient;
					curtRefVoltage = (curtRefVoltage - lstRefVoltage)*coeficient + lstRefVoltage;
					//          curtRefVoltage = curtRefVoltage>1.5*preferredRefVoltage+0.1?1.5*preferredRefVoltage+0.1:curtRefVoltage;
					if (_ABS(curtRefVoltage - lstRefVoltage) > 0.0002)
					{
						Set_RefVoltageTo(curtRefVoltage);
						lstRefVoltage = curtRefVoltage;
					}
				}
				lstTrend = curtTrend;
				if (xQueueReceive(Digital_Load_Command, &current, 270) == pdPASS)
				{
					preferredRefVoltage = current*Calibration_Data->Refk + Calibration_Data->Refb;
					curtRefVoltage = preferredRefVoltage;
					coeficient = 0.01;
					lstRefVoltage = curtRefVoltage;
					Set_RefVoltageTo(curtRefVoltage);
					vTaskDelay(400 / portTICK_RATE_MS);
				}
			}
			CurrentRefVoltage = curtRefVoltage;
		}
	}
}

void CheckCalibrationData()
{
	if (Calibration_Data->Refk > Calibration_Default.Refk*1.5 ||
		Calibration_Data->Refk < Calibration_Default.Refk*0.66 ||
		Calibration_Data->Refb<-0.5 ||
		Calibration_Data->Refb>0.5)
	{
		memcpy(&Calibration_Backup, &Calibration_Default, sizeof(Calibration_t));
		SaveCalibration();
		ShowSmallDialogue("Cali Error", 1000, true);
		ShowSmallDialogue("Reset Done", 1000, true);
	}
}

/**
  * @brief  Digital load Init

  * @param  None

	  @retval None
  */
void Digital_Load_Init()
{
	CheckCalibrationData();
	Digital_Load_Command = xQueueCreate(2, sizeof(float));
	CreateTaskWithExceptionControl(Digital_Load_Handler, "Digital_Load Handler",
		160, NULL, DIGITAL_LOAD_HANDLER_PRORITY, NULL);
}

/**
  * @brief  Send a command to digital load

  * @param  curt:Current(mA)
			command:u8 enum:Start_Load,Stop_Load,Keep_Load

	  @retval None
  */
void Send_Digital_Load_Command(u32 curt, u8 command)
{
	float current = (float)curt / 1000;
	switch (command)
	{
	case Load_Start: xQueueSend(Digital_Load_Command, &current, 0); break;
	case Load_Keep: xQueueSend(Digital_Load_Command, &current, 0); break;
	case Load_Stop:current = -1; xQueueSend(Digital_Load_Command, &current, 0);
	}
}

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
