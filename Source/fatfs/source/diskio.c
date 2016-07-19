/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "sdcard.h"		
#include "sdcardff.h"
#include "W25Q64.h"		

#include "RTC.h"

#include  <stdio.h>

#include "diskio.h"		/* FatFs lower layer API */

/* Definitions of physical drive number for each drive */

vu8 SD_Card_Ready;

extern u8  SD_Type;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
 return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	u8 res = 0;
	switch (pdrv)
	{
	case 0:
		portENTER_CRITICAL();
		if (count == 1)
		{
			res = SD_ReadBlock(sector << 9, (u32 *)(&buff[0]), 512);
		}
		else
		{
			res = SD_ReadMultiBlocks(sector << 9, (u32 *)(&buff[0]), 512, count);
		}
    portEXIT_CRITICAL();
		if (res == SD_OK)
		{
			return RES_OK;
		}
		else
		{
			return RES_ERROR;
		}
	case 1:
		if (count == 1)
		{
			portENTER_CRITICAL();
			W25X_Read_Sector(sector, (u8*)buff);
			portEXIT_CRITICAL();
			return RES_OK;
		}
		else return RES_ERROR;
	}
	return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write(
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	u8 res;
	switch (pdrv)
	{
	case 0:
		portENTER_CRITICAL();
		if (count == 1)
		{
			res = SD_WriteBlock(sector << 9, (u32 *)(&buff[0]), 512);
		}
		else
		{
			res = SD_WriteMultiBlocks(sector << 9, (u32 *)(&buff[0]), 512, count);
		}
    portEXIT_CRITICAL();
		if (res == SD_OK)
		{
			return RES_OK;
		}
		else
		{
			return RES_ERROR;
		}
	case 1:
		if (count == 1)
		{
			portENTER_CRITICAL();
			W25X_Erase_Sector(sector);
			W25X_Write_Sector(sector, (u8*)buff);
			portEXIT_CRITICAL();
			return RES_OK;
		}
		else return RES_ERROR;
	}
	return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;

	switch (cmd)
	{
//	case CTRL_SYNC:
//		break;
	case GET_SECTOR_SIZE:
		if (pdrv == 0)
			*(WORD*)buff = 512;
		else
			*(WORD*)buff = 4096;
		res = RES_OK;
		break;

//	case GET_SECTOR_COUNT:
//		res = RES_OK;
//		break;
//	default:
//		res = RES_PARERR;
//		break;
	}

	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* User defined function to give a current time to fatfs module          */
/* 31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31) */
/* 15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */
DWORD get_fattime(void)
{
#if	 GET_FATTIME_EN
	DWORD temp;
	temp = (RTCTime.w_year - 1980) << 25 | RTCTime.w_month << 21 | RTCTime.w_date << 16 |
		RTCTime.hour << 11 | RTCTime.min << 5 | RTCTime.sec;
	return temp;
#else
	return 0;
#endif
}


