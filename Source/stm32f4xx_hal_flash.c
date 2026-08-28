#include "stm32f4xx_hal_flash_internal.h"

FLASH_ProcessTypeDef pFlash = {
    FLASH_PROC_NONE,
    0U,
    0U,
    0U,
    0U,
    0U,
    HAL_UNLOCKED,
    HAL_FLASH_ERROR_NONE,
    0U,
    0U,
    0U
};

static uint32_t gd32FlashWritableStart;
static uint32_t gd32FlashWritableEnd;

static uint32_t GD32_HAL_FLASH_ProgramWidth(uint32_t type)
{
    if (type == FLASH_TYPEPROGRAM_HALFWORD) return 2U;
    if (type == FLASH_TYPEPROGRAM_WORD) return 4U;
    return 0U;
}

int GD32_HAL_FLASH_IsRangeWritableInternal(uint32_t address, uint32_t length)
{
    uint32_t end;

    if ((length == 0U) || (gd32FlashWritableStart == 0U) ||
        (address < GD32_FLASH_BASE_ADDRESS) ||
        (address < gd32FlashWritableStart) ||
        (address > UINT32_MAX - length))
    {
        return 0;
    }
    end = address + length;
    return (end <= gd32FlashWritableEnd) && (end <= GD32_FLASH_END_ADDRESS);
}

void GD32_HAL_FLASH_SetErrorInternal(GD32_HAL_FLASHStatus status)
{
    switch (status)
    {
        case GD32_HAL_FLASH_PROGRAM_ERROR:
            pFlash.ErrorCode |= HAL_FLASH_ERROR_PGS;
            break;
        case GD32_HAL_FLASH_WRITE_PROTECT_ERROR:
            pFlash.ErrorCode |= HAL_FLASH_ERROR_WRP;
            break;
        case GD32_HAL_FLASH_BUSY:
        case GD32_HAL_FLASH_TIMEOUT:
            pFlash.ErrorCode |= HAL_FLASH_ERROR_OPERATION;
            break;
        case GD32_HAL_FLASH_READY:
        default:
            break;
    }
}

uint32_t GD32_HAL_FLASH_SectorStartInternal(uint32_t sector)
{
    static const uint32_t starts[8] = {
        0x08000000UL, 0x08004000UL, 0x08008000UL, 0x0800C000UL,
        0x08010000UL, 0x08020000UL, 0x08040000UL, 0x08060000UL
    };
    return (sector < 8U) ? starts[sector] : 0U;
}

uint32_t GD32_HAL_FLASH_SectorEndInternal(uint32_t sector)
{
    static const uint32_t ends[8] = {
        0x08004000UL, 0x08008000UL, 0x0800C000UL, 0x08010000UL,
        0x08020000UL, 0x08040000UL, 0x08060000UL, 0x08080000UL
    };
    return (sector < 8U) ? ends[sector] : 0U;
}

HAL_StatusTypeDef GD32_HAL_FLASH_SetWritableRegion(uint32_t StartAddress,
                                                   uint32_t Length)
{
    uint32_t end;
    uintptr_t flash_api_address;
    uint32_t flash_api_page;

    if ((StartAddress == 0U) && (Length == 0U))
    {
        gd32FlashWritableStart = 0U;
        gd32FlashWritableEnd = 0U;
        return HAL_OK;
    }
    if ((Length == 0U) || ((StartAddress & (GD32_FLASH_PAGE_SIZE - 1U)) != 0U) ||
        ((Length & (GD32_FLASH_PAGE_SIZE - 1U)) != 0U) ||
        (StartAddress < GD32_FLASH_BASE_ADDRESS) ||
        (StartAddress > UINT32_MAX - Length))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_FLASH_INVALID_RANGE, StartAddress);
        return HAL_ERROR;
    }
    end = StartAddress + Length;
    if (end > GD32_FLASH_END_ADDRESS)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_FLASH_INVALID_RANGE, end);
        return HAL_ERROR;
    }

    /* 向量表首页始终受保护；目标代码位于 Flash 时，同时保护本驱动所在页。 */
    if (StartAddress < (GD32_FLASH_BASE_ADDRESS + GD32_FLASH_PAGE_SIZE))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_FLASH_INVALID_RANGE, StartAddress);
        return HAL_ERROR;
    }
    flash_api_address = ((uintptr_t)&HAL_FLASH_Program) & ~(uintptr_t)1U;
    if ((flash_api_address >= GD32_FLASH_BASE_ADDRESS) &&
        (flash_api_address < GD32_FLASH_END_ADDRESS))
    {
        flash_api_page = (uint32_t)flash_api_address & ~(GD32_FLASH_PAGE_SIZE - 1U);
        if ((StartAddress < (flash_api_page + GD32_FLASH_PAGE_SIZE)) &&
            (end > flash_api_page))
        {
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_FLASH_INVALID_RANGE,
                               flash_api_page);
            return HAL_ERROR;
        }
    }

    gd32FlashWritableStart = StartAddress;
    gd32FlashWritableEnd = end;
    return HAL_OK;
}

void GD32_HAL_FLASH_GetWritableRegion(uint32_t *StartAddress,
                                      uint32_t *EndAddress)
{
    if (StartAddress != NULL) *StartAddress = gd32FlashWritableStart;
    if (EndAddress != NULL) *EndAddress = gd32FlashWritableEnd;
}

HAL_StatusTypeDef HAL_FLASH_Unlock(void)
{
    GD32_HAL_FLASH_Unlock();
    return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASH_Lock(void)
{
    GD32_HAL_FLASH_Lock();
    return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram,
                                    uint32_t Address,
                                    uint64_t Data)
{
    GD32_HAL_FLASHStatus status;
    const uint32_t width = GD32_HAL_FLASH_ProgramWidth(TypeProgram);

    if (width == 0U)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_FLASH_PROGRAM_TYPE_UNSUPPORTED,
                           TypeProgram);
        return HAL_ERROR;
    }
    if (((Address & (width - 1U)) != 0U) ||
        (GD32_HAL_FLASH_IsRangeWritableInternal(Address, width) == 0))
    {
        GD32_HAL_ErrorHook((gd32FlashWritableStart == 0U) ?
                           GD32_HAL_PORT_ERROR_FLASH_WRITABLE_REGION_REQUIRED :
                           GD32_HAL_PORT_ERROR_FLASH_INVALID_RANGE,
                           Address);
        return HAL_ERROR;
    }

    __HAL_LOCK(&pFlash);
    pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;
    pFlash.ProcedureOnGoing = FLASH_PROC_PROGRAM;
    pFlash.Address = Address;
    pFlash.ProgramWidth = width;
    status = (width == 2U) ?
             GD32_HAL_FLASH_ProgramHalfWord(Address, (uint16_t)Data) :
             GD32_HAL_FLASH_ProgramWord(Address, (uint32_t)Data);
    GD32_HAL_FLASH_SetErrorInternal(status);
    pFlash.ProcedureOnGoing = FLASH_PROC_NONE;
    __HAL_UNLOCK(&pFlash);
    return (status == GD32_HAL_FLASH_READY) ? HAL_OK : HAL_ERROR;
}

HAL_StatusTypeDef HAL_FLASH_Program_IT(uint32_t TypeProgram,
                                       uint32_t Address,
                                       uint64_t Data)
{
    GD32_HAL_FLASHStatus status;
    const uint32_t width = GD32_HAL_FLASH_ProgramWidth(TypeProgram);

    if ((width == 0U) || ((Address & (width - 1U)) != 0U) ||
        (GD32_HAL_FLASH_IsRangeWritableInternal(Address, width) == 0))
    {
        GD32_HAL_ErrorHook((width == 0U) ?
                           GD32_HAL_PORT_ERROR_FLASH_PROGRAM_TYPE_UNSUPPORTED :
                           GD32_HAL_PORT_ERROR_FLASH_INVALID_RANGE,
                           Address);
        return HAL_ERROR;
    }

    __HAL_LOCK(&pFlash);
    pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;
    pFlash.ProcedureOnGoing = FLASH_PROC_PROGRAM;
    pFlash.Address = Address;
    pFlash.ProgramWidth = width;
    status = GD32_HAL_FLASH_StartProgramIT(Address, (uint32_t)Data, width);
    if (status != GD32_HAL_FLASH_READY)
    {
        GD32_HAL_FLASH_SetErrorInternal(status);
        pFlash.ProcedureOnGoing = FLASH_PROC_NONE;
        __HAL_UNLOCK(&pFlash);
        return (status == GD32_HAL_FLASH_BUSY) ? HAL_BUSY : HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef GD32_HAL_FLASH_BeginEraseITInternal(
    FLASH_ProcedureTypeDef procedure,
    uint32_t address,
    uint32_t pages,
    uint32_t sector,
    uint32_t sectors,
    uint32_t sector_end,
    uint32_t voltage)
{
    GD32_HAL_FLASHStatus status;

    __HAL_LOCK(&pFlash);
    pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;
    pFlash.ProcedureOnGoing = procedure;
    pFlash.Address = address;
    pFlash.PagesRemaining = pages;
    pFlash.Sector = sector;
    pFlash.NbSectorsToErase = sectors;
    pFlash.SectorEndAddress = sector_end;
    pFlash.VoltageForErase = (uint8_t)voltage;
    status = GD32_HAL_FLASH_StartErasePageIT(address);
    if (status != GD32_HAL_FLASH_READY)
    {
        GD32_HAL_FLASH_SetErrorInternal(status);
        pFlash.ProcedureOnGoing = FLASH_PROC_NONE;
        __HAL_UNLOCK(&pFlash);
        return (status == GD32_HAL_FLASH_BUSY) ? HAL_BUSY : HAL_ERROR;
    }
    return HAL_OK;
}

static void GD32_HAL_FLASH_FinishIT(void)
{
    pFlash.ProcedureOnGoing = FLASH_PROC_NONE;
    pFlash.PagesRemaining = 0U;
    __HAL_UNLOCK(&pFlash);
}

void HAL_FLASH_IRQHandler(void)
{
    uint32_t interrupts = GD32_HAL_FLASH_GetInterrupts();
    uint32_t error_interrupts = interrupts &
        (GD32_HAL_FLASH_IT_PROGRAM_ERROR |
         GD32_HAL_FLASH_IT_WRITE_PROTECT_ERROR);

    if (error_interrupts != 0U)
    {
        uint32_t return_value = (pFlash.ProcedureOnGoing == FLASH_PROC_SECTERASE) ?
                                pFlash.Sector : pFlash.Address;
        if ((error_interrupts & GD32_HAL_FLASH_IT_PROGRAM_ERROR) != 0U)
            pFlash.ErrorCode |= HAL_FLASH_ERROR_PGS;
        if ((error_interrupts & GD32_HAL_FLASH_IT_WRITE_PROTECT_ERROR) != 0U)
            pFlash.ErrorCode |= HAL_FLASH_ERROR_WRP;
        GD32_HAL_FLASH_ClearInterrupts(interrupts);
        GD32_HAL_FLASH_EndITOperation();
        GD32_HAL_FLASH_FinishIT();
        HAL_FLASH_OperationErrorCallback(return_value);
        return;
    }

    if ((interrupts & GD32_HAL_FLASH_IT_END) != 0U)
    {
        FLASH_ProcedureTypeDef procedure = pFlash.ProcedureOnGoing;
        uint32_t completed_address = pFlash.Address;

        GD32_HAL_FLASH_ClearInterrupts(GD32_HAL_FLASH_IT_END);
        GD32_HAL_FLASH_EndITOperation();

        if (procedure == FLASH_PROC_PROGRAM)
        {
            GD32_HAL_FLASH_FinishIT();
            HAL_FLASH_EndOfOperationCallback(completed_address);
            return;
        }
        if ((procedure != FLASH_PROC_GD32_PAGEERASE) &&
            (procedure != FLASH_PROC_SECTERASE))
        {
            return;
        }

        if (pFlash.PagesRemaining > 0U) --pFlash.PagesRemaining;
        pFlash.Address += GD32_FLASH_PAGE_SIZE;
        if (procedure == FLASH_PROC_GD32_PAGEERASE)
        {
            HAL_FLASH_EndOfOperationCallback(completed_address);
        }
        else if (pFlash.Address >= pFlash.SectorEndAddress)
        {
            HAL_FLASH_EndOfOperationCallback(pFlash.Sector);
            if (pFlash.NbSectorsToErase > 0U) --pFlash.NbSectorsToErase;
            ++pFlash.Sector;
            pFlash.SectorEndAddress = GD32_HAL_FLASH_SectorEndInternal(pFlash.Sector);
        }

        if (pFlash.PagesRemaining == 0U)
        {
            GD32_HAL_FLASH_FinishIT();
            return;
        }
        if (GD32_HAL_FLASH_StartErasePageIT(pFlash.Address) != GD32_HAL_FLASH_READY)
        {
            uint32_t return_value = (procedure == FLASH_PROC_SECTERASE) ?
                                    pFlash.Sector : pFlash.Address;
            pFlash.ErrorCode |= HAL_FLASH_ERROR_OPERATION;
            GD32_HAL_FLASH_EndITOperation();
            GD32_HAL_FLASH_FinishIT();
            HAL_FLASH_OperationErrorCallback(return_value);
        }
    }
}

uint32_t HAL_FLASH_GetError(void)
{
    return pFlash.ErrorCode;
}
