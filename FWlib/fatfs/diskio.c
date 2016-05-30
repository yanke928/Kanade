/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs             (C)ChaN, 2010*/
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
//修改者 ：王晓腾
//时间	 ：2011/02/27
//描述   ：已经支持SDHC卡

#include "SDCard.h"
#include "diskio.h"
#include  <stdio.h>


//当使能文件系统的时间记录功能时，要加入电子钟和日历功能
#define  GET_FATTIME_EN  1

#if	 GET_FATTIME_EN
#include "RTC.h"  //电子钟和日历功能
#endif
	 
vu8 SD_Card_Ready;//SD卡初始化成功标志

extern u8  SD_Type;//SD卡的类型

//这几个变量在msd.c中,记录了sd卡的一些参数											
extern u32 Mass_Block_Count;//扇区数
extern u32 Mass_Block_Size;//扇区大小
extern u32 Mass_Memory_Size;//容量

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
    u8 state;

    if(drv)
    {
        return STA_NOINIT;  //仅支持磁盘0的操作
    }
	
    state = MSD_Init();
	SD_Card_Ready=state;//SD卡初始化成功标志

	
	if(SD_Card_Ready)//SD卡没初始化成功
	{
	   return STA_NODISK;
	}

    if(state == STA_NODISK)
    {
        return STA_NODISK;
    }
    else if(state != 0)
    {
        return STA_NOINIT;  //其他错误：初始化失败
    }
    else
    {
        return 0;           //初始化成功
    }
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
    if(drv)
    {
        return STA_NOINIT;  //仅支持磁盘0操作
    }

	if(SD_Card_Ready)//SD卡没初始化成功
	{
	   return STA_NODISK;
	}

    //检查SD卡是否插入
  //  if(!SD_DET())
  //  {
  //     return STA_NODISK;
  //  }
    return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
	u8 res=0;

    if (drv || !count)
    {   
        return RES_PARERR;  //仅支持单磁盘操作，count不能等于0，否则返回参数错误
    }

	if(SD_Card_Ready)//SD卡没初始化成功
	{
	   return RES_NOTRDY;
	}

   // if(!SD_DET())
   // {
   //     return RES_NOTRDY;  //没有检测到SD卡，报NOT READY错误
   // }

    if(count==1)            //1个sector的读操作      
    { 
	

    	//如果不是SDHC，给定的是sector地址，将其转换成byte地址
    	if(SD_Type!=SD_TYPE_V2HC)
    	{
       		res = MSD_ReadBlock(buff, (u32)sector*512, 512);  
    	}else{
	
	   		res = MSD_ReadBlock(buff, (u32)sector, 512);  
		}
  
    }                                                
    else                    //多个sector的读操作     
    
	{
  	   
		res =  MSD_ReadBuffer(buff, (u32)sector*512, (u32)count*512); 

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
    if(res == 0x00)
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

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	        /* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	u8 res;

    if (drv || !count)
    {    
        return RES_PARERR;  //仅支持单磁盘操作，count不能等于0，否则返回参数错误
    }

	if(SD_Card_Ready)//SD卡没初始化成功
	{
	   return RES_NOTRDY;
	}

	/*
    if(!SD_DET())
    {
        return RES_NOTRDY;  //没有检测到SD卡，报NOT READY错误
    }  */

    // 读写操作
    if(count == 1)
    {

    	//如果不是SDHC，给定的是sector地址，将其转换成byte地址
    	if(SD_Type!=SD_TYPE_V2HC)
    	{	      	   
			res = MSD_WriteBlock((u8*)buff, (u32)sector*512, 512);
		}else{
	    	res = MSD_WriteBlock((u8*)buff, (u32)sector, 512);
		}

    }
    else
    {
 
    	//如果不是SDHC，给定的是sector地址，将其转换成byte地址
    	if(SD_Type!=SD_TYPE_V2HC)
    	{	   
			res = MSD_WriteBuffer((u8*)buff, (u32)sector*512, (u32)count*512);
		}else{
			res = MSD_WriteBuffer((u8*)buff, (u32)sector, (u32)count*512);
		}

 
    }

    // 返回值转换
    if(res == 0)
    {
        return RES_OK;
    }
    else
    {
        return RES_ERROR;
    }
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    DRESULT res;

	if(SD_Card_Ready)//SD卡没初始化成功
	{
	   return RES_NOTRDY;
	}

	//Get_Medium_Characteristics();//获取卡的信息

    if (drv)
    {    
        return RES_PARERR;  //仅支持单磁盘操作，否则返回参数错误
    }
    
    //FATFS目前版本仅需处理CTRL_SYNC，GET_SECTOR_COUNT，GET_BLOCK_SIZ三个命令
    switch(ctrl)
    {
    case CTRL_SYNC:

        SD_CS_ENABLE();
        if(MSD_GetResponse(0xff)==0)
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
	    //Get_Medium_Characteristics();//获取卡的信息
        *(WORD*)buff = Mass_Block_Size;
		//*(WORD*)buff = 512;
        res = RES_OK;
        break;

    case GET_SECTOR_COUNT:
		//Get_Medium_Characteristics();//获取卡的信息
        *(DWORD*)buff = Mass_Block_Count;
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
DWORD get_fattime (void)
{
#if	 GET_FATTIME_EN
    DWORD temp;
	temp=(RTCTime.w_year-1980)<<25|RTCTime.w_month<<21|RTCTime.w_date<<16|
	RTCTime.hour<<11|RTCTime.min<<5|RTCTime.sec;
    return temp;
#else
     return 0;
#endif
}
