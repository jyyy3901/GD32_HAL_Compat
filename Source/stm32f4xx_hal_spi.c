#include "stm32f4xx_hal.h"

#define SPI_ALL_INTERRUPTS (GD32_HAL_SPI_INTERRUPT_TX | \
                            GD32_HAL_SPI_INTERRUPT_RX | \
                            GD32_HAL_SPI_INTERRUPT_ERROR)
#define SPI_END_SPIN_LIMIT 1000000UL

static void SPI_DMATxCplt(DMA_HandleTypeDef *hdma);
static void SPI_DMARxCplt(DMA_HandleTypeDef *hdma);
static void SPI_DMATxRxCplt(DMA_HandleTypeDef *hdma);
static void SPI_DMATxHalf(DMA_HandleTypeDef *hdma);
static void SPI_DMARxHalf(DMA_HandleTypeDef *hdma);
static void SPI_DMATxRxHalf(DMA_HandleTypeDef *hdma);
static void SPI_DMAError(DMA_HandleTypeDef *hdma);

static uint32_t SPI_Address(const SPI_HandleTypeDef *hspi)
{
    const GD32_HAL_Resource *resource;

    if ((hspi != NULL) && (hspi->GD32_RESOURCE != NULL) &&
        (hspi->GD32_RESOURCE->stm32_instance == (uintptr_t)hspi->Instance))
    {
        return hspi->GD32_INSTANCE;
    }
    resource = (hspi != NULL) ?
        GD32_HAL_ResolveInstance((uintptr_t)hspi->Instance,
                                 GD32_HAL_RESOURCE_SPI) : NULL;
    return (resource != NULL) ? resource->gd32_instance : 0U;
}

static uint32_t SPI_FrameBytes(const SPI_HandleTypeDef *hspi)
{
    return (hspi->Init.DataSize == SPI_DATASIZE_16BIT) ? 2U : 1U;
}

static uint16_t SPI_LoadFrame(const SPI_HandleTypeDef *hspi,
                              const uint8_t *data)
{
    uint16_t value = data[0];
    if (SPI_FrameBytes(hspi) == 2U)
    {
        value |= (uint16_t)((uint16_t)data[1] << 8U);
    }
    return value;
}

static void SPI_StoreFrame(const SPI_HandleTypeDef *hspi,
                           uint8_t *data,
                           uint16_t value)
{
    data[0] = (uint8_t)value;
    if (SPI_FrameBytes(hspi) == 2U)
    {
        data[1] = (uint8_t)(value >> 8U);
    }
}

static uint32_t SPI_MapErrors(uint32_t errors)
{
    uint32_t mapped = HAL_SPI_ERROR_NONE;
    if ((errors & GD32_HAL_SPI_ERROR_MODE) != 0U)
    {
        mapped |= HAL_SPI_ERROR_MODF;
    }
    if ((errors & GD32_HAL_SPI_ERROR_CRC) != 0U)
    {
        mapped |= HAL_SPI_ERROR_CRC;
    }
    if ((errors & GD32_HAL_SPI_ERROR_OVERRUN) != 0U)
    {
        mapped |= HAL_SPI_ERROR_OVR;
    }
    if ((errors & GD32_HAL_SPI_ERROR_FRAME) != 0U)
    {
        mapped |= HAL_SPI_ERROR_FRE;
    }
    return mapped;
}

static int SPI_FillConfig(const SPI_HandleTypeDef *hspi,
                          GD32_HAL_SPIConfig *config)
{
    const SPI_InitTypeDef *init = &hspi->Init;
    if (((init->Mode != SPI_MODE_MASTER) && (init->Mode != SPI_MODE_SLAVE)) ||
        ((init->Direction != SPI_DIRECTION_2LINES) &&
         (init->Direction != SPI_DIRECTION_2LINES_RXONLY) &&
         (init->Direction != SPI_DIRECTION_1LINE)) ||
        ((init->DataSize != SPI_DATASIZE_8BIT) &&
         (init->DataSize != SPI_DATASIZE_16BIT)) ||
        ((init->CLKPolarity != SPI_POLARITY_LOW) &&
         (init->CLKPolarity != SPI_POLARITY_HIGH)) ||
        ((init->CLKPhase != SPI_PHASE_1EDGE) &&
         (init->CLKPhase != SPI_PHASE_2EDGE)) ||
        ((init->NSS != SPI_NSS_SOFT) &&
         (init->NSS != SPI_NSS_HARD_INPUT) &&
         (init->NSS != SPI_NSS_HARD_OUTPUT)) ||
        ((init->BaudRatePrescaler != 2U) &&
         (init->BaudRatePrescaler != 4U) &&
         (init->BaudRatePrescaler != 8U) &&
         (init->BaudRatePrescaler != 16U) &&
         (init->BaudRatePrescaler != 32U) &&
         (init->BaudRatePrescaler != 64U) &&
         (init->BaudRatePrescaler != 128U) &&
         (init->BaudRatePrescaler != 256U)) ||
        ((init->FirstBit != SPI_FIRSTBIT_MSB) &&
         (init->FirstBit != SPI_FIRSTBIT_LSB)) ||
        ((init->TIMode != SPI_TIMODE_DISABLE) &&
         (init->TIMode != SPI_TIMODE_ENABLE)) ||
        ((init->CRCCalculation != SPI_CRCCALCULATION_DISABLE) &&
         (init->CRCCalculation != SPI_CRCCALCULATION_ENABLE)) ||
        (init->CRCCalculation == SPI_CRCCALCULATION_ENABLE) ||
        ((init->Mode == SPI_MODE_MASTER) &&
         (init->Direction == SPI_DIRECTION_2LINES_RXONLY)))
    {
        return 0;
    }
    config->master = (init->Mode == SPI_MODE_MASTER) ? 1U : 0U;
    config->direction = (init->Direction == SPI_DIRECTION_2LINES) ? 0U :
                        ((init->Direction == SPI_DIRECTION_2LINES_RXONLY) ?
                         1U : 2U);
    config->data_bits = (init->DataSize == SPI_DATASIZE_16BIT) ? 16U : 8U;
    config->clock_polarity_high =
        (init->CLKPolarity == SPI_POLARITY_HIGH) ? 1U : 0U;
    config->clock_phase_second =
        (init->CLKPhase == SPI_PHASE_2EDGE) ? 1U : 0U;
    config->nss = (uint8_t)init->NSS;
    config->prescaler = (uint16_t)init->BaudRatePrescaler;
    config->lsb_first = (init->FirstBit == SPI_FIRSTBIT_LSB) ? 1U : 0U;
    config->ti_mode = (init->TIMode == SPI_TIMODE_ENABLE) ? 1U : 0U;
    config->crc_enable = 0U;
    config->crc_polynomial = (init->CRCPolynomial != 0U) ?
                             (uint16_t)init->CRCPolynomial : 7U;
    return 1;
}

static HAL_StatusTypeDef SPI_WaitFlag(SPI_HandleTypeDef *hspi,
                                      uint32_t flag,
                                      int set,
                                      uint32_t timeout,
                                      uint32_t tickstart)
{
    for (;;)
    {
        uint32_t flags = GD32_HAL_SPI_GetFlags(SPI_Address(hspi));
        uint32_t errors = GD32_HAL_SPI_GetErrors(SPI_Address(hspi));
        int matches = ((flags & flag) != 0U) ? 1 : 0;
        if (matches == set)
        {
            return HAL_OK;
        }
        if (errors != 0U)
        {
            hspi->ErrorCode |= SPI_MapErrors(errors);
            GD32_HAL_SPI_ClearErrors(SPI_Address(hspi), errors);
            return HAL_ERROR;
        }
        if ((timeout != HAL_MAX_DELAY) &&
            ((timeout == 0U) || ((HAL_GetTick() - tickstart) >= timeout)))
        {
            hspi->ErrorCode |= HAL_SPI_ERROR_FLAG;
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_SPI_TIMEOUT, flag);
            return HAL_TIMEOUT;
        }
    }
}

static int SPI_WaitEndSpin(SPI_HandleTypeDef *hspi)
{
    uint32_t count = SPI_END_SPIN_LIMIT;
    while (((GD32_HAL_SPI_GetFlags(SPI_Address(hspi)) &
             GD32_HAL_SPI_FLAG_BUSY) != 0U) && (count > 0U))
    {
        --count;
    }
    if (count == 0U)
    {
        hspi->ErrorCode |= HAL_SPI_ERROR_FLAG;
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_SPI_TIMEOUT,
                           GD32_HAL_SPI_FLAG_BUSY);
        return 0;
    }
    return 1;
}

static int SPI_ValidTransfer(const SPI_HandleTypeDef *hspi,
                             const void *data,
                             uint16_t size)
{
    return (hspi != NULL) && (hspi->Instance != NULL) &&
           (data != NULL) && (size > 0U) &&
           (GD32_HAL_SPI_IsInstance(SPI_Address(hspi)) != 0);
}

static void SPI_PrepareTx(SPI_HandleTypeDef *hspi,
                          const uint8_t *data,
                          uint16_t size)
{
    hspi->pTxBuffPtr = data;
    hspi->TxXferSize = size;
    hspi->TxXferCount = size;
    hspi->pRxBuffPtr = NULL;
    hspi->RxXferSize = 0U;
    hspi->RxXferCount = 0U;
    hspi->ErrorCode = HAL_SPI_ERROR_NONE;
}

static void SPI_PrepareRx(SPI_HandleTypeDef *hspi,
                          uint8_t *data,
                          uint16_t size)
{
    hspi->pTxBuffPtr = NULL;
    hspi->TxXferSize = 0U;
    hspi->TxXferCount = 0U;
    hspi->pRxBuffPtr = data;
    hspi->RxXferSize = size;
    hspi->RxXferCount = size;
    hspi->ErrorCode = HAL_SPI_ERROR_NONE;
}

static void SPI_PrepareTxRx(SPI_HandleTypeDef *hspi,
                            const uint8_t *tx,
                            uint8_t *rx,
                            uint16_t size)
{
    hspi->pTxBuffPtr = tx;
    hspi->TxXferSize = size;
    hspi->TxXferCount = size;
    hspi->pRxBuffPtr = rx;
    hspi->RxXferSize = size;
    hspi->RxXferCount = size;
    hspi->ErrorCode = HAL_SPI_ERROR_NONE;
}

HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef *hspi)
{
    GD32_HAL_SPIConfig config;
    const GD32_HAL_Resource *resource;
    if ((hspi == NULL) || (hspi->Instance == NULL))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_SPI_INVALID_CONFIG,
                           (hspi != NULL) ? SPI_Address(hspi) : 0U);
        return HAL_ERROR;
    }
    resource = GD32_HAL_ResolveInstance((uintptr_t)hspi->Instance,
                                        GD32_HAL_RESOURCE_SPI);
    if (resource == NULL)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_SPI_INVALID_CONFIG,
                           (uint32_t)(uintptr_t)hspi->Instance);
        return HAL_ERROR;
    }
    hspi->GD32_RESOURCE = resource;
    hspi->GD32_INSTANCE = resource->gd32_instance;
    hspi->GD32_IRQ_NUMBER = resource->gd32_irq;
    if (SPI_FillConfig(hspi, &config) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_SPI_INVALID_CONFIG,
                           hspi->GD32_INSTANCE);
        return HAL_ERROR;
    }
    if (hspi->State == HAL_SPI_STATE_RESET)
    {
        hspi->Lock = HAL_UNLOCKED;
        HAL_SPI_MspInit(hspi);
    }
    __HAL_LOCK(hspi);
    hspi->State = HAL_SPI_STATE_BUSY;
    if (GD32_HAL_SPI_Configure(SPI_Address(hspi), &config) != 0)
    {
        hspi->State = HAL_SPI_STATE_ERROR;
        __HAL_UNLOCK(hspi);
        return HAL_ERROR;
    }
    hspi->ErrorCode = HAL_SPI_ERROR_NONE;
    hspi->DmaActive = 0U;
    hspi->DmaCircular = 0U;
    hspi->State = HAL_SPI_STATE_READY;
    __HAL_UNLOCK(hspi);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_SPI_DeInit(SPI_HandleTypeDef *hspi)
{
    if ((hspi == NULL) || (hspi->Instance == NULL) ||
        (GD32_HAL_SPI_IsInstance(SPI_Address(hspi)) == 0))
    {
        return HAL_ERROR;
    }
    __HAL_LOCK(hspi);
    GD32_HAL_SPI_SetInterrupts(SPI_Address(hspi), SPI_ALL_INTERRUPTS, 0);
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 1, 0);
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 0, 0);
    GD32_HAL_SPI_DeInit(SPI_Address(hspi));
    HAL_SPI_MspDeInit(hspi);
    hspi->State = HAL_SPI_STATE_RESET;
    hspi->ErrorCode = HAL_SPI_ERROR_NONE;
    hspi->DmaActive = 0U;
    hspi->GD32_INSTANCE = 0U;
    hspi->GD32_IRQ_NUMBER = -1;
    hspi->GD32_RESOURCE = NULL;
    __HAL_UNLOCK(hspi);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *hspi,
                                   const uint8_t *pData,
                                   uint16_t Size,
                                   uint32_t Timeout)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t tickstart = HAL_GetTick();
    uint32_t step;
    if (SPI_ValidTransfer(hspi, pData, Size) == 0)
    {
        return HAL_ERROR;
    }
    if (hspi->State != HAL_SPI_STATE_READY)
    {
        return HAL_BUSY;
    }
    __HAL_LOCK(hspi);
    hspi->State = HAL_SPI_STATE_BUSY_TX;
    SPI_PrepareTx(hspi, pData, Size);
    step = SPI_FrameBytes(hspi);
    if (hspi->Init.Direction == SPI_DIRECTION_1LINE)
    {
        GD32_HAL_SPI_SetDirection(SPI_Address(hspi), 3U);
    }
    while ((hspi->TxXferCount > 0U) && (status == HAL_OK))
    {
        status = SPI_WaitFlag(hspi, GD32_HAL_SPI_FLAG_TX_EMPTY, 1,
                              Timeout, tickstart);
        if (status == HAL_OK)
        {
            GD32_HAL_SPI_WriteData(SPI_Address(hspi),
                                   SPI_LoadFrame(hspi, hspi->pTxBuffPtr));
            hspi->pTxBuffPtr += step;
            --hspi->TxXferCount;
        }
        if ((hspi->Init.Direction == SPI_DIRECTION_2LINES) &&
            ((GD32_HAL_SPI_GetFlags(SPI_Address(hspi)) &
              GD32_HAL_SPI_FLAG_RX_NOT_EMPTY) != 0U))
        {
            (void)GD32_HAL_SPI_ReadData(SPI_Address(hspi));
        }
    }
    if (status == HAL_OK)
    {
        status = SPI_WaitFlag(hspi, GD32_HAL_SPI_FLAG_TX_EMPTY, 1,
                              Timeout, tickstart);
    }
    if (status == HAL_OK)
    {
        status = SPI_WaitFlag(hspi, GD32_HAL_SPI_FLAG_BUSY, 0,
                              Timeout, tickstart);
    }
    if ((GD32_HAL_SPI_GetFlags(SPI_Address(hspi)) &
         GD32_HAL_SPI_FLAG_RX_NOT_EMPTY) != 0U)
    {
        (void)GD32_HAL_SPI_ReadData(SPI_Address(hspi));
    }
    GD32_HAL_SPI_ClearErrors(SPI_Address(hspi), GD32_HAL_SPI_ERROR_OVERRUN);
    hspi->State = HAL_SPI_STATE_READY;
    __HAL_UNLOCK(hspi);
    return status;
}

HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi,
                                          const uint8_t *pTxData,
                                          uint8_t *pRxData,
                                          uint16_t Size,
                                          uint32_t Timeout)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t tickstart = HAL_GetTick();
    uint32_t step;
    if ((SPI_ValidTransfer(hspi, pTxData, Size) == 0) ||
        (pRxData == NULL) ||
        (hspi->Init.Direction != SPI_DIRECTION_2LINES))
    {
        return HAL_ERROR;
    }
    if (hspi->State != HAL_SPI_STATE_READY)
    {
        return HAL_BUSY;
    }
    __HAL_LOCK(hspi);
    hspi->State = HAL_SPI_STATE_BUSY_TX_RX;
    SPI_PrepareTxRx(hspi, pTxData, pRxData, Size);
    step = SPI_FrameBytes(hspi);
    while ((hspi->RxXferCount > 0U) && (status == HAL_OK))
    {
        if (hspi->TxXferCount > 0U)
        {
            status = SPI_WaitFlag(hspi, GD32_HAL_SPI_FLAG_TX_EMPTY, 1,
                                  Timeout, tickstart);
            if (status == HAL_OK)
            {
                GD32_HAL_SPI_WriteData(
                    SPI_Address(hspi), SPI_LoadFrame(hspi, hspi->pTxBuffPtr));
                hspi->pTxBuffPtr += step;
                --hspi->TxXferCount;
            }
        }
        if (status == HAL_OK)
        {
            status = SPI_WaitFlag(hspi, GD32_HAL_SPI_FLAG_RX_NOT_EMPTY, 1,
                                  Timeout, tickstart);
        }
        if (status == HAL_OK)
        {
            SPI_StoreFrame(hspi, hspi->pRxBuffPtr,
                           GD32_HAL_SPI_ReadData(SPI_Address(hspi)));
            hspi->pRxBuffPtr += step;
            --hspi->RxXferCount;
        }
    }
    if (status == HAL_OK)
    {
        status = SPI_WaitFlag(hspi, GD32_HAL_SPI_FLAG_BUSY, 0,
                              Timeout, tickstart);
    }
    hspi->State = HAL_SPI_STATE_READY;
    __HAL_UNLOCK(hspi);
    return status;
}

HAL_StatusTypeDef HAL_SPI_Receive(SPI_HandleTypeDef *hspi,
                                  uint8_t *pData,
                                  uint16_t Size,
                                  uint32_t Timeout)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t tickstart = HAL_GetTick();
    uint32_t step;
    if (SPI_ValidTransfer(hspi, pData, Size) == 0)
    {
        return HAL_ERROR;
    }
    if (hspi->Init.Direction == SPI_DIRECTION_2LINES)
    {
        uint16_t index;
        uint8_t dummy[2] = {0xFFU, 0xFFU};
        if (hspi->State != HAL_SPI_STATE_READY)
        {
            return HAL_BUSY;
        }
        for (index = 0U; index < Size; ++index)
        {
            status = HAL_SPI_TransmitReceive(hspi, dummy,
                                             pData + (index * SPI_FrameBytes(hspi)),
                                             1U, Timeout);
            if (status != HAL_OK)
            {
                return status;
            }
        }
        return HAL_OK;
    }
    if (hspi->State != HAL_SPI_STATE_READY)
    {
        return HAL_BUSY;
    }
    __HAL_LOCK(hspi);
    hspi->State = HAL_SPI_STATE_BUSY_RX;
    SPI_PrepareRx(hspi, pData, Size);
    step = SPI_FrameBytes(hspi);
    if (hspi->Init.Direction == SPI_DIRECTION_1LINE)
    {
        GD32_HAL_SPI_SetDirection(SPI_Address(hspi), 2U);
    }
    while ((hspi->RxXferCount > 0U) && (status == HAL_OK))
    {
        status = SPI_WaitFlag(hspi, GD32_HAL_SPI_FLAG_RX_NOT_EMPTY, 1,
                              Timeout, tickstart);
        if (status == HAL_OK)
        {
            SPI_StoreFrame(hspi, hspi->pRxBuffPtr,
                           GD32_HAL_SPI_ReadData(SPI_Address(hspi)));
            hspi->pRxBuffPtr += step;
            --hspi->RxXferCount;
        }
    }
    hspi->State = HAL_SPI_STATE_READY;
    __HAL_UNLOCK(hspi);
    return status;
}

static HAL_StatusTypeDef SPI_StartIT(SPI_HandleTypeDef *hspi,
                                     const uint8_t *tx,
                                     uint8_t *rx,
                                     uint16_t size,
                                     HAL_SPI_StateTypeDef state)
{
    if ((hspi == NULL) || (hspi->Instance == NULL) || (size == 0U) ||
        ((tx == NULL) && (rx == NULL)) ||
        (GD32_HAL_SPI_IsInstance(SPI_Address(hspi)) == 0))
    {
        return HAL_ERROR;
    }
    if (hspi->State != HAL_SPI_STATE_READY)
    {
        return HAL_BUSY;
    }
    __HAL_LOCK(hspi);
    hspi->State = state;
    if (state == HAL_SPI_STATE_BUSY_TX)
    {
        SPI_PrepareTx(hspi, tx, size);
        if (hspi->Init.Direction == SPI_DIRECTION_1LINE)
        {
            GD32_HAL_SPI_SetDirection(SPI_Address(hspi), 3U);
        }
    }
    else if ((state == HAL_SPI_STATE_BUSY_RX) &&
             (hspi->Init.Direction != SPI_DIRECTION_2LINES))
    {
        SPI_PrepareRx(hspi, rx, size);
        if (hspi->Init.Direction == SPI_DIRECTION_1LINE)
        {
            GD32_HAL_SPI_SetDirection(SPI_Address(hspi), 2U);
        }
    }
    else
    {
        SPI_PrepareTxRx(hspi, tx, rx, size);
    }
    __HAL_UNLOCK(hspi);
    GD32_HAL_SPI_SetInterrupts(SPI_Address(hspi),
                               GD32_HAL_SPI_INTERRUPT_ERROR |
                               ((hspi->TxXferCount > 0U) ?
                                GD32_HAL_SPI_INTERRUPT_TX : 0U) |
                               ((hspi->RxXferCount > 0U) ?
                                GD32_HAL_SPI_INTERRUPT_RX : 0U), 1);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_SPI_Transmit_IT(SPI_HandleTypeDef *hspi,
                                      const uint8_t *pData,
                                      uint16_t Size)
{
    return SPI_StartIT(hspi, pData, NULL, Size, HAL_SPI_STATE_BUSY_TX);
}

HAL_StatusTypeDef HAL_SPI_TransmitReceive_IT(SPI_HandleTypeDef *hspi,
                                             const uint8_t *pTxData,
                                             uint8_t *pRxData,
                                             uint16_t Size)
{
    if ((pTxData == NULL) || (pRxData == NULL) ||
        ((hspi != NULL) &&
         (hspi->Init.Direction != SPI_DIRECTION_2LINES)))
    {
        return HAL_ERROR;
    }
    return SPI_StartIT(hspi, pTxData, pRxData, Size,
                       HAL_SPI_STATE_BUSY_TX_RX);
}

HAL_StatusTypeDef HAL_SPI_Receive_IT(SPI_HandleTypeDef *hspi,
                                     uint8_t *pData,
                                     uint16_t Size)
{
    static const uint8_t dummy8[2] = {0xFFU, 0xFFU};
    if ((hspi != NULL) && (hspi->Init.Direction == SPI_DIRECTION_2LINES))
    {
        HAL_StatusTypeDef status = SPI_StartIT(hspi, dummy8, pData, Size,
                                               HAL_SPI_STATE_BUSY_RX);
        if (status == HAL_OK)
        {
            hspi->TxXferSize = Size;
            hspi->TxXferCount = Size;
        }
        return status;
    }
    return SPI_StartIT(hspi, NULL, pData, Size, HAL_SPI_STATE_BUSY_RX);
}

static void SPI_CompleteIT(SPI_HandleTypeDef *hspi)
{
    HAL_SPI_StateTypeDef previous = hspi->State;
    GD32_HAL_SPI_SetInterrupts(SPI_Address(hspi), SPI_ALL_INTERRUPTS, 0);
    if (SPI_WaitEndSpin(hspi) == 0)
    {
        hspi->State = HAL_SPI_STATE_READY;
        HAL_SPI_ErrorCallback(hspi);
        return;
    }
    GD32_HAL_SPI_ClearErrors(SPI_Address(hspi), GD32_HAL_SPI_ERROR_OVERRUN);
    hspi->State = HAL_SPI_STATE_READY;
    if (previous == HAL_SPI_STATE_BUSY_TX)
    {
        HAL_SPI_TxCpltCallback(hspi);
    }
    else if (previous == HAL_SPI_STATE_BUSY_RX)
    {
        HAL_SPI_RxCpltCallback(hspi);
    }
    else
    {
        HAL_SPI_TxRxCpltCallback(hspi);
    }
}

void HAL_SPI_IRQHandler(SPI_HandleTypeDef *hspi)
{
    uint32_t flags;
    uint32_t interrupts;
    uint32_t errors;
    uint32_t step;
    if ((hspi == NULL) || (hspi->Instance == NULL))
    {
        return;
    }
    flags = GD32_HAL_SPI_GetFlags(SPI_Address(hspi));
    interrupts = GD32_HAL_SPI_GetInterrupts(SPI_Address(hspi));
    errors = GD32_HAL_SPI_GetErrors(SPI_Address(hspi));
    step = SPI_FrameBytes(hspi);

    if ((errors != 0U) &&
        ((interrupts & GD32_HAL_SPI_INTERRUPT_ERROR) != 0U))
    {
        hspi->ErrorCode |= SPI_MapErrors(errors);
        GD32_HAL_SPI_ClearErrors(SPI_Address(hspi), errors);
        GD32_HAL_SPI_SetInterrupts(SPI_Address(hspi), SPI_ALL_INTERRUPTS, 0);
        hspi->State = HAL_SPI_STATE_READY;
        HAL_SPI_ErrorCallback(hspi);
        return;
    }

    if (((flags & GD32_HAL_SPI_FLAG_RX_NOT_EMPTY) != 0U) &&
        ((interrupts & GD32_HAL_SPI_INTERRUPT_RX) != 0U) &&
        (hspi->RxXferCount > 0U))
    {
        SPI_StoreFrame(hspi, hspi->pRxBuffPtr,
                       GD32_HAL_SPI_ReadData(SPI_Address(hspi)));
        hspi->pRxBuffPtr += step;
        --hspi->RxXferCount;
        if (hspi->RxXferCount == 0U)
        {
            GD32_HAL_SPI_SetInterrupts(SPI_Address(hspi),
                                       GD32_HAL_SPI_INTERRUPT_RX, 0);
        }
    }
    else if (((flags & GD32_HAL_SPI_FLAG_RX_NOT_EMPTY) != 0U) &&
             (hspi->State == HAL_SPI_STATE_BUSY_TX))
    {
        (void)GD32_HAL_SPI_ReadData(SPI_Address(hspi));
    }

    if (((flags & GD32_HAL_SPI_FLAG_TX_EMPTY) != 0U) &&
        ((interrupts & GD32_HAL_SPI_INTERRUPT_TX) != 0U) &&
        (hspi->TxXferCount > 0U))
    {
        uint16_t value;
        if ((hspi->State == HAL_SPI_STATE_BUSY_RX) &&
            (hspi->Init.Direction == SPI_DIRECTION_2LINES))
        {
            value = (hspi->Init.DataSize == SPI_DATASIZE_16BIT) ?
                    0xFFFFU : 0x00FFU;
        }
        else
        {
            value = SPI_LoadFrame(hspi, hspi->pTxBuffPtr);
            hspi->pTxBuffPtr += step;
        }
        GD32_HAL_SPI_WriteData(SPI_Address(hspi), value);
        --hspi->TxXferCount;
        if (hspi->TxXferCount == 0U)
        {
            GD32_HAL_SPI_SetInterrupts(SPI_Address(hspi),
                                       GD32_HAL_SPI_INTERRUPT_TX, 0);
        }
    }

    if (((hspi->State == HAL_SPI_STATE_BUSY_TX) &&
         (hspi->TxXferCount == 0U)) ||
        ((hspi->State == HAL_SPI_STATE_BUSY_RX) &&
         (hspi->RxXferCount == 0U) && (hspi->TxXferCount == 0U)) ||
        ((hspi->State == HAL_SPI_STATE_BUSY_TX_RX) &&
         (hspi->RxXferCount == 0U) && (hspi->TxXferCount == 0U)))
    {
        SPI_CompleteIT(hspi);
    }
}

static int SPI_ValidateDMA(const SPI_HandleTypeDef *hspi,
                           const DMA_HandleTypeDef *hdma,
                           int transmit)
{
    uint32_t width = (hspi->Init.DataSize == SPI_DATASIZE_16BIT) ?
                     DMA_PDATAALIGN_HALFWORD : DMA_PDATAALIGN_BYTE;
    uint32_t memory_width = (hspi->Init.DataSize == SPI_DATASIZE_16BIT) ?
                            DMA_MDATAALIGN_HALFWORD : DMA_MDATAALIGN_BYTE;
    return (hdma != NULL) && (hdma->Instance != NULL) &&
           (hdma->Parent == hspi) &&
           (hdma->Init.Direction ==
            ((transmit != 0) ? DMA_MEMORY_TO_PERIPH : DMA_PERIPH_TO_MEMORY)) &&
           (hdma->Init.PeriphInc == DMA_PINC_DISABLE) &&
           (hdma->Init.MemInc == DMA_MINC_ENABLE) &&
           (hdma->Init.PeriphDataAlignment == width) &&
           (hdma->Init.MemDataAlignment == memory_width) &&
           ((hdma->Init.Mode == DMA_NORMAL) ||
            (hdma->Init.Mode == DMA_CIRCULAR)) &&
           (GD32_HAL_SPI_IsDMAChannelValid(
                SPI_Address(hspi), transmit,
                GD32_HAL_DMA_MappedInstance(hdma),
                hdma->Init.Channel) != 0);
}

static HAL_StatusTypeDef SPI_DMAStartFailure(SPI_HandleTypeDef *hspi)
{
    if ((hspi->hdmatx != NULL) &&
        (hspi->hdmatx->State == HAL_DMA_STATE_BUSY))
    {
        (void)HAL_DMA_Abort(hspi->hdmatx);
    }
    if ((hspi->hdmarx != NULL) &&
        (hspi->hdmarx->State == HAL_DMA_STATE_BUSY))
    {
        (void)HAL_DMA_Abort(hspi->hdmarx);
    }
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 1, 0);
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 0, 0);
    hspi->DmaActive = 0U;
    hspi->ErrorCode |= HAL_SPI_ERROR_DMA;
    hspi->State = HAL_SPI_STATE_READY;
    __HAL_UNLOCK(hspi);
    return HAL_ERROR;
}

HAL_StatusTypeDef HAL_SPI_Transmit_DMA(SPI_HandleTypeDef *hspi,
                                       const uint8_t *pData,
                                       uint16_t Size)
{
    if (SPI_ValidTransfer(hspi, pData, Size) == 0)
    {
        return HAL_ERROR;
    }
    if (hspi->State != HAL_SPI_STATE_READY)
    {
        return HAL_BUSY;
    }
    if (SPI_ValidateDMA(hspi, hspi->hdmatx, 1) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_SPI_DMA_LINK_INVALID, 1U);
        return HAL_ERROR;
    }
    __HAL_LOCK(hspi);
    hspi->State = HAL_SPI_STATE_BUSY_TX;
    SPI_PrepareTx(hspi, pData, Size);
    hspi->DmaActive = 1U;
    hspi->DmaCircular = (hspi->hdmatx->Init.Mode == DMA_CIRCULAR) ? 1U : 0U;
    hspi->hdmatx->XferCpltCallback = SPI_DMATxCplt;
    hspi->hdmatx->XferHalfCpltCallback = SPI_DMATxHalf;
    hspi->hdmatx->XferErrorCallback = SPI_DMAError;
    if (hspi->Init.Direction == SPI_DIRECTION_1LINE)
    {
        GD32_HAL_SPI_SetDirection(SPI_Address(hspi), 3U);
    }
    if (HAL_DMA_Start_IT(hspi->hdmatx,
                         (uint32_t)(uintptr_t)pData,
                         GD32_HAL_SPI_GetDataAddress(SPI_Address(hspi)),
                         Size) != HAL_OK)
    {
        return SPI_DMAStartFailure(hspi);
    }
    GD32_HAL_SPI_SetInterrupts(SPI_Address(hspi),
                               GD32_HAL_SPI_INTERRUPT_ERROR, 1);
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 1, 1);
    __HAL_UNLOCK(hspi);
    return HAL_OK;
}

static HAL_StatusTypeDef SPI_StartRxDMA(SPI_HandleTypeDef *hspi,
                                        uint8_t *pData,
                                        uint16_t Size)
{
    if (SPI_ValidateDMA(hspi, hspi->hdmarx, 0) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_SPI_DMA_LINK_INVALID, 0U);
        return HAL_ERROR;
    }
    __HAL_LOCK(hspi);
    hspi->State = HAL_SPI_STATE_BUSY_RX;
    SPI_PrepareRx(hspi, pData, Size);
    hspi->DmaActive = 2U;
    hspi->DmaCircular = (hspi->hdmarx->Init.Mode == DMA_CIRCULAR) ? 1U : 0U;
    hspi->hdmarx->XferCpltCallback = SPI_DMARxCplt;
    hspi->hdmarx->XferHalfCpltCallback = SPI_DMARxHalf;
    hspi->hdmarx->XferErrorCallback = SPI_DMAError;
    if (hspi->Init.Direction == SPI_DIRECTION_1LINE)
    {
        GD32_HAL_SPI_SetDirection(SPI_Address(hspi), 2U);
    }
    if (HAL_DMA_Start_IT(hspi->hdmarx,
                         GD32_HAL_SPI_GetDataAddress(SPI_Address(hspi)),
                         (uint32_t)(uintptr_t)pData, Size) != HAL_OK)
    {
        return SPI_DMAStartFailure(hspi);
    }
    GD32_HAL_SPI_SetInterrupts(SPI_Address(hspi),
                               GD32_HAL_SPI_INTERRUPT_ERROR, 1);
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 0, 1);
    __HAL_UNLOCK(hspi);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_SPI_Receive_DMA(SPI_HandleTypeDef *hspi,
                                      uint8_t *pData,
                                      uint16_t Size)
{
    if (SPI_ValidTransfer(hspi, pData, Size) == 0)
    {
        return HAL_ERROR;
    }
    if (hspi->State != HAL_SPI_STATE_READY)
    {
        return HAL_BUSY;
    }
    if (hspi->Init.Direction == SPI_DIRECTION_2LINES)
    {
        HAL_StatusTypeDef status;
        status = HAL_SPI_TransmitReceive_DMA(hspi, pData, pData, Size);
        if (status == HAL_OK)
        {
            hspi->State = HAL_SPI_STATE_BUSY_RX;
        }
        return status;
    }
    return SPI_StartRxDMA(hspi, pData, Size);
}

HAL_StatusTypeDef HAL_SPI_TransmitReceive_DMA(SPI_HandleTypeDef *hspi,
                                              const uint8_t *pTxData,
                                              uint8_t *pRxData,
                                              uint16_t Size)
{
    if ((SPI_ValidTransfer(hspi, pTxData, Size) == 0) ||
        (pRxData == NULL) ||
        (hspi->Init.Direction != SPI_DIRECTION_2LINES))
    {
        return HAL_ERROR;
    }
    if (hspi->State != HAL_SPI_STATE_READY)
    {
        return HAL_BUSY;
    }
    if ((SPI_ValidateDMA(hspi, hspi->hdmatx, 1) == 0) ||
        (SPI_ValidateDMA(hspi, hspi->hdmarx, 0) == 0) ||
        (hspi->hdmatx->Init.Mode != hspi->hdmarx->Init.Mode))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_SPI_DMA_CONFIG_MISMATCH, 3U);
        return HAL_ERROR;
    }
    __HAL_LOCK(hspi);
    hspi->State = HAL_SPI_STATE_BUSY_TX_RX;
    SPI_PrepareTxRx(hspi, pTxData, pRxData, Size);
    hspi->DmaActive = 3U;
    hspi->DmaCircular = (hspi->hdmarx->Init.Mode == DMA_CIRCULAR) ? 1U : 0U;
    hspi->hdmarx->XferCpltCallback = SPI_DMATxRxCplt;
    hspi->hdmarx->XferHalfCpltCallback = SPI_DMATxRxHalf;
    hspi->hdmarx->XferErrorCallback = SPI_DMAError;
    hspi->hdmatx->XferCpltCallback = NULL;
    hspi->hdmatx->XferHalfCpltCallback = NULL;
    hspi->hdmatx->XferErrorCallback = SPI_DMAError;
    if (HAL_DMA_Start_IT(hspi->hdmarx,
                         GD32_HAL_SPI_GetDataAddress(SPI_Address(hspi)),
                         (uint32_t)(uintptr_t)pRxData, Size) != HAL_OK)
    {
        return SPI_DMAStartFailure(hspi);
    }
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 0, 1);
    if (HAL_DMA_Start_IT(hspi->hdmatx,
                         (uint32_t)(uintptr_t)pTxData,
                         GD32_HAL_SPI_GetDataAddress(SPI_Address(hspi)),
                         Size) != HAL_OK)
    {
        return SPI_DMAStartFailure(hspi);
    }
    GD32_HAL_SPI_SetInterrupts(SPI_Address(hspi),
                               GD32_HAL_SPI_INTERRUPT_ERROR, 1);
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 1, 1);
    __HAL_UNLOCK(hspi);
    return HAL_OK;
}

static void SPI_DMANormalFinish(SPI_HandleTypeDef *hspi,
                                HAL_SPI_StateTypeDef previous)
{
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 1, 0);
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 0, 0);
    GD32_HAL_SPI_SetInterrupts(SPI_Address(hspi),
                               GD32_HAL_SPI_INTERRUPT_ERROR, 0);
    hspi->TxXferCount = 0U;
    hspi->RxXferCount = 0U;
    hspi->DmaActive = 0U;
    if (SPI_WaitEndSpin(hspi) == 0)
    {
        hspi->State = HAL_SPI_STATE_READY;
        HAL_SPI_ErrorCallback(hspi);
        return;
    }
    GD32_HAL_SPI_ClearErrors(SPI_Address(hspi), GD32_HAL_SPI_ERROR_OVERRUN);
    hspi->State = HAL_SPI_STATE_READY;
    if (previous == HAL_SPI_STATE_BUSY_TX)
    {
        HAL_SPI_TxCpltCallback(hspi);
    }
    else if (previous == HAL_SPI_STATE_BUSY_RX)
    {
        HAL_SPI_RxCpltCallback(hspi);
    }
    else
    {
        HAL_SPI_TxRxCpltCallback(hspi);
    }
}

static SPI_HandleTypeDef *SPI_DMAParent(DMA_HandleTypeDef *hdma)
{
    return (SPI_HandleTypeDef *)hdma->Parent;
}

static void SPI_DMATxCplt(DMA_HandleTypeDef *hdma)
{
    SPI_HandleTypeDef *hspi = SPI_DMAParent(hdma);
    if (hspi->DmaCircular != 0U)
    {
        HAL_SPI_TxCpltCallback(hspi);
    }
    else
    {
        SPI_DMANormalFinish(hspi, HAL_SPI_STATE_BUSY_TX);
    }
}

static void SPI_DMARxCplt(DMA_HandleTypeDef *hdma)
{
    SPI_HandleTypeDef *hspi = SPI_DMAParent(hdma);
    if (hspi->DmaCircular != 0U)
    {
        HAL_SPI_RxCpltCallback(hspi);
    }
    else
    {
        SPI_DMANormalFinish(hspi, HAL_SPI_STATE_BUSY_RX);
    }
}

static void SPI_DMATxRxCplt(DMA_HandleTypeDef *hdma)
{
    SPI_HandleTypeDef *hspi = SPI_DMAParent(hdma);
    if (hspi->DmaCircular != 0U)
    {
        if (hspi->State == HAL_SPI_STATE_BUSY_RX)
        {
            HAL_SPI_RxCpltCallback(hspi);
        }
        else
        {
            HAL_SPI_TxRxCpltCallback(hspi);
        }
    }
    else
    {
        SPI_DMANormalFinish(hspi, hspi->State);
    }
}

static void SPI_DMATxHalf(DMA_HandleTypeDef *hdma)
{
    HAL_SPI_TxHalfCpltCallback(SPI_DMAParent(hdma));
}

static void SPI_DMARxHalf(DMA_HandleTypeDef *hdma)
{
    HAL_SPI_RxHalfCpltCallback(SPI_DMAParent(hdma));
}

static void SPI_DMATxRxHalf(DMA_HandleTypeDef *hdma)
{
    SPI_HandleTypeDef *hspi = SPI_DMAParent(hdma);
    if (hspi->State == HAL_SPI_STATE_BUSY_RX)
    {
        HAL_SPI_RxHalfCpltCallback(hspi);
    }
    else
    {
        HAL_SPI_TxRxHalfCpltCallback(hspi);
    }
}

static void SPI_DMAError(DMA_HandleTypeDef *hdma)
{
    SPI_HandleTypeDef *hspi = SPI_DMAParent(hdma);
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 1, 0);
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 0, 0);
    GD32_HAL_SPI_SetInterrupts(SPI_Address(hspi),
                               GD32_HAL_SPI_INTERRUPT_ERROR, 0);
    if ((hspi->hdmatx != NULL) && (hspi->hdmatx != hdma) &&
        (hspi->hdmatx->State == HAL_DMA_STATE_BUSY))
    {
        (void)HAL_DMA_Abort(hspi->hdmatx);
    }
    if ((hspi->hdmarx != NULL) && (hspi->hdmarx != hdma) &&
        (hspi->hdmarx->State == HAL_DMA_STATE_BUSY))
    {
        (void)HAL_DMA_Abort(hspi->hdmarx);
    }
    hspi->DmaActive = 0U;
    hspi->ErrorCode |= HAL_SPI_ERROR_DMA;
    hspi->State = HAL_SPI_STATE_READY;
    HAL_SPI_ErrorCallback(hspi);
}

HAL_StatusTypeDef HAL_SPI_DMAPause(SPI_HandleTypeDef *hspi)
{
    if ((hspi == NULL) || (hspi->DmaActive == 0U))
    {
        return HAL_ERROR;
    }
    if ((hspi->DmaActive & 1U) != 0U)
    {
        GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 1, 0);
    }
    if ((hspi->DmaActive & 2U) != 0U)
    {
        GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 0, 0);
    }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_SPI_DMAResume(SPI_HandleTypeDef *hspi)
{
    if ((hspi == NULL) || (hspi->DmaActive == 0U))
    {
        return HAL_ERROR;
    }
    if ((hspi->DmaActive & 2U) != 0U)
    {
        GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 0, 1);
    }
    if ((hspi->DmaActive & 1U) != 0U)
    {
        GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 1, 1);
    }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_SPI_DMAStop(SPI_HandleTypeDef *hspi)
{
    HAL_StatusTypeDef status = HAL_OK;
    if ((hspi == NULL) || (hspi->DmaActive == 0U))
    {
        return HAL_ERROR;
    }
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 1, 0);
    GD32_HAL_SPI_SetDMARequest(SPI_Address(hspi), 0, 0);
    if (((hspi->DmaActive & 1U) != 0U) && (hspi->hdmatx != NULL) &&
        (HAL_DMA_Abort(hspi->hdmatx) != HAL_OK))
    {
        status = HAL_ERROR;
    }
    if (((hspi->DmaActive & 2U) != 0U) && (hspi->hdmarx != NULL) &&
        (HAL_DMA_Abort(hspi->hdmarx) != HAL_OK))
    {
        status = HAL_ERROR;
    }
    hspi->DmaActive = 0U;
    hspi->State = HAL_SPI_STATE_READY;
    if (status != HAL_OK)
    {
        hspi->ErrorCode |= HAL_SPI_ERROR_ABORT;
    }
    return status;
}

HAL_SPI_StateTypeDef HAL_SPI_GetState(const SPI_HandleTypeDef *hspi)
{
    return (hspi != NULL) ? hspi->State : HAL_SPI_STATE_ERROR;
}

uint32_t HAL_SPI_GetError(const SPI_HandleTypeDef *hspi)
{
    return (hspi != NULL) ? hspi->ErrorCode : HAL_SPI_ERROR_FLAG;
}
