//
// Created by xiaotian on 2022/12/3.
//

#ifndef F401_BALANCE_CAR_BOOTLOADER_COMMON_TOOL_H
#define F401_BALANCE_CAR_BOOTLOADER_COMMON_TOOL_H



/**
 * 位带操作，此种经过实际测试，效率和(GPIOG_BASE+20)这种一样
 */
#define BITBAND(addr,bit)  (((addr & 0xF0000000) +0x2000000) + ((addr & 0xFFFFF)*8+bit)*4)
#define MEM_ADDR(addr,bit) *(volatile unsigned int *)BITBAND(addr,bit)


#define PAout(bit)  MEM_ADDR((unsigned int)&GPIOA->ODR,bit)
#define PBout(bit)  MEM_ADDR((unsigned int)&GPIOB->ODR,bit)
#define PCout(bit)  MEM_ADDR((unsigned int)&GPIOC->ODR,bit)
#define PDout(bit)  MEM_ADDR((unsigned int)&GPIOD->ODR,bit)
#define PEout(bit)  MEM_ADDR((unsigned int)&GPIOE->ODR,bit)
#define PFout(bit)  MEM_ADDR((unsigned int)&GPIOF->ODR,bit)
#define PGout(bit)  MEM_ADDR((unsigned int)&GPIOG->ODR,bit)
#define PHout(bit)  MEM_ADDR((unsigned int)&GPIOH->ODR,bit)
//IDR
#define PAin(bit)   MEM_ADDR((unsigned int)&GPIOA->IDR,bit)
#define PBin(bit)   MEM_ADDR((unsigned int)&GPIOB->IDR,bit)
#define PCin(bit)   MEM_ADDR((unsigned int)&GPIOC->IDR,bit)
#define PDin(bit)   MEM_ADDR((unsigned int)&GPIOD->IDR,bit)
#define PEin(bit)   MEM_ADDR((unsigned int)&GPIOE->IDR,bit)
#define PFin(bit)   MEM_ADDR((unsigned int)&GPIOF->IDR,bit)
#define PGin(bit)   MEM_ADDR((unsigned int)&GPIOG->IDR,bit)
#define PHin(bit)   MEM_ADDR((unsigned int)&GPIOH->IDR,bit)



/* Private typedef -----------------------------------------------------------*/
typedef enum {
    FAILED = 0, PASSED = !FAILED
} TestStatus;

/* Private functions ---------------------------------------------------------*/
TestStatus Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint16_t BufferLength);



#endif //F401_BALANCE_CAR_BOOTLOADER_COMMON_TOOL_H
