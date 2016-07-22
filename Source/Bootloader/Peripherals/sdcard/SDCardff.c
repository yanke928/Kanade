//File Name   SDCardff.c
//Description SDCard fatfs

#include <stdbool.h>
#include <stdio.h>

#include "ff.h"

#include "SSD1306.h"

#include "SDCardff.h"

FATFS SD_fatfs;

bool SDCardMountStatus = false;

bool SDCardUnMountedManually = false;
