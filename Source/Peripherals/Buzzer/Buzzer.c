//File Name     Buzzer.c
//Description : Configure the Buzzer  

#include "Buzzer.h"

void BuzzerGPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void TIM4_Configuration()
{
	TIM_TimeBaseInitTypeDef  TIM4_TimeBaseStructure;
	TIM_DeInit(TIM4);
	TIM4_TimeBaseStructure.TIM_Prescaler = 0;
	TIM4_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM4_TimeBaseStructure.TIM_Period = 14400 - 1;
	TIM4_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM4_TimeBaseStructure.TIM_RepetitionCounter = 0x0;
	TIM_TimeBaseInit(TIM4, &TIM4_TimeBaseStructure);
}

void BuzzerPWM_Configuration(void)
{
	TIM_OCInitTypeDef TimOCInitStructure;
	TIM_OCStructInit(&TimOCInitStructure);
	TimOCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TimOCInitStructure.TIM_Pulse = 7200;
	TimOCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TimOCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC1Init(TIM4, &TimOCInitStructure);
	TIM_CtrlPWMOutputs(TIM4, ENABLE);
}

void BuzzerConfig(void)
{
	BuzzerGPIOInit();
	TIM4_Configuration();
	BuzzerPWM_Configuration();
}
