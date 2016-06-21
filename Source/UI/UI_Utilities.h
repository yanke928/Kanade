#ifndef __UI_UTILITIES_H
#define	__UI_UTILITIES_H

#include "stm32f10x.h"

#include "stdbool.h"

enum { HighDensity = 1, LowDensity = 0 };

//see .c for details
u8 GetCentralPosition(u8 startPos, u8 endPos, u8 stringLength);

//see .c for details
u8 GetStringLength(const char string[]);

//see .c for details
void DrawVerticalDashedGrid(u8 x, bool drawOrUnDraw, u8 gridsDensity);

//see .c for details
void DrawHorizonalDashedGrid(u8 y, bool drawOrUnDraw, u8 gridsDensity);

#endif /* __UI_UTILITIES_H */
