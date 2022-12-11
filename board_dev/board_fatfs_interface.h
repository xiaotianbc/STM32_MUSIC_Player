//
// Created by xiaotian on 2022/12/10.
//

#ifndef DAC_PLAY_MUSIC_BOARD_FATFS_INTERFACE_H
#define DAC_PLAY_MUSIC_BOARD_FATFS_INTERFACE_H

void fatfs_Deinit(void);
void fatfs_mount_init(void);
void fatfs_ls(char * output_buffer);
#endif //DAC_PLAY_MUSIC_BOARD_FATFS_INTERFACE_H
