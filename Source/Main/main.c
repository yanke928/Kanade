#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "Startup.h"

int main(void)
{ 
 STM32_Init();
 SystemStart();
}




