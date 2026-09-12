#include "stm32f4xx_hal.h"

STM32_COMPAT_STATIC_ASSERT(SPI1_BASE == 0x40013000UL, spi1_base_test);
STM32_COMPAT_STATIC_ASSERT(offsetof(SPI_TypeDef, DR) == 0x0CU, spi_dr_test);
STM32_COMPAT_STATIC_ASSERT(SPI_SR_RXNE == 0x1UL, spi_rxne_test);

uint32_t RegisterSPIPositive(void)
{
    SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_SPE;
    return SPI1->SR & SPI_SR_RXNE;
}
