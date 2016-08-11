#ifndef __MUSIC_H
#define	__MUSIC_H

#include "stm32f10x.h"

//Define the struct which records both pitch and lastingTime
//eg.ToneFREQ=L7,BeatTimeDivide=8 means it's a low 'DO' with 1/8 pitch
typedef struct
{
	u16  NoteFreq;
	u16  LastingTime;
}SingleToneStruct;

extern const SingleToneStruct Tori_No_Uta[];

extern const SingleToneStruct Ichiban_no_takaramono[];

extern const SingleToneStruct Alarm[];

//see .c for details
void SoundStart(const SingleToneStruct sound[]);

//see .c for details
void SoundStop(void);


#endif /* __MUSIC_H */
