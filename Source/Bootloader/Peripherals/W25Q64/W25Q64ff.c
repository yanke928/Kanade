//File Name   W25Q64ff.c
//Description SPI FLASH fatfs

#include <stdbool.h>

#include "ff.h"

#include "W25Q64ff.h"

FATFS SPI_FLASH_fatfs;   

bool SPIFlashMountStatus = false;


