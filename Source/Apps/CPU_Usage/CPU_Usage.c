//File Name     CPU_Usage.c
//Description : This app measures the cpu usage

#include "FreeRTOS.h"
#include "task.h"
#include "CPU_Usage.h"

#include "SSD1306.h"

#include <stdio.h>

volatile  unsigned int  OSIdleCtr;                                 /* Idle counter                   */
unsigned int      OSIdleCtrMax;             /* Max. value that idle ctr can take in 1 sec.     */
unsigned int      OSIdleCtrRun;             /* Val. reached by idle ctr at run time in 1 sec.  */
float     OSCPUUsage;               /* Percentage of CPU used  */
bool CPU_Stat_Running = false;

void OS_TaskStat(void * pvParameters)
{
	char tempString[10];
	portTickType xLastWakeTime;
	OSIdleCtrMax /= 100;
	if (OSIdleCtrMax == 0)
	{
		OSCPUUsage = 0;
	}
	xLastWakeTime = xTaskGetTickCount();
	for (;;)
	{
		OSIdleCtrRun = OSIdleCtr;           /* Obtain the of the idle counter for the past second */
		OSIdleCtr = 0;                      /* Reset the idle counter for the next second         */
		OSCPUUsage = (100 - OSIdleCtrRun / (float)OSIdleCtrMax);
		OSCPUUsage = ABS(OSCPUUsage);
		sprintf(tempString, "%03.0f%%", OSCPUUsage);
		xSemaphoreTake(OLEDRelatedMutex, portMAX_DELAY);
		OLED_ShowAnyString(0, 0, tempString, NotOnSelect, 12);
		xSemaphoreGive(OLEDRelatedMutex);
		vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_RATE_MS);
	}
}

void  OSStatInit(void)
{
	OSIdleCtr = 0;                          /* Clear idle counter*/
	vTaskDelay(100 / portTICK_RATE_MS);           /* Determine MAX. idle counter value for 1/10 second ,100ms delay */
	OSIdleCtrMax = OSIdleCtr;                    /* Store maximum idle counter count in 1/10 second*/
	xTaskCreate(OS_TaskStat, "OS_TaskStat", 128, NULL, CPU_USAGE_PRIORITY, NULL);
	CPU_Stat_Running = true;
}

void vApplicationIdleHook(void)
{
	OSIdleCtr++;
}
