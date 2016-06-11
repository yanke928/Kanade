#ifndef __PUBLIC_FONTTAB_H
#define	__PUBLIC_FONTTAB_H

#include "stm32f10x.h"

typedef struct    
{
 s8 Index[3];    
 s8 Msk[24];    
}UTF8Font12;

typedef struct    
{
 s8 Index[3];    
 s8 Msk[32];    
}UTF8Font16;

extern const UTF8Font12 UTF8FontTab12[];

extern const UTF8Font16 UTF8FontTab16[];

u16 GetUTF8IndexInFontTab(s8 *ptr, u8 size);

#endif /* __PUBLIC_FONTTAB_H */













