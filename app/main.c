/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mcu_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ff.h"
#include "FreeRTOS_CLI_Public.h"
#include "lwrb.h"
#include "stm324xg_eval_sdio_sd.h"

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




FATFS fs;//文件系统对象
DIR dp1;
FIL fp;//文件对象
FILINFO fno;
char *write_text = "FATFS test success!";
unsigned int write_bytes = 0;
char read_buff[512];
unsigned int read_bytes = 0;
char current_dir_str[256] = {0};

char fatfs_test_command_paste_buffer[512];

void fatfs_test(void *arg) {
    int32_t uart_caches_len;
    size_t ret;

    while (0) {
        vTaskDelay(1000);
    }

    printf_("fatfs_test_command_paste_buffer in task addr is 0x%X \n", &fatfs_test_command_paste_buffer);
    printf_("value 2 in task addr is 0x%X \n", &ret);

    FRESULT res;
    if (FR_OK == f_mount(&fs, "", 1))//挂载SD卡到path: 0:，并创建文件系统对象的句柄
    {
        printf("mount fs success!\n");
    }
    res = f_opendir(&dp1, "");
    if (res != FR_OK) {
        printf_("f_opendir (&dp1, \"0:\") failed\n");
        goto endd;
    }
    while (1) {
        res = f_readdir(&dp1, &fno);
        if (res != FR_OK) {
            printf_("f_readdir (&dp1, &fno) failed\n");
            goto endd;
        }
        if (strlen(fno.fname) < 1) {
            printf_("end of dir\n");
            goto endd;
        }
        printf_("file name:%s\n", fno.fname);
        printf_("file size:%llu\n", fno.fsize);
        vTaskDelay(1000);
    }

    endd:
    f_unmount("");
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
    while (1) {
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

    xTaskCreate(fatfs_test,  /* 任务入口函数 */
                "fatfs_test",    /* 任务名字 */
                4096,    /* 任务栈大小 */
                NULL,        /* 任务入口函数参数 */
                1,  /* 任务的优先级 */
                NULL);  /* 任务控制块指针 */
    vTaskStartScheduler();
    for (;;) {}
}