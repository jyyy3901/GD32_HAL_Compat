#ifndef STM32F4XX_HAL_FLASH_EX_H
#define STM32F4XX_HAL_FLASH_EX_H

#include "stm32f4xx_hal_flash.h"

typedef struct
{
    uint32_t TypeErase;
    uint32_t Banks;
    uint32_t Sector;
    uint32_t NbSectors;
    uint32_t VoltageRange;
} FLASH_EraseInitTypeDef;

typedef struct
{
    uint32_t PageAddress;
    uint32_t NbPages;
} GD32_FLASH_PageEraseInitTypeDef;

#define FLASH_TYPEERASE_SECTORS   0x00000000U
#define FLASH_TYPEERASE_MASSERASE 0x00000001U
#define FLASH_BANK_1 0x00000001U
#define FLASH_VOLTAGE_RANGE_1 0x00000000U
#define FLASH_VOLTAGE_RANGE_2 0x00000001U
#define FLASH_VOLTAGE_RANGE_3 0x00000002U
#define FLASH_VOLTAGE_RANGE_4 0x00000003U

#define FLASH_SECTOR_0 0U
#define FLASH_SECTOR_1 1U
#define FLASH_SECTOR_2 2U
#define FLASH_SECTOR_3 3U
#define FLASH_SECTOR_4 4U
#define FLASH_SECTOR_5 5U
#define FLASH_SECTOR_6 6U
#define FLASH_SECTOR_7 7U

HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit,
                                    uint32_t *SectorError);
HAL_StatusTypeDef HAL_FLASHEx_Erase_IT(FLASH_EraseInitTypeDef *pEraseInit);
HAL_StatusTypeDef GD32_HAL_FLASHEx_ErasePages(
    const GD32_FLASH_PageEraseInitTypeDef *pEraseInit,
    uint32_t *PageError);
HAL_StatusTypeDef GD32_HAL_FLASHEx_ErasePages_IT(
    const GD32_FLASH_PageEraseInitTypeDef *pEraseInit);

#endif /* STM32F4XX_HAL_FLASH_EX_H */
