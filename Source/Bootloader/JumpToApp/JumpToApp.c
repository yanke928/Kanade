#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "misc.h"

#include "JumpToApp.h"

typedef  void(*pFunction)(void);
#define ApplicationAddress 0x08008000
uint32_t JumpAddress;
pFunction Jump_To_Application;

void Jump2App()
{
	__set_PRIMASK(1);
	if (((*(__IO uint32_t *) ApplicationAddress) & 0x2FFE0000) == 0x20000000)
	{
		JumpAddress = *(__IO uint32_t *) (ApplicationAddress + 4);
		Jump_To_Application = (pFunction)JumpAddress;

		__set_MSP(*(__IO uint32_t *) ApplicationAddress);
		Jump_To_Application();
	}
 return;
}

