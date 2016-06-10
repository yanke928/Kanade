#ifndef __TAIWANESE_H
#define	__TAIWANESE_H

#include "stm32f10x.h"

#define ChineseTraditional 1

typedef struct    
{
 s8 Index[3];    
 s8 Msk[24];    
}Big5Font;

typedef struct    
{
 s8 Index[3];    
 s8 Msk[32];    
}Big5Font16;

extern const Big5Font ChineseTraditionalTab12[36];

extern const Big5Font16 ChineseTraditionalTab16[16];

u16 GetChineseTraditionalAddr( s8 *ptr ,u8 size ) ;             

#endif /* __TAIWANESE_H */













