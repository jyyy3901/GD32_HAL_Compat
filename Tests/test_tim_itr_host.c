#include "stm32f4xx_hal_tim.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    uint8_t itr = 0xFFU;

    assert(STM32_TIMER_TriggerToGD32ITI((uintptr_t)TIM1, 0U, &itr) == 1);
    assert(itr == 0U);
    assert(STM32_TIMER_TriggerToGD32ITI((uintptr_t)TIM1, 3U, &itr) == 1);
    assert(itr == 3U);
    assert(STM32_TIMER_TriggerToGD32ITI((uintptr_t)TIM2, 1U, &itr) == 0);
    assert(STM32_TIMER_TriggerToGD32ITI((uintptr_t)TIM4, 3U, &itr) == 0);
    assert(STM32_TIMER_TriggerToGD32ITI((uintptr_t)TIM5, 3U, &itr) == 0);
    assert(STM32_TIMER_TriggerToGD32ITI((uintptr_t)TIM9, 0U, &itr) == 1);
    assert(itr == 0U);
    assert(STM32_TIMER_TriggerToGD32ITI((uintptr_t)TIM9, 2U, &itr) == 0);
    assert(STM32_TIMER_TriggerToGD32ITI(
               (uintptr_t)STM32_HAL_INSTANCE_TOKEN(0x03U, 0x08U),
               0U,
               &itr) == 0);
    assert(STM32_TIMER_TriggerToGD32ITI((uintptr_t)TIM1, 4U, &itr) == 0);
    assert(STM32_TIMER_TriggerToGD32ITI((uintptr_t)TIM1, 0U, NULL) == 0);
    puts("TIM ITR conversion tests: PASS");
    return 0;
}
