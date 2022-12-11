//
// Created by xiaotian on 2022/12/10.
//

#include <string.h>
#include "board_fatfs_interface.h"
#include "ff.h"
#include "main.h"

/**
 * 暂时不支持记录当前文件夹位置
 */

static FATFS fs;//文件系统对象
static DIR current_dir;
static FIL fp;//文件对象
static FILINFO fno;
static FRESULT res;

void fatfs_mount_init(void) {
    if (FR_OK == f_mount(&fs, "", 1))//挂载SD卡到path: 0:，并创建文件系统对象的句柄
    {
        printf_("mount fs success!\r\n");
    }
    res = f_opendir(&current_dir, "");
    if (res != FR_OK) {
        printf_("opendir root failed\r\n");
        return;
    }

}


/**
 * fatfs list current dir
 */
void fatfs_ls(void) {
    fatfs_mount_init();  //重新挂载，防止被msc更改后无法获取最新内容
    res = f_opendir(&current_dir, "");                        /* Open a directory */
    if (res != FR_OK) {
        printf_("opendir failed\r\n");
        return;
    }
    while (1) {
        res = f_readdir(&current_dir, &fno);
        if (res != FR_OK) {
            printf_("f_readdir (&current_dir, &fno) failed\r\n");
            return;
        }
        if (strlen(fno.fname) < 1) {
            printf_("end of dir\r\n");
            return;
        }
        if (fno.fattrib & AM_DIR) { //判断是不是文件夹
            printf_("D: %s \t\t\t", fno.fname);
        } else {
            printf_("F: %s \t\t\t", fno.fname);
        }
        printf_("size:%llu\r\n", fno.fsize);
    }
}

void fatfs_Deinit(void) {
    f_unmount("");
}
