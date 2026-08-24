#ifndef STM32F4XX_HAL_FLASH_INTERNAL_H
#define STM32F4XX_HAL_FLASH_INTERNAL_H

#include "stm32f4xx_hal.h"

int GD32_HAL_FLASH_IsRangeWritableInternal(uint32_t address, uint32_t length);
void GD32_HAL_FLASH_SetErrorInternal(GD32_HAL_FLASHStatus status);
uint32_t GD32_HAL_FLASH_SectorStartInternal(uint32_t sector);
uint32_t GD32_HAL_FLASH_SectorEndInternal(uint32_t sector);
HAL_StatusTypeDef GD32_HAL_FLASH_BeginEraseITInternal(
    FLASH_ProcedureTypeDef procedure,
    uint32_t address,
    uint32_t pages,
    uint32_t sector,
    uint32_t sectors,
    uint32_t sector_end,
    uint32_t voltage);

#endif /* STM32F4XX_HAL_FLASH_INTERNAL_H */
