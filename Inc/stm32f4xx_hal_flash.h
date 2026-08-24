#ifndef STM32F4XX_HAL_FLASH_H
#define STM32F4XX_HAL_FLASH_H

#include "stm32f4xx_hal_def.h"

typedef enum
{
    FLASH_PROC_NONE = 0U,
    FLASH_PROC_SECTERASE,
    FLASH_PROC_PROGRAM,
    FLASH_PROC_GD32_PAGEERASE
} FLASH_ProcedureTypeDef;

typedef struct
{
    __IO FLASH_ProcedureTypeDef ProcedureOnGoing;
    __IO uint32_t NbSectorsToErase;
    __IO uint8_t VoltageForErase;
    __IO uint32_t Sector;
    __IO uint32_t Bank;
    __IO uint32_t Address;
    HAL_LockTypeDef Lock;
    __IO uint32_t ErrorCode;
    __IO uint32_t PagesRemaining;
    __IO uint32_t SectorEndAddress;
    __IO uint32_t ProgramWidth;
} FLASH_ProcessTypeDef;

#define HAL_FLASH_ERROR_NONE      0x00000000U
#define HAL_FLASH_ERROR_PGS       0x00000002U
#define HAL_FLASH_ERROR_PGA       0x00000008U
#define HAL_FLASH_ERROR_WRP       0x00000010U
#define HAL_FLASH_ERROR_OPERATION 0x00000020U

#define FLASH_TYPEPROGRAM_BYTE       0x00000000U
#define FLASH_TYPEPROGRAM_HALFWORD   0x00000001U
#define FLASH_TYPEPROGRAM_WORD       0x00000002U
#define FLASH_TYPEPROGRAM_DOUBLEWORD 0x00000003U

#define FLASH_LATENCY_0 0x00000000U
#define FLASH_LATENCY_1 0x00000001U
#define FLASH_LATENCY_2 0x00000002U

#define GD32_FLASH_BASE_ADDRESS 0x08000000UL
#define GD32_FLASH_SIZE_BYTES   0x00080000UL
#define GD32_FLASH_END_ADDRESS  (GD32_FLASH_BASE_ADDRESS + GD32_FLASH_SIZE_BYTES)
#define GD32_FLASH_PAGE_SIZE    0x00000800UL

extern FLASH_ProcessTypeDef pFlash;

HAL_StatusTypeDef HAL_FLASH_Unlock(void);
HAL_StatusTypeDef HAL_FLASH_Lock(void);
HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram,
                                    uint32_t Address,
                                    uint64_t Data);
HAL_StatusTypeDef HAL_FLASH_Program_IT(uint32_t TypeProgram,
                                       uint32_t Address,
                                       uint64_t Data);
void HAL_FLASH_IRQHandler(void);
uint32_t HAL_FLASH_GetError(void);
void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue);
void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue);

/* 默认无可写窗口；产品必须依据链接脚本显式开放独立数据分区。 */
HAL_StatusTypeDef GD32_HAL_FLASH_SetWritableRegion(uint32_t StartAddress,
                                                   uint32_t Length);
void GD32_HAL_FLASH_GetWritableRegion(uint32_t *StartAddress,
                                      uint32_t *EndAddress);

#endif /* STM32F4XX_HAL_FLASH_H */
