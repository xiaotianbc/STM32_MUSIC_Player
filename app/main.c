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


void app_id(void *a) {
    while (1){
        vTaskDelay(1000);
    };
}


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
    extern void ST_USART_Config(void);//初始化UART
    ST_USART_Config();

    SD_Init();//初始化SDIO
    extern void msc_ram_init(void);  //初始化USB-MSC
    msc_ram_init();

    //使用STM32F407的CCMRAM来保存FreeRTOS的堆空间，此API必须在所有FreeRTOS的API之前调用
    vPortDefineHeapRegions(xHeapRegions);

    vRegisterSampleCLICommands();  //注册freeRTOS-cli的命令
    vUARTCommandConsoleStart(512, 1);//启动freeRTOS-cli任务
    xTaskCreate(app_id,  /* 任务入口函数 */
                "app_main",    /* 任务名字 */
                512,    /* 任务栈大小 */
                NULL,        /* 任务入口函数参数 */
                1,  /* 任务的优先级 */
                NULL);  /* 任务控制块指针 */
    vTaskStartScheduler();//启动freeRTOS任务调度器
    for (;;) {}
}

#if 0
void demo_task(void *a) {
    //用0.5秒打印10次demo task 然后删除任务
    for (int i = 0; i < 10; ++i) {
        printf_("demo task\r\n");
        vTaskDelay(50);
    }
    vTaskDelete(NULL);
}

void app_main(void *a) {
    //每隔1秒钟创建一个demo_task
    while (1) {
        xTaskCreate(demo_task,  /* 任务入口函数 */
                    "task_ls",    /* 任务名字 */
                    4096,    /* 任务栈大小 */
                    NULL,        /* 任务入口函数参数 */
                    1,  /* 任务的优先级 */
                    NULL);  /* 任务控制块指针 */
        vTaskDelay(1000);
    }
}

void main1(void) {
    xTaskCreate(app_main,  /* 任务入口函数 */
                "app_main",    /* 任务名字 */
                512,    /* 任务栈大小 */
                NULL,        /* 任务入口函数参数 */
                1,  /* 任务的优先级 */
                NULL);  /* 任务控制块指针 */
    vTaskStartScheduler();//启动freeRTOS任务调度器
}
#endif