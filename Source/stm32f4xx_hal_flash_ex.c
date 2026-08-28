#include "stm32f4xx_hal_flash_internal.h"

static int GD32_HAL_FLASH_ValidatePages(uint32_t address,
                                        uint32_t pages,
                                        uint32_t *length)
{
    if ((length == NULL) || (pages == 0U) ||
        ((address & (GD32_FLASH_PAGE_SIZE - 1U)) != 0U) ||
        (pages > (UINT32_MAX / GD32_FLASH_PAGE_SIZE)))
    {
        return 0;
    }
    *length = pages * GD32_FLASH_PAGE_SIZE;
    return GD32_HAL_FLASH_IsRangeWritableInternal(address, *length);
}

HAL_StatusTypeDef GD32_HAL_FLASHEx_ErasePages(
    const GD32_FLASH_PageEraseInitTypeDef *pEraseInit,
    uint32_t *PageError)
{
    uint32_t length;
    uint32_t page;

    if (PageError != NULL) *PageError = 0xFFFFFFFFUL;
    if ((pEraseInit == NULL) || (PageError == NULL) ||
        (GD32_HAL_FLASH_ValidatePages(pEraseInit->PageAddress,
                                      pEraseInit->NbPages,
                                      &length) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_FLASH_INVALID_RANGE,
                           (pEraseInit != NULL) ? pEraseInit->PageAddress : 0U);
        return HAL_ERROR;
    }

    __HAL_LOCK(&pFlash);
    pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;
    pFlash.ProcedureOnGoing = FLASH_PROC_GD32_PAGEERASE;
    for (page = 0U; page < pEraseInit->NbPages; ++page)
    {
        const uint32_t address = pEraseInit->PageAddress +
                                 (page * GD32_FLASH_PAGE_SIZE);
        GD32_HAL_FLASHStatus status = GD32_HAL_FLASH_ErasePage(address);
        if (status != GD32_HAL_FLASH_READY)
        {
            *PageError = address;
            GD32_HAL_FLASH_SetErrorInternal(status);
            pFlash.ProcedureOnGoing = FLASH_PROC_NONE;
            __HAL_UNLOCK(&pFlash);
            return HAL_ERROR;
        }
    }
    UNUSED(length);
    pFlash.ProcedureOnGoing = FLASH_PROC_NONE;
    __HAL_UNLOCK(&pFlash);
    return HAL_OK;
}

HAL_StatusTypeDef GD32_HAL_FLASHEx_ErasePages_IT(
    const GD32_FLASH_PageEraseInitTypeDef *pEraseInit)
{
    uint32_t length;

    if ((pEraseInit == NULL) ||
        (GD32_HAL_FLASH_ValidatePages(pEraseInit->PageAddress,
                                      pEraseInit->NbPages,
                                      &length) == 0))
    {
        return HAL_ERROR;
    }
    UNUSED(length);
    return GD32_HAL_FLASH_BeginEraseITInternal(FLASH_PROC_GD32_PAGEERASE,
                                                pEraseInit->PageAddress,
                                                pEraseInit->NbPages,
                                                0U,
                                                0U,
                                                0U,
                                                0U);
}

static int GD32_HAL_FLASH_ValidateSectorErase(
    const FLASH_EraseInitTypeDef *pEraseInit,
    uint32_t *start,
    uint32_t *end)
{
    uint32_t last_sector;

    if ((pEraseInit == NULL) || (start == NULL) || (end == NULL) ||
        (pEraseInit->TypeErase != FLASH_TYPEERASE_SECTORS) ||
        (pEraseInit->Banks != FLASH_BANK_1) ||
        (pEraseInit->Sector > FLASH_SECTOR_7) ||
        (pEraseInit->NbSectors == 0U) ||
        (pEraseInit->NbSectors > (8U - pEraseInit->Sector)) ||
        (pEraseInit->VoltageRange > FLASH_VOLTAGE_RANGE_4))
    {
        return 0;
    }

    last_sector = pEraseInit->Sector + pEraseInit->NbSectors - 1U;
    *start = GD32_HAL_FLASH_SectorStartInternal(pEraseInit->Sector);
    *end = GD32_HAL_FLASH_SectorEndInternal(last_sector);
    return (*start != 0U) && (*end > *start) &&
           GD32_HAL_FLASH_IsRangeWritableInternal(*start, *end - *start);
}

HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit,
                                    uint32_t *SectorError)
{
    uint32_t start;
    uint32_t end;
    uint32_t sector;

    if (SectorError != NULL) *SectorError = 0xFFFFFFFFUL;
    if ((pEraseInit != NULL) &&
        (pEraseInit->TypeErase == FLASH_TYPEERASE_MASSERASE))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_FLASH_MASS_ERASE_UNSUPPORTED, 0U);
        return HAL_ERROR;
    }
    if ((SectorError == NULL) ||
        (GD32_HAL_FLASH_ValidateSectorErase(pEraseInit, &start, &end) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_FLASH_INVALID_RANGE,
                           (pEraseInit != NULL) ? pEraseInit->Sector : 0U);
        return HAL_ERROR;
    }

    __HAL_LOCK(&pFlash);
    pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;
    pFlash.ProcedureOnGoing = FLASH_PROC_SECTERASE;
    for (sector = pEraseInit->Sector;
         sector < (pEraseInit->Sector + pEraseInit->NbSectors);
         ++sector)
    {
        uint32_t address;
        const uint32_t sector_start = GD32_HAL_FLASH_SectorStartInternal(sector);
        const uint32_t sector_end = GD32_HAL_FLASH_SectorEndInternal(sector);
        for (address = sector_start; address < sector_end;
             address += GD32_FLASH_PAGE_SIZE)
        {
            GD32_HAL_FLASHStatus status = GD32_HAL_FLASH_ErasePage(address);
            if (status != GD32_HAL_FLASH_READY)
            {
                *SectorError = sector;
                GD32_HAL_FLASH_SetErrorInternal(status);
                pFlash.ProcedureOnGoing = FLASH_PROC_NONE;
                __HAL_UNLOCK(&pFlash);
                return HAL_ERROR;
            }
        }
    }
    UNUSED(start);
    UNUSED(end);
    pFlash.ProcedureOnGoing = FLASH_PROC_NONE;
    __HAL_UNLOCK(&pFlash);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASHEx_Erase_IT(FLASH_EraseInitTypeDef *pEraseInit)
{
    uint32_t start;
    uint32_t end;

    if ((pEraseInit != NULL) &&
        (pEraseInit->TypeErase == FLASH_TYPEERASE_MASSERASE))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_FLASH_MASS_ERASE_UNSUPPORTED, 0U);
        return HAL_ERROR;
    }
    if (GD32_HAL_FLASH_ValidateSectorErase(pEraseInit, &start, &end) == 0)
    {
        return HAL_ERROR;
    }
    return GD32_HAL_FLASH_BeginEraseITInternal(
        FLASH_PROC_SECTERASE,
        start,
        (end - start) / GD32_FLASH_PAGE_SIZE,
        pEraseInit->Sector,
        pEraseInit->NbSectors,
        GD32_HAL_FLASH_SectorEndInternal(pEraseInit->Sector),
        pEraseInit->VoltageRange);
}
