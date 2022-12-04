//
// Created by xiaotian on 2022/9/13.
//

#include "mcu_spi.h"
#include "main.h"

typedef enum {
    board_flash_spi1 = 0x00,

} board_spi_TypeDef;
#if 0
void muc_spi_init(board_spi_TypeDef spi_number) {
    //初始化flash的spi,cs=pb14
    if (spi_number == board_flash_spi1) {
        //首先要开启外设时钟再修改寄存器
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
        SPI_InitTypeDef SPI_InitStruct = {
                //分频为/2，=APB2_Clock(84MHz)/2= 42Mhz
                .SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2,
                /*W25Q64支持SPI数据传输时序模式0（CPOL = 0、CPHA = 0）和模式3（CPOL = 1、CPHA = 1），
                 * 模式0和模式3主要区别是当SPI主机硬件接口处于空闲状态时，SCLK的电平状态是高电平或者是低电平。
                 * 对于模式0来说，SCLK处于低电平；对于模式3来说，SCLK处于高电平。
                 * 不过，在这两种模式下，芯片都是在SCLK的上升沿采集输入数据，下降沿输出数据。*/
                .SPI_CPHA=1,
                .SPI_CPOL=1,
                //如果spi操作过程没有使用crc校检，则此项无意义
                .SPI_CRCPolynomial=0,
                .SPI_DataSize=SPI_DataSize_8b,
                .SPI_Direction=SPI_Direction_2Lines_FullDuplex,
                .SPI_FirstBit=SPI_FirstBit_MSB,
                .SPI_Mode=SPI_Mode_Master,
                //软件控制cs引脚电平
                .SPI_NSS=SPI_NSS_Soft
        };
        SPI_Init(SPI1, &SPI_InitStruct);
        SPI_Cmd(SPI1,ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
        GPIO_InitTypeDef GPIO_InitStruct={
                .GPIO_Mode=GPIO_Mode_AF,
                .GPIO_OType=GPIO_OType_PP,
                .GPIO_Pin=GPIO_Pin_3|GPIO_Pin_5, //sck, mosi
                .GPIO_PuPd=GPIO_PuPd_NOPULL,
                .GPIO_Speed=GPIO_High_Speed
        };
        //初始化 sck, mosi
        GPIO_Init(GPIOB,&GPIO_InitStruct);
        GPIO_InitStruct.
    }

}
#endif