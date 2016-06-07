#ifndef __JAPANCHINESE_H
#define	__JAPANCHINESE_H

#include "stm32f10x.h"

#define Japanchinese 2   

typedef struct    
{
 s8 Index[2];    
 s8 Msk[24];    
}RB2312Font;

extern const RB2312Font JapanchineseTab12[22];

u16 GetJapanchineseAddr( s8 *ptr ,u8 size );

#endif /* __JAPANCHINESE_H */













