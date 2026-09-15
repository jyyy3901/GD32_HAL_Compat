#include "gd32_hal_port.h"
#include "gd32f403_i2c.h"
#include <assert.h>
#include <stdio.h>

static uint32_t mock_stat0;
static uint32_t mock_stat1;
static uint32_t stat0_reads;
static uint32_t stat1_reads;
static int address_snapshot;

uint32_t GD32_Test_I2C_ReadSTAT0(uint32_t i2c_periph)
{
    assert(i2c_periph == GD32_HAL_I2C0_ADDRESS);
    ++stat0_reads;
    address_snapshot = ((mock_stat0 & I2C_STAT0_ADDSEND) != 0U) ? 1 : 0;
    return mock_stat0;
}

uint32_t GD32_Test_I2C_ReadSTAT1(uint32_t i2c_periph)
{
    assert(i2c_periph == GD32_HAL_I2C0_ADDRESS);
    ++stat1_reads;
    if (address_snapshot != 0)
    {
        mock_stat0 &= ~I2C_STAT0_ADDSEND;
    }
    address_snapshot = 0;
    return mock_stat1;
}

void i2c_flag_clear(uint32_t i2c_periph, i2c_flag_enum flag)
{
    assert(flag == I2C_FLAG_ADDSEND);
    (void)GD32_Test_I2C_ReadSTAT0(i2c_periph);
    (void)GD32_Test_I2C_ReadSTAT1(i2c_periph);
}

int main(void)
{
    uint32_t flags;

    mock_stat0 = I2C_STAT0_SBSEND | I2C_STAT0_ADDSEND |
                 I2C_STAT0_BTC | I2C_STAT0_ADD10SEND |
                 I2C_STAT0_RBNE | I2C_STAT0_TBE;
    mock_stat1 = I2C_STAT1_I2CBSY;

    flags = GD32_HAL_I2C_GetFlags(GD32_HAL_I2C0_ADDRESS);
    assert((flags & GD32_HAL_I2C_FLAG_ADDRESS) != 0U);
    assert((flags & GD32_HAL_I2C_FLAG_BUSY) != 0U);
    assert((flags & GD32_HAL_I2C_FLAG_START) != 0U);
    assert((flags & GD32_HAL_I2C_FLAG_BYTE_TRANSFER) != 0U);
    assert((flags & GD32_HAL_I2C_FLAG_ADDRESS10) != 0U);
    assert((flags & GD32_HAL_I2C_FLAG_RX_NOT_EMPTY) != 0U);
    assert((flags & GD32_HAL_I2C_FLAG_TX_EMPTY) != 0U);
    assert(stat0_reads == 1U);
    assert(stat1_reads == 0U);
    assert((mock_stat0 & I2C_STAT0_ADDSEND) != 0U);

    assert(GD32_HAL_I2C_IsBusBusy(GD32_HAL_I2C0_ADDRESS) != 0);
    assert(stat0_reads == 2U);
    assert(stat1_reads == 0U);
    assert((mock_stat0 & I2C_STAT0_ADDSEND) != 0U);

    GD32_HAL_I2C_ClearAddress(GD32_HAL_I2C0_ADDRESS);
    assert(stat0_reads == 3U);
    assert(stat1_reads == 1U);
    assert((mock_stat0 & I2C_STAT0_ADDSEND) == 0U);

    flags = GD32_HAL_I2C_GetFlags(GD32_HAL_I2C0_ADDRESS);
    assert((flags & GD32_HAL_I2C_FLAG_ADDRESS) == 0U);
    assert((flags & GD32_HAL_I2C_FLAG_BUSY) != 0U);
    assert(stat0_reads == 4U);
    assert(stat1_reads == 2U);

    mock_stat1 = 0U;
    assert(GD32_HAL_I2C_IsBusBusy(GD32_HAL_I2C0_ADDRESS) == 0);
    assert(stat0_reads == 5U);
    assert(stat1_reads == 3U);

    puts("I2C side-effect-safe Port flag tests: PASS");
    return 0;
}
