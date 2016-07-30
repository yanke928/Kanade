/**
  ******************************************************************************
  * @file    mass_mal.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Header for mass_mal.c file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MASS_MAL_H
#define __MASS_MAL_H

#include <stdbool.h>

/* Includes ------------------------------------------------------------------*/
/* Exported types -------------------- ----------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define MAL_OK   0
#define MAL_FAIL 1
#define MAX_LUN  1

extern u8 Usb_Status_Reg;

extern u32 Mass_Memory_Size[2];
extern u32 Mass_Block_Size[2];
extern u32 Mass_Block_Count[2];


u16 MAL_Init(u8 lun);
u16 MAL_GetStatus(u8 lun);
u16 MAL_Read(u8 lun, u32 Memory_Offset, u32 *Readbuff, u16 Transfer_Length);
u16 MAL_Write(u8 lun, u32 Memory_Offset, u32 *Writebuff, u16 Transfer_Length);
bool MAL_Mount(void);
#endif /* __MASS_MAL_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
