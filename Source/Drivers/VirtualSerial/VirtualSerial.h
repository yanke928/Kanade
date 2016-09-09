#ifndef __VIRTUALSERIAL_H
#define	__VIRTUALSERIAL_H

#include "stm32f10x.h"
#include <stdbool.h>

//see .c for details
void VirtualSerial_SendData(u8 *data,u16 length);

//see .c for details
u16 VirtualSerial_RecvData(u8 *pdata,u16 limit);

//see .c for details
void VirtualSerial_SendString(char* string);

#endif /* __VIRTUALSERIAL_H */
