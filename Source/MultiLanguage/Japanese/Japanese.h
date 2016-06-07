#ifndef __JAPANESE_H
#define	__JAPANESE_H

#include "stm32f10x.h"

#define Japanese 3   

typedef struct    
{
 s8 Index[2];    
 s8 Msk[24];    
}JISFont;

extern const JISFont JapaneseTab12[30];

u16 GetJapaneseAddr( s8 *ptr ,u8 size );

#endif /* __JAPANESE_H */













