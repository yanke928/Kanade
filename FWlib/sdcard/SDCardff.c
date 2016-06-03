#include <SDCardff.h>
#include <SDCard.h>
#include "SDCard.h"
#include "stdio.h"
#include "string.h"
#include "SSD1306.h"
#include "stm32f10x_spi.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "platform_config.h"

#define  SD_CS_ENABLE()	  MSD_CS_LOW()
#define  SD_CS_DISABLE()  MSD_CS_HIGH()

FATFS fatfs;            // Work area (file system object) for logical drive

static DWORD acc_size;				/* Work register for fs command */
static WORD acc_files, acc_dirs;

FILINFO Finfo;

char * myts_file = "0:/a.txt";

bool SDCardMountStatus=false;

bool SDExist=false;

u32 SD_Capacity=0;

//static DIR Dir;					/* Directory object */
//DIR start_dirs;          		//目录起点信息

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//列举指定目录下的文件
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FRESULT scan_files (
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
			fn = *Finfo.lfname ? Finfo.lfname : Finfo.fname;
#else
			fn = Finfo.fname;
#endif
			if (Finfo.fattrib & AM_DIR) {
				acc_dirs++;
				*(path+i) = '/'; strcpy(path+i+1, fn);
				res = scan_files(path);
				*(path+i) = '\0';
				if (res != FR_OK) break;
			} else {
 				printf("%s/%s\n", path, fn);
				acc_files++;
				acc_size += Finfo.fsize;
			}
		}
	}
	return res;
}

u32 SDCardFSInit()
{ 
	u32 SD_capp;
	if(SDCardMountStatus == false)
	{
		if(f_mount(0,&fatfs) == FR_OK)//Try to mount sdcard
		{
			SDCardMountStatus = true;
		}
	}
	if(SDCardMountStatus==1)
	{
	 SD_capp = SD_GetSectorCount();
	 SD_capp = SD_capp/2048;
	if(SD_capp)
	{
   SD_Capacity=SD_capp;
	 SDExist=true;		
	}
	else
	{
   SDExist=false;		
	}
	}
	else
	{
	 SDExist=false;		
	}
	return(SD_capp);
}
