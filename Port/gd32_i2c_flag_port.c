#include "gd32_hal_port.h"
#include "gd32f403_i2c.h"

uint32_t GD32_HAL_I2C_GetFlags(uint32_t i2c_address)
{
    const uint32_t stat0 = I2C_STAT0(i2c_address);
    uint32_t flags = 0U;
    if ((stat0 & I2C_STAT0_SBSEND) != 0U)
    {
        flags |= GD32_HAL_I2C_FLAG_START;
    }
    if ((stat0 & I2C_STAT0_ADDSEND) != 0U)
    {
        /*
         * STAT0 followed by STAT1 clears ADDSEND in hardware.  Keep flag
         * observation side-effect-free and report the bus as busy without
         * touching STAT1 until ClearAddress performs the explicit sequence.
         */
        flags |= GD32_HAL_I2C_FLAG_ADDRESS | GD32_HAL_I2C_FLAG_BUSY;
    }
    if ((stat0 & I2C_STAT0_BTC) != 0U)
    {
        flags |= GD32_HAL_I2C_FLAG_BYTE_TRANSFER;
    }
    if ((stat0 & I2C_STAT0_ADD10SEND) != 0U)
    {
        flags |= GD32_HAL_I2C_FLAG_ADDRESS10;
    }
    if ((stat0 & I2C_STAT0_RBNE) != 0U)
    {
        flags |= GD32_HAL_I2C_FLAG_RX_NOT_EMPTY;
    }
    if ((stat0 & I2C_STAT0_TBE) != 0U)
    {
        flags |= GD32_HAL_I2C_FLAG_TX_EMPTY;
    }
    if (((stat0 & I2C_STAT0_ADDSEND) == 0U) &&
        ((I2C_STAT1(i2c_address) & I2C_STAT1_I2CBSY) != 0U))
    {
        flags |= GD32_HAL_I2C_FLAG_BUSY;
    }
    return flags;
}

int GD32_HAL_I2C_IsBusBusy(uint32_t i2c_address)
{
    const uint32_t stat0 = I2C_STAT0(i2c_address);
    if ((stat0 & I2C_STAT0_ADDSEND) != 0U)
    {
        return 1;
    }
    return ((I2C_STAT1(i2c_address) & I2C_STAT1_I2CBSY) != 0U) ? 1 : 0;
}

void GD32_HAL_I2C_ClearAddress(uint32_t i2c_address)
{
    i2c_flag_clear(i2c_address, I2C_FLAG_ADDSEND);
}
