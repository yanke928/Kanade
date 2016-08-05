//File Name     Digital Load.c
//Description : Digital Load API

#include <string.h>

#include "stm32f10x_flash.h"

#include "FreeRTOS_Standard_Include.h"

#include "PWM_Ref.h"
#include "MCP3421.h"

#include "Calibrate.h"

#include "UI_Dialogue.h"
#include "MultiLanguageStrings.h"
#include "Settings.h"

#include "Digital_Load.h"

#define DIGITAL_LOAD_HANDLER_PRORITY tskIDLE_PRIORITY+7

#define _ABS(x) ((x) > 0 ? (x) : -(x))

xQueueHandle Digital_Load_Command;

float CurrentRefVoltage;

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
					if (_ABS(curtRefVoltage - lstRefVoltage) > 0.00005)
					{
						Set_RefVoltageTo(curtRefVoltage);
						lstRefVoltage = curtRefVoltage;
					}
				}
				lstTrend = curtTrend;
				if (xQueueReceive(Digital_Load_Command, &current, 270) == pdPASS)
				{
					preferredRefVoltage = current*Calibration_Data->Refk + Calibration_Data->Refb;
          if(_ABS(curtRefVoltage - preferredRefVoltage) > 0.02)
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
