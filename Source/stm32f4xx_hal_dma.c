#include "stm32f4xx_hal.h"

#define HAL_TIMEOUT_DMA_ABORT 5U
#define DMA_SUPPORTED_INTERRUPTS \
    (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE)
#define DMA_SUPPORTED_FLAGS \
    (GD32_HAL_DMA_FLAG_FULL | GD32_HAL_DMA_FLAG_HALF | GD32_HAL_DMA_FLAG_ERROR)

static uint32_t DMA_Address(const DMA_HandleTypeDef *hdma)
{
    const GD32_HAL_Resource *resource;

    if ((hdma != NULL) && (hdma->GD32_RESOURCE != NULL) &&
        (hdma->GD32_RESOLVED_FROM == (uintptr_t)hdma->Instance))
    {
        return hdma->GD32_INSTANCE;
    }
    resource = (hdma != NULL) ?
        GD32_HAL_ResolveInstance((uintptr_t)hdma->Instance,
                                 GD32_HAL_RESOURCE_DMA) : NULL;
    return (resource != NULL) ? resource->gd32_instance : 0U;
}

static GD32_HAL_DMADirection DMA_DirectionValue(uint32_t direction)
{
    if (direction == DMA_MEMORY_TO_PERIPH)
    {
        return GD32_HAL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    }
    if (direction == DMA_MEMORY_TO_MEMORY)
    {
        return GD32_HAL_DMA_DIRECTION_MEMORY_TO_MEMORY;
    }
    return GD32_HAL_DMA_DIRECTION_PERIPH_TO_MEMORY;
}

static uint8_t DMA_PeriphWidthValue(uint32_t alignment)
{
    if (alignment == DMA_PDATAALIGN_HALFWORD)
    {
        return 2U;
    }
    if (alignment == DMA_PDATAALIGN_WORD)
    {
        return 4U;
    }
    return 1U;
}

static uint8_t DMA_MemoryWidthValue(uint32_t alignment)
{
    if (alignment == DMA_MDATAALIGN_HALFWORD)
    {
        return 2U;
    }
    if (alignment == DMA_MDATAALIGN_WORD)
    {
        return 4U;
    }
    return 1U;
}

static uint8_t DMA_PriorityValue(uint32_t priority)
{
    if (priority == DMA_PRIORITY_MEDIUM)
    {
        return 1U;
    }
    if (priority == DMA_PRIORITY_HIGH)
    {
        return 2U;
    }
    if (priority == DMA_PRIORITY_VERY_HIGH)
    {
        return 3U;
    }
    return 0U;
}

static void DMA_FillPortConfig(const DMA_HandleTypeDef *hdma,
                               GD32_HAL_DMAConfig *config)
{
    config->direction = DMA_DirectionValue(hdma->Init.Direction);
    config->periph_increment =
        (hdma->Init.PeriphInc == DMA_PINC_ENABLE) ? 1U : 0U;
    config->memory_increment =
        (hdma->Init.MemInc == DMA_MINC_ENABLE) ? 1U : 0U;
    config->periph_width = DMA_PeriphWidthValue(hdma->Init.PeriphDataAlignment);
    config->memory_width = DMA_MemoryWidthValue(hdma->Init.MemDataAlignment);
    config->circular = (hdma->Init.Mode == DMA_CIRCULAR) ? 1U : 0U;
    config->priority = DMA_PriorityValue(hdma->Init.Priority);
}

static int DMA_IsBaseConfigValid(const DMA_InitTypeDef *init)
{
    const int direction_valid =
        (init->Direction == DMA_PERIPH_TO_MEMORY) ||
        (init->Direction == DMA_MEMORY_TO_PERIPH) ||
        (init->Direction == DMA_MEMORY_TO_MEMORY);
    const int periph_inc_valid =
        (init->PeriphInc == DMA_PINC_DISABLE) ||
        (init->PeriphInc == DMA_PINC_ENABLE);
    const int memory_inc_valid =
        (init->MemInc == DMA_MINC_DISABLE) ||
        (init->MemInc == DMA_MINC_ENABLE);
    const int periph_width_valid =
        (init->PeriphDataAlignment == DMA_PDATAALIGN_BYTE) ||
        (init->PeriphDataAlignment == DMA_PDATAALIGN_HALFWORD) ||
        (init->PeriphDataAlignment == DMA_PDATAALIGN_WORD);
    const int memory_width_valid =
        (init->MemDataAlignment == DMA_MDATAALIGN_BYTE) ||
        (init->MemDataAlignment == DMA_MDATAALIGN_HALFWORD) ||
        (init->MemDataAlignment == DMA_MDATAALIGN_WORD);
    const int mode_valid =
        (init->Mode == DMA_NORMAL) || (init->Mode == DMA_CIRCULAR) ||
        (init->Mode == DMA_PFCTRL);
    const int priority_valid =
        (init->Priority == DMA_PRIORITY_LOW) ||
        (init->Priority == DMA_PRIORITY_MEDIUM) ||
        (init->Priority == DMA_PRIORITY_HIGH) ||
        (init->Priority == DMA_PRIORITY_VERY_HIGH);

    return direction_valid && periph_inc_valid && memory_inc_valid &&
           periph_width_valid && memory_width_valid && mode_valid &&
           priority_valid;
}

static int DMA_HasUnsupportedConfig(const DMA_InitTypeDef *init)
{
    return (init->FIFOMode != DMA_FIFOMODE_DISABLE) ||
           (init->MemBurst != DMA_MBURST_SINGLE) ||
           (init->PeriphBurst != DMA_PBURST_SINGLE) ||
           ((init->Direction == DMA_MEMORY_TO_MEMORY) &&
            (init->Mode == DMA_CIRCULAR));
}

static int DMA_AreAddressesAligned(const DMA_HandleTypeDef *hdma,
                                   uint32_t source,
                                   uint32_t destination)
{
    const uint32_t periph_mask =
        (uint32_t)DMA_PeriphWidthValue(hdma->Init.PeriphDataAlignment) - 1U;
    const uint32_t memory_mask =
        (uint32_t)DMA_MemoryWidthValue(hdma->Init.MemDataAlignment) - 1U;

    if (hdma->Init.Direction == DMA_PERIPH_TO_MEMORY)
    {
        return ((source & periph_mask) == 0U) &&
               ((destination & memory_mask) == 0U);
    }
    return ((source & memory_mask) == 0U) &&
           ((destination & periph_mask) == 0U);
}

static void DMA_DisableAllInterrupts(DMA_HandleTypeDef *hdma)
{
    GD32_HAL_DMA_SetInterrupt(DMA_Address(hdma), DMA_SUPPORTED_INTERRUPTS, 0);
}

static void DMA_EndTransfer(DMA_HandleTypeDef *hdma, int clear_flags)
{
    const uint32_t address = DMA_Address(hdma);

    GD32_HAL_DMA_Disable(address);
    DMA_DisableAllInterrupts(hdma);
    if (clear_flags != 0)
    {
        GD32_HAL_DMA_ClearFlag(address, DMA_SUPPORTED_FLAGS);
    }
    GD32_HAL_DMA_Release(address, hdma);
    hdma->State = HAL_DMA_STATE_READY;
    __HAL_UNLOCK(hdma);
}

HAL_StatusTypeDef HAL_DMA_Init(DMA_HandleTypeDef *hdma)
{
    GD32_HAL_DMAConfig config;
    const GD32_HAL_Resource *resource;
    uint32_t address;
    uint32_t gd32_request;

    if ((hdma == NULL) || (hdma->Instance == NULL))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE, 0U);
        return HAL_ERROR;
    }
    gd32_request = hdma->Init.Channel;
    resource = GD32_HAL_ResolveInstance((uintptr_t)hdma->Instance,
                                        GD32_HAL_RESOURCE_DMA);
    if (resource == NULL)
    {
        resource = GD32_HAL_ResolveDMAStream((uintptr_t)hdma->Instance,
                                             hdma->Init.Channel,
                                             hdma->Init.Direction,
                                             &gd32_request);
    }
    if (resource == NULL)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE,
                           (uint32_t)(uintptr_t)hdma->Instance);
        hdma->ErrorCode = HAL_DMA_ERROR_PARAM;
        hdma->State = HAL_DMA_STATE_ERROR;
        return HAL_ERROR;
    }
    hdma->GD32_RESOURCE = resource;
    hdma->GD32_INSTANCE = resource->gd32_instance;
    hdma->gd32_dma_periph = resource->gd32_periph;
    hdma->gd32_dma_channel = resource->gd32_channel;
    hdma->GD32_REQUEST = gd32_request;
    hdma->GD32_RESOLVED_FROM = (uintptr_t)hdma->Instance;
    hdma->GD32_IRQ_NUMBER = resource->gd32_irq;
    address = resource->gd32_instance;
    if ((hdma->State == HAL_DMA_STATE_BUSY) ||
        (hdma->State == HAL_DMA_STATE_ABORT) ||
        (hdma->State == HAL_DMA_STATE_TIMEOUT))
    {
        return HAL_BUSY;
    }
    if (DMA_IsBaseConfigValid(&hdma->Init) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_DMA_INVALID_CONFIG, address);
        hdma->ErrorCode = HAL_DMA_ERROR_PARAM;
        hdma->State = HAL_DMA_STATE_ERROR;
        return HAL_ERROR;
    }
    if ((hdma->Init.Mode == DMA_PFCTRL) || DMA_HasUnsupportedConfig(&hdma->Init))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_DMA_UNSUPPORTED_CONFIG, address);
        hdma->ErrorCode = HAL_DMA_ERROR_NOT_SUPPORTED;
        hdma->State = HAL_DMA_STATE_ERROR;
        return HAL_ERROR;
    }
    if (GD32_HAL_DMA_IsRequestValid(address,
                                     hdma->GD32_REQUEST,
                                     DMA_DirectionValue(hdma->Init.Direction)) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_DMA_REQUEST_MISMATCH,
                           hdma->Init.Channel);
        hdma->ErrorCode = HAL_DMA_ERROR_REQUEST;
        hdma->State = HAL_DMA_STATE_ERROR;
        return HAL_ERROR;
    }
    if (GD32_HAL_DMA_Claim(address, hdma) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_DMA_CHANNEL_CONFLICT, address);
        hdma->ErrorCode = HAL_DMA_ERROR_CHANNEL_CONFLICT;
        return HAL_BUSY;
    }
    if (GD32_HAL_DMA_IsEnabled(address) != 0)
    {
        GD32_HAL_DMA_Release(address, hdma);
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_DMA_CHANNEL_CONFLICT, address);
        hdma->ErrorCode = HAL_DMA_ERROR_CHANNEL_CONFLICT;
        return HAL_BUSY;
    }

    hdma->State = HAL_DMA_STATE_BUSY;
    __HAL_UNLOCK(hdma);
    DMA_FillPortConfig(hdma, &config);

    if (GD32_HAL_DMA_Configure(address, &config) != 0)
    {
        GD32_HAL_DMA_Release(address, hdma);
        hdma->ErrorCode = HAL_DMA_ERROR_PARAM;
        hdma->State = HAL_DMA_STATE_ERROR;
        return HAL_ERROR;
    }

    GD32_HAL_DMA_ClearFlag(address, DMA_SUPPORTED_FLAGS);
    hdma->StreamBaseAddress = GD32_HAL_DMA_GetControllerAddress(address);
    hdma->StreamIndex = GD32_HAL_DMA_GetChannelIndex(address);
    hdma->ErrorCode = HAL_DMA_ERROR_NONE;
    hdma->State = HAL_DMA_STATE_READY;
    GD32_HAL_DMA_Release(address, hdma);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_DMA_DeInit(DMA_HandleTypeDef *hdma)
{
    uint32_t address;

    if ((hdma == NULL) || (hdma->Instance == NULL))
    {
        return HAL_ERROR;
    }
    address = DMA_Address(hdma);
    if (GD32_HAL_DMA_IsInstance(address) == 0)
    {
        return HAL_ERROR;
    }
    if (hdma->State == HAL_DMA_STATE_BUSY)
    {
        return HAL_BUSY;
    }
    if (GD32_HAL_DMA_Claim(address, hdma) == 0)
    {
        hdma->ErrorCode = HAL_DMA_ERROR_CHANNEL_CONFLICT;
        return HAL_BUSY;
    }

    GD32_HAL_DMA_Disable(address);
    DMA_DisableAllInterrupts(hdma);
    GD32_HAL_DMA_DeInit(address);
    hdma->XferCpltCallback = NULL;
    hdma->XferHalfCpltCallback = NULL;
    hdma->XferM1CpltCallback = NULL;
    hdma->XferM1HalfCpltCallback = NULL;
    hdma->XferErrorCallback = NULL;
    hdma->XferAbortCallback = NULL;
    hdma->ErrorCode = HAL_DMA_ERROR_NONE;
    hdma->State = HAL_DMA_STATE_RESET;
    hdma->StreamBaseAddress = 0U;
    hdma->StreamIndex = 0U;
    hdma->GD32_INSTANCE = 0U;
    hdma->gd32_dma_periph = 0U;
    hdma->gd32_dma_channel = 0U;
    hdma->GD32_IRQ_NUMBER = -1;
    hdma->GD32_RESOURCE = NULL;
    hdma->GD32_REQUEST = GD32_HAL_DMA_REQUEST_MEMORY;
    hdma->GD32_RESOLVED_FROM = 0U;
    __HAL_UNLOCK(hdma);
    GD32_HAL_DMA_Release(address, hdma);
    return HAL_OK;
}

static HAL_StatusTypeDef DMA_StartTransfer(DMA_HandleTypeDef *hdma,
                                           uint32_t source,
                                           uint32_t destination,
                                           uint32_t length,
                                           int interrupt_mode)
{
    GD32_HAL_DMAConfig config;
    uint32_t address;

    if ((hdma == NULL) || (hdma->Instance == NULL))
    {
        return HAL_ERROR;
    }
    if ((IS_DMA_BUFFER_SIZE(length) == 0) ||
        (DMA_AreAddressesAligned(hdma, source, destination) == 0))
    {
        hdma->ErrorCode = HAL_DMA_ERROR_PARAM;
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_DMA_INVALID_CONFIG, length);
        return HAL_ERROR;
    }

    __HAL_LOCK(hdma);
    if (hdma->State != HAL_DMA_STATE_READY)
    {
        __HAL_UNLOCK(hdma);
        return HAL_BUSY;
    }

    address = DMA_Address(hdma);
    if (GD32_HAL_DMA_Claim(address, hdma) == 0)
    {
        hdma->ErrorCode = HAL_DMA_ERROR_CHANNEL_CONFLICT;
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_DMA_CHANNEL_CONFLICT, address);
        __HAL_UNLOCK(hdma);
        return HAL_BUSY;
    }
    if (GD32_HAL_DMA_IsEnabled(address) != 0)
    {
        GD32_HAL_DMA_Release(address, hdma);
        hdma->ErrorCode = HAL_DMA_ERROR_CHANNEL_CONFLICT;
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_DMA_CHANNEL_CONFLICT, address);
        __HAL_UNLOCK(hdma);
        return HAL_BUSY;
    }

    DMA_FillPortConfig(hdma, &config);
    if (GD32_HAL_DMA_Configure(address, &config) != 0)
    {
        GD32_HAL_DMA_Release(address, hdma);
        hdma->ErrorCode = HAL_DMA_ERROR_PARAM;
        __HAL_UNLOCK(hdma);
        return HAL_ERROR;
    }

    hdma->State = HAL_DMA_STATE_BUSY;
    hdma->ErrorCode = HAL_DMA_ERROR_NONE;
    GD32_HAL_DMA_Disable(address);
    DMA_DisableAllInterrupts(hdma);
    GD32_HAL_DMA_ClearFlag(address, DMA_SUPPORTED_FLAGS);
    GD32_HAL_DMA_SetTransfer(address,
                             DMA_DirectionValue(hdma->Init.Direction),
                             source,
                             destination,
                             length);
    if (interrupt_mode != 0)
    {
        uint32_t interrupts = DMA_IT_TC | DMA_IT_TE;
        if (hdma->XferHalfCpltCallback != NULL)
        {
            interrupts |= DMA_IT_HT;
        }
        GD32_HAL_DMA_SetInterrupt(address, interrupts, 1);
    }
    GD32_HAL_DMA_Enable(address);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_DMA_Start(DMA_HandleTypeDef *hdma,
                                uint32_t SrcAddress,
                                uint32_t DstAddress,
                                uint32_t DataLength)
{
    return DMA_StartTransfer(hdma, SrcAddress, DstAddress, DataLength, 0);
}

HAL_StatusTypeDef HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma,
                                   uint32_t SrcAddress,
                                   uint32_t DstAddress,
                                   uint32_t DataLength)
{
    return DMA_StartTransfer(hdma, SrcAddress, DstAddress, DataLength, 1);
}

static HAL_StatusTypeDef DMA_AbortTransfer(DMA_HandleTypeDef *hdma,
                                           int invoke_callback)
{
    uint32_t address;
    uint32_t tick_start;

    if ((hdma == NULL) || (hdma->Instance == NULL))
    {
        return HAL_ERROR;
    }
    if (hdma->State != HAL_DMA_STATE_BUSY)
    {
        hdma->ErrorCode = HAL_DMA_ERROR_NO_XFER;
        __HAL_UNLOCK(hdma);
        return HAL_ERROR;
    }

    address = DMA_Address(hdma);
    if (invoke_callback != 0)
    {
        hdma->State = HAL_DMA_STATE_ABORT;
    }
    DMA_DisableAllInterrupts(hdma);
    GD32_HAL_DMA_Disable(address);
    tick_start = HAL_GetTick();
    while (GD32_HAL_DMA_IsEnabled(address) != 0)
    {
        if ((HAL_GetTick() - tick_start) > HAL_TIMEOUT_DMA_ABORT)
        {
            hdma->ErrorCode = HAL_DMA_ERROR_TIMEOUT;
            hdma->State = HAL_DMA_STATE_TIMEOUT;
            __HAL_UNLOCK(hdma);
            return HAL_TIMEOUT;
        }
    }

    GD32_HAL_DMA_ClearFlag(address, DMA_SUPPORTED_FLAGS);
    GD32_HAL_DMA_Release(address, hdma);
    hdma->State = HAL_DMA_STATE_READY;
    __HAL_UNLOCK(hdma);
    if ((invoke_callback != 0) && (hdma->XferAbortCallback != NULL))
    {
        hdma->XferAbortCallback(hdma);
    }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_DMA_Abort(DMA_HandleTypeDef *hdma)
{
    return DMA_AbortTransfer(hdma, 0);
}

HAL_StatusTypeDef HAL_DMA_Abort_IT(DMA_HandleTypeDef *hdma)
{
    return DMA_AbortTransfer(hdma, 1);
}

HAL_StatusTypeDef HAL_DMA_PollForTransfer(DMA_HandleTypeDef *hdma,
                                          HAL_DMA_LevelCompleteTypeDef CompleteLevel,
                                          uint32_t Timeout)
{
    uint32_t address;
    uint32_t completion_flag;
    uint32_t tick_start;

    if ((hdma == NULL) || (hdma->Instance == NULL))
    {
        return HAL_ERROR;
    }
    if (hdma->State != HAL_DMA_STATE_BUSY)
    {
        hdma->ErrorCode = HAL_DMA_ERROR_NO_XFER;
        __HAL_UNLOCK(hdma);
        return HAL_ERROR;
    }
    if ((CompleteLevel != HAL_DMA_FULL_TRANSFER) &&
        (CompleteLevel != HAL_DMA_HALF_TRANSFER))
    {
        hdma->ErrorCode = HAL_DMA_ERROR_PARAM;
        return HAL_ERROR;
    }
    if (hdma->Init.Mode == DMA_CIRCULAR)
    {
        hdma->ErrorCode = HAL_DMA_ERROR_NOT_SUPPORTED;
        return HAL_ERROR;
    }

    address = DMA_Address(hdma);
    completion_flag = (CompleteLevel == HAL_DMA_FULL_TRANSFER) ?
                      GD32_HAL_DMA_FLAG_FULL : GD32_HAL_DMA_FLAG_HALF;
    tick_start = HAL_GetTick();
    for (;;)
    {
        if (GD32_HAL_DMA_GetFlag(address, GD32_HAL_DMA_FLAG_ERROR) != 0U)
        {
            GD32_HAL_DMA_ClearFlag(address, GD32_HAL_DMA_FLAG_ERROR);
            hdma->ErrorCode |= HAL_DMA_ERROR_TE;
            DMA_EndTransfer(hdma, 1);
            return HAL_ERROR;
        }
        if (GD32_HAL_DMA_GetFlag(address, completion_flag) != 0U)
        {
            break;
        }
        if ((Timeout != HAL_MAX_DELAY) &&
            ((Timeout == 0U) || ((HAL_GetTick() - tick_start) > Timeout)))
        {
            hdma->ErrorCode = HAL_DMA_ERROR_TIMEOUT;
            DMA_EndTransfer(hdma, 1);
            return HAL_TIMEOUT;
        }
    }

    GD32_HAL_DMA_ClearFlag(address, completion_flag);
    if (CompleteLevel == HAL_DMA_FULL_TRANSFER)
    {
        DMA_EndTransfer(hdma, 1);
    }
    return HAL_OK;
}

void HAL_DMA_IRQHandler(DMA_HandleTypeDef *hdma)
{
    uint32_t address;
    uint32_t interrupts;

    if ((hdma == NULL) || (hdma->Instance == NULL) ||
        (hdma->State != HAL_DMA_STATE_BUSY))
    {
        return;
    }

    address = DMA_Address(hdma);
    interrupts = GD32_HAL_DMA_GetInterrupts(address);
    if (((interrupts & DMA_IT_TE) != 0U) &&
        (GD32_HAL_DMA_GetFlag(address, GD32_HAL_DMA_FLAG_ERROR) != 0U))
    {
        GD32_HAL_DMA_ClearFlag(address, GD32_HAL_DMA_FLAG_ERROR);
        hdma->ErrorCode |= HAL_DMA_ERROR_TE;
        DMA_EndTransfer(hdma, 1);
        if (hdma->XferErrorCallback != NULL)
        {
            hdma->XferErrorCallback(hdma);
        }
        return;
    }

    if (((interrupts & DMA_IT_HT) != 0U) &&
        (GD32_HAL_DMA_GetFlag(address, GD32_HAL_DMA_FLAG_HALF) != 0U))
    {
        GD32_HAL_DMA_ClearFlag(address, GD32_HAL_DMA_FLAG_HALF);
        if (hdma->Init.Mode != DMA_CIRCULAR)
        {
            GD32_HAL_DMA_SetInterrupt(address, DMA_IT_HT, 0);
        }
        if (hdma->XferHalfCpltCallback != NULL)
        {
            hdma->XferHalfCpltCallback(hdma);
        }
    }

    if (((interrupts & DMA_IT_TC) != 0U) &&
        (GD32_HAL_DMA_GetFlag(address, GD32_HAL_DMA_FLAG_FULL) != 0U))
    {
        GD32_HAL_DMA_ClearFlag(address, GD32_HAL_DMA_FLAG_FULL);
        if (hdma->Init.Mode != DMA_CIRCULAR)
        {
            DMA_EndTransfer(hdma, 1);
        }
        if (hdma->XferCpltCallback != NULL)
        {
            hdma->XferCpltCallback(hdma);
        }
    }
}

HAL_StatusTypeDef HAL_DMA_RegisterCallback(DMA_HandleTypeDef *hdma,
                                           HAL_DMA_CallbackIDTypeDef CallbackID,
                                           void (*pCallback)(DMA_HandleTypeDef *hdma))
{
    HAL_StatusTypeDef status = HAL_OK;

    if (hdma == NULL)
    {
        return HAL_ERROR;
    }
    __HAL_LOCK(hdma);
    if (hdma->State != HAL_DMA_STATE_READY)
    {
        status = HAL_ERROR;
    }
    else
    {
        switch (CallbackID)
        {
            case HAL_DMA_XFER_CPLT_CB_ID:
                hdma->XferCpltCallback = pCallback;
                break;
            case HAL_DMA_XFER_HALFCPLT_CB_ID:
                hdma->XferHalfCpltCallback = pCallback;
                break;
            case HAL_DMA_XFER_ERROR_CB_ID:
                hdma->XferErrorCallback = pCallback;
                break;
            case HAL_DMA_XFER_ABORT_CB_ID:
                hdma->XferAbortCallback = pCallback;
                break;
            case HAL_DMA_XFER_M1CPLT_CB_ID:
            case HAL_DMA_XFER_M1HALFCPLT_CB_ID:
                hdma->ErrorCode |= HAL_DMA_ERROR_NOT_SUPPORTED;
                status = HAL_ERROR;
                break;
            case HAL_DMA_XFER_ALL_CB_ID:
            default:
                status = HAL_ERROR;
                break;
        }
    }
    __HAL_UNLOCK(hdma);
    return status;
}

HAL_StatusTypeDef HAL_DMA_UnRegisterCallback(DMA_HandleTypeDef *hdma,
                                             HAL_DMA_CallbackIDTypeDef CallbackID)
{
    HAL_StatusTypeDef status = HAL_OK;

    if (hdma == NULL)
    {
        return HAL_ERROR;
    }
    __HAL_LOCK(hdma);
    if (hdma->State != HAL_DMA_STATE_READY)
    {
        status = HAL_ERROR;
    }
    else
    {
        switch (CallbackID)
        {
            case HAL_DMA_XFER_CPLT_CB_ID:
                hdma->XferCpltCallback = NULL;
                break;
            case HAL_DMA_XFER_HALFCPLT_CB_ID:
                hdma->XferHalfCpltCallback = NULL;
                break;
            case HAL_DMA_XFER_ERROR_CB_ID:
                hdma->XferErrorCallback = NULL;
                break;
            case HAL_DMA_XFER_ABORT_CB_ID:
                hdma->XferAbortCallback = NULL;
                break;
            case HAL_DMA_XFER_ALL_CB_ID:
                hdma->XferCpltCallback = NULL;
                hdma->XferHalfCpltCallback = NULL;
                hdma->XferM1CpltCallback = NULL;
                hdma->XferM1HalfCpltCallback = NULL;
                hdma->XferErrorCallback = NULL;
                hdma->XferAbortCallback = NULL;
                break;
            case HAL_DMA_XFER_M1CPLT_CB_ID:
            case HAL_DMA_XFER_M1HALFCPLT_CB_ID:
            default:
                status = HAL_ERROR;
                break;
        }
    }
    __HAL_UNLOCK(hdma);
    return status;
}

HAL_StatusTypeDef HAL_DMA_CleanCallbacks(DMA_HandleTypeDef *hdma)
{
    return HAL_DMA_UnRegisterCallback(hdma, HAL_DMA_XFER_ALL_CB_ID);
}

HAL_DMA_StateTypeDef HAL_DMA_GetState(const DMA_HandleTypeDef *hdma)
{
    return (hdma == NULL) ? HAL_DMA_STATE_RESET : hdma->State;
}

uint32_t HAL_DMA_GetError(const DMA_HandleTypeDef *hdma)
{
    return (hdma == NULL) ? HAL_DMA_ERROR_NONE : hdma->ErrorCode;
}
