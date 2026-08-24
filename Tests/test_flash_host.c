#include "stm32f4xx_hal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static GD32_HAL_FLASHStatus mock_status = GD32_HAL_FLASH_READY;
static uint32_t program_address;
static uint32_t program_data;
static uint32_t program_width;
static uint32_t erase_addresses[256];
static uint32_t erase_count;
static uint32_t pending_interrupts;
static uint32_t end_callback_values[256];
static uint32_t end_callback_count;
static uint32_t error_callback_value;
static uint32_t error_callback_count;
static uint32_t unlock_calls;
static uint32_t lock_calls;
static uint32_t last_port_error;

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    (void)detail;
    last_port_error = (uint32_t)error;
}

void GD32_HAL_FLASH_Unlock(void) { ++unlock_calls; }
void GD32_HAL_FLASH_Lock(void) { ++lock_calls; }
GD32_HAL_FLASHStatus GD32_HAL_FLASH_ProgramHalfWord(uint32_t address,
                                                    uint16_t data)
{
    program_address = address;
    program_data = data;
    program_width = 2U;
    return mock_status;
}
GD32_HAL_FLASHStatus GD32_HAL_FLASH_ProgramWord(uint32_t address,
                                                uint32_t data)
{
    program_address = address;
    program_data = data;
    program_width = 4U;
    return mock_status;
}
GD32_HAL_FLASHStatus GD32_HAL_FLASH_ErasePage(uint32_t page_address)
{
    erase_addresses[erase_count++] = page_address;
    return mock_status;
}
GD32_HAL_FLASHStatus GD32_HAL_FLASH_StartProgramIT(uint32_t address,
                                                   uint32_t data,
                                                   uint32_t width)
{
    program_address = address;
    program_data = data;
    program_width = width;
    return mock_status;
}
GD32_HAL_FLASHStatus GD32_HAL_FLASH_StartErasePageIT(uint32_t page_address)
{
    erase_addresses[erase_count++] = page_address;
    return mock_status;
}
uint32_t GD32_HAL_FLASH_GetInterrupts(void) { return pending_interrupts; }
void GD32_HAL_FLASH_ClearInterrupts(uint32_t interrupts)
{
    pending_interrupts &= ~interrupts;
}
void GD32_HAL_FLASH_EndITOperation(void) {}

void HAL_FLASH_EndOfOperationCallback(uint32_t value)
{
    end_callback_values[end_callback_count++] = value;
}

void HAL_FLASH_OperationErrorCallback(uint32_t value)
{
    error_callback_value = value;
    ++error_callback_count;
}

static void reset_records(void)
{
    program_address = 0U;
    program_data = 0U;
    program_width = 0U;
    erase_count = 0U;
    pending_interrupts = 0U;
    end_callback_count = 0U;
    error_callback_count = 0U;
    error_callback_value = 0U;
    last_port_error = 0U;
    memset(erase_addresses, 0, sizeof(erase_addresses));
    memset(end_callback_values, 0, sizeof(end_callback_values));
}

int main(void)
{
    GD32_FLASH_PageEraseInitTypeDef pages;
    FLASH_EraseInitTypeDef erase;
    uint32_t error;
    uint32_t index;

    reset_records();
    assert(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                             0x08060000U,
                             0x12345678U) == HAL_ERROR);
    assert(last_port_error == GD32_HAL_PORT_ERROR_FLASH_WRITABLE_REGION_REQUIRED);
    assert(GD32_HAL_FLASH_SetWritableRegion(0x08000000U,
                                            GD32_FLASH_PAGE_SIZE) == HAL_ERROR);
    assert(GD32_HAL_FLASH_SetWritableRegion(0x08060000U, 0x00020000U) == HAL_OK);

    assert(HAL_FLASH_Unlock() == HAL_OK);
    assert(unlock_calls == 1U);
    assert(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                             0x08060000U,
                             0x12345678U) == HAL_OK);
    assert(program_address == 0x08060000U);
    assert(program_data == 0x12345678U);
    assert(program_width == 4U);
    assert(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                             0x08060001U,
                             0x55AAU) == HAL_ERROR);
    assert(HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,
                             0x08060000U,
                             0x5AU) == HAL_ERROR);
    assert(last_port_error == GD32_HAL_PORT_ERROR_FLASH_PROGRAM_TYPE_UNSUPPORTED);

    pages.PageAddress = 0x08060000U;
    pages.NbPages = 2U;
    erase_count = 0U;
    assert(GD32_HAL_FLASHEx_ErasePages(&pages, &error) == HAL_OK);
    assert(error == 0xFFFFFFFFU);
    assert(erase_count == 2U);
    assert(erase_addresses[0] == 0x08060000U);
    assert(erase_addresses[1] == 0x08060800U);

    memset(&erase, 0, sizeof(erase));
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.Banks = FLASH_BANK_1;
    erase.Sector = FLASH_SECTOR_7;
    erase.NbSectors = 1U;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase_count = 0U;
    assert(HAL_FLASHEx_Erase(&erase, &error) == HAL_OK);
    assert(erase_count == 64U);
    assert(erase_addresses[0] == 0x08060000U);
    assert(erase_addresses[63] == 0x0807F800U);

    erase.TypeErase = FLASH_TYPEERASE_MASSERASE;
    assert(HAL_FLASHEx_Erase(&erase, &error) == HAL_ERROR);
    assert(last_port_error == GD32_HAL_PORT_ERROR_FLASH_MASS_ERASE_UNSUPPORTED);
    erase.TypeErase = FLASH_TYPEERASE_SECTORS;

    reset_records();
    assert(HAL_FLASH_Program_IT(FLASH_TYPEPROGRAM_WORD,
                                0x08060000U,
                                0xA5A5A5A5U) == HAL_OK);
    pending_interrupts = GD32_HAL_FLASH_IT_END;
    HAL_FLASH_IRQHandler();
    assert(end_callback_count == 1U);
    assert(end_callback_values[0] == 0x08060000U);
    assert(pFlash.ProcedureOnGoing == FLASH_PROC_NONE);

    reset_records();
    assert(HAL_FLASHEx_Erase_IT(&erase) == HAL_OK);
    for (index = 0U; index < 64U; ++index)
    {
        pending_interrupts = GD32_HAL_FLASH_IT_END;
        HAL_FLASH_IRQHandler();
    }
    assert(erase_count == 64U);
    assert(end_callback_count == 1U);
    assert(end_callback_values[0] == FLASH_SECTOR_7);
    assert(pFlash.ProcedureOnGoing == FLASH_PROC_NONE);

    reset_records();
    assert(HAL_FLASH_Program_IT(FLASH_TYPEPROGRAM_HALFWORD,
                                0x08060000U,
                                0xAA55U) == HAL_OK);
    pending_interrupts = GD32_HAL_FLASH_IT_WRITE_PROTECT_ERROR;
    HAL_FLASH_IRQHandler();
    assert(error_callback_count == 1U);
    assert(error_callback_value == 0x08060000U);
    assert((HAL_FLASH_GetError() & HAL_FLASH_ERROR_WRP) != 0U);

    assert(HAL_FLASH_Lock() == HAL_OK);
    assert(lock_calls == 1U);
    puts("FLASH host tests: PASS");
    return 0;
}
