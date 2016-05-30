#include "stm32f10x.h"
#include "LED.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

int main(void)
{ 
 STM32_Init();
 LEDAnimateInit(LEDAnimation_Startup);
 vTaskStartScheduler();
	while(1)
	{
	 ;
	}
}




