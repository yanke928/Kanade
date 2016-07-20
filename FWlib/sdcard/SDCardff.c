//File Name   SDCardff.c
//Description SDCard fatfs

#include <stdbool.h>

#include "ff.h"

#include "SDCardff.h"

FATFS SD_fatfs;    

bool SDCardMountStatus = false;
