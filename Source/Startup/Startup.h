#ifndef __STARTUP_H
#define	__STARTUP_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

extern xQueueHandle InitStatusMsg;

//see .c for details
void SystemStart(void);

u8 GetStringLength(const char string[]);

void Logo_Init(void);

void InitStatusHandler_Init(void);

void Logo_DeInit(void);

void InitStatus_DeInit(void);

#endif

