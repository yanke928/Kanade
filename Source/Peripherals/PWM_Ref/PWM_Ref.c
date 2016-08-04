//File Name   PWM_Ref.c
//Description Generate reference voltage using PWM 

#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"

#include "PWM_Ref.h"

#define POWER_VOLT 3.3

static void PWMRef_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void PWMRef_SetToGND(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
}

static void TIM1_Configuration(void)
{
	TIM_TimeBaseInitTypeDef  TIM1_TimeBaseStructure;
	TIM_DeInit(TIM1);
	TIM1_TimeBaseStructure.TIM_Prescaler = 2-1;
	TIM1_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM1_TimeBaseStructure.TIM_Period = 36000-1;
	TIM1_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM1_TimeBaseStructure.TIM_RepetitionCounter = 0x0;
	TIM_TimeBaseInit(TIM1, &TIM1_TimeBaseStructure);
}

static void TIM1_PWM_Configuration(void)
{
	TIM_OCInitTypeDef TimOCInitStructure;
	TIM_OCStructInit(&TimOCInitStructure);
	TimOCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TimOCInitStructure.TIM_Pulse = 0;
	TimOCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TimOCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC1Init(TIM1, &TimOCInitStructure);
  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable); 

	//TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

void PWMRef_Init()
{
 PWMRef_GPIO_Init();
 TIM1_Configuration();
 TIM1_PWM_Configuration();
}

void Set_RefVoltageTo(float ref)
{
 u16 pulse;
 if(ref<0.001) 
 {
	 TIM_CtrlPWMOutputs(TIM1, DISABLE);
	 return;
 }
 pulse=ref/POWER_VOLT*(36000-1)>(36000-1)?(36000-1):ref/POWER_VOLT*(36000-1);
 TIM_Cmd(TIM1, ENABLE);
 TIM_CtrlPWMOutputs(TIM1, ENABLE);
 TIM1->CCR1=pulse;
}
