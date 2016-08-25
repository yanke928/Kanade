#ifndef __UI_ADJUST_H
#define	__UI_ADJUST_H

#include "stm32f10x.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef struct
{
	const char *AskString;
	const char *UnitString;
  int  Min;
  int  Max;
  int  Step;
	int  DefaultValue;
	u8   Pos_y;
	u8   FastSpeed;
}UI_Adjust_Param_Struct;

extern xQueueHandle UI_AdjustMsg; 

//see .c for details
void UI_Adjust_Init(UI_Adjust_Param_Struct * adjustParams);

//see .c for details
void UI_Adjust_DeInit(void);

#endif /* __UI_ADJUST_H */
