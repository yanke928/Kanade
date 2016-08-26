#ifndef __DIGITAL_LOAD_H__
#define __DIGITAL_LOAD_H__

#include <stdbool.h>

#include "Digital_Load_Conf.h"
#include "Digital_Load.h"

#define Emergency_Stop() PWMRef_SetToGND()

#define DIGITAL_LOAD_PARAMS_NUM 2
#define DEVELOPER_MODE 0
#define NORMAL_MODE 1

typedef struct
{
 int PowerMax;
 int CurrentMax;
 char* Descriptor;
}Digital_Load_Params_t;

extern const Digital_Load_Params_t* const Digital_Load_Params[DIGITAL_LOAD_PARAMS_NUM];

extern float CurrentRefVoltage;

enum{Load_Start,Load_Keep,Load_Stop};

void Set_Constant_Current(float curt);

void Digital_Load_Init(void);

void Send_Digital_Load_Command(u32 current,u8 command);

void Digital_Load_Calibrate(void);

void Digital_Load_Lock(void);

void Digital_Load_Unlock(void);

#endif /* __DIGITAL_LOAD_H__ */
