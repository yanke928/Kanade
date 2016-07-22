/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "sdcard.h"		
#include "sdcardff.h"
#include "W25Q64.h"		

#include  <stdio.h>

#include "diskio.h"		/* FatFs lower layer API */

/* Definitions of physical drive number for each drive */

#define SDIO_SDCARD 0
#define SPI_FLASH 1

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv)
	{
	case SDIO_SDCARD:
		if (SDCard_Exist()) return RES_OK;
		   else return RES_ERROR;
	case SPI_FLASH:
		return RES_OK;
	}
	return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv)
	{
	case SDIO_SDCARD:
	if(!SDCard_Exist()) return RES_ERROR;
	if (SD_Init() != SD_OK ||
		SD_GetCardInfo(&SDCardInfo) != SD_OK ||
		SD_SelectDeselect((u32)(SDCardInfo.RCA << 16)) != SD_OK ||
		SD_EnableWideBusOperation(SDIO_BusWide_4b) != SD_OK) 	return RES_ERROR;
	return RES_OK;
	case SPI_FLASH:
		return RES_OK;
	}
	return RES_ERROR;
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
	case SDIO_SDCARD:
		res = SD_ReadDisk((u8*)buff, sector, count);
		if (res == SD_OK)
		{
			return RES_OK;
		}
		else
		{
			return RES_ERROR;
		}
	case SPI_FLASH:
		if (count == 1)
		{
			W25X_Read_Sector(sector, (u8*)buff);
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
	case SDIO_SDCARD:
		res = SD_WriteDisk((u8*)buff, sector, count);
		if (res == SD_OK)
		{
			return RES_OK;
		}
		else
		{
			return RES_ERROR;
		}
	case SPI_FLASH:
		if (count == 1)
		{
			W25X_Erase_Sector(sector);
			W25X_Write_Sector(sector, (u8*)buff);
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
	DRESULT res = RES_OK;
	switch (cmd)
	{
	case CTRL_SYNC:
		break;
	case GET_SECTOR_SIZE:
		if (pdrv == 0)
			*(WORD*)buff = 512;
		else
			*(WORD*)buff = 4096;
		res = RES_OK;
		break;
	default:
		res = RES_OK;
		break;
	}

	return res;
}
