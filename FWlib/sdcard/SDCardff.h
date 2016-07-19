#ifndef __SDCARDFF_H
#define __SDCARDFF_H

#include <stdbool.h>

#include "ff.h"
#include "SDCard.h"

extern bool SDCardMountStatus;

extern FATFS SD_fatfs;

extern SD_CardInfo SDCardInfo;

#endif /* __SDCARDFF_H */

