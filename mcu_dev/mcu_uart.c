
#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"
#include "mcu_uart.h"
#include "wujique_log.h"


#define RX3_TEMP_BUF_LEN_MAX       4096/*���ڽ��ջ��峤��*/

volatile uint8_t UartBuf3[RX3_TEMP_BUF_LEN_MAX];//���ڽ��ջ���
volatile u16 UartHead3;//���ڽ��ջ���дָ��
volatile u16 UartEnd3;//���ڽ��ջ����ָ��
volatile uint8_t UartBuf3OverFg = 0;//���������־

/**
 *@brief:      mcu_dev_uart_open
 *@details:    ��ʼ������
 *@param[in]   int32_t comport  
 *@param[out]  ��
 *@retval:     
 */
int32_t mcu_uart_open(int32_t comport)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

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
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ� 3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =3; //��Ӧ���ȼ� 3
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
int32_t mcu_uart_close (int32_t comport)
{

    UartHead3 = 0;
    UartEnd3 = 0;
    //����Ӳ�������Ƿ�Ҫ��������IO�ڡ�
    USART_Cmd(USART3, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE);

    return(0);
}
/**
 *@brief:      mcu_uart_tcflush
 *@details:    �崮�ڽ��ջ���
 *@param[in]   int32_t comport  
 *@param[out]  ��
 *@retval:     
 */
int32_t mcu_uart_tcflush(int32_t comport)
{ 
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
int32_t mcu_uart_set_baud (int32_t comport, int32_t baud, int32_t databits, int32_t parity, int32_t stopbits, int32_t flowctl)
{
    USART_InitTypeDef USART_InitStructure;

    if( mcu_uart_open (comport) == -1) return(-1);
    
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
int32_t mcu_uart_read (int32_t comport, uint8_t *buf, int32_t len)
{
    int32_t i;
    
    if(len <= 0) return(-1);
    if(buf == NULL) return(-1);

    i = 0;

    //uart_printf("rec index:%d, %d\r\n", UartHead3, rec_end3);
    while(UartHead3 != UartEnd3)
    {
        *buf = UartBuf3[UartHead3++];
        if(UartHead3 >= RX3_TEMP_BUF_LEN_MAX) 
            UartHead3 = 0;

        buf ++;
        i ++;
        if(i >= len)
        {
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
int32_t mcu_uart_write (int32_t comport, uint8_t *buf, int32_t len)
{
    u32 t;
    u16 ch;
  
    if (len <= 0) 
        return(-1);
        
    if(buf == NULL) 
        return(-1);
 
    while(len != 0)
    {
        t = 0;
		// �ȴ����ڷ������
        while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
        {
            if(t++ >= 0x1000000)//��ʱ
                return(-1);
        }  
        ch = (u16)(*buf & 0xff);
        USART_SendData(USART3, ch);
        buf++;
        len--;
    }
    
    return(0);
}
/**
 *@brief:      mcu_uart3_IRQhandler
 *@details:    �����жϴ�����
 *@param[in]   void  
 *@param[out]  ��
 *@retval:     
 */
void mcu_uart3_IRQhandler(void)
{
    unsigned short ch;

    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//�ж��ǲ���RXNE�ж�
    {
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);//��RX�ж�
        if(USART_GetITStatus(USART3, USART_IT_ORE) != RESET)
        {
            // �������ݽ����ַ��ж�ʧ
            //wjq_log(LOG_DEBUG, "1");
            ch = USART_ReceiveData(USART3);
            USART_GetITStatus(USART3, USART_IT_ORE); // ���ORE���
            UartEnd3 = UartHead3;
            UartBuf3OverFg = 1;
        }
        else
        {
            ch = USART_ReceiveData(USART3);
            //uart_printf("%02x", ch);
            UartBuf3[UartEnd3++] = (unsigned char)(ch&0xff);
            if(UartEnd3 >= RX3_TEMP_BUF_LEN_MAX)
                UartEnd3 = 0;
                
            if(UartEnd3 == UartHead3)       // ���ڽ��ջ��������
                UartBuf3OverFg = 1;
        }
    }
    
    if(USART_GetITStatus(USART3, USART_IT_FE) != RESET)
    {
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
void mcu_uart_test(void)
{
    uint8_t buf[12];
    int32_t len;
    int32_t res;
    
    len =  mcu_uart_read (3, buf, 10);
    wjq_log(LOG_FUN, "mcu_dev_uart_read :%d\r\n", len);
    res = mcu_uart_write(3, buf, len);
    wjq_log(LOG_FUN, "mcu_dev_uart_write res: %d\r\n", res);
    wjq_log(LOG_FUN, "%s,%s,%d,%s\r\n", __FUNCTION__,__FILE__,__LINE__,__DATE__);
    
}

