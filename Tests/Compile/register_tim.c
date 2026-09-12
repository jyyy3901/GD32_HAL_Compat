#include "stm32f4xx_hal.h"

STM32_COMPAT_STATIC_ASSERT(TIM1_BASE == 0x40012C00UL, tim1_base_test);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, CNT) == 0x24U, tim_cnt_test);
STM32_COMPAT_STATIC_ASSERT(TIM_DIER_UIE == 0x1UL, tim_uie_test);

void RegisterTIMPositive(uint32_t value)
{
    TIM1->CR1 |= TIM_CR1_CEN;
    TIM1->DIER |= TIM_DIER_UIE;
    TIM1->CNT = 0U;
    TIM1->CCR1 = value;
}
