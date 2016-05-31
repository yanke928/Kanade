#ifndef __BUZZER_H
#define	__BUZZER_H
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stdbool.h"

#define  BuzzLow GPIO_ResetBits(GPIOB, GPIO_Pin_6) 

#define  BuzzHigh GPIO_SetBits(GPIOB, GPIO_Pin_6)

void BuzzerConfig(void);

#endif /* __BUZZER_H */
