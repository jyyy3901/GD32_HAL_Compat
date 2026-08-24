#include "stm32f4xx_hal.h"

/* 独立保存弱回调，便于产品覆盖并允许 Host 测试验证回调顺序。 */
__weak void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
}

__weak void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
}

__weak void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
}

__weak void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
}

__weak void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
}

__weak void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
}

__weak void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
}

__weak void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
}

__weak void HAL_UART_AbortTransmitCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
}

__weak void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
}
