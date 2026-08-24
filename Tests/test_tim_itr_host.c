#include "gd32_hal_port.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    uint8_t itr = 0xFFU;

    assert(GD32_HAL_TIMER_ResolveITR(GD32_HAL_TIMER0_ADDRESS, 0U, &itr) == 1);
    assert(itr == 0U);
    assert(GD32_HAL_TIMER_ResolveITR(GD32_HAL_TIMER0_ADDRESS, 3U, &itr) == 1);
    assert(itr == 3U);
    assert(GD32_HAL_TIMER_ResolveITR(GD32_HAL_TIMER1_ADDRESS, 1U, &itr) == 0);
    assert(GD32_HAL_TIMER_ResolveITR(GD32_HAL_TIMER3_ADDRESS, 3U, &itr) == 0);
    assert(GD32_HAL_TIMER_ResolveITR(GD32_HAL_TIMER4_ADDRESS, 3U, &itr) == 0);
    assert(GD32_HAL_TIMER_ResolveITR(GD32_HAL_TIMER8_ADDRESS, 0U, &itr) == 1);
    assert(itr == 0U);
    assert(GD32_HAL_TIMER_ResolveITR(GD32_HAL_TIMER8_ADDRESS, 2U, &itr) == 0);
    assert(GD32_HAL_TIMER_ResolveITR(GD32_HAL_TIMER7_ADDRESS, 0U, &itr) == 0);
    assert(GD32_HAL_TIMER_ResolveITR(GD32_HAL_TIMER0_ADDRESS, 4U, &itr) == 0);
    assert(GD32_HAL_TIMER_ResolveITR(GD32_HAL_TIMER0_ADDRESS, 0U, NULL) == 0);
    puts("TIM ITR conversion tests: PASS");
    return 0;
}
