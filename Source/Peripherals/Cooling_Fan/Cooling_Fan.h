#ifndef __COOLING_FAN_H
#define	__COOLING_FAN_H

#include "stm32f10x.h"

enum{Turn_On,Turn_Off,Auto};

void Cooling_Fan_Service_Init(void);

void Fan_Send_Command(u8 command);

#endif /* __COOLING_FAN_H*/
