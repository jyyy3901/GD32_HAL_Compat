#include "stm32f4xx_hal.h"

STM32_COMPAT_STATIC_ASSERT(GPIOA_BASE == 0x40010800UL, gpioa_base_test);
STM32_COMPAT_STATIC_ASSERT(offsetof(GPIO_TypeDef, ODR) == 0x0CU, gpio_odr_test);
STM32_COMPAT_STATIC_ASSERT(GPIO_ODR_OD5 == 0x20UL, gpio_od5_test);

void RegisterGPIOPositive(void)
{
    GPIOA->ODR ^= GPIO_ODR_OD5;
    GPIOA->BSRR = GPIO_BSRR_BS5;
    (void)(GPIOA->IDR & GPIO_IDR_ID5);
}
