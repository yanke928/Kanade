//File Name   LED.c
//Description LED Driver     

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "LED.h"

#define SYS_CLOCK 72
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

const NonBrightnessColorStruct BasicColors[8] =
{
 {1,0,0,1},
 {0,1,0,1},
 {0,0,1,1},
 {1,1,1,1},
 {0,1,1,1},
 {1,1,0,1},
 {1,1,2,2},
 {2,1,1,2}
};

/**
  * @brief  Init the GPIO of LED
  * @retval None
  */
void LED_GPIO_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}


/**
  * @brief  Send a bit to LED

  * @param  bit:Target bit being transferred

  * @retval None
  */
void LEDSendABit(bool bit)
{
	u8 i;
	/*Send 1*/
	if (bit)
	{
		i = SYS_CLOCK / 16;
		LEDIOHigh;
		while (i)
		{
			i--;
		}
		LEDIOLow;
		i = SYS_CLOCK / 24;
		while (i)
		{
			i--;
		}
	}
	/*Send 0*/
	else
	{
		i = 1;
		LEDIOHigh;
		while (i)
		{
			i--;
		}
		i = SYS_CLOCK / 12;
		LEDIOLow;
		while (i)
		{
			i--;
		}
	}
}

/**
  * @brief  Send a byte to LED

  * @param  bit:Target byte being transferred

  * @retval None
  */
void LEDSendByte(u8 byte)
{
	u8 i;
	for (i = 0; i < 8; i++)
	{
		LEDSendABit((bool)(0x01 << (7 - i)&byte));
	}
}

/**
  * @brief  Set color of LED

  * @param  A structure which includes 24-bit color data

  * @retval None
  */
void SetLEDColor(LEDColorStruct ledColor)
{
	LEDSendByte(ledColor.G);
	LEDSendByte(ledColor.R);
	LEDSendByte(ledColor.B);
}

/**
  * @brief  Display a color of LED in given brightness

  * @param  BasicColor:Given color

			Brightness:Given brightness

  * @retval None
  */
void DisplayBasicColor(u8 color, u8 brightness)
{
	LEDColorStruct ledColor;
	ledColor.R = BasicColors[color].R*brightness / BasicColors[color].Divider;
	ledColor.G = BasicColors[color].G*brightness / BasicColors[color].Divider;
	ledColor.B = BasicColors[color].B*brightness / BasicColors[color].Divider;
	SetLEDColor(ledColor);
}

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



