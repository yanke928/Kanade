#ifndef __MUSIC_H
#define	__MUSIC_H

#include "stm32f10x.h"

//Define the simple title of each pitch													 
#define L1 262
#define L2 294												 
#define L3 330
#define L4 349
#define L5 392
#define L6 440
#define L7 494

#define M1 523
#define M2 578												 
#define M3 659
#define M4 698
#define M5 784
#define M6 880
#define M7 988

#define H1 1046
#define H2 1175												 
#define H3 1318
#define H4 1397
#define H5 1568
#define H6 1760
#define H7 1976								
#define STOP 1000-7

//Define the struct which records both pitch and lastingTime
//eg.ToneFREQ=L7,BeatTimeDivide=8 means it's a low 'DO' with 1/8 pitch
typedef struct
{
	u16 ToneFREQ;
	u8  BeatTimeDivide;
}SingleToneStruct;

extern SingleToneStruct Tori_No_Uta[];

extern SingleToneStruct Alarm[];

//see .c for details
void SoundStart(SingleToneStruct sound[]);

//see .c for details
void SoundStop(void);


#endif /* __MUSIC_H */
