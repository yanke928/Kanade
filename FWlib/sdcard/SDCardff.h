#ifndef __SDCARDFF_H
#define __SDCARDFF_H

#include "misc.h"
#include "ff.h"
#include "stdbool.h"

extern bool SDCardMountStatus;

extern bool SDExist;

u32 SDCardFSInit(void);

extern FATFS fatfs; 

extern u32 SD_Capacity;

#endif /* __SDCARDFF_H */

