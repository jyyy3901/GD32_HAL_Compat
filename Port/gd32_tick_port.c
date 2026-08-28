#include "stm32f4xx_hal.h"

__weak HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    uint32_t reload;
    uint32_t tick_hz;

    if ((TickPriority >= (1UL << __NVIC_PRIO_BITS)) ||
        ((uint32_t)uwTickFreq == 0U))
    {
        return HAL_ERROR;
    }

    SystemCoreClockUpdate();
    tick_hz = 1000U / (uint32_t)uwTickFreq;
    reload = SystemCoreClock / tick_hz;

    if ((reload != 0U) && (reload <= 0x01000000U))
    {
        if (HAL_SYSTICK_Config(reload) != 0U)
        {
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TICK_RELOAD, reload);
            return HAL_ERROR;
        }
    }
    else
    {
        /* 168 MHz 下 10 Hz 超过 24 位容量，改用 HCLK/8 保持 HAL 语义。 */
        reload = (SystemCoreClock / 8U) / tick_hz;
        if ((reload == 0U) || (reload > 0x01000000U) ||
            (HAL_SYSTICK_Config(reload) != 0U))
        {
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TICK_RELOAD, reload);
            return HAL_ERROR;
        }
        HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8);
    }

    HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority, 0U);
    uwTickPrio = TickPriority;
    return HAL_OK;
}

void HAL_SuspendTick(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

void HAL_ResumeTick(void)
{
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}
