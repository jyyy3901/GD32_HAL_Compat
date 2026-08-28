#include "gd32_hal_port.h"
#include <stddef.h>

static int TIMER_STMSource(uint32_t timer, uint8_t itr, uint32_t *source)
{
    static const uint32_t tim1[] =
    {
        GD32_HAL_TIMER4_ADDRESS, GD32_HAL_TIMER1_ADDRESS,
        GD32_HAL_TIMER2_ADDRESS, GD32_HAL_TIMER3_ADDRESS
    };
    static const uint32_t tim2[] =
    {
        GD32_HAL_TIMER0_ADDRESS, 0U,
        GD32_HAL_TIMER2_ADDRESS, GD32_HAL_TIMER3_ADDRESS
    };
    static const uint32_t tim3[] =
    {
        GD32_HAL_TIMER0_ADDRESS, GD32_HAL_TIMER1_ADDRESS,
        GD32_HAL_TIMER4_ADDRESS, GD32_HAL_TIMER3_ADDRESS
    };
    static const uint32_t tim4[] =
    {
        GD32_HAL_TIMER0_ADDRESS, GD32_HAL_TIMER1_ADDRESS,
        GD32_HAL_TIMER2_ADDRESS, 0U
    };
    static const uint32_t tim5[] =
    {
        GD32_HAL_TIMER1_ADDRESS, GD32_HAL_TIMER2_ADDRESS,
        GD32_HAL_TIMER3_ADDRESS, 0U
    };
    /* STM32F401 的 TIM9 ITR2/3 是 TIM10_OC/TIM11_OC，不等同于 GD TRGO。 */
    static const uint32_t tim9[] =
    {
        GD32_HAL_TIMER1_ADDRESS, GD32_HAL_TIMER2_ADDRESS, 0U, 0U
    };
    const uint32_t *row = NULL;

    if (timer == GD32_HAL_TIMER0_ADDRESS) { row = tim1; }
    else if (timer == GD32_HAL_TIMER1_ADDRESS) { row = tim2; }
    else if (timer == GD32_HAL_TIMER2_ADDRESS) { row = tim3; }
    else if (timer == GD32_HAL_TIMER3_ADDRESS) { row = tim4; }
    else if (timer == GD32_HAL_TIMER4_ADDRESS) { row = tim5; }
    else if (timer == GD32_HAL_TIMER8_ADDRESS) { row = tim9; }

    if ((row == NULL) || (row[itr] == 0U))
    {
        return 0;
    }
    *source = row[itr];
    return 1;
}

int STM32_TIMER_TriggerToGD32ITI(uintptr_t stm32_timer_instance,
                                 uint8_t stm_itr,
                                 uint8_t *gd_itr)
{
    const GD32_HAL_Resource *resource =
        GD32_HAL_ResolveInstance(stm32_timer_instance,
                                 GD32_HAL_RESOURCE_TIMER);
    static const uint32_t gd0[] =
    {
        GD32_HAL_TIMER4_ADDRESS, GD32_HAL_TIMER1_ADDRESS,
        GD32_HAL_TIMER2_ADDRESS, GD32_HAL_TIMER3_ADDRESS
    };
    static const uint32_t gd1[] =
    {
        GD32_HAL_TIMER0_ADDRESS, GD32_HAL_TIMER7_ADDRESS,
        GD32_HAL_TIMER2_ADDRESS, GD32_HAL_TIMER3_ADDRESS
    };
    static const uint32_t gd2[] =
    {
        GD32_HAL_TIMER0_ADDRESS, GD32_HAL_TIMER1_ADDRESS,
        GD32_HAL_TIMER4_ADDRESS, GD32_HAL_TIMER3_ADDRESS
    };
    static const uint32_t gd3[] =
    {
        GD32_HAL_TIMER0_ADDRESS, GD32_HAL_TIMER1_ADDRESS,
        GD32_HAL_TIMER2_ADDRESS, GD32_HAL_TIMER7_ADDRESS
    };
    static const uint32_t gd4[] =
    {
        GD32_HAL_TIMER1_ADDRESS, GD32_HAL_TIMER2_ADDRESS,
        GD32_HAL_TIMER3_ADDRESS, GD32_HAL_TIMER7_ADDRESS
    };
    static const uint32_t gd8[] =
    {
        GD32_HAL_TIMER1_ADDRESS, GD32_HAL_TIMER2_ADDRESS,
        GD32_HAL_TIMER9_ADDRESS, GD32_HAL_TIMER10_ADDRESS
    };
    const uint32_t *row = NULL;
    uint32_t timer_address;
    uint32_t source;
    uint8_t i;

    if ((resource == NULL) || (gd_itr == NULL) || (stm_itr > 3U))
    {
        return 0;
    }
    timer_address = resource->gd32_instance;
    if (TIMER_STMSource(timer_address, stm_itr, &source) == 0)
    {
        return 0;
    }
    if (timer_address == GD32_HAL_TIMER0_ADDRESS) { row = gd0; }
    else if (timer_address == GD32_HAL_TIMER1_ADDRESS) { row = gd1; }
    else if (timer_address == GD32_HAL_TIMER2_ADDRESS) { row = gd2; }
    else if (timer_address == GD32_HAL_TIMER3_ADDRESS) { row = gd3; }
    else if (timer_address == GD32_HAL_TIMER4_ADDRESS) { row = gd4; }
    else if (timer_address == GD32_HAL_TIMER8_ADDRESS) { row = gd8; }

    if (row == NULL) { return 0; }
    for (i = 0U; i < 4U; ++i)
    {
        if (row[i] == source)
        {
            *gd_itr = i;
            return 1;
        }
    }
    return 0;
}
