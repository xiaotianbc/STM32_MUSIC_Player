//
// Created by xiaotian on 2022/12/9.
//

#ifndef F407ZG_GCC_DEMO_FREERTOS_CLI_PUBLIC_H
#define F407ZG_GCC_DEMO_FREERTOS_CLI_PUBLIC_H

#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"

extern void vRegisterSampleCLICommands( void );
extern void vUARTCommandConsoleStart( uint16_t usStackSize, UBaseType_t uxPriority );

#endif //F407ZG_GCC_DEMO_FREERTOS_CLI_PUBLIC_H
