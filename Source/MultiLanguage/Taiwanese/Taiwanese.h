#ifndef __TAIWANESE_H
#define	__TAIWANESE_H

#include "stm32f10x.h"

#define Taiwanese 1

typedef struct    
{
 s8 Index[2];    
 s8 Msk[24];    
}Big5Font;

extern const Big5Font TaiwaneseTab12[24];

u16 GetTaiwaneseAddr( s8 *ptr ,u8 size ) ;             

#endif /* __TAIWANESE_H */













