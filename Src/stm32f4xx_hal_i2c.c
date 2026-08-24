#include "stm32f4xx_hal.h"

#define I2C_ALL_INTERRUPTS (GD32_HAL_I2C_INTERRUPT_ERROR | \
                            GD32_HAL_I2C_INTERRUPT_EVENT | \
                            GD32_HAL_I2C_INTERRUPT_BUFFER)

static uint32_t I2C_Address(const I2C_HandleTypeDef *hi2c)
{
    return (uint32_t)(uintptr_t)hi2c->Instance;
}

static uint32_t I2C_MapErrors(uint32_t errors)
{
    uint32_t mapped = HAL_I2C_ERROR_NONE;
    if ((errors & GD32_HAL_I2C_ERROR_BUS) != 0U)
    {
        mapped |= HAL_I2C_ERROR_BERR;
    }
    if ((errors & GD32_HAL_I2C_ERROR_ARBITRATION) != 0U)
    {
        mapped |= HAL_I2C_ERROR_ARLO;
    }
    if ((errors & GD32_HAL_I2C_ERROR_ACK) != 0U)
    {
        mapped |= HAL_I2C_ERROR_AF;
    }
    if ((errors & GD32_HAL_I2C_ERROR_OVERRUN) != 0U)
    {
        mapped |= HAL_I2C_ERROR_OVR;
    }
    if ((errors & GD32_HAL_I2C_ERROR_TIMEOUT) != 0U)
    {
        mapped |= HAL_I2C_ERROR_TIMEOUT;
    }
    return mapped;
}

static int I2C_FillConfig(const I2C_HandleTypeDef *hi2c,
                          GD32_HAL_I2CConfig *config)
{
    const I2C_InitTypeDef *init = &hi2c->Init;
    if ((init->ClockSpeed == 0U) || (init->ClockSpeed > 400000U) ||
        ((init->DutyCycle != I2C_DUTYCYCLE_2) &&
         (init->DutyCycle != I2C_DUTYCYCLE_16_9)) ||
        ((init->AddressingMode != I2C_ADDRESSINGMODE_7BIT) &&
         (init->AddressingMode != I2C_ADDRESSINGMODE_10BIT)) ||
        ((init->DualAddressMode != I2C_DUALADDRESS_DISABLE) &&
         (init->DualAddressMode != I2C_DUALADDRESS_ENABLE)) ||
        ((init->GeneralCallMode != I2C_GENERALCALL_DISABLE) &&
         (init->GeneralCallMode != I2C_GENERALCALL_ENABLE)) ||
        ((init->NoStretchMode != I2C_NOSTRETCH_DISABLE) &&
         (init->NoStretchMode != I2C_NOSTRETCH_ENABLE)) ||
        (init->OwnAddress1 > 0x03FFU) || (init->OwnAddress2 > 0x00FEU))
    {
        return 0;
    }
    config->clock_speed = init->ClockSpeed;
    config->own_address1 = (uint16_t)init->OwnAddress1;
    config->own_address2 = (uint16_t)init->OwnAddress2;
    config->duty_16_9 = (init->DutyCycle == I2C_DUTYCYCLE_16_9) ? 1U : 0U;
    config->address_10bit =
        (init->AddressingMode == I2C_ADDRESSINGMODE_10BIT) ? 1U : 0U;
    config->dual_address =
        (init->DualAddressMode == I2C_DUALADDRESS_ENABLE) ? 1U : 0U;
    config->general_call =
        (init->GeneralCallMode == I2C_GENERALCALL_ENABLE) ? 1U : 0U;
    config->no_stretch =
        (init->NoStretchMode == I2C_NOSTRETCH_ENABLE) ? 1U : 0U;
    return 1;
}

static void I2C_RestoreReceiveControls(I2C_HandleTypeDef *hi2c)
{
    GD32_HAL_I2C_SetAckNext(I2C_Address(hi2c), 0);
    GD32_HAL_I2C_SetAck(I2C_Address(hi2c), 1);
}

static void I2C_Recover(I2C_HandleTypeDef *hi2c)
{
    GD32_HAL_I2CConfig config;
    GD32_HAL_I2C_SetInterrupts(I2C_Address(hi2c), I2C_ALL_INTERRUPTS, 0);
    GD32_HAL_I2C_SoftwareReset(I2C_Address(hi2c));
    if (I2C_FillConfig(hi2c, &config) != 0)
    {
        (void)GD32_HAL_I2C_Configure(I2C_Address(hi2c), &config);
    }
    I2C_RestoreReceiveControls(hi2c);
}

static HAL_StatusTypeDef I2C_Fail(I2C_HandleTypeDef *hi2c,
                                  HAL_StatusTypeDef status,
                                  uint32_t error)
{
    hi2c->ErrorCode |= error;
    GD32_HAL_I2C_Stop(I2C_Address(hi2c));
    if ((status == HAL_TIMEOUT) ||
        ((error & (HAL_I2C_ERROR_BERR | HAL_I2C_ERROR_ARLO)) != 0U))
    {
        I2C_Recover(hi2c);
    }
    else
    {
        I2C_RestoreReceiveControls(hi2c);
    }
    hi2c->State = HAL_I2C_STATE_READY;
    hi2c->Mode = HAL_I2C_MODE_NONE;
    hi2c->EventCount = 0U;
    __HAL_UNLOCK(hi2c);
    return status;
}

static HAL_StatusTypeDef I2C_WaitFlag(I2C_HandleTypeDef *hi2c,
                                      uint32_t flag,
                                      uint32_t timeout,
                                      uint32_t tickstart)
{
    while ((GD32_HAL_I2C_GetFlags(I2C_Address(hi2c)) & flag) == 0U)
    {
        uint32_t errors = GD32_HAL_I2C_GetErrors(I2C_Address(hi2c));
        if (errors != 0U)
        {
            hi2c->ErrorCode |= I2C_MapErrors(errors);
            GD32_HAL_I2C_ClearErrors(I2C_Address(hi2c), errors);
            return HAL_ERROR;
        }
        if ((timeout != HAL_MAX_DELAY) &&
            ((timeout == 0U) || ((HAL_GetTick() - tickstart) >= timeout)))
        {
            hi2c->ErrorCode |= HAL_I2C_ERROR_TIMEOUT;
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_I2C_TIMEOUT, flag);
            return HAL_TIMEOUT;
        }
    }
    return HAL_OK;
}

static HAL_StatusTypeDef I2C_WaitBusFree(I2C_HandleTypeDef *hi2c,
                                         uint32_t timeout,
                                         uint32_t tickstart)
{
    while ((GD32_HAL_I2C_GetFlags(I2C_Address(hi2c)) &
            GD32_HAL_I2C_FLAG_BUSY) != 0U)
    {
        if ((timeout != HAL_MAX_DELAY) &&
            ((timeout == 0U) || ((HAL_GetTick() - tickstart) >= timeout)))
        {
            hi2c->ErrorCode |= HAL_I2C_ERROR_TIMEOUT;
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_I2C_TIMEOUT,
                               GD32_HAL_I2C_FLAG_BUSY);
            return HAL_TIMEOUT;
        }
    }
    return HAL_OK;
}

static uint8_t I2C_Header10(uint16_t address, int receive)
{
    return (uint8_t)(0xF0U | ((address >> 7U) & 0x06U) |
                     ((receive != 0) ? 1U : 0U));
}

static HAL_StatusTypeDef I2C_RequestAddress(I2C_HandleTypeDef *hi2c,
                                            uint16_t address,
                                            int receive,
                                            uint32_t timeout,
                                            uint32_t tickstart)
{
    HAL_StatusTypeDef status;
    GD32_HAL_I2C_Start(I2C_Address(hi2c));
    status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_START,
                          timeout, tickstart);
    if (status != HAL_OK)
    {
        return status;
    }

    if (hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
    {
        GD32_HAL_I2C_SendAddress(I2C_Address(hi2c),
                                 (uint16_t)(address & 0x00FEU), receive);
    }
    else
    {
        GD32_HAL_I2C_WriteData(I2C_Address(hi2c),
                               I2C_Header10(address, 0));
        status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_ADDRESS10,
                              timeout, tickstart);
        if (status != HAL_OK)
        {
            return status;
        }
        GD32_HAL_I2C_WriteData(I2C_Address(hi2c), (uint8_t)address);
        status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_ADDRESS,
                              timeout, tickstart);
        if (status != HAL_OK)
        {
            return status;
        }
        if (receive != 0)
        {
            GD32_HAL_I2C_ClearAddress(I2C_Address(hi2c));
            GD32_HAL_I2C_Start(I2C_Address(hi2c));
            status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_START,
                                  timeout, tickstart);
            if (status != HAL_OK)
            {
                return status;
            }
            GD32_HAL_I2C_WriteData(I2C_Address(hi2c),
                                   I2C_Header10(address, 1));
        }
    }
    return I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_ADDRESS,
                        timeout, tickstart);
}

static int I2C_ValidTransfer(const I2C_HandleTypeDef *hi2c,
                             uint16_t address,
                             const uint8_t *data,
                             uint16_t size)
{
    if ((hi2c == NULL) || (hi2c->Instance == NULL) || (data == NULL) ||
        (size == 0U) ||
        (GD32_HAL_I2C_IsInstance(I2C_Address(hi2c)) == 0))
    {
        return 0;
    }
    if (hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
    {
        return address <= 0x00FEU;
    }
    return address <= 0x03FFU;
}

static void I2C_Prepare(I2C_HandleTypeDef *hi2c,
                        HAL_I2C_StateTypeDef state,
                        HAL_I2C_ModeTypeDef mode,
                        uint16_t address,
                        uint8_t *data,
                        uint16_t size)
{
    hi2c->State = state;
    hi2c->Mode = mode;
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
    hi2c->Devaddress = address;
    hi2c->pBuffPtr = data;
    hi2c->XferSize = size;
    hi2c->XferCount = size;
    hi2c->XferOptions = I2C_NO_OPTION_FRAME;
    hi2c->EventCount = 0U;
}

static HAL_StatusTypeDef I2C_PollReceiveData(I2C_HandleTypeDef *hi2c,
                                             uint32_t timeout,
                                             uint32_t tickstart)
{
    HAL_StatusTypeDef status;
    uint32_t address = I2C_Address(hi2c);

    if (hi2c->XferCount == 1U)
    {
        GD32_HAL_I2C_SetAck(address, 0);
        GD32_HAL_I2C_ClearAddress(address);
        GD32_HAL_I2C_Stop(address);
    }
    else if (hi2c->XferCount == 2U)
    {
        GD32_HAL_I2C_SetAck(address, 0);
        GD32_HAL_I2C_SetAckNext(address, 1);
        GD32_HAL_I2C_ClearAddress(address);
    }
    else
    {
        GD32_HAL_I2C_SetAck(address, 1);
        GD32_HAL_I2C_ClearAddress(address);
    }

    while (hi2c->XferCount > 0U)
    {
        if (hi2c->XferCount == 1U)
        {
            status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_RX_NOT_EMPTY,
                                  timeout, tickstart);
            if (status != HAL_OK)
            {
                return status;
            }
            *hi2c->pBuffPtr++ = GD32_HAL_I2C_ReadData(address);
            --hi2c->XferCount;
        }
        else if (hi2c->XferCount == 2U)
        {
            status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_BYTE_TRANSFER,
                                  timeout, tickstart);
            if (status != HAL_OK)
            {
                return status;
            }
            GD32_HAL_I2C_Stop(address);
            *hi2c->pBuffPtr++ = GD32_HAL_I2C_ReadData(address);
            *hi2c->pBuffPtr++ = GD32_HAL_I2C_ReadData(address);
            hi2c->XferCount = 0U;
        }
        else if (hi2c->XferCount == 3U)
        {
            status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_BYTE_TRANSFER,
                                  timeout, tickstart);
            if (status != HAL_OK)
            {
                return status;
            }
            GD32_HAL_I2C_SetAck(address, 0);
            *hi2c->pBuffPtr++ = GD32_HAL_I2C_ReadData(address);
            --hi2c->XferCount;
        }
        else
        {
            status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_RX_NOT_EMPTY,
                                  timeout, tickstart);
            if (status != HAL_OK)
            {
                return status;
            }
            *hi2c->pBuffPtr++ = GD32_HAL_I2C_ReadData(address);
            --hi2c->XferCount;
        }
    }
    I2C_RestoreReceiveControls(hi2c);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c)
{
    GD32_HAL_I2CConfig config;
    if ((hi2c == NULL) || (hi2c->Instance == NULL) ||
        (GD32_HAL_I2C_IsInstance(I2C_Address(hi2c)) == 0) ||
        (I2C_FillConfig(hi2c, &config) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_I2C_INVALID_CONFIG,
                           (hi2c != NULL) ? I2C_Address(hi2c) : 0U);
        return HAL_ERROR;
    }
    if (hi2c->State == HAL_I2C_STATE_RESET)
    {
        hi2c->Lock = HAL_UNLOCKED;
        HAL_I2C_MspInit(hi2c);
    }
    __HAL_LOCK(hi2c);
    hi2c->State = HAL_I2C_STATE_BUSY;
    if (GD32_HAL_I2C_Configure(I2C_Address(hi2c), &config) != 0)
    {
        hi2c->State = HAL_I2C_STATE_ERROR;
        __HAL_UNLOCK(hi2c);
        return HAL_ERROR;
    }
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
    hi2c->Mode = HAL_I2C_MODE_NONE;
    hi2c->PreviousState = 0U;
    hi2c->EventCount = 0U;
    hi2c->State = HAL_I2C_STATE_READY;
    __HAL_UNLOCK(hi2c);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c)
{
    if ((hi2c == NULL) || (hi2c->Instance == NULL) ||
        (GD32_HAL_I2C_IsInstance(I2C_Address(hi2c)) == 0))
    {
        return HAL_ERROR;
    }
    __HAL_LOCK(hi2c);
    GD32_HAL_I2C_SetInterrupts(I2C_Address(hi2c), I2C_ALL_INTERRUPTS, 0);
    GD32_HAL_I2C_DeInit(I2C_Address(hi2c));
    HAL_I2C_MspDeInit(hi2c);
    hi2c->State = HAL_I2C_STATE_RESET;
    hi2c->Mode = HAL_I2C_MODE_NONE;
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
    __HAL_UNLOCK(hi2c);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c,
                                          uint16_t DevAddress,
                                          uint8_t *pData,
                                          uint16_t Size,
                                          uint32_t Timeout)
{
    HAL_StatusTypeDef status;
    uint32_t tickstart = HAL_GetTick();
    if (I2C_ValidTransfer(hi2c, DevAddress, pData, Size) == 0)
    {
        return HAL_ERROR;
    }
    if (hi2c->State != HAL_I2C_STATE_READY)
    {
        return HAL_BUSY;
    }
    if (I2C_WaitBusFree(hi2c, Timeout, tickstart) != HAL_OK)
    {
        I2C_Recover(hi2c);
        return HAL_BUSY;
    }
    __HAL_LOCK(hi2c);
    I2C_Prepare(hi2c, HAL_I2C_STATE_BUSY_TX, HAL_I2C_MODE_MASTER,
                DevAddress, pData, Size);
    status = I2C_RequestAddress(hi2c, DevAddress, 0, Timeout, tickstart);
    if (status != HAL_OK)
    {
        return I2C_Fail(hi2c, status, hi2c->ErrorCode);
    }
    GD32_HAL_I2C_ClearAddress(I2C_Address(hi2c));
    while (hi2c->XferCount > 0U)
    {
        status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_TX_EMPTY,
                              Timeout, tickstart);
        if (status != HAL_OK)
        {
            return I2C_Fail(hi2c, status, hi2c->ErrorCode);
        }
        GD32_HAL_I2C_WriteData(I2C_Address(hi2c), *hi2c->pBuffPtr++);
        --hi2c->XferCount;
    }
    status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_BYTE_TRANSFER,
                          Timeout, tickstart);
    if (status != HAL_OK)
    {
        return I2C_Fail(hi2c, status, hi2c->ErrorCode);
    }
    GD32_HAL_I2C_Stop(I2C_Address(hi2c));
    hi2c->State = HAL_I2C_STATE_READY;
    hi2c->Mode = HAL_I2C_MODE_NONE;
    __HAL_UNLOCK(hi2c);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c,
                                         uint16_t DevAddress,
                                         uint8_t *pData,
                                         uint16_t Size,
                                         uint32_t Timeout)
{
    HAL_StatusTypeDef status;
    uint32_t tickstart = HAL_GetTick();
    if (I2C_ValidTransfer(hi2c, DevAddress, pData, Size) == 0)
    {
        return HAL_ERROR;
    }
    if (hi2c->State != HAL_I2C_STATE_READY)
    {
        return HAL_BUSY;
    }
    if (I2C_WaitBusFree(hi2c, Timeout, tickstart) != HAL_OK)
    {
        I2C_Recover(hi2c);
        return HAL_BUSY;
    }
    __HAL_LOCK(hi2c);
    I2C_Prepare(hi2c, HAL_I2C_STATE_BUSY_RX, HAL_I2C_MODE_MASTER,
                DevAddress, pData, Size);
    status = I2C_RequestAddress(hi2c, DevAddress, 1, Timeout, tickstart);
    if (status == HAL_OK)
    {
        status = I2C_PollReceiveData(hi2c, Timeout, tickstart);
    }
    if (status != HAL_OK)
    {
        return I2C_Fail(hi2c, status, hi2c->ErrorCode);
    }
    hi2c->State = HAL_I2C_STATE_READY;
    hi2c->Mode = HAL_I2C_MODE_NONE;
    __HAL_UNLOCK(hi2c);
    return HAL_OK;
}

static HAL_StatusTypeDef I2C_MemAddress(I2C_HandleTypeDef *hi2c,
                                        uint16_t DevAddress,
                                        uint16_t MemAddress,
                                        uint16_t MemAddSize,
                                        uint32_t Timeout,
                                        uint32_t tickstart)
{
    HAL_StatusTypeDef status = I2C_RequestAddress(
        hi2c, DevAddress, 0, Timeout, tickstart);
    if (status != HAL_OK)
    {
        return status;
    }
    GD32_HAL_I2C_ClearAddress(I2C_Address(hi2c));
    if (MemAddSize == I2C_MEMADD_SIZE_16BIT)
    {
        status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_TX_EMPTY,
                              Timeout, tickstart);
        if (status != HAL_OK)
        {
            return status;
        }
        GD32_HAL_I2C_WriteData(I2C_Address(hi2c),
                               (uint8_t)(MemAddress >> 8U));
    }
    status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_TX_EMPTY,
                          Timeout, tickstart);
    if (status == HAL_OK)
    {
        GD32_HAL_I2C_WriteData(I2C_Address(hi2c), (uint8_t)MemAddress);
    }
    return status;
}

HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c,
                                    uint16_t DevAddress,
                                    uint16_t MemAddress,
                                    uint16_t MemAddSize,
                                    uint8_t *pData,
                                    uint16_t Size,
                                    uint32_t Timeout)
{
    HAL_StatusTypeDef status;
    uint32_t tickstart = HAL_GetTick();
    if ((I2C_ValidTransfer(hi2c, DevAddress, pData, Size) == 0) ||
        ((MemAddSize != I2C_MEMADD_SIZE_8BIT) &&
         (MemAddSize != I2C_MEMADD_SIZE_16BIT)) ||
        (hi2c->Init.AddressingMode != I2C_ADDRESSINGMODE_7BIT))
    {
        return HAL_ERROR;
    }
    if (hi2c->State != HAL_I2C_STATE_READY)
    {
        return HAL_BUSY;
    }
    if (I2C_WaitBusFree(hi2c, Timeout, tickstart) != HAL_OK)
    {
        I2C_Recover(hi2c);
        return HAL_BUSY;
    }
    __HAL_LOCK(hi2c);
    I2C_Prepare(hi2c, HAL_I2C_STATE_BUSY_TX, HAL_I2C_MODE_MEM,
                DevAddress, pData, Size);
    hi2c->Memaddress = MemAddress;
    hi2c->MemaddSize = MemAddSize;
    status = I2C_MemAddress(hi2c, DevAddress, MemAddress, MemAddSize,
                            Timeout, tickstart);
    while ((status == HAL_OK) && (hi2c->XferCount > 0U))
    {
        status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_TX_EMPTY,
                              Timeout, tickstart);
        if (status == HAL_OK)
        {
            GD32_HAL_I2C_WriteData(I2C_Address(hi2c), *hi2c->pBuffPtr++);
            --hi2c->XferCount;
        }
    }
    if (status == HAL_OK)
    {
        status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_BYTE_TRANSFER,
                              Timeout, tickstart);
    }
    if (status != HAL_OK)
    {
        return I2C_Fail(hi2c, status, hi2c->ErrorCode);
    }
    GD32_HAL_I2C_Stop(I2C_Address(hi2c));
    hi2c->State = HAL_I2C_STATE_READY;
    hi2c->Mode = HAL_I2C_MODE_NONE;
    __HAL_UNLOCK(hi2c);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c,
                                   uint16_t DevAddress,
                                   uint16_t MemAddress,
                                   uint16_t MemAddSize,
                                   uint8_t *pData,
                                   uint16_t Size,
                                   uint32_t Timeout)
{
    HAL_StatusTypeDef status;
    uint32_t tickstart = HAL_GetTick();
    if ((I2C_ValidTransfer(hi2c, DevAddress, pData, Size) == 0) ||
        ((MemAddSize != I2C_MEMADD_SIZE_8BIT) &&
         (MemAddSize != I2C_MEMADD_SIZE_16BIT)) ||
        (hi2c->Init.AddressingMode != I2C_ADDRESSINGMODE_7BIT))
    {
        return HAL_ERROR;
    }
    if (hi2c->State != HAL_I2C_STATE_READY)
    {
        return HAL_BUSY;
    }
    if (I2C_WaitBusFree(hi2c, Timeout, tickstart) != HAL_OK)
    {
        I2C_Recover(hi2c);
        return HAL_BUSY;
    }
    __HAL_LOCK(hi2c);
    I2C_Prepare(hi2c, HAL_I2C_STATE_BUSY_RX, HAL_I2C_MODE_MEM,
                DevAddress, pData, Size);
    hi2c->Memaddress = MemAddress;
    hi2c->MemaddSize = MemAddSize;
    status = I2C_MemAddress(hi2c, DevAddress, MemAddress, MemAddSize,
                            Timeout, tickstart);
    if (status == HAL_OK)
    {
        status = I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_TX_EMPTY,
                              Timeout, tickstart);
    }
    if (status == HAL_OK)
    {
        status = I2C_RequestAddress(hi2c, DevAddress, 1,
                                    Timeout, tickstart);
    }
    if (status == HAL_OK)
    {
        status = I2C_PollReceiveData(hi2c, Timeout, tickstart);
    }
    if (status != HAL_OK)
    {
        return I2C_Fail(hi2c, status, hi2c->ErrorCode);
    }
    hi2c->State = HAL_I2C_STATE_READY;
    hi2c->Mode = HAL_I2C_MODE_NONE;
    __HAL_UNLOCK(hi2c);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c,
                                        uint16_t DevAddress,
                                        uint32_t Trials,
                                        uint32_t Timeout)
{
    uint32_t trial;
    uint32_t tickstart;
    if ((hi2c == NULL) || (hi2c->Instance == NULL) || (Trials == 0U) ||
        (DevAddress > 0x00FEU) ||
        (hi2c->Init.AddressingMode != I2C_ADDRESSINGMODE_7BIT) ||
        (GD32_HAL_I2C_IsInstance(I2C_Address(hi2c)) == 0))
    {
        return HAL_ERROR;
    }
    if (hi2c->State != HAL_I2C_STATE_READY)
    {
        return HAL_BUSY;
    }
    __HAL_LOCK(hi2c);
    hi2c->State = HAL_I2C_STATE_BUSY;
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
    for (trial = 0U; trial < Trials; ++trial)
    {
        tickstart = HAL_GetTick();
        GD32_HAL_I2C_ClearErrors(I2C_Address(hi2c),
                                 GD32_HAL_I2C_GetErrors(I2C_Address(hi2c)));
        GD32_HAL_I2C_Start(I2C_Address(hi2c));
        if (I2C_WaitFlag(hi2c, GD32_HAL_I2C_FLAG_START,
                         Timeout, tickstart) != HAL_OK)
        {
            continue;
        }
        GD32_HAL_I2C_SendAddress(I2C_Address(hi2c), DevAddress, 0);
        while (((GD32_HAL_I2C_GetFlags(I2C_Address(hi2c)) &
                 GD32_HAL_I2C_FLAG_ADDRESS) == 0U) &&
               ((GD32_HAL_I2C_GetErrors(I2C_Address(hi2c)) &
                 GD32_HAL_I2C_ERROR_ACK) == 0U))
        {
            if ((Timeout != HAL_MAX_DELAY) &&
                ((Timeout == 0U) || ((HAL_GetTick() - tickstart) >= Timeout)))
            {
                break;
            }
        }
        GD32_HAL_I2C_Stop(I2C_Address(hi2c));
        if ((GD32_HAL_I2C_GetFlags(I2C_Address(hi2c)) &
             GD32_HAL_I2C_FLAG_ADDRESS) != 0U)
        {
            GD32_HAL_I2C_ClearAddress(I2C_Address(hi2c));
            hi2c->State = HAL_I2C_STATE_READY;
            __HAL_UNLOCK(hi2c);
            return HAL_OK;
        }
        GD32_HAL_I2C_ClearErrors(I2C_Address(hi2c), GD32_HAL_I2C_ERROR_ACK);
    }
    hi2c->ErrorCode = HAL_I2C_ERROR_AF;
    hi2c->State = HAL_I2C_STATE_READY;
    __HAL_UNLOCK(hi2c);
    return HAL_ERROR;
}

static HAL_StatusTypeDef I2C_StartIT(I2C_HandleTypeDef *hi2c,
                                     uint16_t DevAddress,
                                     uint8_t *pData,
                                     uint16_t Size,
                                     HAL_I2C_StateTypeDef state)
{
    if (I2C_ValidTransfer(hi2c, DevAddress, pData, Size) == 0)
    {
        return HAL_ERROR;
    }
    if (hi2c->State != HAL_I2C_STATE_READY)
    {
        return HAL_BUSY;
    }
    if ((GD32_HAL_I2C_GetFlags(I2C_Address(hi2c)) &
         GD32_HAL_I2C_FLAG_BUSY) != 0U)
    {
        return HAL_BUSY;
    }
    __HAL_LOCK(hi2c);
    I2C_Prepare(hi2c, state, HAL_I2C_MODE_MASTER,
                DevAddress, pData, Size);
    GD32_HAL_I2C_SetAckNext(I2C_Address(hi2c), 0);
    GD32_HAL_I2C_SetAck(I2C_Address(hi2c), 1);
    GD32_HAL_I2C_Start(I2C_Address(hi2c));
    __HAL_UNLOCK(hi2c);
    GD32_HAL_I2C_SetInterrupts(I2C_Address(hi2c), I2C_ALL_INTERRUPTS, 1);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit_IT(I2C_HandleTypeDef *hi2c,
                                             uint16_t DevAddress,
                                             uint8_t *pData,
                                             uint16_t Size)
{
    return I2C_StartIT(hi2c, DevAddress, pData, Size,
                       HAL_I2C_STATE_BUSY_TX);
}

HAL_StatusTypeDef HAL_I2C_Master_Receive_IT(I2C_HandleTypeDef *hi2c,
                                            uint16_t DevAddress,
                                            uint8_t *pData,
                                            uint16_t Size)
{
    return I2C_StartIT(hi2c, DevAddress, pData, Size,
                       HAL_I2C_STATE_BUSY_RX);
}

static void I2C_CompleteIT(I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_StateTypeDef previous = hi2c->State;
    GD32_HAL_I2C_SetInterrupts(I2C_Address(hi2c), I2C_ALL_INTERRUPTS, 0);
    I2C_RestoreReceiveControls(hi2c);
    hi2c->State = HAL_I2C_STATE_READY;
    hi2c->Mode = HAL_I2C_MODE_NONE;
    hi2c->EventCount = 0U;
    if (previous == HAL_I2C_STATE_BUSY_TX)
    {
        HAL_I2C_MasterTxCpltCallback(hi2c);
    }
    else
    {
        HAL_I2C_MasterRxCpltCallback(hi2c);
    }
}

static void I2C_ConfigureReceiveAddress(I2C_HandleTypeDef *hi2c)
{
    uint32_t address = I2C_Address(hi2c);
    if (hi2c->XferCount == 1U)
    {
        GD32_HAL_I2C_SetAck(address, 0);
        GD32_HAL_I2C_ClearAddress(address);
        GD32_HAL_I2C_Stop(address);
    }
    else if (hi2c->XferCount == 2U)
    {
        GD32_HAL_I2C_SetAck(address, 0);
        GD32_HAL_I2C_SetAckNext(address, 1);
        GD32_HAL_I2C_ClearAddress(address);
    }
    else
    {
        GD32_HAL_I2C_SetAck(address, 1);
        GD32_HAL_I2C_ClearAddress(address);
    }
}

void HAL_I2C_EV_IRQHandler(I2C_HandleTypeDef *hi2c)
{
    uint32_t flags;
    uint32_t address;
    if ((hi2c == NULL) || (hi2c->Instance == NULL) ||
        ((hi2c->State != HAL_I2C_STATE_BUSY_TX) &&
         (hi2c->State != HAL_I2C_STATE_BUSY_RX)))
    {
        return;
    }
    address = I2C_Address(hi2c);
    flags = GD32_HAL_I2C_GetFlags(address);

    if ((hi2c->EventCount == 0U) &&
        ((flags & GD32_HAL_I2C_FLAG_START) != 0U))
    {
        if (hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_10BIT)
        {
            GD32_HAL_I2C_WriteData(address,
                                   I2C_Header10((uint16_t)hi2c->Devaddress, 0));
            hi2c->EventCount = 1U;
        }
        else
        {
            GD32_HAL_I2C_SendAddress(
                address, (uint16_t)(hi2c->Devaddress & 0xFEU),
                (hi2c->State == HAL_I2C_STATE_BUSY_RX) ? 1 : 0);
            hi2c->EventCount = 3U;
        }
        return;
    }
    if ((hi2c->EventCount == 1U) &&
        ((flags & GD32_HAL_I2C_FLAG_ADDRESS10) != 0U))
    {
        GD32_HAL_I2C_WriteData(address, (uint8_t)hi2c->Devaddress);
        hi2c->EventCount = 2U;
        return;
    }
    if ((hi2c->EventCount == 2U) &&
        ((flags & GD32_HAL_I2C_FLAG_ADDRESS) != 0U))
    {
        if (hi2c->State == HAL_I2C_STATE_BUSY_RX)
        {
            GD32_HAL_I2C_ClearAddress(address);
            GD32_HAL_I2C_Start(address);
            hi2c->EventCount = 4U;
        }
        else
        {
            GD32_HAL_I2C_ClearAddress(address);
            hi2c->EventCount = 5U;
        }
        return;
    }
    if ((hi2c->EventCount == 4U) &&
        ((flags & GD32_HAL_I2C_FLAG_START) != 0U))
    {
        GD32_HAL_I2C_WriteData(address,
                               I2C_Header10((uint16_t)hi2c->Devaddress, 1));
        hi2c->EventCount = 3U;
        return;
    }
    if ((hi2c->EventCount == 3U) &&
        ((flags & GD32_HAL_I2C_FLAG_ADDRESS) != 0U))
    {
        if (hi2c->State == HAL_I2C_STATE_BUSY_RX)
        {
            I2C_ConfigureReceiveAddress(hi2c);
        }
        else
        {
            GD32_HAL_I2C_ClearAddress(address);
        }
        hi2c->EventCount = 5U;
        return;
    }
    if (hi2c->EventCount != 5U)
    {
        return;
    }

    if (hi2c->State == HAL_I2C_STATE_BUSY_TX)
    {
        if ((hi2c->XferCount > 0U) &&
            ((flags & GD32_HAL_I2C_FLAG_TX_EMPTY) != 0U))
        {
            GD32_HAL_I2C_WriteData(address, *hi2c->pBuffPtr++);
            --hi2c->XferCount;
        }
        if ((hi2c->XferCount == 0U) &&
            ((flags & GD32_HAL_I2C_FLAG_BYTE_TRANSFER) != 0U))
        {
            GD32_HAL_I2C_Stop(address);
            I2C_CompleteIT(hi2c);
        }
    }
    else if (hi2c->XferCount == 1U)
    {
        if ((flags & GD32_HAL_I2C_FLAG_RX_NOT_EMPTY) != 0U)
        {
            *hi2c->pBuffPtr++ = GD32_HAL_I2C_ReadData(address);
            hi2c->XferCount = 0U;
            I2C_CompleteIT(hi2c);
        }
    }
    else if (hi2c->XferCount == 2U)
    {
        if ((flags & GD32_HAL_I2C_FLAG_BYTE_TRANSFER) != 0U)
        {
            GD32_HAL_I2C_Stop(address);
            *hi2c->pBuffPtr++ = GD32_HAL_I2C_ReadData(address);
            *hi2c->pBuffPtr++ = GD32_HAL_I2C_ReadData(address);
            hi2c->XferCount = 0U;
            I2C_CompleteIT(hi2c);
        }
    }
    else if (hi2c->XferCount == 3U)
    {
        if ((flags & GD32_HAL_I2C_FLAG_BYTE_TRANSFER) != 0U)
        {
            GD32_HAL_I2C_SetAck(address, 0);
            *hi2c->pBuffPtr++ = GD32_HAL_I2C_ReadData(address);
            --hi2c->XferCount;
        }
    }
    else if ((flags & GD32_HAL_I2C_FLAG_RX_NOT_EMPTY) != 0U)
    {
        *hi2c->pBuffPtr++ = GD32_HAL_I2C_ReadData(address);
        --hi2c->XferCount;
    }
}

void HAL_I2C_IRQHandler(I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_EV_IRQHandler(hi2c);
}

void HAL_I2C_ER_IRQHandler(I2C_HandleTypeDef *hi2c)
{
    uint32_t errors;
    if ((hi2c == NULL) || (hi2c->Instance == NULL))
    {
        return;
    }
    errors = GD32_HAL_I2C_GetErrors(I2C_Address(hi2c));
    if (errors == 0U)
    {
        return;
    }
    hi2c->ErrorCode |= I2C_MapErrors(errors);
    GD32_HAL_I2C_ClearErrors(I2C_Address(hi2c), errors);
    GD32_HAL_I2C_Stop(I2C_Address(hi2c));
    GD32_HAL_I2C_SetInterrupts(I2C_Address(hi2c), I2C_ALL_INTERRUPTS, 0);
    if ((errors & (GD32_HAL_I2C_ERROR_BUS |
                   GD32_HAL_I2C_ERROR_ARBITRATION |
                   GD32_HAL_I2C_ERROR_TIMEOUT)) != 0U)
    {
        I2C_Recover(hi2c);
    }
    else
    {
        I2C_RestoreReceiveControls(hi2c);
    }
    hi2c->State = HAL_I2C_STATE_READY;
    hi2c->Mode = HAL_I2C_MODE_NONE;
    hi2c->EventCount = 0U;
    HAL_I2C_ErrorCallback(hi2c);
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit_DMA(I2C_HandleTypeDef *hi2c,
                                              uint16_t DevAddress,
                                              uint8_t *pData,
                                              uint16_t Size)
{
    UNUSED(hi2c);
    UNUSED(DevAddress);
    UNUSED(pData);
    UNUSED(Size);
    GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_I2C_DMA_UNSUPPORTED, 0U);
    return HAL_ERROR;
}

HAL_StatusTypeDef HAL_I2C_Master_Receive_DMA(I2C_HandleTypeDef *hi2c,
                                             uint16_t DevAddress,
                                             uint8_t *pData,
                                             uint16_t Size)
{
    UNUSED(hi2c);
    UNUSED(DevAddress);
    UNUSED(pData);
    UNUSED(Size);
    GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_I2C_DMA_UNSUPPORTED, 1U);
    return HAL_ERROR;
}

HAL_I2C_StateTypeDef HAL_I2C_GetState(const I2C_HandleTypeDef *hi2c)
{
    return (hi2c != NULL) ? hi2c->State : HAL_I2C_STATE_ERROR;
}

HAL_I2C_ModeTypeDef HAL_I2C_GetMode(const I2C_HandleTypeDef *hi2c)
{
    return (hi2c != NULL) ? hi2c->Mode : HAL_I2C_MODE_NONE;
}

uint32_t HAL_I2C_GetError(const I2C_HandleTypeDef *hi2c)
{
    return (hi2c != NULL) ? hi2c->ErrorCode : HAL_I2C_ERROR_BERR;
}
