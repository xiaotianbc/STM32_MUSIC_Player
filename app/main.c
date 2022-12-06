/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mcu_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "common_tool.h"
#include "stm324xg_eval_sdio_sd.h"


RCC_ClocksTypeDef RCC_Clocks;

GPIO_InitTypeDef GPIO_InitStructure;

void master_task_main(void *arg) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);


    while (1) {
        if (GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_12) == 0) {
            vTaskDelay(20);
            if (GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_12) == 0) {
                printf_("PF12\n");
                vTaskDelay(20);
            }
        }
        //  printf_("hello,master_task_main\n");
        vTaskDelay(20);
    }
    vTaskDelete(NULL);
}


/* Private define ------------------------------------------------------------*/
#define BLOCK_SIZE            512 /* Block Size in Bytes */

#define NUMBER_OF_BLOCKS      50  /* For Multi Blocks operation (Read/Write) */
#define MULTI_BUFFER_SIZE    (BLOCK_SIZE * NUMBER_OF_BLOCKS)

#define SD_OPERATION_ERASE          0
#define SD_OPERATION_BLOCK          1
#define SD_OPERATION_MULTI_BLOCK    2
#define SD_OPERATION_END            3

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t aBuffer_Block_Tx[BLOCK_SIZE];
uint8_t aBuffer_Block_Rx[BLOCK_SIZE];
uint8_t aBuffer_MultiBlock_Tx[MULTI_BUFFER_SIZE];
uint8_t aBuffer_MultiBlock_Rx[MULTI_BUFFER_SIZE];

__IO TestStatus EraseStatus = FAILED;
__IO TestStatus TransferStatus1 = FAILED;
__IO TestStatus TransferStatus2 = FAILED;

SD_Error Status = SD_OK;
__IO uint32_t uwSDCardOperation = SD_OPERATION_ERASE;

/* Private function prototypes -----------------------------------------------*/
static void NVIC_Configuration(void);

static void SD_EraseTest(void);

static void SD_SingleBlockTest(void);

static void SD_MultiBlockTest(void);

static void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset);

static TestStatus eBuffercmp(uint8_t *pBuffer, uint32_t BufferLength);


void sdio_test_main(void *a) {

    /* NVIC Configuration */
    NVIC_Configuration();

    /*------------------------------ SD Init ---------------------------------- */
    if ((Status = SD_Init()) != SD_OK) {
        printf_("(Status = SD_Init()) != SD_OK\n");
        //  STM_EVAL_LEDOn(LED4);
    }

    while ((Status == SD_OK) && (uwSDCardOperation != SD_OPERATION_END) && (SD_Detect() == SD_PRESENT)) {
        switch (uwSDCardOperation) {
            /*-------------------------- SD Erase Test ---------------------------- */
            case (SD_OPERATION_ERASE): {
                SD_EraseTest();
                uwSDCardOperation = SD_OPERATION_BLOCK;
                break;
            }
                /*-------------------------- SD Single Block Test --------------------- */
            case (SD_OPERATION_BLOCK): {
                SD_SingleBlockTest();
                uwSDCardOperation = SD_OPERATION_MULTI_BLOCK;
                break;
            }
                /*-------------------------- SD Multi Blocks Test --------------------- */
            case (SD_OPERATION_MULTI_BLOCK): {
                SD_MultiBlockTest();
                uwSDCardOperation = SD_OPERATION_END;
                break;
            }
        }
        vTaskDelay(10);
    }

}

int main(void) {
    //4位抢占优先级，0位相应优先级
    __NVIC_SetPriorityGrouping(NVIC_PriorityGroup_4);
    /* SysTick end of count event each ms */
    RCC_GetClocksFreq(&RCC_Clocks);
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

    mcu_uart_open(PC_PORT);

//    xTaskCreate(master_task_main,  /* 任务入口函数 */
//                "MASTER",    /* 任务名字 */
//                1024,    /* 任务栈大小 */
//                NULL,        /* 任务入口函数参数 */
//                1,  /* 任务的优先级 */
//                NULL);  /* 任务控制块指针 */
    xTaskCreate(sdio_test_main,  /* 任务入口函数 */
                "sdio_test_main",    /* 任务名字 */
                4096,    /* 任务栈大小 */
                NULL,        /* 任务入口函数参数 */
                1,  /* 任务的优先级 */
                NULL);  /* 任务控制块指针 */
    vTaskStartScheduler();
    while (1) {
        printf_("Hello, world\n");
    }
}


/**
  * @brief  Configures SDIO IRQ channel.
  * @param  None
  * @retval None
  */
static void NVIC_Configuration(void) {
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = SD_SDIO_DMA_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  Tests the SD card erase operation.
  * @param  None
  * @retval None
  */
static void SD_EraseTest(void) {
    /*------------------- Block Erase ------------------------------------------*/
    if (Status == SD_OK) {
        /* Erase NumberOfBlocks Blocks of WRITE_BL_LEN(512 Bytes) */
        Status = SD_Erase(0x00, (BLOCK_SIZE * NUMBER_OF_BLOCKS));
    }

    if (Status == SD_OK) {
        Status = SD_ReadMultiBlocks(aBuffer_MultiBlock_Rx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);

        /* Check if the Transfer is finished */
        Status = SD_WaitReadOperation();

        /* Wait until end of DMA transfer */
        while (SD_GetStatus() != SD_TRANSFER_OK);
    }

    /* Check the correctness of erased blocks */
    if (Status == SD_OK) {
        EraseStatus = eBuffercmp(aBuffer_MultiBlock_Rx, MULTI_BUFFER_SIZE);
    }

    if (EraseStatus == PASSED) {
        printf_("EraseStatus == PASSED\n");
        //  STM_EVAL_LEDOn(LED1);
    } else {
        printf_("EraseStatus != PASSED\n");
        //   STM_EVAL_LEDOff(LED1);
        //  STM_EVAL_LEDOn(LED4);
    }
}

/**
  * @brief  Tests the SD card Single Blocks operations.
  * @param  None
  * @retval None
  */
static void SD_SingleBlockTest(void) {
    /*------------------- Block Read/Write --------------------------*/
    /* Fill the buffer to send */
    Fill_Buffer(aBuffer_Block_Tx, BLOCK_SIZE, 0x320F);

    if (Status == SD_OK) {
        /* Write block of 512 bytes on address 0 */
        Status = SD_WriteBlock(aBuffer_Block_Tx, 0x00, BLOCK_SIZE);
        /* Check if the Transfer is finished */
        Status = SD_WaitWriteOperation();
        while (SD_GetStatus() != SD_TRANSFER_OK);
    }

    if (Status == SD_OK) {
        /* Read block of 512 bytes from address 0 */
        Status = SD_ReadBlock(aBuffer_Block_Rx, 0x00, BLOCK_SIZE);
        /* Check if the Transfer is finished */
        Status = SD_WaitReadOperation();
        while (SD_GetStatus() != SD_TRANSFER_OK);
    }

    /* Check the correctness of written data */
    if (Status == SD_OK) {
        TransferStatus1 = Buffercmp(aBuffer_Block_Tx, aBuffer_Block_Rx, BLOCK_SIZE);
    }

    if (TransferStatus1 == PASSED) {
        printf_("TransferStatus1 == PASSED\n");
        // STM_EVAL_LEDOn(LED2);
    } else {
        printf_("TransferStatus1 != PASSED\n");
        //  STM_EVAL_LEDOff(LED2);
        //  STM_EVAL_LEDOn(LED4);
    }
}

/**
  * @brief  Tests the SD card Multiple Blocks operations.
  * @param  None
  * @retval None
  */
static void SD_MultiBlockTest(void) {
    /* Fill the buffer to send */
    Fill_Buffer(aBuffer_MultiBlock_Tx, MULTI_BUFFER_SIZE, 0x0);

    if (Status == SD_OK) {
        /* Write multiple block of many bytes on address 0 */
        Status = SD_WriteMultiBlocks(aBuffer_MultiBlock_Tx, 0, BLOCK_SIZE, NUMBER_OF_BLOCKS);

        /* Check if the Transfer is finished */
        Status = SD_WaitWriteOperation();
        while (SD_GetStatus() != SD_TRANSFER_OK);
    }

    if (Status == SD_OK) {
        /* Read block of many bytes from address 0 */
        Status = SD_ReadMultiBlocks(aBuffer_MultiBlock_Rx, 0, BLOCK_SIZE, NUMBER_OF_BLOCKS);

        /* Check if the Transfer is finished */
        Status = SD_WaitReadOperation();
        while (SD_GetStatus() != SD_TRANSFER_OK);
    }

    /* Check the correctness of written data */
    if (Status == SD_OK) {
        TransferStatus2 = Buffercmp(aBuffer_MultiBlock_Tx, aBuffer_MultiBlock_Rx, MULTI_BUFFER_SIZE);
    }

    if (TransferStatus2 == PASSED) {
        printf_("TransferStatus2 == PASSED\n");
        //  STM_EVAL_LEDOn(LED3);
    } else {
        printf_("TransferStatus2 != PASSED\n");
        //   STM_EVAL_LEDOff(LED3);
        //   STM_EVAL_LEDOn(LED4);
    }
}


/**
  * @brief  Fills buffer with user predefined data.
  * @param  pBuffer: pointer on the Buffer to fill
  * @param  BufferLength: size of the buffer to fill
  * @param  Offset: first value to fill on the Buffer
  * @retval None
  */
static void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset) {
    uint16_t index = 0;

    /* Put in global buffer same values */
    for (index = 0; index < BufferLength; index++) {
        pBuffer[index] = index + Offset;
    }
}

/**
  * @brief  Checks if a buffer has all its values are equal to zero.
  * @param  pBuffer: buffer to be compared.
  * @param  BufferLength: buffer's length
  * @retval PASSED: pBuffer values are zero
  *         FAILED: At least one value from pBuffer buffer is different from zero.
  */
static TestStatus eBuffercmp(uint8_t *pBuffer, uint32_t BufferLength) {
    while (BufferLength--) {
        /* In some SD Cards the erased state is 0xFF, in others it's 0x00 */
        if ((*pBuffer != 0xFF) && (*pBuffer != 0x00)) {
            return FAILED;
        }

        pBuffer++;
    }

    return PASSED;
}
