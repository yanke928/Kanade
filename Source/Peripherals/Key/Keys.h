#ifndef __KEYS_H
#define	__KEYS_H

#include <stdbool.h>
#include <stm32f10x.h>

//Define the key state with the pin state
#define KEY_ON	0
#define KEY_OFF	1

#define LEFT 0
#define RIGHT 1

//Make the usage like "MIDDLE_KEY==KEY_ON" possible 
#define MIDDLE_KEY GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)
#define LEFT_KEY   GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5)
#define RIGHT_KEY  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3)

#define ANY_KEY_PRESSED MIDDLE_KEY==KEY_ON||LEFT_KEY==KEY_ON||RIGHT_KEY==KEY_ON

 //KeyEvents enum
enum {
	MidDouble = 1,
	MidClick = 2,
	MidLong = 3,
	LeftLong = 4,
	LeftClick = 5,
	RightLong = 6,
	RightClick = 7
};

 //AdvancedKeyEvents enum
enum {
	LeftContinous  =1,
	RightContinous =2
};

typedef struct 
{
 u8 KeyEvent; 
 u8 AdvancedKeyEvent; 
} Key_Message_Struct;

extern volatile u8 KeyEvent;

extern volatile u8 AdvancedKeyEvent;

extern volatile bool IgnoreNextEvent;

extern volatile bool  ContinousPressBeats;

extern u16  ContinousPressBeatTime;

extern xQueueHandle Key_Message; 

void KeyEvents(void);

void Keys_GPIO_Init(void);

void KeysDebug(void);

void SystemDebug(void);

void SetKeyBeatRate(u8 freq);

void Key_Init(void);

void Key_Debug_Init(void);

void ClearKeyEvent(void);

#define IgnoreNextKeyEvent() IgnoreNextEvent=true

#endif /* __KEYS_H */
