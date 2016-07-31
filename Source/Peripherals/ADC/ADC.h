#ifndef __ADC_H
#define	__ADC_H

#include "stm32f10x.h"
#include "stdbool.h"

#define ADC1_DR_Address    ((uint32_t)0x4001244C)  

#define ADC_FILTER_ITEM_NUM  5

#define TEMPTAB_LENGTH 21

#define TEMP_DIVIDER 15

#define _ABS(x)  ((x) > 0 ? (x) : -(x))

enum { Internal = 1, External = 0 };

//ADC Value converted from Temperature sensors
extern volatile uint16_t  ADCConvertedValue[ADC_FILTER_ITEM_NUM];

//ADC Value converted from INTTemperature sensor
extern volatile float FilteredADCValue[ADC_FILTER_ITEM_NUM];

//A public variable for the converted IntTemperature
extern float InternalTemperature;

//A public variable for the converted ExtTemperature
extern float ExternalTemperature;

//A public variable for the converted powerSource voltage
extern float PowerSourceVoltage;

extern volatile u8 CurrentTemperatureSensor;

//see .c for details
void UpdateTemperatureSensors(short lese, short lese2, unsigned char lese3);

//see .c for details
void TemperatureSensors_Init(void);

//see .c for details
void GenerateTempString(char string[],u8 sensorNo);

//see .c for details
void CalculateExtTemp(void);

//see .c for details
bool TemperatureSensorDetect(bool init);

//see .c for details
void TemperatureSensors_Init(void);

//see .c for details
void ShowCurrentTempSensor(void);

#endif /* __ADC_HS */
