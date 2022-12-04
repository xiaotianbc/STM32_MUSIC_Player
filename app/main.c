/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mcu_uart.h"
#include "wujique_log.h"

extern const unsigned char data[240816];


/* Private function prototypes -----------------------------------------------*/
static void TIM6_Config(void);


/* Private functions ---------------------------------------------------------*/

volatile uint8_t is_dac_dma_working_flag = 0;

DAC_InitTypeDef DAC_InitStructure;
DMA_InitTypeDef DMA_InitStructure;

void HAL_DAC_Start_DMA_init(void)
{
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
    DAC_Init(DAC_Channel_2, &DAC_InitStructure);

    /* DMA1_Stream6 channel7 configuration **************************************/
    DMA_DeInit(DMA1_Stream6);
    DMA_InitStructure.DMA_Channel = DMA_Channel_7;
//BeepData[BEEP_DATA_LEN]
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &DAC->DHR8R2; //8位，右对齐
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

    NVIC_InitTypeDef NVIC_InitStruct =
    {
        .NVIC_IRQChannel = DMA1_Stream6_IRQn,
        .NVIC_IRQChannelCmd = ENABLE,
        .NVIC_IRQChannelPreemptionPriority = 1,
        .NVIC_IRQChannelSubPriority = 1
    };
    NVIC_Init(&NVIC_InitStruct);
    DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);


    /* Enable DAC Channel2 */
    DAC_Cmd(DAC_Channel_2, ENABLE);

    /* Enable DMA for DAC Channel2 */
    DAC_DMACmd(DAC_Channel_2, ENABLE);
}

void HAL_DAC_Start_DMA(void *dac_handle, void *dac_channel_1_or_2, uint8_t *Memory0BaseAddr, uint32_t BUFFER_SIZE,
                       void *DAC_ALIGN_8B_R)
{

    /* Enable DMA1_Stream6 */
    DMA_Cmd(DMA1_Stream6, DISABLE);

    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) Memory0BaseAddr;
    DMA_InitStructure.DMA_BufferSize = BUFFER_SIZE;
    DMA_Init(DMA1_Stream6, &DMA_InitStructure);

    /* Enable DMA1_Stream6 */
    DMA_Cmd(DMA1_Stream6, ENABLE);


    is_dac_dma_working_flag = 1;
}

void DMA1_Stream6_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6) != 0)
    {
        DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
        is_dac_dma_working_flag = 0;
    }
}

#define        BUFFER_SIZE        (1024)

uint8_t Buffer0[BUFFER_SIZE] = {0};


/**
 * 播放一段wav音乐的程序，程序上电后，只执行一次，把所有音乐播放完
 */
void Music_Player(void)
{
    uint32_t DataLength = 0;
    uint8_t *DataAddress = NULL;

    DataLength = sizeof(data) - 0x2c;
    DataAddress = (unsigned char *) (data + 0x2c);
    TIM6_Config();
    HAL_DAC_Start_DMA_init();

    while (DataLength >= BUFFER_SIZE)
    {
        memcpy(Buffer0, DataAddress, BUFFER_SIZE);
        DataLength -= BUFFER_SIZE;
        DataAddress += BUFFER_SIZE;

        HAL_DAC_Start_DMA(0, 0,
                          (uint8_t *) Buffer0, BUFFER_SIZE, 0);

        while (is_dac_dma_working_flag != 0);
    }

    TIM_Cmd(TIM6, DISABLE);
    DAC_DMACmd(DAC_Channel_2, DISABLE);
}


/**
  * @brief  TIM6 Configuration
  * @note   TIM6 configuration is based on APB1 frequency
  * @note   TIM6 Update event occurs each TIM6CLK/256
  * @param  None
  * @retval None
  */
static void TIM6_Config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    /* TIM6 Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    /* --------------------------------------------------------
    TIM3 input clock (TIM6CLK) is set to 2 * APB1 clock (PCLK1),
    since APB1 prescaler is different from 1.
      TIM6CLK = 2 * PCLK1
      TIM6CLK = HCLK / 2 = SystemCoreClock /2

    TIM6 Update event occurs each TIM6CLK/256

    Note:
     SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f4xx.c file.
     Each time the core clock (HCLK) changes, user had to call SystemCoreClockUpdate()
     function to update SystemCoreClock variable value. Otherwise, any configuration
     based on this variable will be incorrect.

    ----------------------------------------------------------- */
    /* Time base configuration */
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = 125;
    TIM_TimeBaseStructure.TIM_Prescaler = 42 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

    /* TIM6 TRGO selection */
    TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);

    /* TIM6 enable counter */
    TIM_Cmd(TIM6, ENABLE);
}


RCC_ClocksTypeDef RCC_Clocks;

int main(void)
{
    //4位抢占优先级，0位相应优先级
    __NVIC_SetPriorityGrouping(NVIC_PriorityGroup_4);
    /* SysTick end of count event each ms */
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
    /* Add your application code here */
    /* Insert 50 ms delay */
    Delay(50);
    mcu_uart_open(PC_PORT);

    /* Preconfiguration before using DAC----------------------------------------*/
    GPIO_InitTypeDef GPIO_InitStructure;

    /* DMA1 clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    /* GPIOA clock enable (to be used with DAC) */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    /* DAC Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    /* DAC channel 1 & 2 (DAC_OUT1 = PA.4)(DAC_OUT2 = PA.5) configuration */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    Music_Player();

    while (1)
    {
        Delay(8000);
    }

}
