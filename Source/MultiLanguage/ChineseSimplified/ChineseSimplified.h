#ifndef __JAPANCHINESE_H
#define	__JAPANCHINESE_H

#include "stm32f10x.h"

#define ChineseSimplified 2   

typedef struct    
{
 s8 Index[2];    
 s8 Msk[24];    
}GB2312Font;

typedef struct    
{
 s8 Index[2];    
 s8 Msk[32];    
}GB2312Font16;

extern const GB2312Font ChineseSimplifiedTab12[38];

extern const GB2312Font16 ChineseSimplifiedTab16[16];

u16 GetChineseSimplifiedAddr( s8 *ptr ,u8 size );

#endif /* __JAPANCHINESE_H */













