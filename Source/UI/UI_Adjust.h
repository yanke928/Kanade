#ifndef __UI_ADJUST_H
#define	__UI_ADJUST_H

#include "stm32f10x.h"

typedef struct
{
	char *AskString;
  u16  Min;
  u16  Max;
  u16  Step;
	u16  DefaultValue;
	char *UnitString;
	u8   Pos_y;
	u8   FastSpeed;
}UI_Adjust_Param_Struct;

void UI_Adjust_Init(UI_Adjust_Param_Struct * adjustParams);

#endif /* __UI_ADJUST_H */
