/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"            /* Obtains integer types */
#include "diskio.h"        /* Declarations of disk functions */
#include "stm324xg_eval_sdio_sd.h"

/* Definitions of physical drive number for each drive */
#define DEV_SDIO_TF        0    /* SDIO接口的SD卡 */


static SD_Error Status = SD_OK;
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
        BYTE pdrv        /* Physical drive nmuber to identify the drive */
) {
    SDCardState state;

    switch (pdrv) {
        case DEV_SDIO_TF :
            state = SD_GetState();
            if (state != SD_CARD_ERROR && state != SD_CARD_DISCONNECTED)
                return RES_OK;
            else
                return RES_ERROR;
        default:
            return RES_ERROR;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
        BYTE pdrv                /* Physical drive nmuber to identify the drive */
) {
    DSTATUS stat;
    SD_Error result;

    switch (pdrv) {
        case DEV_SDIO_TF :
            result = SD_Init();
            if (result != SD_OK)
                return 1;
            else
                return 0;
        default:
            return 0;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
        BYTE pdrv,        /* Physical drive nmuber to identify the drive */
        BYTE *buff,        /* Data buffer to store read data */
        LBA_t sector,    /* Start sector in LBA */
        UINT count        /* Number of sectors to read */
) {
    DRESULT res;
    int result;

    switch (pdrv) {
        case DEV_SDIO_TF :
            // translate the arguments here
            if (count > 1)//读取多个扇区
            {
                SD_ReadMultiBlocks(buff, sector * 512, 512, count);
            } else {
                SD_ReadBlock(buff, sector * 512, 512);
            }


            /* Check if the Transfer is finished */
            Status = SD_WaitReadOperation();

            /* Wait until end of DMA transfer */
            while (SD_GetStatus() != SD_TRANSFER_OK);

            return (Status == SD_OK) ? RES_OK : RES_ERROR;
    }

    return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
        BYTE pdrv,            /* Physical drive nmuber to identify the drive */
        const BYTE *buff,    /* Data to be written */
        LBA_t sector,        /* Start sector in LBA */
        UINT count            /* Number of sectors to write */
) {
    DRESULT res;


    switch (pdrv) {
        case DEV_SDIO_TF :
            if (count > 1)//单个或者多个扇区使用不同的函数
            {
                SD_WriteMultiBlocks(buff, sector * 512, 512, count);
            } else {
                SD_WriteBlock(buff, sector * 512, 512);
            }
            /* Check if the Transfer is finished */
            Status = SD_WaitWriteOperation();

            /* Wait until end of DMA transfer */
            while (SD_GetStatus() != SD_TRANSFER_OK);
            res = (Status == SD_OK) ? RES_OK : RES_ERROR;

            return res;
    }

    return RES_PARERR;
}

#endif


extern SD_CardInfo SDCardInfo;

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
        BYTE pdrv,        /* Physical drive nmuber (0..) */
        BYTE cmd,        /* Control code */
        void *buff        /* Buffer to send/receive control data */
) {

    /*
     *  Generic command (Used by FatFs)
//#define CTRL_SYNC			0	/* Complete pending write process (needed at FF_FS_READONLY == 0) */
//#define GET_SECTOR_COUNT	1	/* Get media size (needed at FF_USE_MKFS == 1) */
//#define GET_SECTOR_SIZE		2	/* Get sector size (needed at FF_MAX_SS != FF_MIN_SS) */
//#define GET_BLOCK_SIZE		3	/* Get erase block size (needed at FF_USE_MKFS == 1) */
//#define CTRL_TRIM			4	/* Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) */

    switch (pdrv) {
        case DEV_SDIO_TF :
            switch (cmd) {
                case CTRL_SYNC:
                    return RES_OK;
                    break;
                case GET_SECTOR_COUNT:
                    //把结果保存成uint32_t 保存到buff
                    *(uint32_t *) buff = (uint32_t) ((uint64_t) SDCardInfo.CardCapacity / 512);
                    return RES_OK;
                    break;
                case GET_SECTOR_SIZE:
                    *(uint16_t *) buff = 512;
                    return RES_OK;
                    break;
                case GET_BLOCK_SIZE:
                    *(uint16_t *) buff = 512;
                    return RES_OK;
                    break;
                case CTRL_TRIM:
                    return RES_OK;
                    break;
                default:
                    return RES_PARERR;
            }
        default:
            return RES_PARERR;
    }
}

