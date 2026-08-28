#include "stm32f4xx_hal.h"

__weak void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue)
{
    UNUSED(ReturnValue);
}

__weak void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue)
{
    UNUSED(ReturnValue);
}
