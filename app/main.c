/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mcu_uart.h"
#include "wujique_log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"


RCC_ClocksTypeDef RCC_Clocks;

void master_task_main(void *arg) {
    while (1) {
        printf_("hello,master_task_main\n");
        vTaskDelay(1000);
    }
}


int main(void) {
    //4位抢占优先级，0位相应优先级
    __NVIC_SetPriorityGrouping(NVIC_PriorityGroup_4);
    /* SysTick end of count event each ms */
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

    mcu_uart_open(PC_PORT);

    xTaskCreate((TaskFunction_t) master_task_main,  /* 任务入口函数 */
                "MASTER",    /* 任务名字 */
                1024,    /* 任务栈大小 */
                NULL,        /* 任务入口函数参数 */
                1,  /* 任务的优先级 */
                NULL);  /* 任务控制块指针 */;
    vTaskStartScheduler();
    while (1) {
        printf_("Hello, world\n");
    }
}
