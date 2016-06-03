/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
*                      Pin assignment:
*             ----------------------------------------------
*             |  STM32F10x    |     MSD          Pin        |
*             ----------------------------------------------
*             | PA.0          |   ChipSelect      1         |
*             | PA.7 / MOSI   |   DataIn          2         |
*             |               |   GND             3 (0 V)   |
*             |               |   VDD             4 (3.3 V) |
*             | PA.5 / SCLK   |   Clock           5         |
*             |               |   GND             6 (0 V)   |
*             | PA.6 / MISO   |   DataOut         7         |
*             -----------------------------------------------
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "SDCard.h"
#include "SDCardff.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "startup.h"

#include <stdio.h>

extern vu8 SD_Card_Ready;//SD卡初始化成功标志，在diskio.c中
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


/* Global Variable ------------------------------------------------------------*/  
u8  SD_Type=0;//SD卡的类型,为了支持高速卡而加入

u32 Mass_Block_Count;
u32 Mass_Block_Size;
u32 Mass_Memory_Size;
sMSD_CSD MSD_csd;


/* Private functions ---------------------------------------------------------*/

//进行移植时,需要修改
void SD_SPI_Configuration(void)//配置SPI接口	
{
    SPI_InitTypeDef SPI_InitStructure ;
    GPIO_InitTypeDef GPIO_InitStructure ;

	/* GPIOA and GPIOB Periph clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);


    //启动SPI时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);

    //////下面是SPI相关GPIO初始化//////
    //SPI1模块对应的SCK、MISO、MOSI为AF引脚
    GPIO_InitStructure.GPIO_Pin=SPI_PINS;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz ;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP ;
    GPIO_Init(SPI_PORT,&GPIO_InitStructure);

	  GPIO_InitStructure.GPIO_Pin = SPI_MISO; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   //上拉输入
    GPIO_Init(SPI_PORT,&GPIO_InitStructure);	
    
    
    //PB6 pin:SD_CS
    GPIO_InitStructure.GPIO_Pin=SD_CS_PIN;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz ;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP ;
    GPIO_Init(SD_CS_PORT,&GPIO_InitStructure); 

	GPIO_SetBits(SD_CS_PORT,SD_CS_PIN);


    SPI_Cmd(SD_SPI_NUM,DISABLE);

	//////SPI模块配置//////

  	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  	SPI_InitStructure.SPI_CRCPolynomial = 7;

    SPI_Init(SD_SPI_NUM,&SPI_InitStructure);
    SPI_Cmd(SD_SPI_NUM,ENABLE);
    SPI_I2S_ClearITPendingBit(SD_SPI_NUM, SPI_I2S_IT_RXNE);
}
 
  
//进行移植时,需要修改
void SD_SPI_HightSpeed(void)  //提高SD卡的读写速度
{
	SPI_InitTypeDef SPI_InitStructure ;

    SPI_Cmd(SD_SPI_NUM,DISABLE);

    //////SPI模块配置//////
  	/* SPI1 Config */
  	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  	SPI_InitStructure.SPI_CRCPolynomial = 7;

    SPI_Init(SD_SPI_NUM,&SPI_InitStructure);
    SPI_Cmd(SD_SPI_NUM,ENABLE);
    SPI_I2S_ClearITPendingBit(SD_SPI_NUM, SPI_I2S_IT_RXNE);
}  
/*******************************************************************************
* Function Name  : MSD_WriteByte
* Description    : Write a byte on the MSD.
* Input          : Data: byte to send.
* Output         : None
* Return         : None.
*******************************************************************************/
//进行移植时,需要修改
void MSD_WriteByte(u8 Data)
{
	while((SD_SPI_NUM->SR&1<<1)==0);//等待发送区空				  
	SD_SPI_NUM->DR=Data;	 	    //发送一个byte   	
}

/*******************************************************************************
* Function Name  : MSD_ReadByte
* Description    : Read a byte from the MSD.
* Input          : None.
* Output         : None
* Return         : The received byte.
*******************************************************************************/
//进行移植时,需要修改
u8 MSD_ReadByte(void)
{
	while((SD_SPI_NUM->SR&1<<1)==0);//等待发送区空				  
	SD_SPI_NUM->DR=DUMMY;	 	    //发送一个byte   
	while((SD_SPI_NUM->SR&1<<0)==0);//等待接收完一个byte  
	return SD_SPI_NUM->DR;          //返回收到的数据				    
}

	 
/*******************************************************************************
* Function Name  : SPI1_ReadWriteByte
* Description    : SPI读写一个字节（发送完成后返回本次通讯读取的数据）
* Input          : u8 TxData 待发送的数
* Output         : None
* Return         : u8 RxData 收到的数
*******************************************************************************/
//进行移植时,需要修改
u8 SPI1_ReadWriteByte(u8 TxData)//在高速SD卡的驱动程序中用到
{
	while((SPI1->SR&1<<1)==0);//等待发送区空				  
	SPI1->DR=TxData;	 	  //发送一个byte   
	while((SPI1->SR&1<<0)==0);//等待接收完一个byte  
	return SPI1->DR;          //返回收到的数据				    
}



/*******************************************************************************
* Function Name  : MSD_Init
* Description    : Initializes the MSD/SD communication.
* Input          : None
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed 
*******************************************************************************/
extern void SD_SPI_Configuration(void);
u8 MSD_Init(void)
{
  u8 ret;
#if SDHC_EN
  /*********2011/02/27 by xingxing*********/
  //为了支持高速sd卡而加入的
  ret = SDHC_Init();
  SD_Card_Ready = ret;
  return ret;
#else
  u32 i = 0;	
	
  /* Initialize SPI1 */
  SD_SPI_Configuration();
  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte 0xFF, 10 times with CS high*/
  /* rise CS and MOSI for 80 clocks cycles */
  for (i = 0; i <= 9; i++)
  {
    /* Send dummy byte 0xFF */
    MSD_WriteByte(DUMMY);
  }
  /*------------Put MSD in SPI mode--------------*/
  /* MSD initialized and set to SPI mode properly */
  ret = MSD_GoIdleState();//修改者：WXT 2011/02/26 11:37
  SD_Card_Ready = ret;
  return ret;
  //return (MSD_GoIdleState());	//修改者：WXT 2011/02/26 11:37
#endif
}

/*******************************************************************************
* Function Name  : MSD_WriteBlock
* Description    : Writes a block on the MSD
* Input          : - pBuffer : pointer to the buffer containing the data to be
*                    written on the MSD.
*                  - WriteAddr : address to write on.
*                  - NumByteToWrite: number of data to write
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed 
*******************************************************************************/
u8 MSD_WriteBlock(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  u32 i = 0;
  u8 rvalue = MSD_RESPONSE_FAILURE;

  /* MSD chip select low */
  MSD_CS_LOW();
  /* Send CMD24 (MSD_WRITE_BLOCK) to write multiple block */
  MSD_SendCmd(MSD_WRITE_BLOCK, WriteAddr, 0xFF);

  /* Check if the MSD acknowledged the write block command: R1 response (0x00: no errors) */
  if (!MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
  {
    /* Send a dummy byte */
    MSD_WriteByte(DUMMY);
    /* Send the data token to signify the start of the data */
    MSD_WriteByte(0xFE);
    /* Write the block data to MSD : write count data by block */
    for (i = 0; i < NumByteToWrite; i++)
    {
      /* Send the pointed byte */
      MSD_WriteByte(*pBuffer);
      /* Point to the next location where the byte read will be saved */
      pBuffer++;
    }
    /* Put CRC bytes (not really needed by us, but required by MSD) */
    MSD_ReadByte();
    MSD_ReadByte();
    /* Read data response */
    if (MSD_GetDataResponse() == MSD_DATA_OK)
    {
      rvalue = MSD_RESPONSE_NO_ERROR;
    }
  }

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY);
  /* Returns the reponse */
  return rvalue;
}

/*******************************************************************************
* Function Name  : MSD_ReadBlock
* Description    : Reads a block of data from the MSD.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the MSD.
*                  - ReadAddr : MSD's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the MSD.
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed 
*******************************************************************************/
u8 MSD_ReadBlock(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  u32 i = 0;
  u8 rvalue = MSD_RESPONSE_FAILURE;

  /* MSD chip select low */
  MSD_CS_LOW();
  /* Send CMD17 (MSD_READ_SINGLE_BLOCK) to read one block */
  MSD_SendCmd(MSD_READ_SINGLE_BLOCK, ReadAddr, 0xFF);

  /* Check if the MSD acknowledged the read block command: R1 response (0x00: no errors) */
  if (!MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
  {
    /* Now look for the data token to signify the start of the data */
    if (!MSD_GetResponse(MSD_START_DATA_SINGLE_BLOCK_READ))
    {
      /* Read the MSD block data : read NumByteToRead data */
      for (i = 0; i < NumByteToRead; i++)
      {
        /* Save the received data */
        *pBuffer = MSD_ReadByte();
        /* Point to the next location where the byte read will be saved */
        pBuffer++;
      }
      /* Get CRC bytes (not really needed by us, but required by MSD) */
      MSD_ReadByte();
      MSD_ReadByte();
      /* Set response value to success */
      rvalue = MSD_RESPONSE_NO_ERROR;
    }
  }

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY);
  /* Returns the reponse */
  return rvalue;
}

/*******************************************************************************
* Function Name  : MSD_WriteBuffer
* Description    : Writes many blocks on the MSD
* Input          : - pBuffer : pointer to the buffer containing the data to be
*                    written on the MSD.
*                  - WriteAddr : address to write on.
*                  - NumByteToWrite: number of data to write
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed 
*******************************************************************************/
u8 MSD_WriteBuffer(u8* pBuffer, u32 WriteAddr, u32 NumByteToWrite)
{
  u32 i = 0, NbrOfBlock = 0, Offset = 0;
  u8 rvalue = MSD_RESPONSE_FAILURE;

  /* Calculate number of blocks to write */
  NbrOfBlock = NumByteToWrite / BLOCK_SIZE;
  /* MSD chip select low */
  MSD_CS_LOW();

  /* Data transfer */
  while (NbrOfBlock --)
  {
    /* Send CMD24 (MSD_WRITE_BLOCK) to write blocks */
    MSD_SendCmd(MSD_WRITE_BLOCK, WriteAddr + Offset, 0xFF);

    /* Check if the MSD acknowledged the write block command: R1 response (0x00: no errors) */
    if (MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
    {
      return MSD_RESPONSE_FAILURE;
    }
    /* Send dummy byte */
    MSD_WriteByte(DUMMY);
    /* Send the data token to signify the start of the data */
    MSD_WriteByte(MSD_START_DATA_SINGLE_BLOCK_WRITE);
    /* Write the block data to MSD : write count data by block */
    for (i = 0; i < BLOCK_SIZE; i++)
    {
      /* Send the pointed byte */
      MSD_WriteByte(*pBuffer);
      /* Point to the next location where the byte read will be saved */
      pBuffer++;
    }
    /* Set next write address */
    Offset += 512;
    /* Put CRC bytes (not really needed by us, but required by MSD) */
    MSD_ReadByte();
    MSD_ReadByte();
    /* Read data response */
    if (MSD_GetDataResponse() == MSD_DATA_OK)
    {
      /* Set response value to success */
      rvalue = MSD_RESPONSE_NO_ERROR;
    }
    else
    {
      /* Set response value to failure */
      rvalue = MSD_RESPONSE_FAILURE;
    }
  }

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY);
  /* Returns the reponse */
  return rvalue;
}

/*******************************************************************************
* Function Name  : MSD_ReadBuffer
* Description    : Reads multiple block of data from the MSD.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the MSD.
*                  - ReadAddr : MSD's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the MSD.
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed 
*******************************************************************************/
u8 MSD_ReadBuffer(u8* pBuffer, u32 ReadAddr, u32 NumByteToRead)
{
  u32 i = 0, NbrOfBlock = 0, Offset = 0;
  u8 rvalue = MSD_RESPONSE_FAILURE;

  /* Calculate number of blocks to read */
  NbrOfBlock = NumByteToRead / BLOCK_SIZE;
  /* MSD chip select low */
  MSD_CS_LOW();

  /* Data transfer */
  while (NbrOfBlock --)
  {
    /* Send CMD17 (MSD_READ_SINGLE_BLOCK) to read one block */
    MSD_SendCmd (MSD_READ_SINGLE_BLOCK, ReadAddr + Offset, 0xFF);
    /* Check if the MSD acknowledged the read block command: R1 response (0x00: no errors) */
    if (MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
    {
      return  MSD_RESPONSE_FAILURE;
    }
    /* Now look for the data token to signify the start of the data */
    if (!MSD_GetResponse(MSD_START_DATA_SINGLE_BLOCK_READ))
    {
      /* Read the MSD block data : read NumByteToRead data */
      for (i = 0; i < BLOCK_SIZE; i++)
      {
        /* Read the pointed data */
        *pBuffer = MSD_ReadByte();
        /* Point to the next location where the byte read will be saved */
        pBuffer++;
      }
      /* Set next read address*/
      Offset += 512;
      /* get CRC bytes (not really needed by us, but required by MSD) */
      MSD_ReadByte();
      MSD_ReadByte();
      /* Set response value to success */
      rvalue = MSD_RESPONSE_NO_ERROR;
    }
    else
    {
      /* Set response value to failure */
      rvalue = MSD_RESPONSE_FAILURE;
    }
  }

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY);
  /* Returns the reponse */
  return rvalue;
}

/*******************************************************************************
* Function Name  : MSD_GetCSDRegister
* Description    : Read the CSD card register.
*                  Reading the contents of the CSD register in SPI mode
*                  is a simple read-block transaction.
* Input          : - MSD_csd: pointer on an SCD register structure
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed 
*******************************************************************************/
u8 MSD_GetCSDRegister(sMSD_CSD* MSD_csd)
{
  u32 i = 0;
  u8 rvalue = MSD_RESPONSE_FAILURE;
  u8 CSD_Tab[16];

  /* MSD chip select low */
  MSD_CS_LOW();
  /* Send CMD9 (CSD register) or CMD10(CSD register) */
  MSD_SendCmd(MSD_SEND_CSD, 0, 0xFF);

  /* Wait for response in the R1 format (0x00 is no errors) */
  if (!MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
  {
    if (!MSD_GetResponse(MSD_START_DATA_SINGLE_BLOCK_READ))
    {
      for (i = 0; i < 16; i++)
      {
        /* Store CSD register value on CSD_Tab */
        CSD_Tab[i] = MSD_ReadByte();
      }
    }
    /* Get CRC bytes (not really needed by us, but required by MSD) */
    MSD_WriteByte(DUMMY);
    MSD_WriteByte(DUMMY);
    /* Set response value to success */
    rvalue = MSD_RESPONSE_NO_ERROR;
  }

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY);

  /* Byte 0 */
  MSD_csd->CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
  MSD_csd->SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
  MSD_csd->Reserved1 = CSD_Tab[0] & 0x03;
  /* Byte 1 */
  MSD_csd->TAAC = CSD_Tab[1] ;
  /* Byte 2 */
  MSD_csd->NSAC = CSD_Tab[2];
  /* Byte 3 */
  MSD_csd->MaxBusClkFrec = CSD_Tab[3];
  /* Byte 4 */
  MSD_csd->CardComdClasses = CSD_Tab[4] << 4;
  /* Byte 5 */
  MSD_csd->CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
  MSD_csd->RdBlockLen = CSD_Tab[5] & 0x0F;
  /* Byte 6 */
  MSD_csd->PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
  MSD_csd->WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
  MSD_csd->RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
  MSD_csd->DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
  MSD_csd->Reserved2 = 0; /* Reserved */
  MSD_csd->DeviceSize = (CSD_Tab[6] & 0x03) << 10;
  /* Byte 7 */
  MSD_csd->DeviceSize |= (CSD_Tab[7]) << 2;
  /* Byte 8 */
  MSD_csd->DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;
  MSD_csd->MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
  MSD_csd->MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);
  /* Byte 9 */
  MSD_csd->MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
  MSD_csd->MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
  MSD_csd->DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
  /* Byte 10 */
  MSD_csd->DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
  MSD_csd->EraseGrSize = (CSD_Tab[10] & 0x7C) >> 2;
  MSD_csd->EraseGrMul = (CSD_Tab[10] & 0x03) << 3;
  /* Byte 11 */
  MSD_csd->EraseGrMul |= (CSD_Tab[11] & 0xE0) >> 5;
  MSD_csd->WrProtectGrSize = (CSD_Tab[11] & 0x1F);
  /* Byte 12 */
  MSD_csd->WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
  MSD_csd->ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
  MSD_csd->WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
  MSD_csd->MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;
  /* Byte 13 */
  MSD_csd->MaxWrBlockLen |= (CSD_Tab[13] & 0xc0) >> 6;
  MSD_csd->WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
  MSD_csd->Reserved3 = 0;
  MSD_csd->ContentProtectAppli = (CSD_Tab[13] & 0x01);
  /* Byte 14 */
  MSD_csd->FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
  MSD_csd->CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
  MSD_csd->PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
  MSD_csd->TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
  MSD_csd->FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
  MSD_csd->ECC = (CSD_Tab[14] & 0x03);
  /* Byte 15 */
  MSD_csd->CRC_Check = (CSD_Tab[15] & 0xFE) >> 1;
  MSD_csd->Reserved4 = 1;

  /* Return the reponse */
  return rvalue;
}

/*******************************************************************************
* Function Name  : MSD_GetCIDRegister
* Description    : Read the CID card register.
*                  Reading the contents of the CID register in SPI mode
*                  is a simple read-block transaction.
* Input          : - MSD_cid: pointer on an CID register structure
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed 
*******************************************************************************/
u8 MSD_GetCIDRegister(sMSD_CID* MSD_cid)
{
  u32 i = 0;
  u8 rvalue = MSD_RESPONSE_FAILURE;
  u8 CID_Tab[16];

  /* MSD chip select low */
  MSD_CS_LOW();
  /* Send CMD10 (CID register) */
  MSD_SendCmd(MSD_SEND_CID, 0, 0xFF);

  /* Wait for response in the R1 format (0x00 is no errors) */
  if (!MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
  {
    if (!MSD_GetResponse(MSD_START_DATA_SINGLE_BLOCK_READ))
    {
      /* Store CID register value on CID_Tab */
      for (i = 0; i < 16; i++)
      {
        CID_Tab[i] = MSD_ReadByte();
      }
    }
    /* Get CRC bytes (not really needed by us, but required by MSD) */
    MSD_WriteByte(DUMMY);
    MSD_WriteByte(DUMMY);
    /* Set response value to success */
    rvalue = MSD_RESPONSE_NO_ERROR;
  }

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY);

  /* Byte 0 */
  MSD_cid->ManufacturerID = CID_Tab[0];
  /* Byte 1 */
  MSD_cid->OEM_AppliID = CID_Tab[1] << 8;
  /* Byte 2 */
  MSD_cid->OEM_AppliID |= CID_Tab[2];
  /* Byte 3 */
  MSD_cid->ProdName1 = CID_Tab[3] << 24;
  /* Byte 4 */
  MSD_cid->ProdName1 |= CID_Tab[4] << 16;
  /* Byte 5 */
  MSD_cid->ProdName1 |= CID_Tab[5] << 8;
  /* Byte 6 */
  MSD_cid->ProdName1 |= CID_Tab[6];
  /* Byte 7 */
  MSD_cid->ProdName2 = CID_Tab[7];
  /* Byte 8 */
  MSD_cid->ProdRev = CID_Tab[8];
  /* Byte 9 */
  MSD_cid->ProdSN = CID_Tab[9] << 24;
  /* Byte 10 */
  MSD_cid->ProdSN |= CID_Tab[10] << 16;
  /* Byte 11 */
  MSD_cid->ProdSN |= CID_Tab[11] << 8;
  /* Byte 12 */
  MSD_cid->ProdSN |= CID_Tab[12];
  /* Byte 13 */
  MSD_cid->Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
  /* Byte 14 */
  MSD_cid->ManufactDate = (CID_Tab[13] & 0x0F) << 8;
  /* Byte 15 */
  MSD_cid->ManufactDate |= CID_Tab[14];
  /* Byte 16 */
  MSD_cid->CRC_Check = (CID_Tab[15] & 0xFE) >> 1;
  MSD_cid->Reserved2 = 1;

  /* Return the reponse */
  return rvalue;
}

/*******************************************************************************
* Function Name  : MSD_SendCmd
* Description    : Send 5 bytes command to the MSD card.
* Input          : - Cmd: the user expected command to send to MSD card
*                  - Arg: the command argument
*                  - Crc: the CRC
* Output         : None
* Return         : None
*******************************************************************************/
void MSD_SendCmd(u8 Cmd, u32 Arg, u8 Crc)
{
  u32 i = 0x00;
  u8 Frame[6];

  /* Construct byte1 */
  Frame[0] = (Cmd | 0x40);
  /* Construct byte2 */
  Frame[1] = (u8)(Arg >> 24);
  /* Construct byte3 */
  Frame[2] = (u8)(Arg >> 16);
  /* Construct byte4 */
  Frame[3] = (u8)(Arg >> 8);
  /* Construct byte5 */
  Frame[4] = (u8)(Arg);
  /* Construct CRC: byte6 */
  Frame[5] = (Crc);

  /* Send the Cmd bytes */
  for (i = 0; i < 6; i++)
  {
    MSD_WriteByte(Frame[i]);
  }
}

/*******************************************************************************
* Function Name  : MSD_GetDataResponse
* Description    : Get MSD card data response.
* Input          : None
* Output         : None
* Return         : The MSD status: Read data response xxx0<status>1
*                   - status 010: Data accecpted
*                   - status 101: Data rejected due to a crc error
*                   - status 110: Data rejected due to a Write error.
*                   - status 111: Data rejected due to other error.
*******************************************************************************/
u8 MSD_GetDataResponse(void)
{
  u32 i = 0;
  u8 response, rvalue;

  while (i <= 64)
  {
    /* Read resonse */
    response = MSD_ReadByte();
    /* Mask unused bits */
    response &= 0x1F;

    switch (response)
    {
      case MSD_DATA_OK:
      {
        rvalue = MSD_DATA_OK;
        break;
      }

      case MSD_DATA_CRC_ERROR:
        return MSD_DATA_CRC_ERROR;

      case MSD_DATA_WRITE_ERROR:
        return MSD_DATA_WRITE_ERROR;

      default:
      {
        rvalue = MSD_DATA_OTHER_ERROR;
        break;
      }
    }
    /* Exit loop in case of data ok */
    if (rvalue == MSD_DATA_OK)
      break;
    /* Increment loop counter */
    i++;
  }
  /* Wait null data */
  while (MSD_ReadByte() == 0);
  /* Return response */
  return response;
}

/*******************************************************************************
* Function Name  : MSD_GetResponse
* Description    : Returns the MSD response.
* Input          : None
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed 
*******************************************************************************/
u8 MSD_GetResponse(u8 Response)
{
  u32 Count = 0xff;

  /* Check if response is got or a timeout is happen */
  while ((MSD_ReadByte() != Response) && Count)
  {
    Count--;
  }

  if (Count == 0)
  {
    /* After time out */
    return MSD_RESPONSE_FAILURE;
  }
  else
  {
    /* Right response got */
    return MSD_RESPONSE_NO_ERROR;
  }
}

/*******************************************************************************
* Function Name  : MSD_GetStatus
* Description    : Returns the MSD status.
* Input          : None
* Output         : None
* Return         : The MSD status.
*******************************************************************************/
u16 MSD_GetStatus(void)
{
  u16 Status = 0;

  /* MSD chip select low */
  MSD_CS_LOW();
  /* Send CMD13 (MSD_SEND_STATUS) to get MSD status */
  MSD_SendCmd(MSD_SEND_STATUS, 0, 0xFF);

  Status = MSD_ReadByte();
  Status |= (u16)(MSD_ReadByte() << 8);

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte 0xFF */
  MSD_WriteByte(DUMMY);

  return Status;
}

/*******************************************************************************
* Function Name  : MSD_GoIdleState
* Description    : Put MSD in Idle state.
* Input          : None
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed 
*******************************************************************************/
u8 MSD_GoIdleState(void)
{
//超时退出
#if  XING_DO
   u32 Count = 0xFFF*2;
#endif

  /* MSD chip select low */
  MSD_CS_LOW();
  /* Send CMD0 (GO_IDLE_STATE) to put MSD in SPI mode */
  MSD_SendCmd(MSD_GO_IDLE_STATE, 0, 0x95);

  /* Wait for In Idle State Response (R1 Format) equal to 0x01 */
  if (MSD_GetResponse(MSD_IN_IDLE_STATE))
  {
    /* No Idle State Response: return response failue */
    return MSD_RESPONSE_FAILURE;
  }
  /*----------Activates the card initialization process-----------*/
  do
  {
    /* MSD chip select high */
    MSD_CS_HIGH();
    /* Send Dummy byte 0xFF */
    MSD_WriteByte(DUMMY);

    /* MSD chip select low */
    MSD_CS_LOW();

    /* Send CMD1 (Activates the card process) until response equal to 0x0 */
    MSD_SendCmd(MSD_SEND_OP_COND, 0, 0xFF);
    /* Wait for no error Response (R1 Format) equal to 0x00 */

//超时退出
#if  XING_DO
    Count--;
    if(Count==0){
       /* No Idle State Response: return response failue */
       return MSD_RESPONSE_FAILURE;
    }
#endif

  }
  while (MSD_GetResponse(MSD_RESPONSE_NO_ERROR));

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte 0xFF */
  MSD_WriteByte(DUMMY);

#if  XING_DO
  SD_SPI_HightSpeed();//提升SPI速度
  Get_Medium_Characteristics();//获取卡的信息
  SD_Card_Ready = 0;
#else
  SD_SPI_HightSpeed();//提升SPI速度
#endif

  return MSD_RESPONSE_NO_ERROR;
}


/*******************************************************************************
* Function Name  : Get_Medium_Characteristics.
* Description    : Get the microSD card size.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Get_Medium_Characteristics(void)
{
  u32 temp1 = 0;
  u32 temp2 = 0;

  MSD_GetCSDRegister(&MSD_csd);

  temp1 = MSD_csd.DeviceSize + 1;
  temp2 = 1 << (MSD_csd.DeviceSizeMul + 2);

  Mass_Block_Count = temp1 * temp2;

  Mass_Block_Size =  1 << MSD_csd.RdBlockLen;

  Mass_Memory_Size = (Mass_Block_Count * Mass_Block_Size);

}

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/





//>>>>>>>>>>>>>>>>>>>>>>>>>>>start of  code<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//整理者: Sasukewxt
//时间  : 2011/02/27
//说明  : 以下加入的代码是为了使系统支持高速SDHC卡
//------------------------------------------------

/******************************************************************************* 
*  本文件为SPI操作SD卡的底层驱动文件
*  包括SPI模块及相关IO的初始化，SPI读写SD卡（写指令，读数据等）
*******************************************************************************/   

	 
/*******************************************************************************
* Function Name  : SD_SendCommand
* Description    : 向SD卡发送一个命令
* Input          : u8 cmd   命令 
*                  u32 arg  命令参数
*                  u8 crc   crc校验值
* Output         : None
* Return         : u8 r1 SD卡返回的响应
*******************************************************************************/
u8 SD_SendCommand(u8 cmd, u32 arg, u8 crc)
{
    unsigned char r1;
    unsigned char Retry = 0;

    //????????
    SPI1_ReadWriteByte(0xff);
    //片选端置低，选中SD卡
    SD_CS_ENABLE();

    //发送
    SPI1_ReadWriteByte(cmd | 0x40);                         //分别写入命令
    SPI1_ReadWriteByte(arg >> 24);
    SPI1_ReadWriteByte(arg >> 16);
    SPI1_ReadWriteByte(arg >> 8);
    SPI1_ReadWriteByte(arg);
    SPI1_ReadWriteByte(crc);
    
    //等待响应，或超时退出
    while((r1 = SPI1_ReadWriteByte(0xFF))==0xFF)
    {
        Retry++;
        if(Retry > RE_TRY_CNT)break; 
    }   
    //关闭片选
    SD_CS_DISABLE();
    //在总线上额外增加8个时钟，让SD卡完成剩下的工作
    SPI1_ReadWriteByte(0xFF);

    //返回状态值
    return r1;
}


/*******************************************************************************
* Function Name  : SD_SendCommand_NoDeassert
* Description    : 向SD卡发送一个命令(结束是不失能片选，还有后续数据传来）
* Input          : u8 cmd   命令 
*                  u32 arg  命令参数
*                  u8 crc   crc校验值
* Output         : None
* Return         : u8 r1 SD卡返回的响应
*******************************************************************************/
u8 SD_SendCommand_NoDeassert(u8 cmd, u32 arg, u8 crc)
{
    unsigned char r1;
    unsigned char Retry = 0;

    SPI1_ReadWriteByte(0xff);
    //片选端置低，选中SD卡
    SD_CS_ENABLE();

    //发送
    SPI1_ReadWriteByte(cmd | 0x40);                         //分别写入命令
    SPI1_ReadWriteByte(arg >> 24);
    SPI1_ReadWriteByte(arg >> 16);
    SPI1_ReadWriteByte(arg >> 8);
    SPI1_ReadWriteByte(arg);
    SPI1_ReadWriteByte(crc);

    //等待响应，或超时退出
    while((r1 = SPI1_ReadWriteByte(0xFF))==0xFF)
    {
        Retry++;
        if(Retry > RE_TRY_CNT)break;   
    }
    //返回响应值
    return r1;
}

/*******************************************************************************
* Function Name  : SDHC_Init
* Description    : 初始化SDHC卡
* Input          : None
* Output         : None
* Return         : u8 
*                  0：NO_ERR
*                  1：TIME_OUT
*                  99：NO_CARD
*******************************************************************************/
u8 SDHC_Init(void)// 初始化高速SD卡,由MSD_Init调用
{
    u16 i;      // 用来循环计数
    u8 r1;      // 存放SD卡的返回值
    u16 retry;  // 用来进行超时计数
    u8 buff[6];
					 
	/* Initialize SPI1 */
    SD_SPI_Configuration();

	SD_CS_ENABLE();
					  
    // 纯延时，等待SD卡上电完成
    for(i=0;i<0x2f00;i++);

    //先产生>74个脉冲，让SD卡自己初始化完成
    for(i=0;i<10;i++)
    {
        SPI1_ReadWriteByte(0xFF);
    }

    //-----------------SD卡复位到idle开始-----------------
    //循环连续发送CMD0，直到SD卡返回0x01,进入IDLE状态
    //超时则直接退出
    retry = 0;
    do
    {
        //发送CMD0，让SD卡进入IDLE状态
        r1 = SD_SendCommand(CMD0, 0, 0x95);
        retry++;
    }while((r1 != 0x01) && (retry<RE_TRY_CNT));
    //跳出循环后，检查原因：初始化成功？or 重试超时？
    if(retry==RE_TRY_CNT) return 1;   //超时返回1	  
    //-----------------SD卡复位到idle结束-----------------	 
    //获取卡片的SD版本信息
    r1 = SD_SendCommand_NoDeassert(8, 0x1aa, 0x87);	     
    //如果卡片版本信息是v1.0版本的，即r1=0x05，则进行以下初始化
    if(r1 == 0x05)
    {
        //设置卡类型为SDV1.0，如果后面检测到为MMC卡，再修改为MMC
        SD_Type = SD_TYPE_V1;	   
        //如果是V1.0卡，CMD8指令后没有后续数据
        //片选置高，结束本次命令
        SD_CS_DISABLE();
        //多发8个CLK，让SD结束后续操作
        SPI1_ReadWriteByte(0xFF);	  
        //-----------------SD卡、MMC卡初始化开始-----------------	 
        //发卡初始化指令CMD55+ACMD41
        // 如果有应答，说明是SD卡，且初始化完成
        // 没有回应，说明是MMC卡，额外进行相应初始化
        retry = 0;
        do
        {
            //先发CMD55，应返回0x01；否则出错
            r1 = SD_SendCommand(CMD55, 0, 0);
            if(r1 != 0x01)return r1;	  
            //得到正确响应后，发ACMD41，应得到返回值0x00，否则重试200次
            r1 = SD_SendCommand(ACMD41, 0, 0);
            retry++;
        }while((r1!=0x00) && (retry<RE_TRY_CNT));
        // 判断是超时还是得到正确回应
        // 若有回应：是SD卡；没有回应：是MMC卡
        
        //----------MMC卡额外初始化操作开始------------
        if(retry==RE_TRY_CNT)
        {
            retry = 0;
            //发送MMC卡初始化命令（没有测试）
            do
            {
                r1 = SD_SendCommand(1, 0, 0);
                retry++;
            }while((r1!=0x00)&& (retry<RE_TRY_CNT));
            if(retry==RE_TRY_CNT)return 1;   //MMC卡初始化超时		    
            //写入卡类型
            SD_Type = SD_TYPE_MMC;
        }
        //----------MMC卡额外初始化操作结束------------	    
        //设置SPI为高速模式
		//SD_SPI_HightSpeed();//提升SPI速度  
		SPI1_ReadWriteByte(0xFF);
        
        //禁止CRC校验	   
		r1 = SD_SendCommand(CMD59, 0, 0x95);
        if(r1 != 0x00)return r1;  //命令错误，返回r1   	   
        //设置Sector Size
        r1 = SD_SendCommand(CMD16, 512, 0x95);
        if(r1 != 0x00)return r1;//命令错误，返回r1		 
        //-----------------SD卡、MMC卡初始化结束-----------------

    }//SD卡为V1.0版本的初始化结束	 
    //下面是V2.0卡的初始化
    //其中需要读取OCR数据，判断是SD2.0还是SD2.0HC卡
    else if(r1 == 0x01)
    {
        //V2.0的卡，CMD8命令后会传回4字节的数据，要跳过再结束本命令
        buff[0] = SPI1_ReadWriteByte(0xFF);  //should be 0x00
        buff[1] = SPI1_ReadWriteByte(0xFF);  //should be 0x00
        buff[2] = SPI1_ReadWriteByte(0xFF);  //should be 0x01
        buff[3] = SPI1_ReadWriteByte(0xFF);  //should be 0xAA	    
        SD_CS_DISABLE();	  
        SPI1_ReadWriteByte(0xFF);//the next 8 clocks			 
        //判断该卡是否支持2.7V-3.6V的电压范围
        //if(buff[2]==0x01 && buff[3]==0xAA) //不判断，让其支持的卡更多
        {	  
            retry = 0;
            //发卡初始化指令CMD55+ACMD41
    		do
    		{
    			r1 = SD_SendCommand(CMD55, 0, 0);
    			if(r1!=0x01)return r1;	   
    			r1 = SD_SendCommand(ACMD41, 0x40000000, 0);
                if(retry>RE_TRY_CNT)return r1;  //超时则返回r1状态  
            }while(r1!=0);		  
            //初始化指令发送完成，接下来获取OCR信息		   
            //-----------鉴别SD2.0卡版本开始-----------
            r1 = SD_SendCommand_NoDeassert(CMD58, 0, 0);
            if(r1!=0x00)return r1;  //如果命令没有返回正确应答，直接退出，返回应答		 
            //读OCR指令发出后，紧接着是4字节的OCR信息
            buff[0] = SPI1_ReadWriteByte(0xFF);
            buff[1] = SPI1_ReadWriteByte(0xFF); 
            buff[2] = SPI1_ReadWriteByte(0xFF);
            buff[3] = SPI1_ReadWriteByte(0xFF);

            //OCR接收完成，片选置高
            SD_CS_DISABLE();
            SPI1_ReadWriteByte(0xFF);

            //检查接收到的OCR中的bit30位（CCS），确定其为SD2.0还是SDHC
            //如果CCS=1：SDHC   CCS=0：SD2.0
            if(buff[0]&0x40)SD_Type = SD_TYPE_V2HC;    //检查CCS	 
            else SD_Type = SD_TYPE_V2;	    
            //-----------鉴别SD2.0卡版本结束----------- 
            //设置SPI为高速模式
			//SD_SPI_HightSpeed();//提升SPI速度 
        }	    
    }
    return r1;
}

//获取SD卡的CSD信息，包括容量和速度信息
//输入:u8 *cid_data(存放CID的内存，至少16Byte）	    
//返回值:0：NO_ERR
//		 1：错误	
u8 CSD_Tab[16];

u8 SD_GetCSD(u8 *csd_data)
{
  u32 i = 0;
  u8 rvalue = MSD_RESPONSE_FAILURE;
  

  /* MSD chip select low */
  MSD_CS_LOW();
  /* Send CMD9 (CSD register) or CMD10(CSD register) */
  MSD_SendCmd(MSD_SEND_CSD, 0, 0xFF);

  /* Wait for response in the R1 format (0x00 is no errors) */
  if (!MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
  {
    if (!MSD_GetResponse(MSD_START_DATA_SINGLE_BLOCK_READ))
    {
      for (i = 0; i < 16; i++)
      {
        /* Store CSD register value on CSD_Tab */
        CSD_Tab[i] = MSD_ReadByte();
      }
    }
    /* Get CRC bytes (not really needed by us, but required by MSD) */
    MSD_WriteByte(DUMMY);
    MSD_WriteByte(DUMMY);
    /* Set response value to success */
    rvalue = MSD_RESPONSE_NO_ERROR;
  }

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY);
  
  return rvalue;
}  
//获取SD卡的总扇区数（扇区数）   
//返回值:0： 取容量出错 
//       其他:SD卡的容量(扇区数/512字节)
//每扇区的字节数必为512，因为如果不是512，则初始化不能通过.														  
u32 SD_GetSectorCount(void)
{
    u32 Capacity;  
    u8 n;
	u32 csize;  
	u8 * csd = CSD_Tab;
	//取CSD信息，如果期间出错，返回0
    if(SD_GetCSD(csd)!=0) return 0;	    
    //如果为SDHC卡，按照下面方式计算
	
    if((csd[0]&0xC0)==0x40)	 //V2.00的卡
    {	
		csize = csd[9] + ((u32)csd[8] << 8) + 1;
		Capacity = (u32)csize << 10;//得到扇区数	 		   
    }else//V1.XX的卡
    {	
		n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
		csize = (csd[8] >> 6) + ((u16)csd[7] << 2) + ((u16)(csd[6] & 3) << 10) + 1;
		Capacity= (u32)csize << (n - 9);//得到扇区数   
    }
    return Capacity;
}

/**
  * @brief  Sdcard init task

	  @retval None
  */
void sdcard_Init()
{
 u32 sd_capacity;
 char tempString[20];
 xQueueSend(InitStatusMsg, "Detecting SD...", 0);
 MSD_Init();
 vTaskDelay(100 / portTICK_RATE_MS);
 MSD_Init();
 sd_capacity=SDCardFSInit();
 if(sd_capacity)
 {
	 sprintf(tempString,"Capcity:%dMB",sd_capacity);
	 xQueueSend(InitStatusMsg, tempString, 0);
 } 
 else
 {
 	 xQueueSend(InitStatusMsg, "No SDCard", 0); 
 }
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>end of  code<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
