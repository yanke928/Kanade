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
	int result;
	if (pdrv)
	{
		return STA_NOINIT;
	}
	result = SD_Init();
	if (result == 0) { return RES_OK; }
	else { return STA_NOINIT; }
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
		return RES_PARERR;  //仅支持单磁盘操作，count不能等于0，否则返回参数错误
	}
	//    if(!SD_DET())
	//    {
	//        return RES_NOTRDY;  //没有检测到SD卡，报NOT READY错误
	//    }



	if (count == 1)            //1个sector的读操作      
	{
		res = SD_ReadSingleBlock(sector, buff);
	}
	else                    //多个sector的读操作     
	{
		res = SD_ReadMultiBlock(sector, buff, count);
	}
	/*
	do
	{
		if(SD_ReadSingleBlock(sector, buff)!=0)
		{
			res = 1;
			break;
		}
		buff+=512;
	}while(--count);
	*/
	//处理返回值，将SPI_SD_driver.c的返回值转成ff.c的返回值
	if (res == 0x00)
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
		return RES_PARERR;  //仅支持单磁盘操作，count不能等于0，否则返回参数错误
	}
	//    if(!SD_DET())
	//    {
	//        return RES_NOTRDY;  //没有检测到SD卡，报NOT READY错误
	//    }

		// 读写操作
	if (count == 1)
	{
		res = SD_WriteSingleBlock(sector, buff);
	}
	else
	{
		res = SD_WriteMultiBlock(sector, buff, count);
	}
	// 返回值转换
	if (res == 0)
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
		return RES_PARERR;  //仅支持单磁盘操作，否则返回参数错误
	}

	//FATFS目前版本仅需处理CTRL_SYNC，GET_SECTOR_COUNT，GET_BLOCK_SIZ三个命令
	switch (cmd)
	{
	case CTRL_SYNC:
		SD_CS_ENABLE();
		if (SD_WaitReady() == 0)
		{
			res = RES_OK;
		}
		else
		{
			res = RES_ERROR;
		}
		SD_CS_DISABLE();
		break;

	case GET_BLOCK_SIZE:
		*(WORD*)buff = 512;
		res = RES_OK;
		break;

	case GET_SECTOR_COUNT:
		*(DWORD*)buff = SD_GetCapacity();
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


