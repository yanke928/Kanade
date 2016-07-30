/**
  ******************************************************************************
  * @file    mass_mal.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Medium Access Layer interface
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"  	

#include "stm32f10x_sdio.h"

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
		if(!SDCard_Init(false)) Status=MAL_FAIL;
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
		if(!SDCard_Exist()) return MAL_FAIL;
		NbrOfBlock = Transfer_Length / 512;
		STA=SD_WriteDisk((u8*)Writebuff,Memory_Offset/512, Transfer_Length/512);
	  if(STA==SD_OK) STA=MAL_OK;
	  else STA=MAL_FAIL;
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
		if(!SDCard_Exist()) return MAL_FAIL;
		NbrOfBlock = Transfer_Length / 512;
		STA=SD_ReadDisk((u8*)Readbuff,Memory_Offset/512, Transfer_Length/512);
	  if(STA==SD_OK) STA=MAL_OK;
	  else STA=MAL_FAIL;
	case 1:
		STA = 0;
	  NbrOfBlock = Transfer_Length / 4096;
		if (NbrOfBlock == 1)
			W25X_Read_Sector(Memory_Offset / 4096, (u8*)Readbuff);
		else return MAL_FAIL;
	default:
		return MAL_FAIL;
	}
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
		if(!SDCard_Exist()) return MAL_FAIL;
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
		if(1)
		{
		 Mass_Block_Size[1] = 4096;
		 Mass_Block_Count[1] = 2048;
		}
		else
		{
		 Mass_Block_Size[1] = 0;
		 Mass_Block_Count[1] = 0;		 
		 return MAL_FAIL;
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


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
