#ifndef __DIGITAL_LOAD_H__
#define __DIGITAL_LOAD_H__

#include <stdbool.h>

#include "Digital_Load_Conf.h"
#include "Digital_Load.h"

#define Emergency_Stop() PWMRef_SetToGND()

extern float CurrentRefVoltage;

enum{Load_Start,Load_Keep,Load_Stop};

void Set_Constant_Current(float curt);

void Digital_Load_Init(void);

void Send_Digital_Load_Command(u32 current,u8 command);

void Digital_Load_Calibrate(void);

void Digital_Load_Lock(void);

void Digital_Load_Unlock(void);

#endif /* __DIGITAL_LOAD_H__ */
