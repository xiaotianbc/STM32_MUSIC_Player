/*
 * FreeRTOS V202112.00
 * 1 tab == 4 spaces!
 */

/*
本节代码提供了一个基本的基于中断的调试串口驱动 xx
本节代码提供了一个修改后的的基于DMA双缓冲发送+环形缓冲区接收的调试串口驱动 xx
*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/* Library includes. */
#include "main.h"

/* Demo application includes. */
#include "serial.h"
#include "mcu_uart.h"
#include "lwrb.h"
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/* UART interrupt handler. */
//void vUARTInterruptHandler(void);

/*-----------------------------------------------------------*/

/*
 * See the serial2.h header file.
 */
xComPortHandle xSerialPortInitMinimal(unsigned long ulWantedBaud, unsigned portBASE_TYPE uxQueueLength) {
    xComPortHandle xReturn = 0;
    /* 当前demo只支持1个串口，但是为了遵守头文件的约定，还是返回一个值. */
    return xReturn;
}

/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialGetChar(xComPortHandle pxPort, signed char *pcRxedChar, TickType_t xBlockTime) {
    /* The port handle is not required as this driver only supports one port. */
    (void) pxPort;

    /* Get the next character from the buffer.  Return false if no characters
    are available, or arrive before xBlockTime expires. */

    if (lwrb_read(&usart_rx_rb, pcRxedChar, 1)) {
        //如果确实收到了，结果已经保存在入参的pcRxedChar 里，这里只需要返回成功这个结果就行
        return pdTRUE;
    } else {
        return pdFALSE;
    }
}

/*-----------------------------------------------------------*/

#define vSerialPutString_BUFFERSIZE 64
//定义一个二维数组，用于DMA手动双缓冲发送，避免数据传输过程中被修改
static uint8_t serial_put_string_nodma_cache[2][vSerialPutString_BUFFERSIZE] = {0};

void vSerialPutString(xComPortHandle pxPort, const signed char *const pcString, unsigned short usStringLength) {
    (void) pxPort;
    signed char *pxNext;
    pxNext = (signed char *) pcString;

    //手动设置DMA的双缓冲模式，每次传输完成后切换缓冲区，避免数据竞争
    static uint8_t current_dma_buffer_index = 0;

    //因为STM32的CCMRAM不能被DMA访问，所以这里需要手动拷贝到其他内存区域才行
    while (usStringLength > vSerialPutString_BUFFERSIZE) {
        memcpy(serial_put_string_nodma_cache[current_dma_buffer_index], pxNext, vSerialPutString_BUFFERSIZE);
        mcu_uart_send_buffer_dma(serial_put_string_nodma_cache[current_dma_buffer_index], vSerialPutString_BUFFERSIZE);
        current_dma_buffer_index = 1 - current_dma_buffer_index; //切换当前操作的缓冲区
        usStringLength -= vSerialPutString_BUFFERSIZE;
        pxNext += vSerialPutString_BUFFERSIZE;
    }
    if (usStringLength > 0) {
        memcpy(serial_put_string_nodma_cache[current_dma_buffer_index], pxNext, usStringLength);
        mcu_uart_send_buffer_dma(serial_put_string_nodma_cache[current_dma_buffer_index], usStringLength);
        current_dma_buffer_index = 1 - current_dma_buffer_index; //切换当前操作的缓冲区
    }
}

/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialPutChar(xComPortHandle pxPort, signed char cOutChar, TickType_t xBlockTime) {
    signed portBASE_TYPE xReturn;

    //STM32的CCMRAM不能用DMA，转到普通内存区域再传输
    serial_put_string_nodma_cache[0][0] = cOutChar;
    mcu_uart_send_buffer_dma(serial_put_string_nodma_cache[0], 1);
    return xReturn;
}

/*-----------------------------------------------------------*/

void vSerialClose(xComPortHandle xPort) {
    /* Not supported as not required by the demo application. */
}

/*-----------------------------------------------------------*/



	
