/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/main.h 
  * @author  MCD Application Team
  * @version V1.8.1
  * @date    27-January-2022
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "printf.h"

//freeRTOS
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"


//#include "core_cm4.h"
//#include "cmsis_gcc.h"
#include "common_tool.h"
#include "board_led.h"
#include "printf.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define ATCCM __attribute__((section (".ccmram")))
/* Exported functions ------------------------------------------------------- */
extern void msc_ram_init(void);  //初始化USB-MSC


#endif /* __MAIN_H */

