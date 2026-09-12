#include "stm32f4xx_hal.h"

static uint32_t UART_Address(const UART_HandleTypeDef *huart)
{
    const GD32_HAL_Resource *resource;

    if ((huart != NULL) && (huart->GD32_RESOURCE != NULL) &&
        (huart->GD32_RESOURCE->stm32_instance == (uintptr_t)huart->Instance))
    {
        return huart->GD32_INSTANCE;
    }
    resource = (huart != NULL) ?
        GD32_HAL_ResolveInstance((uintptr_t)huart->Instance,
                                 GD32_HAL_RESOURCE_UART) : NULL;
    return (resource != NULL) ? resource->gd32_instance : 0U;
}

static uint32_t UART_MapErrorFlags(uint32_t port_flags)
{
    uint32_t error = HAL_UART_ERROR_NONE;

    if ((port_flags & GD32_HAL_UART_ERROR_PE) != 0U)
    {
        error |= HAL_UART_ERROR_PE;
    }
    if ((port_flags & GD32_HAL_UART_ERROR_NE) != 0U)
    {
        error |= HAL_UART_ERROR_NE;
    }
    if ((port_flags & GD32_HAL_UART_ERROR_FE) != 0U)
    {
        error |= HAL_UART_ERROR_FE;
    }
    if ((port_flags & GD32_HAL_UART_ERROR_ORE) != 0U)
    {
        error |= HAL_UART_ERROR_ORE;
    }
    return error;
}

static int UART_IsConfigValid(const UART_HandleTypeDef *huart)
{
    return IS_UART_BAUDRATE(huart->Init.BaudRate) &&
           IS_UART_WORD_LENGTH(huart->Init.WordLength) &&
           IS_UART_STOPBITS(huart->Init.StopBits) &&
           IS_UART_PARITY(huart->Init.Parity) &&
           IS_UART_MODE(huart->Init.Mode) &&
           IS_UART_HARDWARE_FLOW_CONTROL(huart->Init.HwFlowCtl) &&
           IS_UART_OVERSAMPLING(huart->Init.OverSampling);
}

static GD32_HAL_UARTParity UART_ParityValue(uint32_t parity)
{
    if (parity == UART_PARITY_EVEN)
    {
        return GD32_HAL_UART_PARITY_EVEN;
    }
    if (parity == UART_PARITY_ODD)
    {
        return GD32_HAL_UART_PARITY_ODD;
    }
    return GD32_HAL_UART_PARITY_NONE;
}

static uint8_t UART_ModeValue(uint32_t mode)
{
    uint8_t value = 0U;

    if ((mode & UART_MODE_TX) != 0U)
    {
        value |= (uint8_t)GD32_HAL_UART_MODE_TX;
    }
    if ((mode & UART_MODE_RX) != 0U)
    {
        value |= (uint8_t)GD32_HAL_UART_MODE_RX;
    }
    return value;
}

static uint8_t UART_FlowValue(uint32_t flow)
{
    uint8_t value = 0U;

    if ((flow & UART_HWCONTROL_RTS) != 0U)
    {
        value |= (uint8_t)GD32_HAL_UART_FLOW_RTS;
    }
    if ((flow & UART_HWCONTROL_CTS) != 0U)
    {
        value |= (uint8_t)GD32_HAL_UART_FLOW_CTS;
    }
    return value;
}

static void UART_EndTxTransfer(UART_HandleTypeDef *huart)
{
    __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
    __HAL_UART_DISABLE_IT(huart, UART_IT_TC);
    huart->gState = HAL_UART_STATE_READY;
}

static void UART_EndRxTransfer(UART_HandleTypeDef *huart)
{
    __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(huart, UART_IT_PE);
    __HAL_UART_DISABLE_IT(huart, UART_IT_ERR);
    __HAL_UART_DISABLE_IT(huart, UART_IT_IDLE);
    huart->RxState = HAL_UART_STATE_READY;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;
}

static HAL_StatusTypeDef UART_WaitOnFlagUntilTimeout(UART_HandleTypeDef *huart,
                                                     GD32_HAL_UARTFlag flag,
                                                     uint32_t tick_start,
                                                     uint32_t timeout,
                                                     int receive_path)
{
    const uint32_t address = UART_Address(huart);

    while (GD32_HAL_UART_GetFlag(address, flag) == 0U)
    {
        if (receive_path != 0)
        {
            const uint32_t error_flags = GD32_HAL_UART_GetErrorFlags(address);
            if (error_flags != 0U)
            {
                GD32_HAL_UART_ClearErrors(address);
                huart->ErrorCode = UART_MapErrorFlags(error_flags);
                return HAL_ERROR;
            }
        }

        if ((timeout != HAL_MAX_DELAY) &&
            ((timeout == 0U) || ((HAL_GetTick() - tick_start) > timeout)))
        {
            return HAL_TIMEOUT;
        }
    }
    return HAL_OK;
}

static void UART_StoreReceivedData(const UART_HandleTypeDef *huart,
                                   uint8_t **buffer,
                                   uint16_t data)
{
    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) &&
        (huart->Init.Parity == UART_PARITY_NONE))
    {
        uint16_t *destination = (uint16_t *)(void *)(*buffer);
        *destination = data & 0x01FFU;
        *buffer += 2U;
    }
    else
    {
        uint8_t mask = 0xFFU;
        if ((huart->Init.WordLength == UART_WORDLENGTH_8B) &&
            (huart->Init.Parity != UART_PARITY_NONE))
        {
            mask = 0x7FU;
        }
        **buffer = (uint8_t)data & mask;
        *buffer += 1U;
    }
}

static void UART_TransmitOneIT(UART_HandleTypeDef *huart)
{
    uint16_t data;

    if (huart->gState != HAL_UART_STATE_BUSY_TX)
    {
        return;
    }

    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) &&
        (huart->Init.Parity == UART_PARITY_NONE))
    {
        const uint16_t *source = (const uint16_t *)(const void *)huart->pTxBuffPtr;
        data = *source & 0x01FFU;
        huart->pTxBuffPtr += 2U;
    }
    else
    {
        data = *huart->pTxBuffPtr & 0x00FFU;
        huart->pTxBuffPtr += 1U;
    }

    GD32_HAL_UART_WriteData(UART_Address(huart), data);
    --huart->TxXferCount;
    if (huart->TxXferCount == 0U)
    {
        __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);
        __HAL_UART_ENABLE_IT(huart, UART_IT_TC);
    }
}

static void UART_ReceiveOneIT(UART_HandleTypeDef *huart)
{
    uint16_t data;

    if (huart->RxState != HAL_UART_STATE_BUSY_RX)
    {
        return;
    }

    data = GD32_HAL_UART_ReadData(UART_Address(huart));
    UART_StoreReceivedData(huart, &huart->pRxBuffPtr, data);
    --huart->RxXferCount;
    if (huart->RxXferCount == 0U)
    {
        UART_EndRxTransfer(huart);
        huart->RxEventType = HAL_UART_RXEVENT_TC;
        HAL_UART_RxCpltCallback(huart);
    }
}

static int UART_IsDMADataConfigValid(const UART_HandleTypeDef *huart,
                                     const DMA_HandleTypeDef *hdma,
                                     int transmit)
{
    const uint32_t expected_direction = (transmit != 0) ?
                                        DMA_MEMORY_TO_PERIPH :
                                        DMA_PERIPH_TO_MEMORY;
    const uint32_t expected_periph_alignment =
        ((huart->Init.WordLength == UART_WORDLENGTH_9B) &&
         (huart->Init.Parity == UART_PARITY_NONE)) ?
        DMA_PDATAALIGN_HALFWORD : DMA_PDATAALIGN_BYTE;
    const uint32_t expected_memory_alignment =
        (expected_periph_alignment == DMA_PDATAALIGN_HALFWORD) ?
        DMA_MDATAALIGN_HALFWORD : DMA_MDATAALIGN_BYTE;

    return (hdma->Init.Direction == expected_direction) &&
           (hdma->Init.PeriphInc == DMA_PINC_DISABLE) &&
           (hdma->Init.MemInc == DMA_MINC_ENABLE) &&
           (hdma->Init.PeriphDataAlignment == expected_periph_alignment) &&
           (hdma->Init.MemDataAlignment == expected_memory_alignment) &&
           ((hdma->Init.Mode == DMA_NORMAL) ||
            (hdma->Init.Mode == DMA_CIRCULAR));
}

static int UART_IsDMAHandleValid(UART_HandleTypeDef *huart,
                                 DMA_HandleTypeDef *hdma,
                                 int transmit)
{
    const uint32_t address = UART_Address(huart);

    if (GD32_HAL_UART_IsDMACapable(address) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_UART_DMA_UNSUPPORTED, address);
        return 0;
    }
    if ((hdma == NULL) || (hdma->Instance == NULL) || (hdma->Parent != huart))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_UART_DMA_LINK_INVALID, address);
        return 0;
    }
    if ((GD32_HAL_UART_IsDMAChannelValid(
             address,
             transmit,
             GD32_HAL_DMA_MappedInstance(hdma),
             GD32_HAL_DMA_MappedRequest(hdma)) == 0) ||
        (UART_IsDMADataConfigValid(huart, hdma, transmit) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_UART_DMA_CONFIG_MISMATCH,
                           address);
        return 0;
    }
    return 1;
}

static void UART_DMATransmitCplt(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)hdma->Parent;

    if (huart == NULL)
    {
        return;
    }
    if (hdma->Init.Mode == DMA_CIRCULAR)
    {
        HAL_UART_TxCpltCallback(huart);
        return;
    }

    huart->TxXferCount = 0U;
    GD32_HAL_UART_DisableDMARequest(UART_Address(huart), 1);
    __HAL_UART_ENABLE_IT(huart, UART_IT_TC);
}

static void UART_DMATxHalfCplt(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)hdma->Parent;

    if (huart != NULL)
    {
        HAL_UART_TxHalfCpltCallback(huart);
    }
}

static void UART_DMAReceiveCplt(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)hdma->Parent;

    if (huart == NULL)
    {
        return;
    }
    if (hdma->Init.Mode != DMA_CIRCULAR)
    {
        huart->RxXferCount = 0U;
        GD32_HAL_UART_DisableDMARequest(UART_Address(huart), 0);
        UART_EndRxTransfer(huart);
    }
    huart->RxEventType = HAL_UART_RXEVENT_TC;
    HAL_UART_RxCpltCallback(huart);
}

static void UART_DMARxHalfCplt(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)hdma->Parent;

    if (huart != NULL)
    {
        huart->RxEventType = HAL_UART_RXEVENT_HT;
        HAL_UART_RxHalfCpltCallback(huart);
    }
}

static void UART_DMAError(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)hdma->Parent;
    const uint32_t address = (huart != NULL) ? UART_Address(huart) : 0U;

    if (huart == NULL)
    {
        return;
    }
    if ((huart->gState == HAL_UART_STATE_BUSY_TX) &&
        (GD32_HAL_UART_IsDMARequestActive(address, 1) != 0))
    {
        huart->TxXferCount = 0U;
        GD32_HAL_UART_DisableDMARequest(address, 1);
        if ((huart->hdmatx != NULL) && (huart->hdmatx != hdma) &&
            (huart->hdmatx->Parent == huart) &&
            (HAL_DMA_GetState(huart->hdmatx) == HAL_DMA_STATE_BUSY))
        {
            huart->hdmatx->XferAbortCallback = NULL;
            (void)HAL_DMA_Abort(huart->hdmatx);
        }
        UART_EndTxTransfer(huart);
    }
    if ((huart->RxState == HAL_UART_STATE_BUSY_RX) &&
        (GD32_HAL_UART_IsDMARequestActive(address, 0) != 0))
    {
        huart->RxXferCount = 0U;
        GD32_HAL_UART_DisableDMARequest(address, 0);
        if ((huart->hdmarx != NULL) && (huart->hdmarx != hdma) &&
            (huart->hdmarx->Parent == huart) &&
            (HAL_DMA_GetState(huart->hdmarx) == HAL_DMA_STATE_BUSY))
        {
            huart->hdmarx->XferAbortCallback = NULL;
            (void)HAL_DMA_Abort(huart->hdmarx);
        }
        UART_EndRxTransfer(huart);
    }
    huart->ErrorCode |= HAL_UART_ERROR_DMA;
    HAL_UART_ErrorCallback(huart);
}

static void UART_DMAAbortOnError(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)hdma->Parent;

    if (huart != NULL)
    {
        huart->RxXferCount = 0U;
        HAL_UART_ErrorCallback(huart);
    }
}

static HAL_StatusTypeDef UART_StopDMAHandle(UART_HandleTypeDef *huart,
                                            DMA_HandleTypeDef *hdma,
                                            int transmit,
                                            int interrupt_mode)
{
    const uint32_t address = UART_Address(huart);
    HAL_StatusTypeDef status;

    GD32_HAL_UART_DisableDMARequest(address, transmit);
    if ((hdma == NULL) || (hdma->Parent != huart))
    {
        huart->ErrorCode |= HAL_UART_ERROR_DMA;
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_UART_DMA_LINK_INVALID, address);
        return HAL_ERROR;
    }

    hdma->XferAbortCallback = NULL;
    status = (interrupt_mode != 0) ? HAL_DMA_Abort_IT(hdma) :
                                     HAL_DMA_Abort(hdma);
    if (status != HAL_OK)
    {
        huart->ErrorCode |= HAL_UART_ERROR_DMA;
        return status;
    }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart)
{
    GD32_HAL_UARTConfig config;
    const GD32_HAL_Resource *resource;
    HAL_UART_StateTypeDef previous_gstate;
    HAL_UART_StateTypeDef previous_rxstate;
    uint32_t address;

    if ((huart == NULL) || (huart->Instance == NULL))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE, 0U);
        return HAL_ERROR;
    }

    resource = GD32_HAL_ResolveInstance((uintptr_t)huart->Instance,
                                        GD32_HAL_RESOURCE_UART);
    if (resource == NULL)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE,
                           (uint32_t)(uintptr_t)huart->Instance);
        return HAL_ERROR;
    }
    huart->GD32_RESOURCE = resource;
    huart->GD32_INSTANCE = resource->gd32_instance;
    huart->GD32_IRQ_NUMBER = resource->gd32_irq;
    address = resource->gd32_instance;
    if (UART_IsConfigValid(huart) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_UART_INVALID_CONFIG, address);
        return HAL_ERROR;
    }
    if (huart->Init.OverSampling != UART_OVERSAMPLING_16)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_UART_OVERSAMPLING_UNSUPPORTED,
                           huart->Init.OverSampling);
        return HAL_ERROR;
    }
    if ((huart->Init.HwFlowCtl != UART_HWCONTROL_NONE) &&
        (GD32_HAL_UART_IsFlowControlCapable(address) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_UART_FLOW_CONTROL_UNSUPPORTED,
                           address);
        return HAL_ERROR;
    }

    previous_gstate = huart->gState;
    previous_rxstate = huart->RxState;
    if (huart->gState == HAL_UART_STATE_RESET)
    {
        huart->Lock = HAL_UNLOCKED;
        HAL_UART_MspInit(huart);
    }

    huart->gState = HAL_UART_STATE_BUSY;
    config.baud_rate = huart->Init.BaudRate;
    config.word_length = (huart->Init.WordLength == UART_WORDLENGTH_9B) ? 9U : 8U;
    config.stop_bits = (huart->Init.StopBits == UART_STOPBITS_2) ? 2U : 1U;
    config.parity = UART_ParityValue(huart->Init.Parity);
    config.mode = UART_ModeValue(huart->Init.Mode);
    config.flow_control = UART_FlowValue(huart->Init.HwFlowCtl);

    if (GD32_HAL_UART_Configure(address, &config) != 0)
    {
        huart->gState = previous_gstate;
        huart->RxState = previous_rxstate;
        if (previous_gstate == HAL_UART_STATE_RESET)
        {
            HAL_UART_MspDeInit(huart);
        }
        return HAL_ERROR;
    }

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_READY;
    huart->RxState = HAL_UART_STATE_READY;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;
    huart->RxEventType = HAL_UART_RXEVENT_TC;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_DeInit(UART_HandleTypeDef *huart)
{
    uint32_t address;

    if ((huart == NULL) || (huart->Instance == NULL))
    {
        return HAL_ERROR;
    }
    address = UART_Address(huart);
    if (GD32_HAL_UART_IsInstance(address) == 0)
    {
        return HAL_ERROR;
    }

    huart->gState = HAL_UART_STATE_BUSY;
    GD32_HAL_UART_Disable(address);
    GD32_HAL_UART_DeInit(address);
    HAL_UART_MspDeInit(huart);

    huart->pTxBuffPtr = NULL;
    huart->pRxBuffPtr = NULL;
    huart->TxXferSize = 0U;
    huart->TxXferCount = 0U;
    huart->RxXferSize = 0U;
    huart->RxXferCount = 0U;
    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_RESET;
    huart->RxState = HAL_UART_STATE_RESET;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;
    huart->RxEventType = HAL_UART_RXEVENT_TC;
    huart->GD32_INSTANCE = 0U;
    huart->GD32_IRQ_NUMBER = -1;
    huart->GD32_RESOURCE = NULL;
    __HAL_UNLOCK(huart);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart,
                                    const uint8_t *pData,
                                    uint16_t Size,
                                    uint32_t Timeout)
{
    const uint8_t *data8;
    const uint16_t *data16;
    uint32_t tick_start;

    if ((huart == NULL) || (huart->gState != HAL_UART_STATE_READY))
    {
        return (huart == NULL) ? HAL_ERROR : HAL_BUSY;
    }
    if ((pData == NULL) || (Size == 0U))
    {
        return HAL_ERROR;
    }

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_BUSY_TX;
    huart->TxXferSize = Size;
    huart->TxXferCount = Size;
    tick_start = HAL_GetTick();
    data8 = pData;
    data16 = (const uint16_t *)(const void *)pData;

    while (huart->TxXferCount > 0U)
    {
        HAL_StatusTypeDef status = UART_WaitOnFlagUntilTimeout(
            huart, GD32_HAL_UART_FLAG_TX_EMPTY, tick_start, Timeout, 0);
        if (status != HAL_OK)
        {
            huart->gState = HAL_UART_STATE_READY;
            return status;
        }

        if ((huart->Init.WordLength == UART_WORDLENGTH_9B) &&
            (huart->Init.Parity == UART_PARITY_NONE))
        {
            GD32_HAL_UART_WriteData(UART_Address(huart), *data16 & 0x01FFU);
            ++data16;
        }
        else
        {
            GD32_HAL_UART_WriteData(UART_Address(huart), *data8 & 0x00FFU);
            ++data8;
        }
        --huart->TxXferCount;
    }

    if (UART_WaitOnFlagUntilTimeout(huart,
                                    GD32_HAL_UART_FLAG_TX_COMPLETE,
                                    tick_start,
                                    Timeout,
                                    0) != HAL_OK)
    {
        huart->gState = HAL_UART_STATE_READY;
        return HAL_TIMEOUT;
    }

    huart->gState = HAL_UART_STATE_READY;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *huart,
                                   uint8_t *pData,
                                   uint16_t Size,
                                   uint32_t Timeout)
{
    uint8_t *receive_pointer;
    uint32_t tick_start;

    if ((huart == NULL) || (huart->RxState != HAL_UART_STATE_READY))
    {
        return (huart == NULL) ? HAL_ERROR : HAL_BUSY;
    }
    if ((pData == NULL) || (Size == 0U))
    {
        return HAL_ERROR;
    }

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->RxState = HAL_UART_STATE_BUSY_RX;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;
    huart->RxXferSize = Size;
    huart->RxXferCount = Size;
    tick_start = HAL_GetTick();
    receive_pointer = pData;

    while (huart->RxXferCount > 0U)
    {
        HAL_StatusTypeDef status = UART_WaitOnFlagUntilTimeout(
            huart, GD32_HAL_UART_FLAG_RX_NOT_EMPTY, tick_start, Timeout, 1);
        uint32_t error_flags;
        uint16_t data;

        if (status != HAL_OK)
        {
            huart->RxState = HAL_UART_STATE_READY;
            return status;
        }

        error_flags = GD32_HAL_UART_GetErrorFlags(UART_Address(huart));
        data = GD32_HAL_UART_ReadData(UART_Address(huart));
        if (error_flags != 0U)
        {
            huart->ErrorCode = UART_MapErrorFlags(error_flags);
            huart->RxState = HAL_UART_STATE_READY;
            return HAL_ERROR;
        }

        UART_StoreReceivedData(huart, &receive_pointer, data);
        --huart->RxXferCount;
    }

    huart->RxState = HAL_UART_STATE_READY;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *huart,
                                       const uint8_t *pData,
                                       uint16_t Size)
{
    if ((huart == NULL) || (huart->gState != HAL_UART_STATE_READY))
    {
        return (huart == NULL) ? HAL_ERROR : HAL_BUSY;
    }
    if ((pData == NULL) || (Size == 0U))
    {
        return HAL_ERROR;
    }

    huart->pTxBuffPtr = pData;
    huart->TxXferSize = Size;
    huart->TxXferCount = Size;
    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_BUSY_TX;
    __HAL_UART_ENABLE_IT(huart, UART_IT_TXE);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *huart,
                                      uint8_t *pData,
                                      uint16_t Size)
{
    if ((huart == NULL) || (huart->RxState != HAL_UART_STATE_READY))
    {
        return (huart == NULL) ? HAL_ERROR : HAL_BUSY;
    }
    if ((pData == NULL) || (Size == 0U))
    {
        return HAL_ERROR;
    }

    huart->pRxBuffPtr = pData;
    huart->RxXferSize = Size;
    huart->RxXferCount = Size;
    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->RxState = HAL_UART_STATE_BUSY_RX;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

    if (huart->Init.Parity != UART_PARITY_NONE)
    {
        __HAL_UART_ENABLE_IT(huart, UART_IT_PE);
    }
    __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);
    __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart,
                                        const uint8_t *pData,
                                        uint16_t Size)
{
    DMA_HandleTypeDef *hdma;
    HAL_StatusTypeDef status;
    uint32_t address;

    if ((huart == NULL) || (huart->Instance == NULL))
    {
        return HAL_ERROR;
    }
    if (huart->gState != HAL_UART_STATE_READY)
    {
        return HAL_BUSY;
    }
    if ((pData == NULL) || (Size == 0U))
    {
        return HAL_ERROR;
    }

    address = UART_Address(huart);
    hdma = huart->hdmatx;
    if (UART_IsDMAHandleValid(huart, hdma, 1) == 0)
    {
        huart->ErrorCode |= HAL_UART_ERROR_DMA;
        return HAL_ERROR;
    }
    if (GD32_HAL_UART_IsDMARequestActive(address, 1) != 0)
    {
        return HAL_BUSY;
    }

    huart->pTxBuffPtr = pData;
    huart->TxXferSize = Size;
    huart->TxXferCount = Size;
    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_BUSY_TX;
    hdma->XferCpltCallback = UART_DMATransmitCplt;
    hdma->XferHalfCpltCallback = UART_DMATxHalfCplt;
    hdma->XferErrorCallback = UART_DMAError;
    hdma->XferAbortCallback = NULL;

    status = HAL_DMA_Start_IT(hdma,
                              (uint32_t)(uintptr_t)pData,
                              GD32_HAL_UART_GetDataAddress(address),
                              Size);
    if (status != HAL_OK)
    {
        huart->ErrorCode = HAL_UART_ERROR_DMA;
        huart->gState = HAL_UART_STATE_READY;
        return (status == HAL_BUSY) ? HAL_BUSY : HAL_ERROR;
    }

    __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_TC);
    GD32_HAL_UART_EnableDMARequest(address, 1);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef *huart,
                                       uint8_t *pData,
                                       uint16_t Size)
{
    DMA_HandleTypeDef *hdma;
    HAL_StatusTypeDef status;
    uint32_t address;

    if ((huart == NULL) || (huart->Instance == NULL))
    {
        return HAL_ERROR;
    }
    if (huart->RxState != HAL_UART_STATE_READY)
    {
        return HAL_BUSY;
    }
    if ((pData == NULL) || (Size == 0U))
    {
        return HAL_ERROR;
    }

    address = UART_Address(huart);
    hdma = huart->hdmarx;
    if (UART_IsDMAHandleValid(huart, hdma, 0) == 0)
    {
        huart->ErrorCode |= HAL_UART_ERROR_DMA;
        return HAL_ERROR;
    }
    if (GD32_HAL_UART_IsDMARequestActive(address, 0) != 0)
    {
        return HAL_BUSY;
    }

    huart->pRxBuffPtr = pData;
    huart->RxXferSize = Size;
    huart->RxXferCount = Size;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;
    huart->RxEventType = HAL_UART_RXEVENT_TC;
    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->RxState = HAL_UART_STATE_BUSY_RX;
    hdma->XferCpltCallback = UART_DMAReceiveCplt;
    hdma->XferHalfCpltCallback = UART_DMARxHalfCplt;
    hdma->XferErrorCallback = UART_DMAError;
    hdma->XferAbortCallback = NULL;

    status = HAL_DMA_Start_IT(hdma,
                              GD32_HAL_UART_GetDataAddress(address),
                              (uint32_t)(uintptr_t)pData,
                              Size);
    if (status != HAL_OK)
    {
        huart->ErrorCode = HAL_UART_ERROR_DMA;
        huart->RxState = HAL_UART_STATE_READY;
        return (status == HAL_BUSY) ? HAL_BUSY : HAL_ERROR;
    }

    __HAL_UART_CLEAR_OREFLAG(huart);
    if (huart->Init.Parity != UART_PARITY_NONE)
    {
        __HAL_UART_ENABLE_IT(huart, UART_IT_PE);
    }
    __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);
    GD32_HAL_UART_EnableDMARequest(address, 0);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_DMAPause(UART_HandleTypeDef *huart)
{
    uint32_t address;

    if ((huart == NULL) || (huart->Instance == NULL))
    {
        return HAL_ERROR;
    }
    address = UART_Address(huart);
    if ((huart->gState == HAL_UART_STATE_BUSY_TX) &&
        (GD32_HAL_UART_IsDMARequestActive(address, 1) != 0))
    {
        GD32_HAL_UART_DisableDMARequest(address, 1);
    }
    if ((huart->RxState == HAL_UART_STATE_BUSY_RX) &&
        (GD32_HAL_UART_IsDMARequestActive(address, 0) != 0))
    {
        __HAL_UART_DISABLE_IT(huart, UART_IT_PE);
        __HAL_UART_DISABLE_IT(huart, UART_IT_ERR);
        GD32_HAL_UART_DisableDMARequest(address, 0);
    }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_DMAResume(UART_HandleTypeDef *huart)
{
    uint32_t address;

    if ((huart == NULL) || (huart->Instance == NULL))
    {
        return HAL_ERROR;
    }
    address = UART_Address(huart);
    if (huart->gState == HAL_UART_STATE_BUSY_TX)
    {
        if ((UART_IsDMAHandleValid(huart, huart->hdmatx, 1) == 0) ||
            (HAL_DMA_GetState(huart->hdmatx) != HAL_DMA_STATE_BUSY))
        {
            huart->ErrorCode |= HAL_UART_ERROR_DMA;
            return HAL_ERROR;
        }
        GD32_HAL_UART_EnableDMARequest(address, 1);
    }
    if (huart->RxState == HAL_UART_STATE_BUSY_RX)
    {
        if ((UART_IsDMAHandleValid(huart, huart->hdmarx, 0) == 0) ||
            (HAL_DMA_GetState(huart->hdmarx) != HAL_DMA_STATE_BUSY))
        {
            huart->ErrorCode |= HAL_UART_ERROR_DMA;
            return HAL_ERROR;
        }
        __HAL_UART_CLEAR_OREFLAG(huart);
        if (huart->Init.Parity != UART_PARITY_NONE)
        {
            __HAL_UART_ENABLE_IT(huart, UART_IT_PE);
        }
        __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);
        GD32_HAL_UART_EnableDMARequest(address, 0);
    }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_DMAStop(UART_HandleTypeDef *huart)
{
    HAL_StatusTypeDef result = HAL_OK;
    uint32_t address;

    if ((huart == NULL) || (huart->Instance == NULL))
    {
        return HAL_ERROR;
    }
    address = UART_Address(huart);
    if ((huart->gState == HAL_UART_STATE_BUSY_TX) &&
        (((huart->hdmatx != NULL) &&
          (huart->hdmatx->Parent == huart) &&
          (HAL_DMA_GetState(huart->hdmatx) == HAL_DMA_STATE_BUSY)) ||
         (GD32_HAL_UART_IsDMARequestActive(address, 1) != 0) ||
         (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_TC) != 0)))
    {
        if (((huart->hdmatx != NULL) &&
             (huart->hdmatx->Parent == huart) &&
             (HAL_DMA_GetState(huart->hdmatx) == HAL_DMA_STATE_BUSY)) ||
            (GD32_HAL_UART_IsDMARequestActive(address, 1) != 0))
        {
            if (UART_StopDMAHandle(huart, huart->hdmatx, 1, 0) != HAL_OK)
            {
                result = HAL_ERROR;
            }
        }
        else
        {
            GD32_HAL_UART_DisableDMARequest(address, 1);
        }
        huart->TxXferCount = 0U;
        UART_EndTxTransfer(huart);
    }
    if ((huart->RxState == HAL_UART_STATE_BUSY_RX) &&
        (((huart->hdmarx != NULL) &&
          (huart->hdmarx->Parent == huart) &&
          (HAL_DMA_GetState(huart->hdmarx) == HAL_DMA_STATE_BUSY)) ||
         (GD32_HAL_UART_IsDMARequestActive(address, 0) != 0)))
    {
        if (UART_StopDMAHandle(huart, huart->hdmarx, 0, 0) != HAL_OK)
        {
            result = HAL_ERROR;
        }
        huart->RxXferCount = 0U;
        UART_EndRxTransfer(huart);
    }
    return result;
}

void HAL_UART_IRQHandler(UART_HandleTypeDef *huart)
{
    uint32_t address;
    uint32_t error_flags;
    uint32_t active_errors = 0U;
    int rx_interrupt;
    int rx_dma_active;

    if ((huart == NULL) || (huart->Instance == NULL))
    {
        return;
    }
    address = UART_Address(huart);
    if (GD32_HAL_UART_IsInstance(address) == 0)
    {
        return;
    }

    error_flags = GD32_HAL_UART_GetErrorFlags(address);
    rx_interrupt = __HAL_UART_GET_IT_SOURCE(huart, UART_IT_RXNE);
    rx_dma_active = GD32_HAL_UART_IsDMARequestActive(address, 0);
    if (((error_flags & GD32_HAL_UART_ERROR_PE) != 0U) &&
        __HAL_UART_GET_IT_SOURCE(huart, UART_IT_PE))
    {
        active_errors |= GD32_HAL_UART_ERROR_PE;
    }
    if (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_ERR))
    {
        active_errors |= error_flags &
                         (GD32_HAL_UART_ERROR_FE |
                          GD32_HAL_UART_ERROR_NE |
                          GD32_HAL_UART_ERROR_ORE);
    }
    if (((error_flags & GD32_HAL_UART_ERROR_ORE) != 0U) && (rx_interrupt != 0))
    {
        active_errors |= GD32_HAL_UART_ERROR_ORE;
    }

    if (active_errors != 0U)
    {
        huart->ErrorCode |= UART_MapErrorFlags(active_errors);
        if ((GD32_HAL_UART_GetFlag(address, GD32_HAL_UART_FLAG_RX_NOT_EMPTY) != 0U) &&
            (rx_interrupt != 0) && (huart->RxState == HAL_UART_STATE_BUSY_RX))
        {
            UART_ReceiveOneIT(huart);
        }
        else
        {
            GD32_HAL_UART_ClearErrors(address);
        }

        if (((huart->ErrorCode & HAL_UART_ERROR_ORE) != 0U) ||
            (rx_dma_active != 0))
        {
            UART_EndRxTransfer(huart);
            if (rx_dma_active != 0)
            {
                GD32_HAL_UART_DisableDMARequest(address, 0);
                if ((huart->hdmarx != NULL) &&
                    (huart->hdmarx->Parent == huart))
                {
                    huart->hdmarx->XferAbortCallback = UART_DMAAbortOnError;
                    if (HAL_DMA_Abort_IT(huart->hdmarx) != HAL_OK)
                    {
                        UART_DMAAbortOnError(huart->hdmarx);
                    }
                }
                else
                {
                    huart->ErrorCode |= HAL_UART_ERROR_DMA;
                    HAL_UART_ErrorCallback(huart);
                }
            }
            else
            {
                HAL_UART_ErrorCallback(huart);
            }
        }
        else
        {
            HAL_UART_ErrorCallback(huart);
            huart->ErrorCode = HAL_UART_ERROR_NONE;
        }
        return;
    }

    if ((GD32_HAL_UART_GetFlag(address, GD32_HAL_UART_FLAG_RX_NOT_EMPTY) != 0U) &&
        (rx_interrupt != 0))
    {
        UART_ReceiveOneIT(huart);
        return;
    }

    if ((GD32_HAL_UART_GetFlag(address, GD32_HAL_UART_FLAG_TX_EMPTY) != 0U) &&
        __HAL_UART_GET_IT_SOURCE(huart, UART_IT_TXE))
    {
        UART_TransmitOneIT(huart);
        return;
    }

    if ((GD32_HAL_UART_GetFlag(address, GD32_HAL_UART_FLAG_TX_COMPLETE) != 0U) &&
        __HAL_UART_GET_IT_SOURCE(huart, UART_IT_TC))
    {
        UART_EndTxTransfer(huart);
        HAL_UART_TxCpltCallback(huart);
    }
}

static HAL_StatusTypeDef UART_AbortTransfers(UART_HandleTypeDef *huart,
                                             int abort_tx,
                                             int abort_rx,
                                             int interrupt_mode)
{
    HAL_StatusTypeDef result = HAL_OK;
    uint32_t address;
    int tx_dma_active;
    int rx_dma_active;

    if ((huart == NULL) || (huart->Instance == NULL))
    {
        return HAL_ERROR;
    }
    address = UART_Address(huart);
    tx_dma_active = (abort_tx != 0) &&
                    ((GD32_HAL_UART_IsDMARequestActive(address, 1) != 0) ||
                     ((huart->hdmatx != NULL) &&
                      (huart->hdmatx->Parent == huart) &&
                      (HAL_DMA_GetState(huart->hdmatx) == HAL_DMA_STATE_BUSY)));
    rx_dma_active = (abort_rx != 0) &&
                    ((GD32_HAL_UART_IsDMARequestActive(address, 0) != 0) ||
                     ((huart->hdmarx != NULL) &&
                      (huart->hdmarx->Parent == huart) &&
                      (HAL_DMA_GetState(huart->hdmarx) == HAL_DMA_STATE_BUSY)));

    if (abort_tx != 0)
    {
        UART_EndTxTransfer(huart);
        if ((tx_dma_active != 0) &&
            (UART_StopDMAHandle(huart,
                                huart->hdmatx,
                                1,
                                interrupt_mode) != HAL_OK))
        {
            result = HAL_ERROR;
        }
        huart->TxXferCount = 0U;
    }
    if (abort_rx != 0)
    {
        UART_EndRxTransfer(huart);
        if ((rx_dma_active != 0) &&
            (UART_StopDMAHandle(huart,
                                huart->hdmarx,
                                0,
                                interrupt_mode) != HAL_OK))
        {
            result = HAL_ERROR;
        }
        huart->RxXferCount = 0U;
    }
    return result;
}

HAL_StatusTypeDef HAL_UART_Abort(UART_HandleTypeDef *huart)
{
    HAL_StatusTypeDef status = UART_AbortTransfers(huart, 1, 1, 0);

    if (status == HAL_OK)
    {
        huart->ErrorCode = HAL_UART_ERROR_NONE;
    }
    return status;
}

HAL_StatusTypeDef HAL_UART_AbortTransmit(UART_HandleTypeDef *huart)
{
    return UART_AbortTransfers(huart, 1, 0, 0);
}

HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef *huart)
{
    return UART_AbortTransfers(huart, 0, 1, 0);
}

HAL_StatusTypeDef HAL_UART_Abort_IT(UART_HandleTypeDef *huart)
{
    HAL_StatusTypeDef status = UART_AbortTransfers(huart, 1, 1, 1);

    if (status == HAL_OK)
    {
        huart->ErrorCode = HAL_UART_ERROR_NONE;
        HAL_UART_AbortCpltCallback(huart);
    }
    return status;
}

HAL_StatusTypeDef HAL_UART_AbortTransmit_IT(UART_HandleTypeDef *huart)
{
    HAL_StatusTypeDef status = UART_AbortTransfers(huart, 1, 0, 1);
    if (status == HAL_OK)
    {
        HAL_UART_AbortTransmitCpltCallback(huart);
    }
    return status;
}

HAL_StatusTypeDef HAL_UART_AbortReceive_IT(UART_HandleTypeDef *huart)
{
    HAL_StatusTypeDef status = UART_AbortTransfers(huart, 0, 1, 1);
    if (status == HAL_OK)
    {
        HAL_UART_AbortReceiveCpltCallback(huart);
    }
    return status;
}

HAL_UART_StateTypeDef HAL_UART_GetState(const UART_HandleTypeDef *huart)
{
    if (huart == NULL)
    {
        return HAL_UART_STATE_RESET;
    }
    return (HAL_UART_StateTypeDef)((uint32_t)huart->gState |
                                   (uint32_t)huart->RxState);
}

uint32_t HAL_UART_GetError(const UART_HandleTypeDef *huart)
{
    return (huart == NULL) ? HAL_UART_ERROR_NONE : huart->ErrorCode;
}
