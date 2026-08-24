#include "stm32f4xx_hal.h"

__weak void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    UNUSED(hspi);
}

__weak void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    UNUSED(hspi);
}

__weak void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    UNUSED(hspi);
}

__weak void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    UNUSED(hspi);
}

__weak void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    UNUSED(hspi);
}

__weak void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
    UNUSED(hspi);
}

__weak void HAL_SPI_RxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
    UNUSED(hspi);
}

__weak void HAL_SPI_TxRxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
    UNUSED(hspi);
}

__weak void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    UNUSED(hspi);
}
