#include "stm32f10x.h"

#include "Keys.h"
#include "JumpToApp.h"

int main(void)
{ 
  Keys_GPIO_Init();
	if(LEFT_KEY==KEY_ON)
	{
	 while(1);
	}
	Jump2App();
}



