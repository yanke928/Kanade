#ifndef __SDCARDFF_H
#define __SDCARDFF_H

#include <stdbool.h>

#include "ff.h"
#include "SDCard.h"

extern bool SDCardMountStatus;

extern bool SDCardUnMountedManually;

extern FATFS SD_fatfs;

void SDCardPlugAndPlay_Service_Init(void);

#endif /* __SDCARDFF_H */

