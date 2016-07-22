//File Name   W25Q64.c
//Description SPI FLASH Driver

#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_gpio.h"

#include "W25Q64.h"
#include "W25Q64ff.h"

#define USE_BANK_SIZE         (unsigned long)0X10000

#define DUMMY_BYTE           0xff

/*******************************************************************************
* Function Name  : SPI_ReadWriteByte
* Description    : SPI读写一个字节（发送完成后返回本次通讯读取的数据）
* Input          : u8 TxData 待发送的数
* Output         : None
* Return         : u8 RxData 收到的数
*******************************************************************************/
u8 SPI_ReadWriteByte(u8 TxData)
{
	while ((SPI1->SR & 1 << 1) == 0);//等待发送区空				  
	SPI1->DR = TxData;	 	  //发送一个byte   
	while ((SPI1->SR & 1 << 0) == 0);//等待接收完一个byte  
	return SPI1->DR;          //返回收到的数据		
}

/**
  * @brief Init GPIO for SPI FLASH CS pin
  */
void W25X_CS_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	FLASH_CS_DISABLE();

}

/**
  * @brief Init hardware of W25Q64
  */
void W25Q64_Hardware_Init()
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	W25X_CS_Init();
	FLASH_CS_DISABLE();

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                    //设置SPI1为主模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  //SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;                //串行时钟在不操作时，时钟为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                //第二个时钟沿开始采样数据
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                //NSS信号由软件（使用SSI位）管理
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; //定义波特率预分频的值:波特率预分频值为8
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;    //数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;    //CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);
	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE);   //使能SPI1外设	 
}

/**
  * @brief Init W25Q64
  */
void W25Q64_Init()
{
 FRESULT res;
 W25Q64_Hardware_Init();
 res = f_mount(&SPI_FLASH_fatfs, "1:/", 1);
 if(res==FR_OK) SPIFlashMountStatus=true;
}

/**
  * @brief Receive a byte with dummy command
  */
u8 SPI_Read_Byte(void)
{
	return (SPI_ReadWriteByte(DUMMY_BYTE));
}

#if USE_GETCHIPID
/**
  * @brief Get chip ID
  */
int W25X_GetChipID(void)
{
	int nID = 0;

	FLASH_CS_ENABLE();

	SPI_ReadWriteByte(W25X_JedecDeviceID);
	nID = SPI_Read_Byte();
	nID <<= 8;
	nID |= SPI_Read_Byte();
	nID <<= 8;
	nID |= SPI_Read_Byte();

	FLASH_CS_DISABLE();

	return nID;
}
#endif

#if USE_READ_STATUSREG
/**
  * @brief Read status register
  */
u8 W25X_Read_StatusReg(void)
{
	u8 u8 = 0;
	FLASH_CS_ENABLE();

	SPI_ReadWriteByte(W25X_ReadStatusReg);
	u8 = SPI_Read_Byte();

	FLASH_CS_DISABLE();
	return u8;
}
#endif

#if USE_WRITE_STATUSREG
/**
  * @brief Write status register
  */
void W25X_Write_StatusReg(u8 reg)
{
	FLASH_CS_ENABLE();
	SPI_Write_Byte(W25X_WriteStatusReg);
	SPI_Write_Byte(reg);
	FLASH_CS_DISABLE();
}
#endif

#if USE_WRITE_ENABLE
/**
  * @brief Disable write protection
  */
void W25X_Write_Enable(void)
{
	FLASH_CS_ENABLE();
	SPI_ReadWriteByte(W25X_WriteEnable);
	FLASH_CS_DISABLE();
}
#endif

#if USE_WRITE_ENABLE
/**
  * @brief Enable write protection
  */
void W25X_Write_Disable(void)
{
	FLASH_CS_ENABLE();
	SPI_ReadWriteByte(W25X_WriteDisable);
	FLASH_CS_DISABLE();
}
#endif

#if USE_WAIT_BUSY
/**
  * @brief Wait busy
  */
void W25X_Wait_Busy(void)
{
	while (W25X_Read_StatusReg() == 0x03)
		W25X_Read_StatusReg();
}
#endif

#if USE_ERASE_SECTOR
/**
  * @brief Erase sector
  */
void W25X_Erase_Sector(uint32_t nDest)
{
	nDest *= FLASH_SECTOR_SIZE;

	FLASH_CS_ENABLE();
	W25X_Write_Enable();
	FLASH_CS_ENABLE();
	SPI_ReadWriteByte(W25X_SectorErase);
	SPI_ReadWriteByte((u8)((nDest & 0xFFFFFF) >> 16));
	SPI_ReadWriteByte((u8)((nDest & 0xFFFF) >> 8));
	SPI_ReadWriteByte((u8)nDest & 0xFF);
	FLASH_CS_DISABLE();
	W25X_Wait_Busy();
}
#endif

#if USE_ERASE_BLOCK
/**
  * @brief Erase block
  */
void W25X_Erase_Block(uint32_t nDest)
{
	nDest *= FLASH_BLOCK_SIZE;

	FLASH_CS_ENABLE();
	W25X_Write_Enable();
	FLASH_CS_ENABLE();
	SPI_ReadWriteByte(W25X_SectorErase);
	SPI_ReadWriteByte((u8)((nDest & 0xFFFFFF) >> 16));
	SPI_ReadWriteByte((u8)((nDest & 0xFFFF) >> 8));
	SPI_ReadWriteByte((u8)nDest & 0xFF);
	FLASH_CS_DISABLE();
	W25X_Wait_Busy();
}
#endif

#if USE_ERASE_CHIP
/**
  * @brief Erase chip
  */
void W25X_Erase_Chip(void)
{
	FLASH_CS_ENABLE();
	W25X_Write_Enable();
	FLASH_CS_ENABLE();
	W25X_Wait_Busy();
	FLASH_CS_ENABLE();
	SPI_ReadWriteByte(W25X_ChipErase);
	FLASH_CS_DISABLE();
	W25X_Wait_Busy();
}
#endif

#if USE_READ_BYTES
/**
  * @brief Read into buffer
  */
void W25X_Read_Bytes(uint32_t nDest, u8* pBuffer, u8 nBytes)
{
	uint16_t i;

	FLASH_CS_ENABLE();
	SPI_ReadWriteByte(W25X_ReadData);
	SPI_ReadWriteByte((u8)(nDest >> 16));
	SPI_ReadWriteByte((u8)(nDest >> 8));
	SPI_ReadWriteByte((u8)nDest);
	for (i = 0; i < nBytes; i++)
	{
		pBuffer[i] = SPI_Read_Byte();
	}
	FLASH_CS_DISABLE();
	W25X_Wait_Busy();
}
#endif

#if USE_READ_SECTOR
/**
  * @brief Read sector
  */
void W25X_Read_Sector(uint32_t nSector, u8* pBuffer)
{
	uint16_t i;

	nSector *= FLASH_SECTOR_SIZE;

	FLASH_CS_ENABLE();
	SPI_ReadWriteByte(W25X_ReadData);
	SPI_ReadWriteByte((u8)(nSector >> 16));
	SPI_ReadWriteByte((u8)(nSector >> 8));
	SPI_ReadWriteByte((u8)nSector);

	for (i = 0; i < FLASH_SECTOR_SIZE; i++)
	{
		pBuffer[i] = SPI_Read_Byte();
	}
	FLASH_CS_DISABLE();
	W25X_Wait_Busy();
}
#endif

#if USE_WRITE_BYTES
/**
  * @brief Write from buffer
  */
void W25X_Write_Bytes(uint32_t nSector, u8* pBuffer, u8 nBytes)
{
	int i;

	FLASH_CS_ENABLE();
	W25X_Write_Enable();
	FLASH_CS_ENABLE();

	SPI_ReadWriteByte(W25X_PageProgram);
	SPI_ReadWriteByte((u8)(nSector >> 16));
	SPI_ReadWriteByte((u8)(nSector >> 8));
	SPI_ReadWriteByte((u8)nSector);
	for (i = 0; i < nBytes; i++)
	{
		SPI_ReadWriteByte(pBuffer[i]);
	}
	FLASH_CS_DISABLE();
	W25X_Wait_Busy();
}
#endif

#if USE_WRITE_SECTOR
/**
  * @brief Write a sector
  */
void W25X_Write_Sector(uint32_t nSector, u8* pBuffer)
{
	int i, j;

	nSector *= FLASH_SECTOR_SIZE;

	for (j = 0; j < FLASH_PAGES_PER_SECTOR; j++)
	{
		FLASH_CS_ENABLE();
		W25X_Write_Enable();
		FLASH_CS_ENABLE();

		SPI_ReadWriteByte(W25X_PageProgram);
		SPI_ReadWriteByte((u8)(nSector >> 16));
		SPI_ReadWriteByte((u8)(nSector >> 8));
		SPI_ReadWriteByte((u8)nSector);

		for (i = 0; i < FLASH_PAGE_SIZE; i++)
			SPI_ReadWriteByte(pBuffer[i]);

		pBuffer += FLASH_PAGE_SIZE;
		nSector += FLASH_PAGE_SIZE;

		FLASH_CS_DISABLE();
		W25X_Wait_Busy();
	}
}
#endif
