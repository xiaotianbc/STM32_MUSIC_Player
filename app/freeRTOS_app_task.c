//
// Created by xiaotian on 2022/12/11.
//
#include "main.h"
#include "freeRTOS_app_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "board_fatfs_interface.h"
#include "ff.h"
#include "mcu_uart.h"

static FIL fp;

static uint8_t music_buffer[2][2048];

void task_play_music(char *arg) {


    FRESULT res;
    UINT br;
    fatfs_mount_init();
    res = f_open(&fp, arg, FA_READ);
    if (res != FR_OK) {
        printf_("open %s file failed\r\n");
        goto task_del;
    }

    res = f_read(&fp, music_buffer[0], 2048, &br);
    if (res == FR_OK && br == 2048) { //开始查找子数组
        mcu_uart_send_buffer_dma(music_buffer[0], br);
    } else {
        if (res != FR_OK) //如果是打开有问题，直接退出
            goto task_del;
        //到这里说明打开没有问题，只是文件长度不够，但是对于播放音乐来说，不合理

    }

    task_del:
    fatfs_Deinit();
    vTaskDelete(NULL);
}