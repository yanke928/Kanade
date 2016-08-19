#ifndef __ADC_H
#define	__ADC_H

#include "stm32f10x.h"
#include "stdbool.h"

#define ADC1_DR_Address    ((uint32_t)0x4001244C)  

#define ADC_FILTER_ITEM_NUM  6

//ADC Value converted from Temperature sensors
extern volatile uint16_t  ADCConvertedValue[ADC_FILTER_ITEM_NUM];

//ADC Value converted from INTTemperature sensor
extern volatile float FilteredADCValue[ADC_FILTER_ITEM_NUM];

//A public variable for the converted powerSource voltage
extern float PowerSourceVoltage;

void ADC_Hardware_Init(void);

void Quick_ADC_Collect(void);

void Restart_ADC_And_DMA(void);

#endif /* __ADC_H */
