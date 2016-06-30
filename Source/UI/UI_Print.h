#ifndef __UI_PRINT_H
#define	__UI_PRINT_H

#include "stm32f10x.h"

#include "stdbool.h"

void UI_PrintMultiLineString(u8 x1,u8 y1,u8 x2,u8 y2,const char *string,bool select,u8 fontSize);

#endif /* __UI_PRINT_H */


