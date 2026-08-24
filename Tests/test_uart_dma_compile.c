#include "stm32f4xx_hal.h"

HAL_StatusTypeDef GD32_HAL_TestUARTDMAAPI(UART_HandleTypeDef *huart,
                                          uint8_t *data)
{
    HAL_StatusTypeDef status;

    status = HAL_UART_Transmit_DMA(huart, data, 1U);
    status = (status == HAL_OK) ? HAL_UART_Receive_DMA(huart, data, 1U) : status;
    status = (status == HAL_OK) ? HAL_UART_DMAPause(huart) : status;
    status = (status == HAL_OK) ? HAL_UART_DMAResume(huart) : status;
    return (status == HAL_OK) ? HAL_UART_DMAStop(huart) : status;
}
