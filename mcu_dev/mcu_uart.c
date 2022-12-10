
#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"
#include "mcu_uart.h"
#include "wujique_log.h"


//����3  TX�� DMA1_Stream3 RX DMA1_Stream1

/* Definition for USARTx resources ******************************************/
#define USARTx                           USART3
#define USARTx_CLK                       RCC_APB1Periph_USART3
#define USARTx_CLK_INIT                  RCC_APB1PeriphClockCmd
#define USARTx_IRQn                      USART3_IRQn
#define USARTx_IRQHandler                USART3_IRQHandler

#define USARTx_TX_PIN                    GPIO_Pin_10
#define USARTx_TX_GPIO_PORT              GPIOB
#define USARTx_TX_GPIO_CLK               RCC_AHB1Periph_GPIOB
#define USARTx_TX_SOURCE                 GPIO_PinSource10
#define USARTx_TX_AF                     GPIO_AF_USART3

#define USARTx_RX_PIN                    GPIO_Pin_11
#define USARTx_RX_GPIO_PORT              GPIOB
#define USARTx_RX_GPIO_CLK               RCC_AHB1Periph_GPIOB
#define USARTx_RX_SOURCE                 GPIO_PinSource11
#define USARTx_RX_AF                     GPIO_AF_USART3

/* Definition for DMAx resources ********************************************/
#define USARTx_DR_ADDRESS                ((uint32_t)USART3 + 0x04)

#define USARTx_DMA                       DMA1
#define USARTx_DMAx_CLK                  RCC_AHB1Periph_DMA1

#define USARTx_TX_DMA_CHANNEL            DMA_Channel_4
#define USARTx_TX_DMA_STREAM             DMA1_Stream3
#define USARTx_TX_DMA_FLAG_FEIF          DMA_FLAG_FEIF3
#define USARTx_TX_DMA_FLAG_DMEIF         DMA_FLAG_DMEIF3
#define USARTx_TX_DMA_FLAG_TEIF          DMA_FLAG_TEIF3
#define USARTx_TX_DMA_FLAG_HTIF          DMA_FLAG_HTIF3
#define USARTx_TX_DMA_FLAG_TCIF          DMA_FLAG_TCIF3

#define USARTx_RX_DMA_CHANNEL            DMA_Channel_4
#define USARTx_RX_DMA_STREAM             DMA1_Stream1
#define USARTx_RX_DMA_FLAG_FEIF          DMA_FLAG_FEIF1
#define USARTx_RX_DMA_FLAG_DMEIF         DMA_FLAG_DMEIF1
#define USARTx_RX_DMA_FLAG_TEIF          DMA_FLAG_TEIF1
#define USARTx_RX_DMA_FLAG_HTIF          DMA_FLAG_HTIF1
#define USARTx_RX_DMA_FLAG_TCIF          DMA_FLAG_TCIF1

#define USARTx_DMA_TX_IRQn               DMA1_Stream3_IRQn
#define USARTx_DMA_RX_IRQn               DMA1_Stream1_IRQn
#define USARTx_DMA_TX_IRQHandler         DMA1_Stream3_IRQHandler
#define USARTx_DMA_RX_IRQHandler         DMA1_Stream1_IRQHandler


#define RX3_TEMP_BUF_LEN_MAX       4096/*���ڽ��ջ��峤��*/

volatile uint8_t UartBuf3[RX3_TEMP_BUF_LEN_MAX];//���ڽ��ջ���
volatile u16 UartHead3;//���ڽ��ջ���дָ��
volatile u16 UartEnd3;//���ڽ��ջ����ָ��
volatile uint8_t UartBuf3OverFg = 0;//���������־


/* Transmit buffer size */
#define BUFFERSIZE                       100
static DMA_InitTypeDef DMA_InitStructure;

uint8_t aTxBuffer[BUFFERSIZE] = "USART DMA Example: Communication between two USART using DMA\n";
uint8_t aRxBuffer[BUFFERSIZE];
__IO uint32_t TimeOut = 0x0;

/* Uncomment the line below if you will use the USART in Transmitter Mode */
#define USART_TRANSMITTER
/* Uncomment the line below if you will use the USART in Receiver Mode */
//#define USART_RECEIVER

/**
  * @brief  Configures the USART Peripheral.
  * @param  None
  * @retval None
  */
void ST_USART_Config(void) {
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Peripheral Clock Enable -------------------------------------------------*/
    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(USARTx_TX_GPIO_CLK | USARTx_RX_GPIO_CLK, ENABLE);

    /* Enable USART clock */
    USARTx_CLK_INIT(USARTx_CLK, ENABLE);

    /* Enable the DMA clock */
    RCC_AHB1PeriphClockCmd(USARTx_DMAx_CLK, ENABLE);

    /* USARTx GPIO configuration -----------------------------------------------*/
    /* Connect USART pins to AF7 */
    GPIO_PinAFConfig(USARTx_TX_GPIO_PORT, USARTx_TX_SOURCE, USARTx_TX_AF);
    GPIO_PinAFConfig(USARTx_RX_GPIO_PORT, USARTx_RX_SOURCE, USARTx_RX_AF);

    /* Configure USART Tx and Rx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = USARTx_TX_PIN;
    GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = USARTx_RX_PIN;
    GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStructure);

    /* USARTx configuration ----------------------------------------------------*/
    /* Enable the USART OverSampling by 8 */
    USART_OverSampling8Cmd(USARTx, ENABLE);

    /* USARTx configured as follows:
          - BaudRate = 5250000 baud
             - Maximum BaudRate that can be achieved when using the Oversampling by 8
               is: (USART APB Clock / 8)
               Example:
                  - (USART3 APB1 Clock / 8) = (42 MHz / 8) = 5250000 baud
                  - (USART1 APB2 Clock / 8) = (84 MHz / 8) = 10500000 baud
             - Maximum BaudRate that can be achieved when using the Oversampling by 16
               is: (USART APB Clock / 16)
               Example: (USART3 APB1 Clock / 16) = (42 MHz / 16) = 2625000 baud
               Example: (USART1 APB2 Clock / 16) = (84 MHz / 16) = 5250000 baud
          - Word Length = 8 Bits
          - one Stop Bit
          - No parity
          - Hardware flow control disabled (RTS and CTS signals)
          - Receive and transmit enabled
    */
    USART_InitStructure.USART_BaudRate = 2000000;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    /* When using Parity the word length must be configured to 9 bits */
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USARTx, &USART_InitStructure);

    /* Configure DMA controller to manage USART TX and RX DMA request ----------*/

    /* Configure DMA Initialization Structure */
    DMA_InitStructure.DMA_BufferSize = BUFFERSIZE;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) (&(USARTx->DR));
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    /* Configure TX DMA */
    DMA_InitStructure.DMA_Channel = USARTx_TX_DMA_CHANNEL;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) aTxBuffer;
    DMA_Init(USARTx_TX_DMA_STREAM, &DMA_InitStructure);
    /* Configure RX DMA */
    DMA_InitStructure.DMA_Channel = USARTx_RX_DMA_CHANNEL;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) aRxBuffer;
    DMA_Init(USARTx_RX_DMA_STREAM, &DMA_InitStructure);

    /* Enable USART */
    USART_Cmd(USARTx, ENABLE);

    //�򿪴�������ж�
    DMA_ITConfig(USARTx_TX_DMA_STREAM, DMA_IT_TC, ENABLE);
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;//��ռ���ȼ� 3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //��Ӧ���ȼ� 3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //ʹ���ж�
    NVIC_Init(&NVIC_InitStructure); //ִ��NVIC����
    /* ��������DMA���͹��� */
    USART_DMACmd(USARTx, USART_DMAReq_Tx, ENABLE);

}

//DMA���䷱æ��־λ
static volatile uint8_t USART3_TX_DMA_IS_BUSY = 0;

void DMA1_Stream3_IRQHandler(void) {
    ITStatus ret = DMA_GetITStatus(USARTx_TX_DMA_STREAM, DMA_IT_TCIF3);
    if (ret == SET) {
        //����жϱ�־λ
        DMA_ClearITPendingBit(USARTx_TX_DMA_STREAM, DMA_IT_TCIF3);
        /* Clear DMA Transfer Complete Flags */
        DMA_ClearFlag(USARTx_TX_DMA_STREAM, USARTx_TX_DMA_FLAG_TCIF);
        /* Clear USART Transfer Complete Flags */
        USART_ClearFlag(USARTx, USART_FLAG_TC);
        DMA_Cmd(USARTx_TX_DMA_STREAM, DISABLE);
        USART3_TX_DMA_IS_BUSY = 0; //��־λ����
    }
}


/**
 * ʹ��DMA����һ��buffer������
 * @param buffer
 * @param len buffer����
 */
void mcu_uart_send_buffer_dma(uint8_t *buffer, uint32_t len) {
    while (USART3_TX_DMA_IS_BUSY);
    USART3_TX_DMA_IS_BUSY = 1;

    DMA_MemoryTargetConfig(USARTx_TX_DMA_STREAM, (uint32_t) buffer,
                           DMA_Memory_0);
    DMA_SetCurrDataCounter(USARTx_TX_DMA_STREAM, len);
    /* Enable DMA USART TX Stream */
    DMA_Cmd(USARTx_TX_DMA_STREAM, ENABLE);
}

void usart_app(void) {


#ifdef USART_RECEIVER

    /* Enable DMA USART RX Stream */
  DMA_Cmd(USARTx_RX_DMA_STREAM,ENABLE);

  /* Enable USART DMA RX Requsts */
  USART_DMACmd(USARTx, USART_DMAReq_Rx, ENABLE);

  /* Waiting the end of Data transfer */
  while (USART_GetFlagStatus(USARTx,USART_FLAG_TC)==RESET);
  while (DMA_GetFlagStatus(USARTx_RX_DMA_STREAM,USARTx_RX_DMA_FLAG_TCIF)==RESET);

  /* Clear DMA Transfer Complete Flags */
  DMA_ClearFlag(USARTx_RX_DMA_STREAM,USARTx_RX_DMA_FLAG_TCIF);
  /* Clear USART Transfer Complete Flags */
  USART_ClearFlag(USARTx,USART_FLAG_TC);

  if (Buffercmp(aTxBuffer, aRxBuffer, BUFFERSIZE) != FAILED)
  {
    /* Turn ON LED2 */
    STM_EVAL_LEDOn(LED2);
  }
  else
  {
    /* Turn ON LED3 */
    STM_EVAL_LEDOn(LED3);
  }

#endif /* USART_RECEIVER */

//    while (1)
//    {
//    }
}

/**
 * ��ʼ������ʱ�ӣ����裬�򿪽����ж�
 * @param comport ��Ҫ�򿪵Ĵ��ں�
 * @return
 */
int32_t mcu_uart_open(int32_t comport) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};

    UartHead3 = 0;
    UartEnd3 = 0;

    // ��GPIOʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    // �򿪴���ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    //GPIOB10 ����Ϊ USART3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    //GPIOB11 ����Ϊ USART3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

    // TXD ---- PB10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //����
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //���츴�����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //����
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    // RXD ---- PB11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //����
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //���츴�����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //����
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //USART ��ʼ������
    USART_InitStructure.USART_BaudRate = 2000000;//������;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ 8 λ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�շ�
    USART_Init(USART3, &USART_InitStructure); //��ʼ������
    USART_Cmd(USART3, ENABLE); //ʹ�ܴ���
    USART_ClearFlag(USART3, USART_FLAG_TC);//���жϱ�־

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�����ж�
    //Usart3 NVIC ���ã���Ҫ�������ж����ȼ�
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;//��ռ���ȼ� 3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //��Ӧ���ȼ� 3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //ʹ���ж�
    NVIC_Init(&NVIC_InitStructure); //ִ��NVIC����

    return (0);
}

/**
 *@brief:      mcu_uart_close
 *@details:    �رմ���
 *@param[in]   int32_t comport  
 *@param[out]  ��
 *@retval:     
 */
int32_t mcu_uart_close(int32_t comport) {

    UartHead3 = 0;
    UartEnd3 = 0;
    //����Ӳ�������Ƿ�Ҫ��������IO�ڡ�
    USART_Cmd(USART3, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE);

    return (0);
}

/**
 *@brief:      mcu_uart_tcflush
 *@details:    �崮�ڽ��ջ���
 *@param[in]   int32_t comport  
 *@param[out]  ��
 *@retval:     
 */
int32_t mcu_uart_tcflush(int32_t comport) {
    UartHead3 = UartEnd3;

    return 0;
}

/**
 *@brief:      mcu_uart_set_baud
 *@details:       ���ô��ڲ�����
 *@param[in]  int32_t comport   
               int32_t baud      
               int32_t databits  
               int32_t parity    
               int32_t stopbits  
               int32_t flowctl   
 *@param[out]  ��
 *@retval:     
 */
int32_t
mcu_uart_set_baud(int32_t comport, int32_t baud, int32_t databits, int32_t parity, int32_t stopbits, int32_t flowctl) {
    USART_InitTypeDef USART_InitStructure;

    if (mcu_uart_open(comport) == -1) return (-1);

    // �رմ���
    USART_Cmd(USART3, DISABLE);
    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART3, &USART_InitStructure);
    // �򿪴���
    mcu_uart_tcflush(comport);
    USART_Cmd(USART3, ENABLE);

    return 0;
}

/**
 *@brief:      mcu_uart_read
 *@details:    ����������
 *@param[in]   int32_t comport  
               uint8_t *buf      
               int32_t len      
 *@param[out]  ��
 *@retval:     
 */
int32_t mcu_uart_read(int32_t comport, uint8_t *buf, int32_t len) {
    int32_t i;

    if (len <= 0) return (-1);
    if (buf == NULL) return (-1);

    i = 0;

    //uart_printf("rec index:%d, %d\r\n", UartHead3, rec_end3);
    while (UartHead3 != UartEnd3) {
        *buf = UartBuf3[UartHead3++];
        if (UartHead3 >= RX3_TEMP_BUF_LEN_MAX)
            UartHead3 = 0;

        buf++;
        i++;
        if (i >= len) {
            break;
        }
    }
    return (i);
}

/**
 *@brief:      mcu_uart_write
 *@details:    д��������
 *@param[in]   int32_t comport  
               uint8_t *buf      
               int32_t len      
 *@param[out]  ��
 *@retval:     
 */
int32_t mcu_uart_write(int32_t comport, uint8_t *buf, int32_t len) {
    u32 t;
    u16 ch;

    if (len <= 0)
        return (-1);

    if (buf == NULL)
        return (-1);

    while (len != 0) {
        t = 0;
        // �ȴ����ڷ������
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {
            if (t++ >= 0x1000000)//��ʱ
                return (-1);
        }
        ch = (u16) (*buf & 0xff);
        USART_SendData(USART3, ch);
        buf++;
        len--;
    }

    return (0);
}

int32_t mcu_uart_sendstr(char *buf) {
    u32 t;
    u16 ch;

    if (buf == NULL)
        return (-1);

    while (*buf != 0) {
        // �ȴ����ڷ������
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {
        }
        USART_SendData(USART3, *buf);
        buf++;
    }
    return (0);
}


void USART3_IRQHandler1(void) {
    unsigned short ch;

    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//�ж��ǲ���RXNE�ж�
    {
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);//��RX�ж�
        if (USART_GetITStatus(USART3, USART_IT_ORE) != RESET) {
            // �������ݽ����ַ��ж�ʧ
            //wjq_log(LOG_DEBUG, "1");
            ch = USART_ReceiveData(USART3);
            USART_GetITStatus(USART3, USART_IT_ORE); // ���ORE���
            UartEnd3 = UartHead3;
            UartBuf3OverFg = 1;
        } else {
            ch = USART_ReceiveData(USART3);
            //uart_printf("%02x", ch);
            UartBuf3[UartEnd3++] = (unsigned char) (ch & 0xff);
            if (UartEnd3 >= RX3_TEMP_BUF_LEN_MAX)
                UartEnd3 = 0;

            if (UartEnd3 == UartHead3)       // ���ڽ��ջ��������
                UartBuf3OverFg = 1;
        }
    }

    if (USART_GetITStatus(USART3, USART_IT_FE) != RESET) {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART3, USART_IT_FE);
        USART_ReceiveData(USART3);

    }
#if 0
    /* If the USART3 detects a parity error */
    if(USART_GetITStatus(USART3, USART_IT_PE) != RESET)
    {
        while(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET)
        {
        }
        /* Clear the USART3 Parity error pending bit */
        USART_ClearITPendingBit(USART3, USART_IT_PE);
        USART_ReceiveData(USART3);
    }
    /* If a Overrun error is signaled by the card */
    if(USART_GetITStatus(USART3, USART_IT_ORE) != RESET)
    {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART3, USART_IT_ORE);
        USART_ReceiveData(USART3);
    }
    /* If a Noise error is signaled by the card */
    if(USART_GetITStatus(USART3, USART_IT_NE) != RESET)
    {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART3, USART_IT_NE);
        USART_ReceiveData(USART3);
    }
#endif
}

/**
 *@brief:      mcu_uart_test
 *@details:    ���ڲ���
 *@param[in]   void  
 *@param[out]  ��
 *@retval:     
 */
void mcu_uart_test(void) {
    uint8_t buf[12];
    int32_t len;
    int32_t res;

    len = mcu_uart_read(3, buf, 10);
    wjq_log(LOG_FUN, "mcu_dev_uart_read :%d\r\n", len);
    res = mcu_uart_write(3, buf, len);
    wjq_log(LOG_FUN, "mcu_dev_uart_write res: %d\r\n", res);
    wjq_log(LOG_FUN, "%s,%s,%d,%s\r\n", __FUNCTION__, __FILE__, __LINE__, __DATE__);

}

