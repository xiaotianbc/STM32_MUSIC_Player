/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mcu_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ff.h"
#include "FreeRTOS_CLI_Public.h"
#include "lwrb.h"
#include "stm324xg_eval_sdio_sd.h"
#include "board_fatfs_interface.h"
#include "board_dac_sound.h"

RCC_ClocksTypeDef RCC_Clocks;

GPIO_InitTypeDef GPIO_InitStructure;

void master_task_main(void *arg) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    while (1) {
        if (GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_12) == 0) {
            vTaskDelay(20);
            if (GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_12) == 0) {
                printf_("PF12\n");
                vTaskDelay(20);
            }
        }
        //  printf_("hello,master_task_main\n");
        vTaskDelay(20);
    }
    vTaskDelete(NULL);
}


/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/





/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
HeapRegion_t xHeapRegions[] =
        {
                //stm32F407-CCMRAM
                {(uint8_t *) 0x10000000UL, 0x10000},
                {NULL,                     0}
        };

int main(void) {
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//4位抢占优先级，0位响应优先级
    //mcu_uart_open(CP2102_PORT);
    ST_USART_Config();

    SD_Init();//初始化SDIO
    extern void msc_ram_init(void);  //初始化USB-MSC
    msc_ram_init();

    //使用STM32F407的CCMRAM来保存FreeRTOS的堆空间，此API必须在所有FreeRTOS的API之前调用
    vPortDefineHeapRegions(xHeapRegions);

    vRegisterSampleCLICommands();  //注册freeRTOS-cli的命令
    vUARTCommandConsoleStart(512, 1);//启动freeRTOS-cli任务
    vTaskStartScheduler();//启动freeRTOS任务调度器
    for (;;) {}
}
