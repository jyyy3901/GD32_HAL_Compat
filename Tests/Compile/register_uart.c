#include "stm32f4xx_hal.h"

STM32_COMPAT_STATIC_ASSERT(USART1_BASE == 0x40013800UL, usart1_base_test);
STM32_COMPAT_STATIC_ASSERT(offsetof(USART_TypeDef, DR) == 0x04U, usart_dr_test);
STM32_COMPAT_STATIC_ASSERT(USART_SR_RXNE == 0x20UL, usart_rxne_test);

uint32_t RegisterUARTPositive(void)
{
    USART1->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE;
    if ((USART1->SR & USART_SR_RXNE) != 0U)
    {
        return USART1->DR;
    }
    return 0U;
}
