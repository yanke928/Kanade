#ifndef __LED_H
#define	__LED_H

#include <stdbool.h>

#include "stm32f10x.h"

#define  LEDIOLow()   GPIO_ResetBits(GPIOC, GPIO_Pin_3) 

#define  LEDIOHigh()  GPIO_SetBits(GPIOC, GPIO_Pin_3)

#define END true

//LEDColorStruct typedef
typedef struct
{
 u8 R;
 u8 G;
 u8 B;
}
LEDColorStruct;

//NonBrightnessColorStruct typedef
typedef struct 
{
 u8 R;
 u8 G;
 u8 B;
 u8 Divider;
}
NonBrightnessColorStruct;

//Basic colors enum
enum
{
 Red=0,
 Green=1,
 Blue=2,
 White=3,
 Cyan=4,
 Orange=5,
 KanadeC=6,
 Pink=7,
};

//see .c for details
void LED_GPIO_Init(void);

//see .c for details
void SetLEDColor(LEDColorStruct ledColor);

//see .c for details
void DisplayBasicColor(u8 color,u8 brightness);

#endif /* __LED_H */
