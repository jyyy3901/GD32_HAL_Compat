#include "stm32f4xx_hal.h"

STM32_COMPAT_STATIC_ASSERT(EXTI_BASE == 0x40010400UL, exti_base_test);
STM32_COMPAT_STATIC_ASSERT(offsetof(EXTI_TypeDef, PR) == 0x14U, exti_pr_test);

void RegisterEXTIPositive(void)
{
    EXTI->IMR |= EXTI_IMR_MR5;
    EXTI->PR = EXTI_PR_PR5;
}
