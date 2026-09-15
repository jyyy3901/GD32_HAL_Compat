#include "gd32_hal_port.h"
#include "gd32f403_i2c.h"
#include <stddef.h>

int GD32_HAL_I2C_IsInstance(uint32_t i2c_address)
{
    return (i2c_address == GD32_HAL_I2C0_ADDRESS) ||
           (i2c_address == GD32_HAL_I2C1_ADDRESS);
}

int GD32_HAL_I2C_Configure(uint32_t i2c_address,
                           const GD32_HAL_I2CConfig *config)
{
    uint32_t duty;
    uint32_t address_format;

    if ((GD32_HAL_I2C_IsInstance(i2c_address) == 0) || (config == NULL) ||
        (config->clock_speed == 0U) || (config->clock_speed > 400000U) ||
        (config->own_address1 > 0x03FFU) ||
        (config->own_address2 > 0x00FEU))
    {
        return -1;
    }

    duty = (config->duty_16_9 != 0U) ? I2C_DTCY_16_9 : I2C_DTCY_2;
    address_format = (config->address_10bit != 0U) ?
                     I2C_ADDFORMAT_10BITS : I2C_ADDFORMAT_7BITS;

    i2c_deinit(i2c_address);
    i2c_clock_config(i2c_address, config->clock_speed, duty);
    i2c_mode_addr_config(i2c_address, I2C_I2CMODE_ENABLE,
                         address_format, config->own_address1);
    if (config->dual_address != 0U)
    {
        i2c_dualaddr_enable(i2c_address, config->own_address2);
    }
    else
    {
        i2c_dualaddr_disable(i2c_address);
    }
    i2c_slave_response_to_gcall_config(
        i2c_address,
        (config->general_call != 0U) ? I2C_GCEN_ENABLE : I2C_GCEN_DISABLE);
    i2c_stretch_scl_low_config(
        i2c_address,
        (config->no_stretch != 0U) ?
        I2C_SCLSTRETCH_DISABLE : I2C_SCLSTRETCH_ENABLE);
    i2c_ackpos_config(i2c_address, I2C_ACKPOS_CURRENT);
    i2c_ack_config(i2c_address, I2C_ACK_ENABLE);
    i2c_enable(i2c_address);
    return 0;
}

void GD32_HAL_I2C_DeInit(uint32_t i2c_address)
{
    i2c_deinit(i2c_address);
}

void GD32_HAL_I2C_Enable(uint32_t i2c_address)
{
    i2c_enable(i2c_address);
}

void GD32_HAL_I2C_Disable(uint32_t i2c_address)
{
    i2c_disable(i2c_address);
}

void GD32_HAL_I2C_Start(uint32_t i2c_address)
{
    i2c_start_on_bus(i2c_address);
}

void GD32_HAL_I2C_Stop(uint32_t i2c_address)
{
    i2c_stop_on_bus(i2c_address);
}

void GD32_HAL_I2C_SetAck(uint32_t i2c_address, int enable)
{
    i2c_ack_config(i2c_address,
                   (enable != 0) ? I2C_ACK_ENABLE : I2C_ACK_DISABLE);
}

void GD32_HAL_I2C_SetAckNext(uint32_t i2c_address, int next)
{
    i2c_ackpos_config(i2c_address,
                      (next != 0) ? I2C_ACKPOS_NEXT : I2C_ACKPOS_CURRENT);
}

void GD32_HAL_I2C_SendAddress(uint32_t i2c_address,
                              uint16_t address,
                              int receive)
{
    i2c_master_addressing(i2c_address, address,
                          (receive != 0) ? I2C_RECEIVER : I2C_TRANSMITTER);
}

void GD32_HAL_I2C_WriteData(uint32_t i2c_address, uint8_t data)
{
    i2c_data_transmit(i2c_address, data);
}

uint8_t GD32_HAL_I2C_ReadData(uint32_t i2c_address)
{
    return i2c_data_receive(i2c_address);
}

uint32_t GD32_HAL_I2C_GetErrors(uint32_t i2c_address)
{
    uint32_t errors = 0U;
    if (i2c_flag_get(i2c_address, I2C_FLAG_BERR) == SET)
    {
        errors |= GD32_HAL_I2C_ERROR_BUS;
    }
    if (i2c_flag_get(i2c_address, I2C_FLAG_LOSTARB) == SET)
    {
        errors |= GD32_HAL_I2C_ERROR_ARBITRATION;
    }
    if (i2c_flag_get(i2c_address, I2C_FLAG_AERR) == SET)
    {
        errors |= GD32_HAL_I2C_ERROR_ACK;
    }
    if (i2c_flag_get(i2c_address, I2C_FLAG_OUERR) == SET)
    {
        errors |= GD32_HAL_I2C_ERROR_OVERRUN;
    }
    if (i2c_flag_get(i2c_address, I2C_FLAG_PECERR) == SET)
    {
        errors |= GD32_HAL_I2C_ERROR_PEC;
    }
    if (i2c_flag_get(i2c_address, I2C_FLAG_SMBTO) == SET)
    {
        errors |= GD32_HAL_I2C_ERROR_TIMEOUT;
    }
    return errors;
}

void GD32_HAL_I2C_ClearErrors(uint32_t i2c_address, uint32_t errors)
{
    if ((errors & GD32_HAL_I2C_ERROR_BUS) != 0U)
    {
        i2c_flag_clear(i2c_address, I2C_FLAG_BERR);
    }
    if ((errors & GD32_HAL_I2C_ERROR_ARBITRATION) != 0U)
    {
        i2c_flag_clear(i2c_address, I2C_FLAG_LOSTARB);
    }
    if ((errors & GD32_HAL_I2C_ERROR_ACK) != 0U)
    {
        i2c_flag_clear(i2c_address, I2C_FLAG_AERR);
    }
    if ((errors & GD32_HAL_I2C_ERROR_OVERRUN) != 0U)
    {
        i2c_flag_clear(i2c_address, I2C_FLAG_OUERR);
    }
    if ((errors & GD32_HAL_I2C_ERROR_PEC) != 0U)
    {
        i2c_flag_clear(i2c_address, I2C_FLAG_PECERR);
    }
    if ((errors & GD32_HAL_I2C_ERROR_TIMEOUT) != 0U)
    {
        i2c_flag_clear(i2c_address, I2C_FLAG_SMBTO);
    }
}

void GD32_HAL_I2C_SetInterrupts(uint32_t i2c_address,
                                uint32_t interrupts,
                                int enable)
{
    if ((interrupts & GD32_HAL_I2C_INTERRUPT_ERROR) != 0U)
    {
        (enable != 0) ? i2c_interrupt_enable(i2c_address, I2C_INT_ERR) :
                        i2c_interrupt_disable(i2c_address, I2C_INT_ERR);
    }
    if ((interrupts & GD32_HAL_I2C_INTERRUPT_EVENT) != 0U)
    {
        (enable != 0) ? i2c_interrupt_enable(i2c_address, I2C_INT_EV) :
                        i2c_interrupt_disable(i2c_address, I2C_INT_EV);
    }
    if ((interrupts & GD32_HAL_I2C_INTERRUPT_BUFFER) != 0U)
    {
        (enable != 0) ? i2c_interrupt_enable(i2c_address, I2C_INT_BUF) :
                        i2c_interrupt_disable(i2c_address, I2C_INT_BUF);
    }
}

uint32_t GD32_HAL_I2C_GetInterrupts(uint32_t i2c_address)
{
    uint32_t interrupts = 0U;
    if ((I2C_CTL1(i2c_address) & I2C_CTL1_ERRIE) != 0U)
    {
        interrupts |= GD32_HAL_I2C_INTERRUPT_ERROR;
    }
    if ((I2C_CTL1(i2c_address) & I2C_CTL1_EVIE) != 0U)
    {
        interrupts |= GD32_HAL_I2C_INTERRUPT_EVENT;
    }
    if ((I2C_CTL1(i2c_address) & I2C_CTL1_BUFIE) != 0U)
    {
        interrupts |= GD32_HAL_I2C_INTERRUPT_BUFFER;
    }
    return interrupts;
}

void GD32_HAL_I2C_SoftwareReset(uint32_t i2c_address)
{
    i2c_software_reset_config(i2c_address, I2C_SRESET_SET);
    i2c_software_reset_config(i2c_address, I2C_SRESET_RESET);
}
