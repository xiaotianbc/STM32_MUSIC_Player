//
// Created by xiaotian on 2022/9/14.
//

#include "lfs_port.h"
#include "sfud.h"

//Littlefs 测试代码
#if 0

void TestLFS(const struct lfs_config *cfg, lfs_t *lfs, lfs_file_t *file) {
    // mount the filesystem
    int err = lfs_mount(lfs, cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(lfs, cfg);
        lfs_mount(lfs, cfg);
    }

    // read current count
    uint32_t boot_countr = 0;
    lfs_file_open(lfs, file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(lfs, file, &boot_countr, sizeof(boot_countr));

    // update boot count
    boot_countr += 1;
    //将文件的位置更改为文件的开头
    lfs_file_rewind(lfs, file);
    lfs_file_write(lfs, file, &boot_countr, sizeof(boot_countr));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(lfs, file);

    // release any resources we were using
    lfs_unmount(lfs);

    // print the boot count
    wjq_log(LOG_INFO, "boot_countr: %d\n", boot_countr);
}

lfs_t lfs;             // lfs 文件系统对象
lfs_file_t file;       // lfs 文件对象
struct lfs_config cfg; // lfs 文件系统配置结构体
void run_test_lfs(void) {

    uint8_t bCanTest = 0;


    /* SFUD initialize */
    int err = lfs_spi_flash_init(&cfg);
    if (err) {
        wjq_log(LOG_INFO, "LFS init failed!\n");
    } else {
        bCanTest = 1;
    }
    while (1) {
        if (bCanTest) {
            TestLFS(&cfg, &lfs, &file);
        }
        Delay(2000);
    }
}


#endif



#define W25Q32_SECTOR_SIZE 4096
#define W25Q32_SECTOR_NUM 2048 // 总大小: 4096 * 2048 = 8M byte

int lfs_spi_flash_init(struct lfs_config *cfg) {
  //  sFLASH_Init();
    if (sfud_init() == SFUD_SUCCESS) {
        cfg->read = lfs_spi_flash_read;
        cfg->prog = lfs_spi_flash_prog;
        cfg->erase = lfs_spi_flash_erase;
        cfg->sync = lfs_spi_flash_sync;

        // 最小读取字节数，所有的读取操作字节数必须是它的整数倍
        cfg->read_size = 16;
        // 最小写入字节数，所有的写入操作字节数必须是它的整数倍
        cfg->prog_size = 16;
        // 擦除块操作的字节数，该选项不影响 RAM 消耗，可以比物理擦除尺寸大
        // 但是每个文件至少占用一个块，必须是读取和写入操作字节数的整数倍
        cfg->block_size = W25Q32_SECTOR_SIZE;
        // 设备上可擦除块的数量，即容量
        cfg->block_count = W25Q32_SECTOR_NUM;
        // littlefs 系统删除元数据日志并将元数据移动到另一个块之前的擦除周期数。
        // 建议取值范围为 100 ~ 1000，较大数值有较好的性能但是会导致磨损分布不一致
        // 取值 -1 的话，即为禁用块级磨损均衡
        cfg->block_cycles = 500;
        // 块缓存大小，每个缓存都会在 RAM 中缓冲一部分块数据，
        // littlefs 系统需要一个读取缓存、一个写入缓存，每个文件还需要一个额外的缓存。
        // 更大的缓存可以通过存储更多的数据并降低磁盘访问数量等手段来提高性能
        cfg->cache_size = 16;
        // 先行缓冲大小，更大的先行缓冲可以提高分配操作中可被发现的块数量
        // 即分配块时每次步进多少个块，16就表示每次分配16个块
        // 先行缓冲以紧凑的bit位形式来存储，故 RAM 中的一个字节可以对应8个块
        // 该值必须是8的整数倍
        cfg->lookahead_size = 16;
        return LFS_ERR_OK;
    } else {
        return LFS_ERR_IO;
    }
}

/*
 * @brief 从指定块内的某区域读数据
 * @param [in] lfs_config格式参数
 * @param [in] block 逻辑块索引号，从0开始
 * @param [in] off 块内偏移，该值需能被read_size整除
 * @param [out] 读出数据的输出缓冲区
 * @param [in] size 要读取的字节数，该值需能被read_size整除，lfs在读取时会确保不会跨块；
 * @retval 0 成功, < 0 错误码
 */
int lfs_spi_flash_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
    // check if read is valid
    LFS_ASSERT(off % cfg->read_size == 0);
    LFS_ASSERT(size % cfg->read_size == 0);
    LFS_ASSERT(block < cfg->block_count);

    const sfud_flash *flash = sfud_get_device_table() + 0;
    sfud_read(flash, block * cfg->block_size + off, size, (uint8_t *)buffer);
   // sFLASH_ReadBuffer((uint8_t *)buffer,block * cfg->block_size + off,size);
    return LFS_ERR_OK;
}


/*
 * @brief 将数据写入指定块内的某区域。该区域必须已经先被擦除过，可以返回 LFS_ERR_CORRUPT 表示该块已损坏
 * @param [in] lfs_config格式参数
 * @param [in] block 逻辑块索引号，从0开始
 * @param [in] off 块内偏移，该值需能被rprog_size整除
 * @param [in] 写入数据的缓冲区
 * @param [in] size 要写入的字节数，该值需能被read_size整除，lfs在读取时会确保不会跨块；
 * @retval 0 成功, < 0 错误码
 */
int lfs_spi_flash_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    // check if write is valid
    LFS_ASSERT(off % cfg->prog_size == 0);
    LFS_ASSERT(size % cfg->prog_size == 0);
    LFS_ASSERT(block < cfg->block_count);
    const sfud_flash *flash = sfud_get_device_table() + 0;
    sfud_write(flash, block * cfg->block_size + off, size, (uint8_t *)buffer);
 //   sFLASH_WriteBuffer((uint8_t *)buffer, block * cfg->block_size + off, size);
    return LFS_ERR_OK;
}


/*
 * @brief 擦除指定块。块在写入之前必须先被擦除过，被擦除块的状态是未定义
 * @param [in] lfs_config格式参数
 * @param [in] block 要擦除的逻辑块索引号，从0开始
 * @retval 0 成功, < 0 错误码
 */
int lfs_spi_flash_erase(const struct lfs_config *cfg, lfs_block_t block) {
    // check if erase is valid
    LFS_ASSERT(block < cfg->block_count);

    const sfud_flash *flash = sfud_get_device_table() + 0;
    sfud_erase(flash, block * cfg->block_size, cfg->block_size);
   // sFLASH_EraseSector(block * cfg->block_size);
    return LFS_ERR_OK;
}

/*
 * @brief 对底层块设备做同步操作。若底层块设备不没有同步这项操作可以直接返回
 * @param [in] lfs_config格式参数;
 * @retval 0 成功, < 0 错误码
 */
int lfs_spi_flash_sync(const struct lfs_config *cfg) {
    return LFS_ERR_OK;
}


