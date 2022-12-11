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



void fatfs_test(void *arg) {

//
//    res = f_open(&fp, "0:/helloworld.txt", FA_READ | FA_WRITE);//挂载SD卡到path: 0:
//    if (res == FR_OK) {
//        f_read(&fp, read_buff, 512, &read_bytes);
//        if (read_bytes > 0) {
//            mcu_uart_write(0, (uint8_t *) read_buff, (int) read_bytes);
//        }
//        read_bytes = sprintf_(read_buff, "\ngot %d bytes this time\n", read_bytes);
//        f_write(&fp, read_buff, read_bytes, &read_bytes);    /* Write data to the file */
//        f_sync(&fp);
//    }
    fatfs_mount_init();
    while (1) {
        fatfs_ls();
        vTaskDelay(1000);
    }
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
    //4位抢占优先级，0位响应优先级
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    //mcu_uart_open(CP2102_PORT);
    extern void ST_USART_Config(void);
    ST_USART_Config();

    SD_Init();//先初始化SDIO
    extern void msc_ram_init(void);
    msc_ram_init();

    //使用STM32F407的CCMRAM来保存FreeRTOS的堆空间，此API必须在所以FreeRTOS的API之前调用
    vPortDefineHeapRegions(xHeapRegions);

    vRegisterSampleCLICommands();
    vUARTCommandConsoleStart(512, 1);

//    xTaskCreate(fatfs_test,  /* 任务入口函数 */
//                "fatfs_test",    /* 任务名字 */
//                4096,    /* 任务栈大小 */
//                NULL,        /* 任务入口函数参数 */
//                1,  /* 任务的优先级 */
//                NULL);  /* 任务控制块指针 */
    vTaskStartScheduler();
    for (;;) {}
}