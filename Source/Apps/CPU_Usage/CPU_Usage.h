#ifndef __CPUSTATE_H__
#define __CPUSTATE_H__

#include <stdbool.h>

#define ABS(x) (x)>=0?(x):(-x)//ABS definition
/*-----------------------------------------------------------------------------------------*/

#define CPU_USAGE_PRIORITY tskIDLE_PRIORITY+5


/*-----------------------------------------------------------------------------------------*/
void  OSStatInit(void);
/*-----------------------------------------------------------------------------------------*/
extern  volatile  unsigned int  OSIdleCtr;                                 /* Idle counter                   */
extern  unsigned int      OSIdleCtrMax;             /* Max. value that idle ctr can take in 1 sec.     */
extern  unsigned int      OSIdleCtrRun;             /* Val. reached by idle ctr at run time in 1 sec.  */
extern float      OSCPUUsage;               /* Percentage of CPU used  */
extern bool CPU_Stat_Running;

#endif /* __CPUSTATE_H__ */
