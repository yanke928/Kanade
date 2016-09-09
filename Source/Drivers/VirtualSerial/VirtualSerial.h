#ifndef __VIRTUALSERIAL_H
#define	__VIRTUALSERIAL_H

#include "stm32f10x.h"
#include <stdbool.h>

#include "FreeRTOS_Standard_Include.h"

extern xQueueHandle Virtual_Serial_Packet_Received_Msg;

extern u8 ReceivedPacket[];

//see .c for details
void VirtualSerial_SendData(u8 *data,u16 length);

//see .c for details
u16 VirtualSerial_RecvData(u8 *pdata,u16 limit);

//see .c for details
void VirtualSerial_SendString(char* string);

//see .c for details
void VirtualSerial_Packet_Service_Init(void);

//see .c for details
void VirtualSerial_Init(void);

#endif /* __VIRTUALSERIAL_H */
