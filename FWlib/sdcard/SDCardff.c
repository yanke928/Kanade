#include <SDCardff.h>
#include <SDCard.h>
#include "SDCard.h"
#include "stdio.h"
#include "string.h"
#include "SSD1306.h"
#include "stm32f10x_spi.h"
#include "stm32f10x.h"
#include "core_cm3.h"

FATFS fatfs;            // Work area (file system object) for logical drive

static DWORD acc_size;				/* Work register for fs command */
static WORD acc_files, acc_dirs;

FILINFO Finfo;

char * myts_file = "0:/a.txt";

bool SDCardMountStatus = false;
SD_CardInfo SDCardInfo;

u32 SD_Capacity = 0;

FRESULT scan_files(
	char* path		/* Pointer to the path name working buffer */
)
{
	DIR dirs;
	FRESULT res;
	BYTE i;
	char *fn;


	if ((res = f_opendir(&dirs, path)) == FR_OK) {
		i = strlen(path);
		while (((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {
			if (Finfo.fname[0] == '.') continue;
#if _USE_LFN
			fn = Finfo.fname;
#else
			fn = Finfo.fname;
#endif
			if (Finfo.fattrib & AM_DIR) {
				acc_dirs++;
				*(path + i) = '/'; strcpy(path + i + 1, fn);
				res = scan_files(path);
				*(path + i) = '\0';
				if (res != FR_OK) break;
			}
			else {
				printf("%s/%s\n", path, fn);
				acc_files++;
				acc_size += Finfo.fsize;
			}
		}
	}
	return res;
}

