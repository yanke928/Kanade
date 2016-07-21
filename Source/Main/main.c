#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f10x_rcc.h"
#include "misc.h"

#include "led.h"

#include "Startup.h"

#include <string.h>

#define USER_APP_OFFSET 0x00003000

#define USER_APP_START_ADDR USER_APP_OFFSET+0x08000000

__asm void MSR_MSP(u32 addr)
{
  MSR MSP, r0
  BX r14
}

int main(void)
{ 
 NVIC_SetVectorTable(NVIC_VectTab_FLASH ,USER_APP_OFFSET);
 STM32_Init();
 SystemStart();
}

