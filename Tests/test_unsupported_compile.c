#include "stm32f4xx_hal.h"

void GD32_HAL_TestUnsupportedI2C3(void)
{
    __HAL_RCC_I2C3_CLK_ENABLE();
}
