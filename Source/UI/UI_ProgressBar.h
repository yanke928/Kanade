#ifndef __UI_PROGRESSBAR_H
#define	__UI_PROGRESSBAR_H

#include "stm32f10x.h"
#include "stdbool.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "SSD1306.h"

typedef struct
{
 OLED_PositionStruct Pos1;
 OLED_PositionStruct Pos2;	
 float MaxValue;
 float MinValue;
}ProgressBar_Param_Struct;

extern xQueueHandle UI_ProgressBarMsg;

void UI_ProgressBar_Init(ProgressBar_Param_Struct * progressBarParams);

#endif /* __UI_PROGRESSBAR_H */
