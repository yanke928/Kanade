//File Name   £ºCommand.c
//Description : CommandLines 

#include "Command.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "FreeRTOS_CLI.h"

#include "SSD1306.h"


BaseType_t TaskStatsCommand(char  *pcWriteBuffer,
	size_t xWriteBufferLen,
	const char  *pcCommandString)
{
	OLED_ShowString(0, 0, "Shabi");
	return pdFALSE;
}


CLI_Command_Definition_t  xListTasksCommand =
{
	"ListTasks",
	"List the tasks running\r\n",
	TaskStatsCommand,
	1
};

void RegisterCommands(void)
{
	;
}

