#ifndef __FASTCHARGE_TRIGGER_CIRCUIT_H
#define	__FASTCHARGE_TRIGGER_CIRCUIT_H

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#define FastCharge_Enable_Small_Divider_DP() GPIO_SetBits(GPIOC, GPIO_Pin_4)
#define FastCharge_Disable_Small_Divider_DP() GPIO_ResetBits(GPIOC, GPIO_Pin_4)
#define FastCharge_Enable_Big_Divider_DP() GPIO_SetBits(GPIOC, GPIO_Pin_5)
#define FastCharge_Disable_Big_Divider_DP() GPIO_ResetBits(GPIOC, GPIO_Pin_5)

#define FastCharge_Enable_Small_Divider_DM() GPIO_SetBits(GPIOB, GPIO_Pin_0)
#define FastCharge_Disable_Small_Divider_DM() GPIO_ResetBits(GPIOB, GPIO_Pin_0)
#define FastCharge_Enable_Big_Divider_DM() GPIO_SetBits(GPIOB, GPIO_Pin_1)
#define FastCharge_Disable_Big_Divider_DM() GPIO_ResetBits(GPIOB, GPIO_Pin_1)

#define SetDM_GND() FastCharge_Disable_Small_Divider_DM();FastCharge_Disable_Big_Divider_DM()
#define SetDM_0V6() FastCharge_Disable_Small_Divider_DM();FastCharge_Enable_Big_Divider_DM()
#define SetDM_3V3() FastCharge_Enable_Small_Divider_DM();FastCharge_Enable_Big_Divider_DM()
#define SetDM_2V7() FastCharge_Enable_Small_Divider_DM();FastCharge_Disable_Big_Divider_DM()

#define SetDP_GND() FastCharge_Disable_Small_Divider_DP();FastCharge_Disable_Big_Divider_DP()
#define SetDP_0V6() FastCharge_Disable_Small_Divider_DP();FastCharge_Enable_Big_Divider_DP()
#define SetDP_3V3() FastCharge_Enable_Small_Divider_DP();FastCharge_Enable_Big_Divider_DP()
#define SetDP_2V7() FastCharge_Enable_Small_Divider_DP();FastCharge_Disable_Big_Divider_DP()

enum {Release=0,QC2_Normal,QC2_9V,QC2_12V,QC2_20V,QC3_Increase,QC3_Decrease,MTK_Increase,MTK_Decrease};

extern xQueueHandle FastCharge_Msg;

//see .c for details
void FastCharge_Trigger_GPIO_Enable(void);

//see .c for details
void FastCharge_Trigger_Release(void);

//see .c for details
void FastCharge_Trigger_Service_Init(void);

#endif /* __FASTCHARGE_TRIGGER_CIRCUIT_H */
