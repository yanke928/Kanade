#ifndef __PWM_REF_H
#define	__PWM_REF_H

#include <stdbool.h>

#include "stm32f10x.h"

void PWMRef_Init(void);

void Set_RefVoltageTo(float ref);

void PWMRef_SetToGND(void);

#endif /* __PWM_REF_H */

