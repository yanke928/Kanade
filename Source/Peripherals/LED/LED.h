#ifndef __LED_H
#define	__LED_H

#include "stm32f10x.h"
#include "stdbool.h"

#define  LEDIOLow   GPIO_ResetBits(GPIOC, GPIO_Pin_3) 

#define  LEDIOHigh  GPIO_SetBits(GPIOC, GPIO_Pin_3)

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

//LEDAnimateSliceStruct typedef
//This typedef defines the smallest unit of LEDAnimation
//which contains:
//The Color(enumed in "Basic colors enum") of the slice
//The lasting time of the slice
//The brightness of the slice
//End flag of the slice
typedef struct
{
 u8 Color;
 u16 LastingTime;
 u8 Brightness;
 bool EOFAnimate;
}LEDAnimateSliceStruct;

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

extern const LEDAnimateSliceStruct LEDAnimation_Startup[];

extern const LEDAnimateSliceStruct LEDAnimation_EBDException[];

extern const LEDAnimateSliceStruct LED_Test[];

//see .c for details
void LED_GPIO_Init(void);

//see .c for details
void SetLEDColor(LEDColorStruct ledColor);

//see .c for details
void DisplayBasicColor(u8 color,u8 brightness);

//see .c for details
void LEDAnimateHandler(void *pvParameters);

//see .c for details
void LED_Animate_Init(const LEDAnimateSliceStruct animate[]);

//see .c for details
void LED_Animate_DeInit(void);

#endif /* __LED_H */
