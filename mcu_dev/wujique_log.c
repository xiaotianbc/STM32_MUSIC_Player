#include <stdarg.h>
#include <stdio.h>

#include "stm32f4xx.h"
#include "mcu_uart.h"
#include "wujique_log.h"

LOG_L LogLevel = LOG_DEBUG;//ϵͳ������Ϣ�ȼ�
/*
ʹ�ô������������Ϣ
*/
int8_t string[256];//������Ϣ���壬���������Ϣһ�β����Դ���256

extern int vsprintf(char *s, const char *format, __va_list arg);

/**
 *@brief:      uart_printf
 *@details:    �Ӵ��ڸ�ʽ�����������Ϣ
 *@param[in]   int8_t *fmt  
               ...      
 *@param[out]  ��
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

    mcu_uart_write(CP2102_PORT, (u8 *) &string[0], length);  //д����

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

    mcu_uart_write(CP2102_PORT, (u8 *) &string[0], length);  //д����

    va_end(ap);
}

/**
 *@brief:      PrintFormat
 *@details:    ��ʽ�����BUF�е�����
 *@param[in]   u8 *wbuf  
               int32_t wlen  
 *@param[out]  ��
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


