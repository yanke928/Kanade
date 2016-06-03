#ifndef __USBMETER_H
#define	__USBMETER_H

#define USBMETER_ONLY 0
#define USBMETER_RECORD 1

#include "stm32f10x.h"
#include "stdbool.h"

void DisplayBasicData(char tempString[],u8 currentStatus);

void DisplayRecordData(char tempString[]);

void USBMeter_Init(u8 status);

#endif /* __USBMETER_H */
