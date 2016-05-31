//File Name   ��Music.c
//Description : Play Music with buzzer 
#include "Music.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "Buzzer.h"
#include "stdbool.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define SOUND_PLAYER_PRIORITY tskIDLE_PRIORITY+4

xTaskHandle SoundPlayerHandle;

const SingleToneStruct MyScore[] =
{
 {750,0},
 // {STOP,4},{M5,4},{M6,4},{H1,4},{M7,1},{STOP,4},{M5,4},
 // {M6,4},{H1,4},{H2,1},{STOP,4},{M5,4},{M6,4},{M7,4},
 // {H5,1},{STOP,2},{H4,4},{H3,2},{STOP,4},{M5,4},
 // {M6,4},{H1,4},{M7,1},{STOP,4},{M5,4},{M6,4},{H1,4},{H2,1},
 // {STOP,4},{M3,4},{M5,4},{M7,4},{M6,1},{STOP,1},{STOP,4},
  {L6,4},{L6,4},{L7,4},{M1,4},{M5,4},{M3,2},{M3,4},{M3,4},
  {M2,8},{M3,8},{M3,1},{STOP,2},{M2,2},{M3,2},{M5,2},{M2,4},
  {L7,4},{M1,4},{L7,2},{L7,4},{L6,8},{L3,8},{L3,1},{STOP,2},
  {L6,4},{L6,4},{L7,4},{M1,4},{M1,5},{M3,2},{M3,4},{M2,8},
  {M3,8},{M3,1},{STOP,2},{M2,4},{M3,4},{M5,4},{M3,4},{M5,4},
  {H1,4},{M7,2},{M6,4},{M3,8},{M3,2},{M2,4},{M3,2},{M5,2},
  {M6,2},{M7,2},{M3,2},{M3,4},{M2,8},{M3,8},{M3,1},{STOP,2},
  {M2,4},{M3,4},{M5,2},{M2,4},{L7,4},{M1,4},{L7,2},{L7,4},
  {L6,8},{L3,8},{L3,1},{STOP,2},{L6,4},{L6,4},{L7,4},{M1,4},
  {M5,4},{M3,2},{M3,4},{M3,4},{M2,8},{M3,8},{M3,1},{STOP,2},
  {M2,4},{M3,4},{M5,4},{M3,4},{M5,4},{H1,4},{M7,1},{M6,4},
  {M6,1},{STOP,2},{M6,2},{M7,1},{M6,4},{M6,1},{STOP,2},{M6,4},
  {M6,2},{M6,2},{M6,2},{M6,2},{M5,2},{M5,2},{STOP,4},{M6,2},
  {M7,4},{H1,2},{M7,2},{M6,2},{STOP,4},{M3,4},{M2,4},{M3,4},
  {M3,1},{M2,4},{M1,4},{M1,2},{STOP,2},{M6,4},{M6,2},{M6,2},
  {M6,2},{M6,2},{M5,2},{M5,2},{STOP,4},{M3,2},{M5,4},{M6,2},
  {M7,2},{H1,2},{H1,1},{H1,2},{STOP,2},{STOP,1},{STOP,2},
  {M6,4},{M6,2},{M6,2},{M6,2},{M6,2},{M5,2},{M5,2},{STOP,4},
  {M6,2},{M7,4},{H1,2},{M7,2},{M6,2},{STOP,4},{M3,4},{M2,4},
  {M3,4},{M3,1},{M2,4},{M1,4},{M1,2},{STOP,2},{M2,4},{M2,2},
  {M2,2},{M2,2},{M2,2},{M3,2},{M5,2},{M5,4},{M3,2},{M5,4},
  {M6,2},{STOP,2},{H1,2},{M7,4},{M6,4},{M6,1},{STOP,2},{M5,2},
  {M3,4},{M3,4},{M3,4},{M2,8},{M3,8},{M3,1},{STOP,2},{M3,2},
  {M4,2},{M5,2},{M2,1},{L7,2},{L7,1},{STOP,2},{L7,4},{M1,4},
  {M2,2},{M3,4},{M3,4},{M3,4},{M5,8},{M3,8},{M3,1},{M3,1},
  {STOP,2},{M2,4},{M1,4},{M2,2},{M2,4},{M1,4},{M2,4},{M5,2},
  {M5,4},{M5,1},{STOP,1},{M3,4},{M3,4},{M3,4},{M2,8},{M3,8},
  {M3,1},{STOP,2},{M3,2},{M4,2},{M5,2},{M2,1},{L7,2},{L7,1},
  {STOP,2},{L7,4},{M1,4},{M2,2},{M4,4},{M4,4},{M4,4},{M3,8},
  {M4,8},{M4,1},{M4,1},{STOP,2},{M2,4},{M1,4},{M2,2},{M2,4},
  {M1,4},{M2,4},{M3,2},{M3,4},{STOP,2},
  //{L6,4},{L6,4},{L7,4},
  // {M1,4},{M5,4},{M3,2},{M3,4},{M3,4},{M2,8},{M3,8},{M3,1},
  // {STOP,2},{M2,4},{M3,4},{M5,4},{M3,4},{M5,4},{H1,4},{M7,2},
  // {M6,4},{M3,8},{M3,2},{M2,4},{M3,2},{M5,2},{M6,2},{M7,2},
  // {M3,2},{M3,4},{M3,4},{M2,8},{M3,8},{M3,1},
  // {STOP,2},{M2,4},{M3,2},{M5,2},
  // {M2,4},{L7,4},{M1,4},{L7,2},{L7,4},{L6,8},{L3,8},{M3,1},
  // {STOP,2},{L6,4},{L6,4},{L7,4},{M1,4},{M5,4},{M3,2},{M3,4},
  // {M2,8},{M3,8},{M3,1},{STOP,2},{M2,4},{M3,4},{M5,4},{M3,4},
  // {M5,4},{H1,4},{M7,1},{M6,4},{M6,1},{STOP,2},{M6,2},{M7,1},
  // {M6,4},{M6,1},{STOP,1},{STOP,1},{STOP,1},
   {STOP,1},{STOP,1},{STOP,1},
   {0,0}
};

/**
  * @brief  Play sound
  * @retval : None
  */

void SoundPlayer(void *pvParameters)
{
	TIM_TimeBaseInitTypeDef  TIM4_TimeBaseStructure;
	TIM_OCInitTypeDef TimOCInitStructure;
	portTickType xLastWakeTime;
	int nextFREQ;
	int nextTime;
	int addr;
	addr=0;
	while(1)
	{
	if (MyScore[addr].ToneFREQ != 0) //If it is not the end of the score
	{
		nextFREQ = MyScore[addr].ToneFREQ;//Get the FREQ of the tone in the next pitch
		nextTime = MyScore[0].ToneFREQ / MyScore[addr].BeatTimeDivide;//Calculate the time of the next tone 
	}
	else
	{
		nextFREQ = MyScore[1].ToneFREQ;//Go to the start of the score
		nextTime = MyScore[0].ToneFREQ / MyScore[1].BeatTimeDivide;//The first pitch of the score
		addr = 1;
	}
	taskENTER_CRITICAL();
	TIM_Cmd(TIM4, DISABLE);//Turn off the timer for safety
	TIM4_TimeBaseStructure.TIM_Prescaler = 10;
	TIM4_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM4_TimeBaseStructure.TIM_Period = (100000*72) / nextFREQ - 1;//Get the countValue according to FREQ
	TIM4_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM4_TimeBaseStructure.TIM_RepetitionCounter = 0x0;
	TIM_TimeBaseInit(TIM4, &TIM4_TimeBaseStructure);
	TIM_OCStructInit(&TimOCInitStructure);
	TimOCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TimOCInitStructure.TIM_Pulse = (50000*72) / nextFREQ;
	TimOCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TimOCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC1Init(TIM4, &TimOCInitStructure);
	TIM_CtrlPWMOutputs(TIM4, ENABLE);
	taskEXIT_CRITICAL();
	if (MyScore[addr].ToneFREQ != STOP)
		TIM_Cmd(TIM4, ENABLE);
	addr++;
  vTaskDelayUntil(&xLastWakeTime, nextTime / portTICK_RATE_MS);
}
}


/**
  * @brief  Start playing music

  * @retval : TimerNo for kanade
  */
void SoundStart(void)
{
	BuzzerConfig();
	if(SoundPlayerHandle!=NULL)
	{
		vTaskDelete(SoundPlayerHandle);
	}
	xTaskCreate(SoundPlayer, "Sound Player",
		128, NULL, SOUND_PLAYER_PRIORITY, &SoundPlayerHandle);	
}

/**
  * @brief   Stop playing music

  * @Param : None

  * @retval: None
  */
void SoundStop(void)
{ 
	TIM_DeInit(TIM4);
  vTaskDelete(SoundPlayerHandle);
}
