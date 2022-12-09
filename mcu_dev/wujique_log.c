#include <stdarg.h>
#include <stdio.h>

#include "stm32f4xx.h"
#include "mcu_uart.h"
#include "wujique_log.h"

LOG_L LogLevel = LOG_DEBUG;//系统调试信息等级
/*
使用串口输出调试信息
*/
int8_t string[256];//调试信息缓冲，输出调试信息一次不可以大于256

extern int vsprintf(char *s, const char *format, __va_list arg);

/**
 *@brief:      uart_printf
 *@details:    从串口格式化输出调试信息
 *@param[in]   int8_t *fmt  
               ...      
 *@param[out]  无
 *@retval:     
 */
static void uart_printf(int8_t *fmt, ...) {
    int32_t length = 0;
    va_list ap;

    int8_t *pt;

    va_start(ap, fmt);
    vsprintf((char *) string, (const char *) fmt, ap);
    pt = &string[0];
    while (*pt != '\0') {
        length++;
        pt++;
    }

    mcu_uart_write(CP2102_PORT, (u8 *) &string[0], length);  //写串口

    va_end(ap);
}

void wjq_log(LOG_L l, int8_t *fmt, ...) {
    if (l > LogLevel)
        return;

    int32_t length = 0;
    va_list ap;

    int8_t *pt;

    va_start(ap, fmt);
    vsprintf((char *) string, (const char *) fmt, ap);
    pt = &string[0];
    while (*pt != '\0') {
        length++;
        pt++;
    }

    mcu_uart_write(CP2102_PORT, (u8 *) &string[0], length);  //写串口

    va_end(ap);
}

/**
 *@brief:      PrintFormat
 *@details:    格式化输出BUF中的数据
 *@param[in]   u8 *wbuf  
               int32_t wlen  
 *@param[out]  无
 *@retval:     
 */
void PrintFormat(u8 *wbuf, int32_t wlen) {
    int32_t i;
    for (i = 0; i < wlen; i++) {
        if ((0 == (i & 0x0f)))//&&(0 != i))
        {
            uart_printf("\r\n");
        }
        uart_printf("%02x ", wbuf[i]);
    }
    uart_printf("\r\n");
}


