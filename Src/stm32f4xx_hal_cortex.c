#include "stm32f4xx_hal.h"

void HAL_NVIC_SetPriorityGrouping(uint32_t PriorityGroup)
{
    NVIC_SetPriorityGrouping(PriorityGroup);
}

void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority)
{
    const uint32_t priority_group = NVIC_GetPriorityGrouping();
    NVIC_SetPriority(IRQn, NVIC_EncodePriority(priority_group, PreemptPriority, SubPriority));
}

void HAL_NVIC_EnableIRQ(IRQn_Type IRQn)
{
    NVIC_EnableIRQ(IRQn);
}

void HAL_NVIC_DisableIRQ(IRQn_Type IRQn)
{
    NVIC_DisableIRQ(IRQn);
}

void HAL_NVIC_SystemReset(void)
{
    NVIC_SystemReset();
}

uint32_t HAL_SYSTICK_Config(uint32_t TicksNumb)
{
    return SysTick_Config(TicksNumb);
}

uint32_t HAL_NVIC_GetPriorityGrouping(void)
{
    return NVIC_GetPriorityGrouping();
}

void HAL_NVIC_GetPriority(IRQn_Type IRQn,
                          uint32_t PriorityGroup,
                          uint32_t *pPreemptPriority,
                          uint32_t *pSubPriority)
{
    NVIC_DecodePriority(NVIC_GetPriority(IRQn),
                        PriorityGroup,
                        pPreemptPriority,
                        pSubPriority);
}

uint32_t HAL_NVIC_GetPendingIRQ(IRQn_Type IRQn)
{
    return NVIC_GetPendingIRQ(IRQn);
}

void HAL_NVIC_SetPendingIRQ(IRQn_Type IRQn)
{
    NVIC_SetPendingIRQ(IRQn);
}

void HAL_NVIC_ClearPendingIRQ(IRQn_Type IRQn)
{
    NVIC_ClearPendingIRQ(IRQn);
}

uint32_t HAL_NVIC_GetActive(IRQn_Type IRQn)
{
    return NVIC_GetActive(IRQn);
}

void HAL_SYSTICK_CLKSourceConfig(uint32_t CLKSource)
{
    if (CLKSource == SYSTICK_CLKSOURCE_HCLK)
    {
        SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    }
    else
    {
        SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;
    }
}

void HAL_SYSTICK_IRQHandler(void)
{
    HAL_SYSTICK_Callback();
}

__weak void HAL_SYSTICK_Callback(void)
{
}

#if (__MPU_PRESENT == 1U)
void HAL_MPU_Disable(void)
{
    __DMB();
    SCB->SHCSR &= ~SCB_SHCSR_MEMFAULTENA_Msk;
    MPU->CTRL = 0U;
}

void HAL_MPU_Enable(uint32_t MPU_Control)
{
    MPU->CTRL = MPU_Control | MPU_CTRL_ENABLE_Msk;
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
    __DSB();
    __ISB();
}

void HAL_MPU_EnableRegion(uint32_t RegionNumber)
{
    MPU->RNR = RegionNumber;
    MPU->RASR |= MPU_RASR_ENABLE_Msk;
}

void HAL_MPU_DisableRegion(uint32_t RegionNumber)
{
    MPU->RNR = RegionNumber;
    MPU->RASR &= ~MPU_RASR_ENABLE_Msk;
}

void HAL_MPU_ConfigRegion(MPU_Region_InitTypeDef *MPU_Init)
{
    if (MPU_Init == NULL)
    {
        return;
    }

    MPU->RNR = MPU_Init->Number;
    MPU->RASR &= ~MPU_RASR_ENABLE_Msk;
    MPU->RBAR = MPU_Init->BaseAddress;
    MPU->RASR = ((uint32_t)MPU_Init->DisableExec << MPU_RASR_XN_Pos) |
                ((uint32_t)MPU_Init->AccessPermission << MPU_RASR_AP_Pos) |
                ((uint32_t)MPU_Init->TypeExtField << MPU_RASR_TEX_Pos) |
                ((uint32_t)MPU_Init->IsShareable << MPU_RASR_S_Pos) |
                ((uint32_t)MPU_Init->IsCacheable << MPU_RASR_C_Pos) |
                ((uint32_t)MPU_Init->IsBufferable << MPU_RASR_B_Pos) |
                ((uint32_t)MPU_Init->SubRegionDisable << MPU_RASR_SRD_Pos) |
                ((uint32_t)MPU_Init->Size << MPU_RASR_SIZE_Pos) |
                ((uint32_t)MPU_Init->Enable << MPU_RASR_ENABLE_Pos);
}
#endif

void HAL_CORTEX_ClearEvent(void)
{
    __SEV();
    __WFE();
}
