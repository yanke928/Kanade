#ifndef __DATAPIN2PROTOCOL_H
#define	__DATAPIN2PROTOCOL_H

#include "stm32f10x.h"

typedef struct
{
 char* DeviceName;
 float DPMinAndMax[2];
 float DMMinAndMax[2];
 float MaxDeltaV;
 float MainVoltMinAndMax[2];
}ProtocolDescriptor;

extern const ProtocolDescriptor ProtocolTab[];

extern u8 PossibleProtocolTab[10];

extern u8 PossibleProtocolNum;

//see .c for details
void DataPin2Protocol_Service_Init(void);

#endif /* __DATAPIN2PROTOCOL_H */
