#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f10x_rcc.h"
#include "misc.h"

#include "led.h"

#include "Startup.h"

#include <string.h>

#define USER_APP_OFFSET 0x00002000

#define USER_APP_START_ADDR USER_APP_OFFSET+0x80000000

__asm void MSR_MSP(u32 addr) //设置堆栈指针
{
  MSR MSP, r0
  BX r14
}

int main(void)
{ 
 STM32_Init();
 SystemStart();
}




