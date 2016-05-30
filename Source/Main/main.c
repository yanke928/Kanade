#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "LED.h"
#include "SSD1306.h"

int main(void)
{ 
 STM32_Init();
 LEDAnimateInit(LEDAnimation_Startup);
 OLED_Init();
 vTaskStartScheduler();
	while(1)
	{
	 ;
	}
}




