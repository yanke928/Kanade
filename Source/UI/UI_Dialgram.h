#ifndef __UI_DIALGRAM_H
#define	__UI_DIALGRAM_H

#include "stm32f10x.h"
#include "stdbool.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef struct
{
	float MaxValues[2];
	float MinValues[2];
	u16 RecordLength;
	const char *MaxAndMinSprintfCommandStrings[2];
	const char *DataSprintfCommandStrings[2];
	const char *DataNumSprintfCommandString;
	float *DataPointers[2];
	u8 Item1AutoNumStart;
	u8 Item1AutoNumStep;
}Dialgram_Param_Struct;

extern xQueueHandle UI_DialogueMsg; 

void UI_Dialgram_Init(Dialgram_Param_Struct * dialgramParams);

#endif /* __UI_DIALGRAM_H */
