//File Name    LED_Animate.c
//Description  LED animate driver

#include <stdio.h>

#include "stm32f10x.h"

#include "LED.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "LED_Animate.h"

#define LED_ANIMATION_PRIORITY tskIDLE_PRIORITY+2

LEDAnimateSliceStruct* LEDAnimatePointer = NULL;

xTaskHandle LEDAnimateTaskHandle;

xQueueHandle LEDAnimateExitMsg;

const LEDAnimateSliceStruct LEDAnimation_Startup[] =
{
 {Cyan,200,6,false},//Cyan with brightness of 2
 {KanadeC,200,6,false},//Kanade with brightness of 2
 {0,0,0,END}//End of the animation
};

const LEDAnimateSliceStruct LED_Test[] =
{
 {Red,200,10,false},
 {Green,200,10,false},
 {Blue,200,10},
 {0,0,0,END}//End of the animation
};

const LEDAnimateSliceStruct LEDAnimation_EBDException[] =
{
 {Red,200,6,false},//Red with brightness of 2
 {Cyan,200,6,false},//Cyan with brightness of 2
 {0,0,0,END}//End of the animation
};

/**
  * @brief  FreeRTOS task

  * @param  Pointer of the anomation

  * @retval None
  */
void LEDAnimateHandler(void *pvParameters)
{
	u16 currentPos = 0;
	u8 exitCode;
	LEDAnimateSliceStruct * animation = pvParameters;
	while (1)
	{
		if (animation[currentPos].EOFAnimate == END) currentPos = 0;
		taskENTER_CRITICAL();
		DisplayBasicColor(animation[currentPos].Color,
			animation[currentPos].Brightness);
		taskEXIT_CRITICAL();
		if(xQueueReceive(LEDAnimateExitMsg, &exitCode, animation[currentPos].LastingTime / portTICK_RATE_MS) == pdPASS)
		{
		 vQueueDelete(LEDAnimateExitMsg);
		 vTaskDelay(1/portTICK_RATE_MS);
		 LEDAnimateTaskHandle=NULL;
		 vTaskDelete(NULL);
		}
		currentPos++;
	}
}

/**
  * @brief  DeInit an LED animation

  * @param  None

  * @retval None
  */
void LED_Animate_DeInit(void)
{ 
	u8 exitCode;
	LEDColorStruct dark = { 0,0,0 };
	if (LEDAnimateTaskHandle != NULL)
		xQueueSend(LEDAnimateExitMsg, &exitCode, portMAX_DELAY);
	while(LEDAnimateTaskHandle!=NULL)
	{
	 vTaskDelay(2/portTICK_RATE_MS);
	}
	SetLEDColor(dark);
	LEDAnimateTaskHandle = NULL;
}


/**
  * @brief  Init an LED animation

  * @param  animate:The pointer of the animation

  * @retval None
  */
void LED_Animate_Init(const LEDAnimateSliceStruct animate[])
{
	LED_GPIO_Init();
	if (LEDAnimateTaskHandle != NULL)
		LED_Animate_DeInit();
  LEDAnimateExitMsg = xQueueCreate(1, sizeof(u8));
	CreateTaskWithExceptionControl(LEDAnimateHandler, "LED Animation Handler",
		128, (LEDAnimateSliceStruct*)animate, LED_ANIMATION_PRIORITY, &LEDAnimateTaskHandle);
}



