#include "stm32f4xx_hal.h"

__weak void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    UNUSED(hadc);
}

__weak void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
    UNUSED(hadc);
}

__weak void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    UNUSED(hadc);
}

__weak void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    UNUSED(hadc);
}

__weak void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    UNUSED(hadc);
}
