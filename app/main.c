/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mcu_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ff.h"


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

void fatfs_test(void *arg) {
    FRESULT res;
    if (FR_OK == f_mount(&fs, "0:", 1))//挂载SD卡到path: 0:，并创建文件系统对象的句柄
    {
        printf("mount fs success!\n");
    }
    res = f_opendir(&dp1, "0:");
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
    f_unmount("0:");
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

int main(void) {
    //4位抢占优先级，0位相应优先级
    __NVIC_SetPriorityGrouping(NVIC_PriorityGroup_4);
    /* SysTick end of count event each ms */
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

    mcu_uart_open(PC_PORT);

//    xTaskCreate(master_task_main,  /* 任务入口函数 */
//                "MASTER",    /* 任务名字 */
//                1024,    /* 任务栈大小 */
//                NULL,        /* 任务入口函数参数 */
//                1,  /* 任务的优先级 */
//                NULL);  /* 任务控制块指针 */
    xTaskCreate(fatfs_test,  /* 任务入口函数 */
                "fatfs_test",    /* 任务名字 */
                4096,    /* 任务栈大小 */
                NULL,        /* 任务入口函数参数 */
                1,  /* 任务的优先级 */
                NULL);  /* 任务控制块指针 */
    vTaskStartScheduler();
    while (1) {
        printf_("Hello, world\n");
    }
}



