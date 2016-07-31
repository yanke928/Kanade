#ifndef __LED_Animate_H
#define	__LED_Animate_H

#include <stdbool.h>

#include "stm32f10x.h"

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

extern const LEDAnimateSliceStruct LEDAnimation_Startup[];

extern const LEDAnimateSliceStruct LEDAnimation_EBDException[];

extern const LEDAnimateSliceStruct LED_Test[];

void LED_Animate_Init(const LEDAnimateSliceStruct animate[]);

void LED_Animate_DeInit(void);

#endif /* __LED_Animate_H */
