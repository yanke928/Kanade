#ifndef __KEYS_H
#define	__KEYS_H

#include <stdbool.h>
#include <stm32f10x.h>
#include "stm32f10x_gpio.h"

//Define the key state with the pin state
#define KEY_ON	0
#define KEY_OFF	1

#define LEFT 0
#define RIGHT 1

//Make the usage like "MIDDLE_KEY==KEY_ON" possible 
#define MIDDLE_KEY GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)
#define LEFT_KEY   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5)
#define RIGHT_KEY  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3)

void Keys_GPIO_Init(void);

#endif /* __KEYS_H */
