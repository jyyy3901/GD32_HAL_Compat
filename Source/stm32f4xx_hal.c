#include "stm32f4xx_hal.h"

__IO uint32_t uwTick = 0U;
uint32_t uwTickPrio = (1UL << __NVIC_PRIO_BITS);
HAL_TickFreqTypeDef uwTickFreq = HAL_TICK_FREQ_DEFAULT;
__IO GD32_HAL_PortError gd32HalLastPortError = GD32_HAL_PORT_ERROR_NONE;
__IO uint32_t gd32HalLastPortErrorDetail = 0U;

HAL_StatusTypeDef HAL_Init(void)
{
    SystemCoreClockUpdate();
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    if (GD32_HAL_PortInit() != 0)
    {
        return HAL_ERROR;
    }

    if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK)
    {
        return HAL_ERROR;
    }

    HAL_MspInit();
    return HAL_OK;
}

HAL_StatusTypeDef HAL_DeInit(void)
{
    HAL_SuspendTick();
    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL = 0U;

    if (GD32_HAL_PortDeInit() != 0)
    {
        return HAL_ERROR;
    }

    HAL_MspDeInit();
    return HAL_OK;
}

__weak void HAL_MspInit(void)
{
}

__weak void HAL_MspDeInit(void)
{
}

__weak void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    gd32HalLastPortError = error;
    gd32HalLastPortErrorDetail = detail;
}

__weak void HAL_IncTick(void)
{
    uwTick += (uint32_t)uwTickFreq;
}

__weak uint32_t HAL_GetTick(void)
{
    return uwTick;
}

uint32_t HAL_GetTickPrio(void)
{
    return uwTickPrio;
}

HAL_StatusTypeDef HAL_SetTickFreq(HAL_TickFreqTypeDef Freq)
{
    HAL_TickFreqTypeDef previous;

    if ((Freq != HAL_TICK_FREQ_10HZ) &&
        (Freq != HAL_TICK_FREQ_100HZ) &&
        (Freq != HAL_TICK_FREQ_1KHZ))
    {
        return HAL_ERROR;
    }

    if (uwTickFreq == Freq)
    {
        return HAL_OK;
    }

    previous = uwTickFreq;
    uwTickFreq = Freq;
    if (HAL_InitTick(uwTickPrio) != HAL_OK)
    {
        uwTickFreq = previous;
        (void)HAL_InitTick(uwTickPrio);
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_TickFreqTypeDef HAL_GetTickFreq(void)
{
    return uwTickFreq;
}

__weak void HAL_Delay(uint32_t Delay)
{
    const uint32_t tick_start = HAL_GetTick();
    uint32_t wait = Delay;

    if (wait < HAL_MAX_DELAY)
    {
        wait += (uint32_t)uwTickFreq;
    }

    while ((HAL_GetTick() - tick_start) < wait)
    {
        /* 等待 SysTick；若在中断中调用，SysTick 优先级必须更高。 */
    }
}

uint32_t HAL_GetHalVersion(void)
{
    return GD32_HAL_COMPAT_VERSION;
}

uint32_t HAL_GetUIDw0(void)
{
    return GD32_HAL_ReadUIDWord(0U);
}

uint32_t HAL_GetUIDw1(void)
{
    return GD32_HAL_ReadUIDWord(1U);
}

uint32_t HAL_GetUIDw2(void)
{
    return GD32_HAL_ReadUIDWord(2U);
}
