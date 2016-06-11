//File Name   £ºKeys.c
//Description : Configure the LMR Keys   

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stdbool.h"
#include "SSD1306.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>
#include "Keys.h"

#define KEY_EVENT_HANDLER_PRIORITY tskIDLE_PRIORITY+3

/**The variables that KeyEvents() uses

 *KeyEvent :
  Show the current state of keys,
  must be cleared after use

 *LeftTime,MidTime,RightTime:
  Key pressed time for every key respectively,
  used in KeyEvents() only

 *MidInterval:
  The time interval between two clicks(if exist),
  used in KeyEvents() only

 *MidKeyPressed:
  The flag indicates if a key event takes place,
  must be cleared after use
	used in KeyEvents() only

 *Mid/Left/RightKeyLongPressed:
	Flags that indicate if a long press event taks place,
	used to prevent the mistake of a click event after
	the long press event
	used in KeyEvents() only

 */
u8   volatile KeyEvent = 0;
u8   volatile AdvancedKeyEvent = 0;
u16  ContinousPressBeatTime = 10;
u16  ContinousPressTimeCount = 0;
u16  LeftTime = 0, MidTime = 0, RightTime = 0;
u16  MidInterval = 0;
bool volatile ContinousPressBeats;
bool MidKeyPressed;
bool MidKeyLongPressed, LeftKeyLongPressed, RightKeyLongPressed;

volatile bool IgnoreNextEvent = false;

xQueueHandle Key_Message; 

/**
  * @brief  Configure the pins that the keys use
  * @retval : None
  */

void Keys_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * @brief  Detect the state of keys for every 10ms,
	change the value of KeyEvents for other progresses
  * @retval : None
  */

void KeyEvents(void)
{
	if ((MidTime > PRESSTIME) && (MIDDLE_KEY == KEY_OFF) && (MidKeyLongPressed == false))
	{
		if (MidKeyPressed == true)
		{
			if (IgnoreNextEvent == false)
				KeyEvent = MidDouble;
			else KeyEvent = 0;
			IgnoreNextEvent = false;
			MidTime = 0;
			MidKeyPressed = false;
			return;
		}
		MidKeyPressed = true;
	}
	if ((MidTime > PRESSTIME) && (MidKeyPressed == false) && (MidTime > LONGPRESSTIME))
	{
		if (IgnoreNextEvent == false)
			KeyEvent = MidLong;
		else KeyEvent = 0;
		IgnoreNextEvent = false;
		MidTime = 0;
		MidKeyPressed = false;
		MidKeyLongPressed = true;
		return;
	}
	if ((RightTime > PRESSTIME) && (RIGHT_KEY == KEY_OFF) && (RightKeyLongPressed == false))
	{
		if (IgnoreNextEvent == false)
			KeyEvent = RightClick;
		else KeyEvent = 0;
		IgnoreNextEvent = false;
		RightTime = 0;
		return;
	}

	if ((RightTime > LONGPRESSTIME))
	{
		RightKeyLongPressed = true;
		if (IgnoreNextEvent == false)
			KeyEvent = RightLong;
		else KeyEvent = 0;
		IgnoreNextEvent = false;
		RightTime = 0;
	}
	if ((LeftTime > PRESSTIME) && (LEFT_KEY == KEY_OFF) && (LeftKeyLongPressed == false))
	{
		if (IgnoreNextEvent == false)
			KeyEvent = LeftClick;
		else KeyEvent = 0;
		IgnoreNextEvent = false;
		LeftTime = 0;
		return;
	}
	if (LeftTime > LONGPRESSTIME)
	{
		LeftKeyLongPressed = true;
		if (IgnoreNextEvent == false)
			KeyEvent = LeftLong;
		else KeyEvent = 0;
		IgnoreNextEvent = false;
		LeftTime = 0;
	}
	if (LEFT_KEY == KEY_ON)
	{
		LeftTime++;
		if (KeyEvent == LeftLong)
		{
			AdvancedKeyEvent = LeftContinous;
			ContinousPressTimeCount++;
			if (ContinousPressTimeCount >= ContinousPressBeatTime)
			{
				ContinousPressBeats = true;
				ContinousPressTimeCount = 0;
			}
		}
	}
	else
	{
		LeftTime = 0;
		LeftKeyLongPressed = false;
		if (AdvancedKeyEvent == LeftContinous)
		{
			AdvancedKeyEvent = 0;
			ContinousPressBeats = 0;
			ContinousPressTimeCount = 0;
			ContinousPressBeats = false;
		}
	}
	if (MIDDLE_KEY == KEY_ON)
	{
		MidTime++;
	}
	else
	{
		MidTime = 0;
		MidKeyLongPressed = false;
	}
	if (RIGHT_KEY == KEY_ON)
	{
		RightTime++;
		if (KeyEvent == RightLong)
		{
			AdvancedKeyEvent = RightContinous;
			ContinousPressTimeCount++;
			if (ContinousPressTimeCount >= ContinousPressBeatTime)
			{
				ContinousPressBeats = true;
				ContinousPressTimeCount = 0;
			}
		}
	}
	else
	{
		RightTime = 0;
		RightKeyLongPressed = false;
		if (AdvancedKeyEvent == RightContinous)
		{
			AdvancedKeyEvent = 0;
			ContinousPressBeats = 0;
			ContinousPressTimeCount = 0;
			ContinousPressBeats = false;
		}
	}
	if ((MidKeyPressed == true) && (MIDDLE_KEY == KEY_OFF) && (MidKeyLongPressed == false))
	{
		MidInterval++;
	}
	if ((MidInterval > DOUBLECLICKTIMEINTERVAL) && (MidKeyLongPressed == false))
	{
		MidInterval = 0;
		MidKeyPressed = false;
		KeyEvent = MidClick;
	}
}

void ClearKeyEvent(Key_Message_Struct message)
{
 xQueueReceive(Key_Message, & message, 0 );
}

/**
  * @brief  KeyEvent handler(task)
  * @retval : None
  */
void KeyEventHandler(void *pvParameters)
{
 Key_Message_Struct message;
 Key_Message_Struct outOfDateMessage;
 while(1)
 {
	/*Get status of keys*/
  KeyEvents();
	/*If a key event takes place*/
	if((KeyEvent!=0&&AdvancedKeyEvent==0)||(AdvancedKeyEvent!=0&&ContinousPressBeats!=0))
	{
	 /*Handle advanced key event priorly*/
	 if(AdvancedKeyEvent!=0)
	 {
	  message.KeyEvent=0;
		message.AdvancedKeyEvent=AdvancedKeyEvent;
		ContinousPressBeats=0;
		//KeyEvent=0;
		/*Try to send a new message to the queue,at least 100ms is given for the receiver to handle the last event*/
		if (xQueueSend(Key_Message , & message, 100/portTICK_RATE_MS)!=pdPASS)
		{
		 /*Clear the queue and resend the message*/
		 xQueueReceive(Key_Message, & outOfDateMessage, 0 );
		 xQueueSend(Key_Message , & message, 100/portTICK_RATE_MS);
		}
	 }
	 else
	 {
	  message.KeyEvent=KeyEvent;
		message.AdvancedKeyEvent=0;	
		ContinousPressBeats=0;
		KeyEvent=0;		 
		/*Try to send a new message to the queue,at least 100ms is given for the receiver to handle the last event*/
		if (xQueueSend(Key_Message , & message, 100/portTICK_RATE_MS)!=pdPASS)
		{
		 /*Clear the queue and resend the message*/
		 xQueueReceive(Key_Message, & outOfDateMessage, 10 );
		 xQueueSend(Key_Message , & message, 100/portTICK_RATE_MS);
		}	 
	 }
	}
	vTaskDelay(10/portTICK_RATE_MS);
 }
}

/**
  * @brief  Key debug handler(task)
  * @retval : None
  */
void KeyDebugHandler(void *pvParameters)
{
 Key_Message_Struct message;
 while(1)
 {
  while(xQueueReceive(Key_Message, & message, portMAX_DELAY ) != pdPASS ); 
	OLED_Clear();
	if(message.KeyEvent!=0)
	{
	 switch(message.KeyEvent)
	 {
		 case LeftLong:OLED_ShowString(0,0,"LeftLong");break;
		 case LeftClick:OLED_ShowString(0,0,"LeftClick");break;
		 case MidLong:OLED_ShowString(0,0,"MidLong");break;
		 case MidClick:OLED_ShowString(0,0,"MidClick");break;
		 case MidDouble:OLED_ShowString(0,0,"MidDouble");break;
		 case RightClick:OLED_ShowString(0,0,"RightClick");break;
		 case RightLong:OLED_ShowString(0,0,"RightLong");break;
	 }
	}
	else
	{
	 switch(message.AdvancedKeyEvent)
	 {
		 case LeftContinous:OLED_ShowString(0,0,"LeftContinous");break;
		 case RightContinous:OLED_ShowString(0,0,"RightContinous");break;
	 }	 
	}
 }
}

/**
  * @brief  Init keyEvent task
  * @retval : None
  */
void Key_Init(void)
{
 Keys_GPIO_Init();
 Key_Message=xQueueCreate(1, sizeof(Key_Message_Struct));
 xTaskCreate(KeyEventHandler,"Key Event Handler",
	40,NULL,KEY_EVENT_HANDLER_PRIORITY,NULL); 
}

/**
  * @brief  Init keyEvent debug task
  * @retval : None
  */
void Key_Debug_Init(void)
{
 Key_Init(); 
 xTaskCreate(KeyDebugHandler,"Key Debug Handler",
	32,NULL,KEY_EVENT_HANDLER_PRIORITY,NULL);  
}

/**
  * @brief  Set KeyBeat Frequency when a continous press event takes place
  * @retval : None
  */
void SetKeyBeatRate(u8 freq)
{
	ContinousPressBeatTime = 100 / freq;
}
