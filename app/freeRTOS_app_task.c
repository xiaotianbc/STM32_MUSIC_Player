//
// Created by xiaotian on 2022/12/11.
//
#include "main.h"
#include "freeRTOS_app_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "board_fatfs_interface.h"

void task_ls(void *arg) {
    while (1) {
        fatfs_ls();
        vTaskSuspend(NULL); //把自己挂起
    }
    vTaskDelete(NULL);
}

void task_play_music(void *arg) {

    for (int i = 0; i < 10; ++i) {
        fatfs_mount_init();
        printf_("Hello, task\r\n");
        vTaskDelay(100);
    }

    vTaskDelete(NULL);
}