/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2016-04-23
 */

#include <sfud.h>
#include <stdarg.h>
#include <stdio.h>
#include "main.h"
#include "wujique_log.h"



#if 0

#define SFUD_DEMO_TEST_BUFFER_SIZE                     1024
static uint8_t sfud_demo_test_buf[SFUD_DEMO_TEST_BUFFER_SIZE];

/**
 * SFUD demo for the first flash device test.
 *
 * @param addr flash start address
 * @param size test flash size
 * @param size test flash data buffer
 */
static void sfud_demo(uint32_t addr, size_t size, uint8_t *data) {
    sfud_err result = SFUD_SUCCESS;
    const sfud_flash *flash = sfud_get_device_table() + 0;
    size_t i;
    /* prepare write data */
    for (i = 0; i < size; i++) {
        data[i] = i;
    }
    /* erase test */
    result = sfud_erase(flash, addr, size);
    if (result == SFUD_SUCCESS) {

        wjq_log(LOG_INFO, "Erase the %s flash data finish. Start from 0x%08X, size is %ld.\r\n", flash->name, addr,
                size);
    } else {
        wjq_log(LOG_INFO, "Erase the %s flash data failed.\r\n", flash->name);
        return;
    }
    /* write test */
    result = sfud_write(flash, addr, size, data);
    if (result == SFUD_SUCCESS) {
        wjq_log(LOG_INFO, "Write the %s flash data finish. Start from 0x%08X, size is %ld.\r\n", flash->name, addr,
                size);
    } else {
        wjq_log(LOG_INFO, "Write the %s flash data failed.\r\n", flash->name);
        return;
    }
    /* read test */
    result = sfud_read(flash, addr, size, data);
    if (result == SFUD_SUCCESS) {
        wjq_log(LOG_INFO, "Read the %s flash data success. Start from 0x%08X, size is %ld. The data is:\r\n",
                flash->name, addr,
                size);
        wjq_log(LOG_INFO, "Offset (h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n");
        for (i = 0; i < size; i++) {
            if (i % 16 == 0) {
                wjq_log(LOG_INFO, "[%08X] ", addr + i);
            }
            wjq_log(LOG_INFO, "%02X ", data[i]);
            if (((i + 1) % 16 == 0) || i == size - 1) {
                wjq_log(LOG_INFO, "\r\n");
            }
        }
        wjq_log(LOG_INFO, "\r\n");
    } else {
        wjq_log(LOG_INFO, "Read the %s flash data failed.\r\n", flash->name);
    }
    /* data check */
    for (i = 0; i < size; i++) {
        if (data[i] != i % 256) {
            wjq_log(LOG_INFO, "Read and check write data has an error. Write the %s flash data failed.\r\n",
                    flash->name);
            break;
        }
    }
    if (i == size) {
        wjq_log(LOG_INFO, "The %s flash test is success.\r\n", flash->name);
    }
}

#endif



typedef struct {
    SPI_TypeDef *spix;
    GPIO_TypeDef *cs_gpiox;
    uint16_t cs_gpio_pin;
} spi_user_data, *spi_user_data_t;

static char log_buf[256];

void sfud_log_debug(const char *file, const long line, const char *format, ...);

static void rcc_configuration(spi_user_data_t spi) {
    if (spi->spix == SPI1) {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
    } else if (spi->spix == SPI2) {
        /* you can add SPI2 code here */
    }
}

static void gpio_configuration(spi_user_data_t spi) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    if (spi->spix == SPI1) {

        /*!< Connect SPI pins to AF5 */
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);


        /* SCK:  MISO:  MOSI: */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 ;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 ;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        /* CS: PB14 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_SetBits(GPIOB, GPIO_Pin_14);
    } else if (spi->spix == SPI2) {
        /* you can add SPI2 code here */
    }
}

static void spi_configuration(spi_user_data_t spi) {
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //SPI ??????????????????????????????
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                      //???????????? SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  //SPI ???????????? 8 ????????????
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                         //???????????????
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                       //?????????????????????????????????
    //TODO ???????????????????????? CS
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                          //??????  NSS ????????? SSI ?????????
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; //???????????????????????? 2
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                 //??????????????? MSB ?????????
    SPI_InitStructure.SPI_CRCPolynomial = 7;                           // CRC ?????????????????????

    SPI_I2S_DeInit(spi->spix);
    SPI_Init(spi->spix, &SPI_InitStructure);

    SPI_CalculateCRC(spi->spix, DISABLE);
    SPI_Cmd(spi->spix, ENABLE);
}

static void spi_lock(const sfud_spi *spi) {
    __disable_irq();
}

static void spi_unlock(const sfud_spi *spi) {
    __enable_irq();
}

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
        size_t read_size) {
    sfud_err result = SFUD_SUCCESS;
    uint8_t send_data, read_data;
    spi_user_data_t spi_dev = (spi_user_data_t) spi->user_data;

    if (write_size) {
        SFUD_ASSERT(write_buf);
    }
    if (read_size) {
        SFUD_ASSERT(read_buf);
    }

    GPIO_ResetBits(spi_dev->cs_gpiox, spi_dev->cs_gpio_pin);
    /* ?????????????????? */
    for (size_t i = 0, retry_times; i < write_size + read_size; i++) {
        /* ?????????????????????????????? SPI ????????????????????????????????? dummy(0xFF) ??? SPI ?????? */
        if (i < write_size) {
            send_data = *write_buf++;
        } else {
            send_data = SFUD_DUMMY_DATA;
        }
        /* ???????????? */
        retry_times = 1000;
        while (SPI_I2S_GetFlagStatus(spi_dev->spix, SPI_I2S_FLAG_TXE) == RESET) {
            SFUD_RETRY_PROCESS(NULL, retry_times, result);
        }
        if (result != SFUD_SUCCESS) {
            goto exit;
        }
        SPI_I2S_SendData(spi_dev->spix, send_data);
        /* ???????????? */
        retry_times = 1000;
        while (SPI_I2S_GetFlagStatus(spi_dev->spix, SPI_I2S_FLAG_RXNE) == RESET) {
            SFUD_RETRY_PROCESS(NULL, retry_times, result);
        }
        if (result != SFUD_SUCCESS) {
            goto exit;
        }
        read_data = SPI_I2S_ReceiveData(spi_dev->spix);
        /* ????????????????????????????????????????????? SPI ????????????????????????????????? */
        if (i >= write_size) {
            *read_buf++ = read_data;
        }
    }

exit:
    GPIO_SetBits(spi_dev->cs_gpiox, spi_dev->cs_gpio_pin);

    return result;
}

/* about 100 microsecond delay */
static void retry_delay_100us(void) {
    uint32_t delay = 120;
    while(delay--);
}

static spi_user_data spi1 = { .spix = SPI1, .cs_gpiox = GPIOB, .cs_gpio_pin = GPIO_Pin_14 };
sfud_err sfud_spi_port_init(sfud_flash *flash) {
    sfud_err result = SFUD_SUCCESS;

    switch (flash->index) {
    case SFUD_SST25_DEVICE_INDEX: {
        /* RCC ????????? */
        rcc_configuration(&spi1);
        /* GPIO ????????? */
        gpio_configuration(&spi1);
        /* SPI ??????????????? */
        spi_configuration(&spi1);
        /* ?????? Flash ?????????????????????????????? */
        flash->spi.wr = spi_write_read;
        flash->spi.lock = spi_lock;
        flash->spi.unlock = spi_unlock;
        flash->spi.user_data = &spi1;
        /* about 100 microsecond delay */
        flash->retry.delay = retry_delay_100us;
        /* adout 60 seconds timeout */
        flash->retry.times = 60 * 10000;

        break;
    }
    }

    return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);

    wjq_log(LOG_DEBUG,"[SFUD](%s:%ld) ", file, line);
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    wjq_log(LOG_DEBUG,"%s\r\n", log_buf);
    va_end(args);
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    wjq_log(LOG_DEBUG,"[SFUD]");
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    wjq_log(LOG_DEBUG,"%s\r\n", log_buf);
    va_end(args);
}


