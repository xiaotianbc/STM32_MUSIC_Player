//
// Created by xiaotian on 2022/9/14.
//

#include "spiffs_port.h"
#include "spi_flash.h"
#include "spiffs.h"
#include "wujique_log.h"


#define LOG_PAGE_SIZE       256

static u8_t spiffs_work_buf[LOG_PAGE_SIZE * 2];
static u8_t spiffs_fds[32 * 4];
static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE + 32) * 4];

static s32_t my_spiffs_read(u32_t addr, u32_t size, u8_t *dst) {
    sFLASH_ReadBuffer(dst, addr, size);
    return SPIFFS_OK;
}

static s32_t my_spiffs_write(u32_t addr, u32_t size, u8_t *src) {
    sFLASH_WriteBuffer(src, addr, size);
    return SPIFFS_OK;
}

static s32_t my_spiffs_erase(u32_t addr, u32_t size) {
    wjq_log(LOG_INFO,"my_spiffs_erase: size:%d\n", size);
    if (size<4096)size=4096;
    while (size){
        sFLASH_EraseSector(addr);
        size-=4096;
        addr+=4096;
    }
    return SPIFFS_OK;
    uint32_t end_addr = addr + size; //结束的地址
    uint32_t end_addr_gap = end_addr % 0x1000; //结束的地址距离其所在扇区的距离
    uint32_t end_addr_aligned = end_addr - end_addr_gap; //结束的地址距离其所在扇区的距离

    uint32_t start_addr_gap = addr % 0x1000; //起始扇区距离
    uint32_t start_addr_aligned = addr - start_addr_gap; //起始扇区地址
    //从起始扇区的开始位置擦除到结束扇区的开始位置
    for (uint32_t i = start_addr_aligned; i <= end_addr_aligned; i = i + 1000) {
        wjq_log(LOG_INFO, "Erase Sectoring: 0x%X\n", i);
        sFLASH_EraseSector(i);
    }
    return SPIFFS_OK;
}

void my_spiffs_mount(spiffs *fs) {
    spiffs_config cfg;

    cfg.hal_read_f = my_spiffs_read;
    cfg.hal_write_f = my_spiffs_write;
    cfg.hal_erase_f = my_spiffs_erase;

    int res = SPIFFS_mount(&fs,
                           &cfg,
                           spiffs_work_buf,
                           spiffs_fds,
                           sizeof(spiffs_fds),
                           spiffs_cache_buf,
                           sizeof(spiffs_cache_buf),
                           0);
    wjq_log(LOG_INFO, "mount res: %i\n", res);
}

//spiffs 测试代码
#if 0

static void test_spiffs() {
    char buf[13];
    uint8_t write_buf[1280];
    for (int i = 0; i < 128; ++i) {
        write_buf[i]=i%0xff;
    }

    // Surely, I've mounted spiffs before entering here

    spiffs_file fd = SPIFFS_open(&fs, "my_file", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
    if (SPIFFS_write(&fs, fd, (u8_t *) "Hello world2", 13) < 0)
        wjq_log(LOG_INFO, "errno %i\n", SPIFFS_errno(&fs));
    SPIFFS_close(&fs, fd);

    fd = SPIFFS_open(&fs, "my_file", SPIFFS_RDWR, 0);
    if (SPIFFS_read(&fs, fd, (u8_t *) buf, 13) < 0)
        wjq_log(LOG_INFO, "errno %i\n", SPIFFS_errno(&fs));
    SPIFFS_close(&fs, fd);
    wjq_log(LOG_INFO, "--> %s <--\n", buf);
}
#endif