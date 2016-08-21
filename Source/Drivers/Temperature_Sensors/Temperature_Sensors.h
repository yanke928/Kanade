#ifndef __TEMPERATURE_SENSORS_H
#define	__TEMPERATURE_SENSORS_H

#include <stdbool.h>

#include "stm32f10x.h"

#define TEMPTAB_LENGTH 21

#define TEMP_DIVIDER 15

enum { Internal = 1, External = 0 ,MOS = 2};

//A public variable for the converted ExtTemperature
extern float ExternalTemperature;

//A public variable for the converted MOSTemperature
extern float MOSTemperature;

extern float InternalTemperature;

extern volatile u8 CurrentTemperatureSensor;

void CalculateExtTemp(void);

void GenerateTempString(char string[], u8 sensorNo);

void TemperatureSensors_Init(void);

void QuickGet_Enviroment_Temperature(void);

#endif /* __TEMPERATURE_SENSORS_H */
