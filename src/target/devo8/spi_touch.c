/*
    This project is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/spi.h>
#include "target.h"

#define START  (1 << 7)

#define ADR_Y   (1 << 4)
#define ADR_RST (2 << 4)
#define ADR_Z1  (3 << 4)
#define ADR_Z2  (4 << 4)
#define ADR_X   (5 << 4)

#define MODE   (0 << 3)
#define SER    (1 << 2)
#define PD     (0 << 0)

#define READ_X  START | ADR_X   | MODE | SER | PD
#define READ_Y  START | ADR_Y   | MODE | SER | PD
#define READ_Z1 START | ADR_Z1  | MODE | SER | PD
#define READ_Z2 START | ADR_Z2  | MODE | SER | PD
#define RESET   START | ADR_RST | MODE | SER | PD | 0x01

/*
PB0 : Chip Select
PB5 : PenIRQ
PA5 : SPI1_SCK
PA6 : SPI1_MISO
PA7 : SPI1_MOSI
*/

#define CS_HI() gpio_set(GPIOB, GPIO0)
#define CS_LO() gpio_clear(GPIOB, GPIO0)
#define pen_is_down() (! gpio_get(GPIOB, GPIO5))

u8 read_channel(u8 address)
{
    spi_xfer(SPI1, address);
    while(pen_is_down())
        ;
    return spi_xfer(SPI1, 0x00);
}

void SPITouch_Init()
{
#if 0
    /* Enable SPI1 */
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_SPI1EN);
    /* Enable GPIOA */
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);
    /* Enable GPIOB */
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
#endif
    /* CS */
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO0);

    /* PenIRQ is pull-up input*/
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT,
                  GPIO_CNF_INPUT_PULL_UPDOWN, GPIO5);
    gpio_set(GPIOB, GPIO5);

    CS_LO();
    spi_xfer(SPI1, RESET);
    CS_HI();
#if 0
    /* SCK, MOSI */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5 | GPIO7);
    /* MISO */
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
                  GPIO_CNF_INPUT_FLOAT, GPIO6);
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
#endif
}

struct touch SPITouch_GetCoords()
{
    struct touch res;
    CS_LO();
    res.x = read_channel(READ_X);
    res.y = read_channel(READ_Y);
    res.z1 = read_channel(READ_Z1);
    res.z2 = read_channel(READ_Z2);
    CS_HI();
    return res;
}

int SPITouch_IRQ()
{
    return pen_is_down();
}

