#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/spi.h>
#include "../tx.h"

#define CS_HI() gpio_set(GPIOB, GPIO2)
#define CS_LO() gpio_clear(GPIOB, GPIO2)

/*
 *
 */
void Initialize_SPIFlash()
{
    /* Enable SPI1 */
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_SPI1EN);
    /* Enable GPIOA */
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);
    /* Enable GPIOB */
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);

    /* CS */
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO2);

    /* SCK, MOSI */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5 | GPIO7);
    /* MISO */
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
                  GPIO_CNF_INPUT_FLOAT, GPIO6);
    CS_HI();
    /* Includes enable */
    spi_init_master(SPI1, 
                    SPI_CR1_BAUDRATE_FPCLK_DIV_4,
                    SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_2, 
                    SPI_CR1_DFF_8BIT,
                    SPI_CR1_MSBFIRST);
    spi_enable_software_slave_management(SPI1);
    spi_set_nss_high(SPI1);

    spi_enable(SPI1);
}
/*
 *
 */
u32 ReadFlashID()
{
    u32 result;
    CS_LO();

    spi_xfer(SPI1, 0x90);
    spi_xfer(SPI1, 0x00);
    spi_xfer(SPI1, 0x00);
    spi_xfer(SPI1, 0x00); /* Mfr, Dev */
    result  = (u8)spi_xfer(SPI1, 0);
    result <<= 8;
    result |= (u8)spi_xfer(SPI1, 0);
    result <<= 8;
    result |= (u8)spi_xfer(SPI1, 0);
    result <<= 8;
    result |= (u8)spi_xfer(SPI1, 0);
   
    CS_HI();
    return result;
}
/*
 *
 */
void WriteFlashWriteEnable()
{
    CS_LO();
    spi_xfer(SPI1, 0x06);
    CS_HI();
}
/*
 *
 */
void WriteFlashWriteDisable()
{
    CS_LO();
    spi_xfer(SPI1, 0x04);
    CS_HI();
}
/*
 *
 */
void DisableHWRYBY()
{
    CS_LO();
    spi_xfer(SPI1, 0x80);
    CS_HI();
}
/*
 *
 */
void WaitForWriteComplete()
{
    u8 sr;
    CS_LO();
    spi_xfer(SPI1, 0x05);
    do
    {
        sr = spi_xfer(SPI1, 0x00);
    }while(sr & 0x01); 
    CS_HI();
}
/*
 *
 */
void EraseSector(u32 sectorAddress)
{
    WriteFlashWriteEnable();

    CS_LO();
    spi_xfer(SPI1, 0x20);
    spi_xfer(SPI1, (u8)(sectorAddress >> 16));
    spi_xfer(SPI1, (u8)(sectorAddress >>  8));
    spi_xfer(SPI1, (u8)(sectorAddress));
    CS_HI();

    WaitForWriteComplete();
}
/*
 *
 */
void BulkErase()
{
    printf("BulkErase...\n\r");

    WriteFlashWriteEnable();

    CS_LO();
    spi_xfer(SPI1, 0xC7);
    CS_HI();

    WaitForWriteComplete();
}
/*
 * Length should be multiple of 2
 */
void WriteBytes(u32 writeAddress, u32 length, u8 * buffer)
{
    u32 i;

    printf("WriteBytes...\n\r");
    
    DisableHWRYBY();

    WriteFlashWriteEnable();

    CS_LO();
    spi_xfer(SPI1, 0xAD);
    spi_xfer(SPI1, (u8)(writeAddress >> 16));
    spi_xfer(SPI1, (u8)(writeAddress >>  8));
    spi_xfer(SPI1, (u8)(writeAddress));
    spi_xfer(SPI1, buffer[0]);
    spi_xfer(SPI1, buffer[1]);
    CS_HI();

    WaitForWriteComplete();

    for(i=2;i<length;i+=2)
    {
        CS_LO();
        spi_xfer(SPI1, 0xAD);
        spi_xfer(SPI1, buffer[i]);
        spi_xfer(SPI1, buffer[i+1]);
        CS_HI();

        WaitForWriteComplete();
    }

    WriteFlashWriteDisable();
}
/*
 *
 */
void ReadBytes(u32 readAddress, u32 length, u8 * buffer)
{
    u32 i;
    CS_LO();
    spi_xfer(SPI1, 0x03);
    spi_xfer(SPI1, (u8)(readAddress >> 16));
    spi_xfer(SPI1, (u8)(readAddress >>  8));
    spi_xfer(SPI1, (u8)(readAddress));

    for(i=0;i<length;i++)
    {
        spi_xfer(SPI1, buffer[i]);
    }

    CS_HI();
}
