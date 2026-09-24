#include "gd32_hal_port.h"
#include "gd32f403_fmc.h"

static GD32_HAL_FLASHStatus GD32_HAL_FLASH_MapStatus(fmc_state_enum status)
{
    switch (status)
    {
        case FMC_READY: return GD32_HAL_FLASH_READY;
        case FMC_BUSY: return GD32_HAL_FLASH_BUSY;
        case FMC_PGERR: return GD32_HAL_FLASH_PROGRAM_ERROR;
        case FMC_WPERR: return GD32_HAL_FLASH_WRITE_PROTECT_ERROR;
        case FMC_TOERR:
        default:
            return GD32_HAL_FLASH_TIMEOUT;
    }
}

static void GD32_HAL_FLASH_PrepareIT(void)
{
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_interrupt_enable(FMC_INT_BANK0_END);
    fmc_interrupt_enable(FMC_INT_BANK0_ERR);
}

void GD32_HAL_FLASH_Unlock(void)
{
    fmc_unlock();
}

void GD32_HAL_FLASH_Lock(void)
{
    fmc_lock();
}

GD32_HAL_FLASHStatus GD32_HAL_FLASH_ProgramHalfWord(uint32_t address,
                                                    uint16_t data)
{
    return GD32_HAL_FLASH_MapStatus(fmc_halfword_program(address, data));
}

GD32_HAL_FLASHStatus GD32_HAL_FLASH_ProgramWord(uint32_t address,
                                                uint32_t data)
{
    return GD32_HAL_FLASH_MapStatus(fmc_word_program(address, data));
}

GD32_HAL_FLASHStatus GD32_HAL_FLASH_ErasePage(uint32_t page_address)
{
    return GD32_HAL_FLASH_MapStatus(fmc_page_erase(page_address));
}

GD32_HAL_FLASHStatus GD32_HAL_FLASH_StartProgramIT(uint32_t address,
                                                   uint32_t data,
                                                   uint32_t width)
{
    if ((FMC_CTL0 & FMC_CTL0_LK) != 0U)
    {
        return GD32_HAL_FLASH_WRITE_PROTECT_ERROR;
    }
    if (fmc_flag_get(FMC_FLAG_BANK0_BUSY) != RESET)
    {
        return GD32_HAL_FLASH_BUSY;
    }
    if ((width != 2U) && (width != 4U))
    {
        return GD32_HAL_FLASH_PROGRAM_ERROR;
    }

    GD32_HAL_FLASH_PrepareIT();
    FMC_CTL0 |= FMC_CTL0_PG;
    if (width == 2U)
    {
        REG16(address) = (uint16_t)data;
    }
    else
    {
        REG32(address) = data;
    }
    return GD32_HAL_FLASH_READY;
}

GD32_HAL_FLASHStatus GD32_HAL_FLASH_StartErasePageIT(uint32_t page_address)
{
    if ((FMC_CTL0 & FMC_CTL0_LK) != 0U)
    {
        return GD32_HAL_FLASH_WRITE_PROTECT_ERROR;
    }
    if (fmc_flag_get(FMC_FLAG_BANK0_BUSY) != RESET)
    {
        return GD32_HAL_FLASH_BUSY;
    }

    GD32_HAL_FLASH_PrepareIT();
    FMC_CTL0 |= FMC_CTL0_PER;
    FMC_ADDR0 = page_address;
    FMC_CTL0 |= FMC_CTL0_START;
    __NOP();
    __NOP();
    return GD32_HAL_FLASH_READY;
}

uint32_t GD32_HAL_FLASH_GetInterrupts(void)
{
    uint32_t interrupts = GD32_HAL_FLASH_IT_NONE;

    if (fmc_interrupt_flag_get(FMC_INT_FLAG_BANK0_END) != RESET)
    {
        interrupts |= GD32_HAL_FLASH_IT_END;
    }
    if (fmc_interrupt_flag_get(FMC_INT_FLAG_BANK0_PGERR) != RESET)
    {
        interrupts |= GD32_HAL_FLASH_IT_PROGRAM_ERROR;
    }
    if (fmc_interrupt_flag_get(FMC_INT_FLAG_BANK0_WPERR) != RESET)
    {
        interrupts |= GD32_HAL_FLASH_IT_WRITE_PROTECT_ERROR;
    }
    return interrupts;
}

void GD32_HAL_FLASH_ClearInterrupts(uint32_t interrupts)
{
    if ((interrupts & GD32_HAL_FLASH_IT_END) != 0U)
    {
        fmc_interrupt_flag_clear(FMC_INT_FLAG_BANK0_END);
    }
    if ((interrupts & GD32_HAL_FLASH_IT_PROGRAM_ERROR) != 0U)
    {
        fmc_interrupt_flag_clear(FMC_INT_FLAG_BANK0_PGERR);
    }
    if ((interrupts & GD32_HAL_FLASH_IT_WRITE_PROTECT_ERROR) != 0U)
    {
        fmc_interrupt_flag_clear(FMC_INT_FLAG_BANK0_WPERR);
    }
}

void GD32_HAL_FLASH_EndITOperation(void)
{
    FMC_CTL0 &= ~(FMC_CTL0_PG | FMC_CTL0_PER);
    fmc_interrupt_disable(FMC_INT_BANK0_END);
    fmc_interrupt_disable(FMC_INT_BANK0_ERR);
}
