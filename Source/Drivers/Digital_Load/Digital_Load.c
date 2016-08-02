//File Name     Digital Load.c
//Description : Digital Load API

#include "FreeRTOS_Standard_Include.h"

#include "PWM_Ref.h"
#include "MCP3421.h"

#include "Digital_Load.h"

#define DIGITAL_LOAD_HANDLER_PRORITY tskIDLE_PRIORITY+6

#define _ABS(x) ((x) > 0 ? (x) : -(x))

xQueueHandle Digital_Load_Command;

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

	double coeficient = 0.05;
	bool  lstTrend;
	bool  curtTrend;
	for (;;)
	{
		while (xQueueReceive(Digital_Load_Command, &current, portMAX_DELAY) != pdPASS);
		if (current > 0 && current<4.1)
		{
			lstRefVoltage = (double)SENSOR_RESISTANCE*(double)SENSOR_REFERENCE_GAIN*current / 1000;
			Set_RefVoltageTo(lstRefVoltage);
			vTaskDelay(500 / portTICK_RATE_MS);
			lstTrend = true;
			coeficient = 0.02;
		}
		for (;;)
		{
			if (current < 0 || current> 4.1 )
			{
				Set_RefVoltageTo(0); break;
			}
			else
			{
				curtRefVoltage = current / CurrentMeterData.Current*lstRefVoltage;
				if (current > CurrentMeterData.Current) curtTrend = true;
				else curtTrend = false;
				if (curtRefVoltage != lstRefVoltage)
				{
					if (lstTrend == curtTrend) coeficient = coeficient * 2;
					else coeficient = 0.02;

					coeficient = coeficient > 0.5 ? 0.5 : coeficient;
					coeficient = coeficient < 0.02 ? 0.05 : coeficient;
					curtRefVoltage = (curtRefVoltage - lstRefVoltage)*coeficient + lstRefVoltage;
					if (_ABS(curtRefVoltage - lstRefVoltage) > 0.0002)
					{
						Set_RefVoltageTo(curtRefVoltage);
						lstRefVoltage = curtRefVoltage;
					}
				}
				lstTrend = curtTrend;
				xQueueReceive(Digital_Load_Command, &current, 270);
			}
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
	Digital_Load_Command = xQueueCreate(2, sizeof(float));
	CreateTaskWithExceptionControl(Digital_Load_Handler, "Digital_Load Handler",
		128, NULL, DIGITAL_LOAD_HANDLER_PRORITY, NULL);
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
