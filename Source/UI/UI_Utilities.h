#ifndef __UI_UTILITIES_H
#define	__UI_UTILITIES_H

#include "stm32f10x.h"

//see .c for details
u8 GetCentralPosition(u8 startPos, u8 endPos, u8 stringLength);

//see .c for details
u8 GetStringLength(char string[]);

#endif /* __UI_UTILITIES_H */
