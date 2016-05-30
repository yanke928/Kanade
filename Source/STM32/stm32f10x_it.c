/**
  ******************************************************************************
  * @file    Project/Template/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.0.0
  * @date    04/06/2009
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

  /* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"

/** @addtogroup Template_Project
  * @{
  */

  /* Private typedef -----------------------------------------------------------*/
  /* Private define ------------------------------------------------------------*/
  /* Private macro -------------------------------------------------------------*/
  /* Private variables ---------------------------------------------------------*/
  /* Private function prototypes -----------------------------------------------*/
  /* Private functions ---------------------------------------------------------*/

  /******************************************************************************/
  /*            Cortex-M3 Processor Exceptions Handlers                         */
  /******************************************************************************/

//void ShowSystemHalted(char string1[])
//{
//	DialogueBoxConfigurationStruct dialogueBoxConfig;
//	strcpy(dialogueBoxConfig.TitleString, "System Halted!!!");
//	strcpy(dialogueBoxConfig.SubString1, string1);
//	strcpy(dialogueBoxConfig.SubString2, "Occurred!!");
//	dialogueBoxConfig.LastingTime = HALT;
//	dialogueBoxConfig.OccupyThread=true;
//	ShowDialogue(dialogueBoxConfig); 
//}

  /**
	* @brief  This function handles NMI exception.
	* @param  None
	* @retval : None
	*/
void NMI_Handler(void)
{
//	ShowSystemHalted("A NMI Fault");
}


/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval : None
  */
void HardFault_Handler(void)
{
//	#if DEBUG_EN
//	char tempString[20];
//	sprintf(tempString,"A HardFault(%02d)",FaultDebug);
//	ShowSystemHalted(tempString);
//	#else
//	ShowSystemHalted("A HardFault");
//	#endif
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval : None
  */
void MemManage_Handler(void)
{
//	ShowSystemHalted("An MMU Fault");
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval : None
  */
void BusFault_Handler(void)
{
//	ShowSystemHalted("A BusFault");
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval : None
  */
void UsageFault_Handler(void)
{
//	ShowSystemHalted("A Usage Fault");
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval : None
  */
void SVC_Handler(void)
{
//	ShowSystemHalted("An SVCall Fault");
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval : None
  */
void DebugMon_Handler(void)
{
	while (1);
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval : None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval : None
  */
void SysTick_Handler(void)
{

}

/**
  * @brief  This function handles USB inquiry.
  * @param  None
  * @retval : None
  */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
//	__USBCDC_ISR();
}



/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval : None
  */
  /*void PPP_IRQHandler(void)
  {
  }*/

  /**
	* @}
	*/


	/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
