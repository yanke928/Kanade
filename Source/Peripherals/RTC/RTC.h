#ifndef  __RTC_H
#define	 __RTC_H

#include "stm32f10x.h"
#include "stdbool.h"

struct Data_Time
{
	uint8_t hour;
	uint8_t min;
	uint8_t sec;			
	uint16_t w_year;
	uint8_t  w_month;
	uint8_t  w_date;
	uint8_t  week;
};

extern bool RTCUpdateExecute[3];
extern void(*RTCUpdateFunctions[3])(void);

extern volatile struct Data_Time RTCTime; 

//see .c for details
uint8_t RTC_Hardware_Init(void);

//see .c for details
uint8_t Is_Leap_Year(uint16_t year);

//see .c for details
uint8_t Time_Update(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec);

//see .c for details
uint8_t Time_Get(void);

//see .c for details
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day);

//see .c for details
uint16_t Usart_Scanf(uint32_t value,uint8_t count);

//see .c for details
void Time_Set(void);

//see .c for details
void GenerateRTCTimeString(char string[]);

//see .c for details
void GenerateRTCDateString(char string[]);

//see .c for details
void GenerateRTCWeekString(char string[]);

//see .c for details
void RTCCheck(void);
//void TimeSettings(void);
void RTC_Init(void);

#endif
