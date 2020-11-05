#ifndef USERTASK_H
#define USERTASK_H

#ifdef __cplusplus
#define CPPCEXPORT extern "C"
#else
#define CPPCEXPORT
#endif

#include "stm32f4xx_hal.h"

CPPCEXPORT void MainLoopTask();
CPPCEXPORT void SysTickTask();
CPPCEXPORT void BootTaskAfterHwInit();


#endif //USERTASK_H
