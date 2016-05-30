/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

//#include "stm32f10x_
#include "misc.h"
#include "stm32f10x_spi.h"

/* ISR Priority 0(highest)-15(lowest)*/
//数字越小优先级越高
#define DMA_CHANEL_Priority 6
#define USART1_IRQn_Priority 7
#define USART2_IRQn_Priority 8
#define USB_IRQn_Priority 9
#define EXTI15_10_IRQn_Priority 10
#define TIME3_IRQn_Priority 11
#define TIME4_IRQn_Priority 12
#define RTC_IRQn_Priority 13
#define EXIT9_5_IRQn_Priority 14

//位带操作,实现51类似的GPIO控制功能
//具体实现思想,参考<<CM3权威指南>>第五章(87页~92页).
//IO口操作宏定义
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO口地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 


//IO口操作,只对单一的IO口!
//确保n的值小于16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入 


//定义 GPIO PORT 对于输出的 在初始化中 将全部做输出用
typedef enum 
{
	LED_RUN_PIN = 0,		//PB14
	LED_TX_PIN,				//PB15
	LED_RX_PIN,				//PA8
	BEEP_PIN,				//PC13
	USB_DISCONNECT_PIN,		//PA15
	S485EN_PIN				//PA1
}GPIO_PIN_DEF;

typedef enum
{ 					
	ExKey1=0,		 
	ExKey2,	
}ExKeY;

static GPIO_TypeDef* GPIO_OUT_PORT[6]={GPIOB, GPIOB,GPIOA,GPIOC,GPIOA,GPIOA};
static const u16 GPIO_OUT_PIN[6]={GPIO_Pin_14, GPIO_Pin_15,GPIO_Pin_8,GPIO_Pin_13,GPIO_Pin_15,GPIO_Pin_1};

#define GPIO_OH(pin)	GPIO_SetBits(GPIO_OUT_PORT[pin], GPIO_OUT_PIN[pin]);	//输出高
#define GPIO_OL(pin)	GPIO_ResetBits(GPIO_OUT_PORT[pin], GPIO_OUT_PIN[pin]); //输出低

#define RdKey1()		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)
#define RdKey2()		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5)

#define ReadBit(reg,bit) ((reg>>bit)&1)

#endif /* __PLATFORM_CONFIG_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
