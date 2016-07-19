/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : mass_mal.c
* Author             : MCD Application Team
* Version            : V2.2.0
* Date               : 06/13/2008
* Description        : Medium Access Layer interface
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"  	

#include "stm32f10x_sdio.h"

//#include "mmc_sd.h"
#include "mass_mal.h"
#include "usb_lib.h"
#include "sdcard.h"
#include "sdcardff.h"
#include "W25Q64.h"
#include "W25Q64ff.h"

u32 Mass_Memory_Size[2];
u32 Mass_Block_Size[2];
u32 Mass_Block_Count[2];

u8 Usb_Status_Reg = 0;

/*******************************************************************************
* Function Name  : MAL_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 MAL_Init(u8 lun)
{
	u16 Status = MAL_OK;
	switch (lun)
	{
	case 0:
		break;
	case 1:
		break;
	default:
		return MAL_FAIL;
	}
	return Status;
}

/*******************************************************************************
* Function Name  : MAL_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : 0,OK
				   1,FAIL
*******************************************************************************/
u16 MAL_Write(u8 lun, u32 Memory_Offset, u32 *Writebuff, u16 Transfer_Length)
{
	u8 STA;
	u8 NbrOfBlock;
	
	switch (lun)
	{
	case 0:
		NbrOfBlock = Transfer_Length / 512;
		if (NbrOfBlock == 1)
			STA = SD_WriteBlock(Memory_Offset, (u32*)Writebuff, 512);
		else
			STA = SD_WriteMultiBlocks(Memory_Offset, (u32*)Writebuff, 512, NbrOfBlock);
		break;
	case 1:
		STA = 0;
	  NbrOfBlock = Transfer_Length / 4096;
		if (NbrOfBlock == 1)
		{
			W25X_Erase_Sector(Memory_Offset / 4096);
			W25X_Write_Sector(Memory_Offset / 4096, (u8*)Writebuff);
		}
		else return MAL_FAIL;
	default:
		return MAL_FAIL;
	}
	if (STA != 0)return MAL_FAIL;
	return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : 0,OK
				   1,FAIL
*******************************************************************************/
u16 MAL_Read(u8 lun, u32 Memory_Offset, u32 *Readbuff, u16 Transfer_Length)
{
	u8 STA;
	u8 NbrOfBlock;
	
	switch (lun)
	{
	case 0:
		NbrOfBlock = Transfer_Length / 512;
		if (NbrOfBlock == 1)
			STA = SD_ReadBlock(Memory_Offset, (u32*)Readbuff, 512);
		else
			STA = SD_ReadMultiBlocks(Memory_Offset, (u32*)Readbuff, 512, NbrOfBlock);
		break;
	case 1:
		STA = 0;
	  NbrOfBlock = Transfer_Length / 4096;
		if (NbrOfBlock == 1)
			W25X_Read_Sector(Memory_Offset / 4096, (u8*)Readbuff);
		else return MAL_FAIL;
	default:
		return MAL_FAIL;
	}
	if (STA != 0)return MAL_FAIL;
	return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_GetStatus
* Description    : Get status
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 MAL_GetStatus(u8 lun)
{
	uint32_t DeviceSizeMul = 0, NumberOfBlocks = 0;
	SD_Error Status;
	if (lun == 0)
	{
		if (SD_Init() == SD_OK)
		{
			SD_GetCardInfo(&SDCardInfo);
			SD_SelectDeselect((uint32_t)(SDCardInfo.RCA << 16));
			DeviceSizeMul = (SDCardInfo.SD_csd.DeviceSizeMul + 2);

			if (SDCardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)
			{
				Mass_Block_Count[0] = (SDCardInfo.SD_csd.DeviceSize + 1) * 1024;
			}
			else
			{
				NumberOfBlocks = ((1 << (SDCardInfo.SD_csd.RdBlockLen)) / 512);
				Mass_Block_Count[0] = ((SDCardInfo.SD_csd.DeviceSize + 1) * (1 << DeviceSizeMul) << (NumberOfBlocks / 2));
			}
			Mass_Block_Size[0] = 512;

			Status = SD_SelectDeselect((uint32_t)(SDCardInfo.RCA << 16));
			Status = SD_EnableWideBusOperation(SDIO_BusWide_4b);
			if (Status != SD_OK)
			{
				return MAL_FAIL;
			}

			Status = SD_SetDeviceMode(SD_DMA_MODE);
			if (Status != SD_OK)
			{
				return MAL_FAIL;
			}
			return MAL_OK;
		}
		else
		{
		 Mass_Block_Size[0] = 0;
		 Mass_Block_Count[0] = 0;	
		}			
		return MAL_FAIL;
	}
	else if (lun == 1)
	{
		if(SPIFlashMountStatus)
		{
		 Mass_Block_Size[1] = 4096;
		 Mass_Block_Count[1] = 2048;
		}
		else
		{
		 Mass_Block_Size[1] = 0;
		 Mass_Block_Count[1] = 0;		 
		}
		return MAL_OK;
	}
	return MAL_FAIL;
}

bool MAL_Mount()
{
	USB_Init();
	return true;
}


/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
