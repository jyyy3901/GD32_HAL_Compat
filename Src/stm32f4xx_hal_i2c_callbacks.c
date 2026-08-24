#include "stm32f4xx_hal.h"

__weak void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);
}

__weak void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);
}

__weak void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);
}

__weak void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);
}

__weak void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);
}

__weak void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);
}

__weak void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);
}
