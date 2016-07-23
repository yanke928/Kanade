#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f10x_rcc.h"
#include "stm32f10x_dma.h"
#include "misc.h"

#include "led.h"

#include "Startup.h"

#include <string.h>

#define USER_APP_OFFSET 0x00008000

#define USER_APP_START_ADDR USER_APP_OFFSET+0x08000000

void NVIC_DeInit(void);

int main(void)
{ 
 //RCC_DeInit();
 //DMA_DeInit(DMA2_Channel4);
 //DMA_DeInit(DMA1_Channel1);
 NVIC_DeInit();
 NVIC_SetVectorTable(NVIC_VectTab_FLASH ,USER_APP_OFFSET);
 STM32_Init();
 SystemStart();
}

void NVIC_DeInit(void) 
{ 
  u32 index = 0; 
   
  NVIC->ICER[0] = 0xFFFFFFFF; 
  NVIC->ICER[1] = 0x000007FF; 
  NVIC->ICPR[0] = 0xFFFFFFFF; 
  NVIC->ICPR[1] = 0x000007FF; 
   
  for(index = 0; index < 0x0B; index++) 
  { 
     NVIC->IP[index] = 0x00000000; 
  }  
}
