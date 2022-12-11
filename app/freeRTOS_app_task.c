//
// Created by xiaotian on 2022/12/11.
//
#include "main.h"
#include "freeRTOS_app_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "board_fatfs_interface.h"

void task_ls(void *arg) {
    fatfs_ls();
    vTaskDelete(NULL);
}

void task_play_music(void *arg) {
    fatfs_mount_init();

    for (int i = 0; i < 10; ++i) {
        printf_("Hello, task\r\n");
        vTaskDelay(1000);
    }

    vTaskDelete(NULL);
}