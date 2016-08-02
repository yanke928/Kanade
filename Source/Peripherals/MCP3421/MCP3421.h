#ifndef __MCP3421_H
#define	__MCP3421_H

#include <stdio.h>
#include <stdbool.h>

#include "stm32f10x.h"
#include "misc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"  

typedef struct {
	float Voltage;
	float Current;
	float Power;
	float VoltageDP;
	float VoltageDM;
}USBMeterStruct;

typedef struct {
	float Capacity;
	float Work;
	float PlatformVolt;
}SumupStruct;

extern USBMeterStruct CurrentMeterData;

extern USBMeterStruct FilteredMeterData;

extern volatile SumupStruct CurrentSumUpData;

extern xSemaphoreHandle USBMeterState_Mutex;

extern xQueueHandle MCP3421_Current_Updated_Msg;

bool MCP3421_Init(void);

u32 GetResultFromMCP3421(unsigned char address,u8 num);

#endif /* __MCP3421_H */
