//File Name   FastCharge_Trigger_Circuit.c
//Description FastCharge trigger hardware

#include "stm32f10x_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "FastCharge_Trigger_Circuit.h"

#define FAST_CHARGE_TRIGGER_SERVICE_PRIORITY tskIDLE_PRIORITY+5

xQueueHandle FastCharge_Msg;


/**
  * @brief  Init GPIO for FastCharge trigger

  * @retval None
  */
void FastCharge_Trigger_GPIO_Enable(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
  * @brief  Release DP pin(the state of datapins will not be determined by MCU)

  * @retval None
  */
void FastCharge_Trigger_DP_Release(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
  * @brief  Release DM pin(the state of datapins will not be determined by MCU)

  * @retval None
  */
void FastCharge_Trigger_DM_Release(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
  * @brief  Release data pins(the state of datapins will not be determined by MCU)

  * @retval None
  */
void FastCharge_Trigger_Release(void)
{
	FastCharge_Trigger_DP_Release();
	FastCharge_Trigger_DM_Release();
}

/**
  * @brief  QC2.0 9V trigger

  * @retval None
  */
void QC2_9V_Trigger(u8 lastMode)
{
	if (lastMode != QC2_9V&&lastMode != QC2_12V&&lastMode != QC2_20V)
	{
		FastCharge_Trigger_GPIO_Enable();
		SetDP_0V6();
		FastCharge_Trigger_DM_Release();
		vTaskDelay(1500 / portTICK_RATE_MS);
	}
	FastCharge_Trigger_GPIO_Enable();
	SetDP_3V3();
	SetDM_0V6();
}

/**
  * @brief  QC2.0 normal trigger

  * @retval None
  */
void QC2_Normal_Trigger(void)
{
	FastCharge_Trigger_GPIO_Enable();
	FastCharge_Trigger_DM_Release();
	FastCharge_Trigger_DP_Release();
}

/**
  * @brief  QC2.0 12V trigger

  * @retval None
  */
void QC2_12V_Trigger(u8 lastMode)
{
	if (lastMode != QC2_9V&&lastMode != QC2_12V&&lastMode != QC2_20V)
	{
		FastCharge_Trigger_GPIO_Enable();
		SetDP_0V6();
		FastCharge_Trigger_DM_Release();
		vTaskDelay(1500 / portTICK_RATE_MS);
	}
	FastCharge_Trigger_GPIO_Enable();
	SetDP_0V6();
	SetDM_0V6();
}

/**
  * @brief  QC2.0 20V trigger

  * @retval None
  */
void QC2_20V_Trigger(u8 lastMode)
{
	if (lastMode != QC2_9V&&lastMode != QC2_12V&&lastMode != QC2_20V)
	{
		FastCharge_Trigger_GPIO_Enable();
		SetDP_0V6();
		FastCharge_Trigger_DM_Release();
		vTaskDelay(1500 / portTICK_RATE_MS);
	}
	FastCharge_Trigger_GPIO_Enable();
	SetDP_3V3();
	SetDM_3V3();
}

void QC3_Increase_Voltage(u8 lastMode)
{
	FastCharge_Trigger_GPIO_Enable();
	if (lastMode != QC3_Increase&&lastMode != QC3_Increase)
	{
		FastCharge_Trigger_GPIO_Enable();
		SetDP_0V6();
		FastCharge_Trigger_DM_Release();
		vTaskDelay(1500 / portTICK_RATE_MS);
	}
	FastCharge_Trigger_GPIO_Enable();
	SetDM_0V6();
	SetDP_0V6();
	vTaskDelay(10 / portTICK_RATE_MS);
	SetDM_3V3();
	SetDP_0V6();
}

void QC3_Decrease_Voltage(u8 lastMode)
{
	FastCharge_Trigger_GPIO_Enable();
	if (lastMode != QC3_Decrease&&lastMode != QC3_Increase)
	{
		FastCharge_Trigger_GPIO_Enable();
		SetDP_0V6();
		FastCharge_Trigger_DM_Release();
		vTaskDelay(1500 / portTICK_RATE_MS);
	}
	FastCharge_Trigger_GPIO_Enable();
	SetDM_3V3();
	SetDP_3V3();
	vTaskDelay(10 / portTICK_RATE_MS);
	SetDM_3V3();
	SetDP_0V6();
}


/**
  * @brief  FastCharge_Trigger_Service

  * @retval None
  */
void FastCharge_Trigger_Service(void *pvParameters)
{
	u8 lastMode = 0;
	u8 mode;
	for (;;)
	{
		while (xQueueReceive(FastCharge_Msg, &mode, portMAX_DELAY) != pdPASS);
		switch (mode)
		{
		case Release:FastCharge_Trigger_Release(); break;

		case QC2_Normal:QC2_Normal_Trigger(); break;
		case QC2_9V:QC2_9V_Trigger(lastMode); break;
		case QC2_12V:QC2_12V_Trigger(lastMode); break;
		case QC2_20V:QC2_20V_Trigger(lastMode); break;

		case QC3_Increase:QC3_Increase_Voltage(lastMode); break;
		case QC3_Decrease:QC3_Decrease_Voltage(lastMode); break;
		}
		lastMode = mode;
	}
}


/**
  * @brief  Init FastCharge_Trigger_Service

  * @retval None
  */
void FastCharge_Trigger_Service_Init(void)
{
	FastCharge_Msg = xQueueCreate(2, sizeof(u8));
	CreateTaskWithExceptionControl(FastCharge_Trigger_Service, "FastCharge_Trigger_Service",
		128, NULL, FAST_CHARGE_TRIGGER_SERVICE_PRIORITY, NULL);
}
