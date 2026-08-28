#ifndef STM32F4XX_HAL_TIM_EX_H
#define STM32F4XX_HAL_TIM_EX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal_tim.h"

HAL_StatusTypeDef HAL_TIMEx_MasterConfigSynchronization(
    TIM_HandleTypeDef *htim,
    const TIM_MasterConfigTypeDef *sMasterConfig);

/* 互补输出、死区/刹车、Hall、编码器和 DMA Burst 本阶段不做伪兼容。 */

#ifdef __cplusplus
}
#endif

#endif /* STM32F4XX_HAL_TIM_EX_H */
