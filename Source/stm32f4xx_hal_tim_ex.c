#include "stm32f4xx_hal.h"

HAL_StatusTypeDef HAL_TIMEx_MasterConfigSynchronization(
    TIM_HandleTypeDef *htim,
    const TIM_MasterConfigTypeDef *sMasterConfig)
{
    uint32_t trigger;
    if ((htim == NULL) || (sMasterConfig == NULL) ||
        ((sMasterConfig->MasterOutputTrigger & ~0x70U) != 0U) ||
        ((sMasterConfig->MasterSlaveMode != TIM_MASTERSLAVEMODE_DISABLE) &&
         (sMasterConfig->MasterSlaveMode != TIM_MASTERSLAVEMODE_ENABLE)))
    {
        return HAL_ERROR;
    }
    trigger = sMasterConfig->MasterOutputTrigger >> 4U;
    return (GD32_HAL_TIMER_ConfigureMaster(
                htim->GD32_INSTANCE,
                (uint8_t)trigger,
                sMasterConfig->MasterSlaveMode == TIM_MASTERSLAVEMODE_ENABLE) == 0) ?
           HAL_OK : HAL_ERROR;
}
