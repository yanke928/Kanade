#ifndef __STM32_FLASH_H
#define __STM32_FLASH_H

#define STM32_FLASH_SIZE 512

#define STM32_FLASH_BASE	0x08000000

#define FLASH_APP_ADDR		0x08008000 

#define STM32_SECTOR_SIZE	2048

u16 STMFLASH_ReadHalfWord(u32 faddr);	
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead); 

void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		
void STMFLASH_WriteOnePage(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);

#endif 

