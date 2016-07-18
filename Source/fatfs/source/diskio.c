/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
//#include "usbdisk.h"	/* Example: Header file of existing USB MSD control module */
//#include "atadrive.h"	/* Example: Header file of existing ATA harddisk control module */
#include "sdcard.h"		/* Example: Header file of existing MMC/SDC contorl module */
#include "sdcardff.h"

#include "RTC.h"

#include  <stdio.h>

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
	switch (pdrv)
	{
	case 0:
		return RES_OK;
	case 1:
		return RES_OK;
	case 2:
		return RES_OK;
	default:
		return STA_NOINIT;
	}
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	return 0;
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
	if (pdrv || !count)
	{
		return RES_PARERR; 
	}
	
	if (count == 1)            
	{
		res = SD_ReadBlock( sector<<9, (u32 *)(&buff[0]) , 512);
	}
	else                 
	{
		res = SD_ReadMultiBlocks(sector<<9, (u32 *)(&buff[0]) , 512 , count );
	}
	
	if (res == SD_OK)
	{
		return RES_OK;
	}
	else
	{
		return RES_ERROR;
	}
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

	if (pdrv || !count)
	{
		return RES_PARERR;  
	}
	
	if (count == 1)
	{
		res = SD_WriteBlock( sector<<9, (u32 *)(&buff[0]) , 512);
	}
	else
	{
		res = SD_WriteMultiBlocks(sector<<9, (u32 *)(&buff[0]) , 512 , count );
	}
	
	if (res == SD_OK)
	{
		return RES_OK;
	}
	else
	{
		return RES_ERROR;
	}
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


	if (pdrv)
	{
		return RES_PARERR; 
	}

	switch (cmd)
	{
	case CTRL_SYNC:
		   break;
	case GET_BLOCK_SIZE:
		*(WORD*)buff = 512;
		res = RES_OK;
		break;

	case GET_SECTOR_COUNT:
		res = RES_OK;
		break;
	default:
		res = RES_PARERR;
		break;
	}

	return res;
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


