#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#include "stm32_flash.h"

u16 STMFLASH_BUF[STM32_SECTOR_SIZE/2];

u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}

void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);
		ReadAddr+=2;
	}
}

void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;
	}  
} 

void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	  
	u16 secoff;	   
	u16 secremain; 
 	u16 i;    
	u32 offaddr;  
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;
	FLASH_Unlock();					
	offaddr=WriteAddr-STM32_FLASH_BASE;		
	secpos=offaddr/STM32_SECTOR_SIZE;			
	secoff=(offaddr%STM32_SECTOR_SIZE)/2;		
	secremain=STM32_SECTOR_SIZE/2-secoff;	
	if(NumToWrite<=secremain)
		{
		secremain=NumToWrite;
		}
	else
		{
		return;
		}
	while(1) 
	{	
		STMFLASH_Read(secpos*STM32_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM32_SECTOR_SIZE/2);
		for(i=0;i<secremain;i++)
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;
		}
		if(i<secremain)
		{
			FLASH_ErasePage(secpos*STM32_SECTOR_SIZE+STM32_FLASH_BASE);
			for(i=0;i<secremain;i++)
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM32_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM32_SECTOR_SIZE/2);
		}else STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);		   
		if(NumToWrite==secremain)break;
		else
		{
			secpos++;				
			secoff=0;				 
		   	pBuffer+=secremain;  
			WriteAddr+=secremain;	   
		   	NumToWrite-=secremain;	
			if(NumToWrite>(STM32_SECTOR_SIZE/2))secremain=STM32_SECTOR_SIZE/2;//ÏÂÒ»¸öÉÈÇø»¹ÊÇÐ´²»Íê
			else secremain=NumToWrite;
		}	 
	};	
	FLASH_Lock();
}

void STMFLASH_WriteOnePage(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   
	u32 offaddr;   
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))
		{
		return;
		}
	FLASH_Unlock();					
	
	offaddr=WriteAddr-STM32_FLASH_BASE;		
	secpos=offaddr/STM32_SECTOR_SIZE;			
	
	FLASH_ErasePage(secpos*STM32_SECTOR_SIZE+STM32_FLASH_BASE);
	STMFLASH_Write_NoCheck(WriteAddr,pBuffer,NumToWrite);		   	

	FLASH_Lock();
}
