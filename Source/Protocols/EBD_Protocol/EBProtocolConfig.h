#ifndef __EBPROTOCOLCONFIG_H
#define	__EBPROTOCOLCONFIG_H

#include "stm32f10x.h"
#include "stdbool.h"

#define EBD_MODEL_NUM 3

typedef struct
{
	u16 CurrentDivider;
	bool DataPinSupport;
	u8 TimePerUpdate;
	u16 CurrentMax;
	u32 PowerMax;
}EBD_Protocol_Config_Struct;

extern const EBD_Protocol_Config_Struct* const EBD_Protocol_Config[EBD_MODEL_NUM];

extern const char* const EBD_Protocol_Descriptors[EBD_MODEL_NUM];

#endif /* __EBPROTOCOLCONFIG_H */
