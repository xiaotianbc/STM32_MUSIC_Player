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
    if (xQueueReceive(xRxedChars, pcRxedChar, xBlockTime)) {
        //如果确实收到了，结果已经保存在入参的pcRxedChar 里，这里只需要返回成功这个结果就行
        return pdTRUE;
    } else {
        return pdFALSE;
    }
}

/*-----------------------------------------------------------*/

void vSerialPutString(xComPortHandle pxPort, const signed char *const pcString, unsigned short usStringLength) {
    signed char *pxNext;

    /* A couple of parameters that this port does not use. */
    (void) usStringLength;
    (void) pxPort;

    /* NOTE: This implementation does not handle the queue being full as no
    block time is used! */

    /* The port handle is not required as this driver only supports UART1. */
    (void) pxPort;

    /* Send each character in the string, one at a time. */
    pxNext = (signed char *) pcString;
    while (*pxNext) {
        xSerialPutChar(pxPort, *pxNext, serNO_BLOCK);
        pxNext++;
    }
}

/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialPutChar(xComPortHandle pxPort, signed char cOutChar, TickType_t xBlockTime) {
    signed portBASE_TYPE xReturn;
    uint8_t cChar;

//把需要发送的字符串发送到 xCharsForTx 队列里
    if (xQueueSend(xCharsForTx, &cOutChar, xBlockTime) == pdPASS) {
        xReturn = pdPASS;
        USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
        if (xQueueReceive(xCharsForTx, &cChar, 0) == pdTRUE) {
            //    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {
            //    }  //如果使用轮询发送，就把这里注释解开
            USART3->DR = cChar;
        }
    } else {
        xReturn = pdFAIL;
    }

    return xReturn;
}

/*-----------------------------------------------------------*/

void vSerialClose(xComPortHandle xPort) {
    /* Not supported as not required by the demo application. */
}

/*-----------------------------------------------------------*/



void USART3_IRQHandler(void) {
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





	
