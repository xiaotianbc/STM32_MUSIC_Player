/*
 * FreeRTOS V202112.00
 * 1 tab == 4 spaces!
 */

/*
本节代码提供了一个基本的基于中断的调试串口驱动
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

/* Misc defines. */
#define serINVALID_QUEUE                ( ( QueueHandle_t ) 0 )
#define serNO_BLOCK                        ( ( TickType_t ) 0 )
#define serTX_BLOCK_TIME                ( 40 / portTICK_PERIOD_MS )

/*-----------------------------------------------------------*/

/* The queue used to hold received characters. */

static QueueHandle_t xRxedChars;  //接收字符队列，每次接收到字符放进去，取字符就从里面取
static QueueHandle_t xCharsForTx; //发送字符队列，需要发送的字符放进去

/*-----------------------------------------------------------*/

/* UART interrupt handler. */
//void vUARTInterruptHandler(void);

/*-----------------------------------------------------------*/

/*
 * See the serial2.h header file.
 */
xComPortHandle xSerialPortInitMinimal(unsigned long ulWantedBaud, unsigned portBASE_TYPE uxQueueLength) {
    xComPortHandle xReturn = 0;

    /* Create the queues used to hold Rx/Tx characters. */
    xRxedChars = xQueueCreate(uxQueueLength, ( unsigned portBASE_TYPE ) sizeof( signed char ));
    xCharsForTx = xQueueCreate(uxQueueLength + 1, ( unsigned portBASE_TYPE ) sizeof( signed char ));

    //如果不等于0，就说明创建成功，执行串口初始化？。。好像也不需要啊
    if ((xRxedChars != serINVALID_QUEUE) && (xCharsForTx != serINVALID_QUEUE)) {
        //   mcu_uart_open(CP2102_PORT);
    } else {
        xReturn = (xComPortHandle) 0;
    }

    /* 当前demo只支持1个串口，但是为了遵守头文件的约定，还是返回一个值. */
    return xReturn;
}

/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialGetChar(xComPortHandle pxPort, signed char *pcRxedChar, TickType_t xBlockTime) {
    /* The port handle is not required as this driver only supports one port. */
    (void) pxPort;

    /* Get the next character from the buffer.  Return false if no characters
    are available, or arrive before xBlockTime expires. */
    // if (xQueueReceive(xRxedChars, pcRxedChar, xBlockTime)) {
    if (lwrb_read(&usart_rx_rb, pcRxedChar, 1)) {
        //如果确实收到了，结果已经保存在入参的pcRxedChar 里，这里只需要返回成功这个结果就行
        return pdTRUE;
    } else {
        return pdFALSE;
    }
}

/*-----------------------------------------------------------*/

//定义一个二维数组，用于DMA手动双缓冲发送，避免数据传输过程中被修改
static uint8_t serial_put_string_nodma_cache[2][64] = {0};

void vSerialPutString(xComPortHandle pxPort, const signed char *const pcString, unsigned short usStringLength) {
    (void) pxPort;
    signed char *pxNext;
    pxNext = (signed char *) pcString;

    //手动设置DMA的双缓冲模式，每次传输完成后切换缓冲区，避免数据竞争
    static uint8_t current_dma_buffer_index = 0;

    while (usStringLength > 64) {
        memcpy(serial_put_string_nodma_cache[current_dma_buffer_index], pxNext, 64);
        mcu_uart_send_buffer_dma(serial_put_string_nodma_cache[current_dma_buffer_index], 64);
        current_dma_buffer_index = 1 - current_dma_buffer_index; //切换当前操作的缓冲区
        usStringLength -= 64;
        pxNext += 64;
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
    uint8_t cChar;
//
////把需要发送的字符串发送到 xCharsForTx 队列里
//    if (xQueueSend(xCharsForTx, &cOutChar, xBlockTime) == pdPASS) {
//        xReturn = pdPASS;
//        USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
//        if (xQueueReceive(xCharsForTx, &cChar, 0) == pdTRUE) {
//            //    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {
//            //    }  //如果使用轮询发送，就把这里注释解开
//            USART3->DR = cChar;
//        }
//    } else {
//        xReturn = pdFAIL;
//    }
    // mcu_uart_send_buffer_dma(&cOutChar, 1);
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



void USART3_IRQHandler2(void) {
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    char cChar;

    if (USART_GetITStatus(USART3, USART_IT_TXE) == SET) {
        /* The interrupt was caused by the THR becoming empty.  Are there any
        more characters to transmit? */
        //检查队列里还有没用元素需要发送
        if (xQueueReceiveFromISR(xCharsForTx, &cChar, &xHigherPriorityTaskWoken) == pdTRUE) {
            /* A character was retrieved from the queue so can be sent to the
            THR now. */
            USART_SendData(USART3, cChar);
        } else {
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
        }
    }

    //如果接收到字符，把收到的字符放到 xRxedChars 队列里
    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
        cChar = USART_ReceiveData(USART3);
        xQueueSendFromISR(xRxedChars, &cChar, &xHigherPriorityTaskWoken);
    }

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}





	
