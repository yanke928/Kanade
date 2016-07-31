//File Name   Usarts.c
//Description Configure Usart1     

#include <stdarg.h>

#include "stm32f10x_dma.h"

#include "FreeRTOS_Standard_Include.h"

#include "Usarts.h"
#include "Startup.h"
#include "EBProtocol.h"


/**
  * @brief  Init Usart1

	  @retval None
  */
void USART1_Init(void)
{
	/*State init structures*/
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/*Usart1 mode configurations*/
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_Even;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_BaudRate = 9600;
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_ClearFlag(USART2, USART_FLAG_TC);

	/*Usart1 NVIC configurations*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	USART_Cmd(USART1, ENABLE);//Enable Usart1

	/*Usart1 GPIO configurations*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * @brief  Init Usart1 DMA

	  @retval None
  */
void USART1_TX_DMA_Init()
{
	/*State init structures*/
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	/*Enable clocks*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/*DMA NVIC configurations*/
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	DMA_DeInit(DMA1_Channel4);//Deinit DMA1_Channel 4  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);//Peripheral address

	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)EBDCommandStringCache;//Buffer address

	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; //PeripheralDST

	DMA_InitStructure.DMA_BufferSize = EBD_COMMAND_LENGTH;//Buffer length

	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//Memory increse mode

	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;//Byte length

	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
}

void DMA1_Channel4_IRQHandler(void)
{
	//portBASE_TYPE xHigherPriorityTaskWoken = pdPASS;
	DMA_ClearFlag(DMA1_FLAG_TC4);
	DMA_Cmd(DMA1_Channel4, DISABLE);
	// xQueueSendFromISR(InitStatusMsg, "DGD", &xHigherPriorityTaskWoken);
}
