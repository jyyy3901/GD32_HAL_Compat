#include "stm32f4xx_hal.h"

static void ADC_DMAConvCplt(DMA_HandleTypeDef *hdma);
static void ADC_DMAHalfConvCplt(DMA_HandleTypeDef *hdma);
static void ADC_DMAError(DMA_HandleTypeDef *hdma);

static uint32_t ADC_Address(const ADC_HandleTypeDef *hadc)
{
    const GD32_HAL_Resource *resource;

    if ((hadc != NULL) && (hadc->GD32_RESOURCE != NULL) &&
        (hadc->GD32_RESOURCE->stm32_instance == (uintptr_t)hadc->Instance))
    {
        return hadc->GD32_INSTANCE;
    }
    resource = (hadc != NULL) ?
        GD32_HAL_ResolveInstance((uintptr_t)hadc->Instance,
                                 GD32_HAL_RESOURCE_ADC) : NULL;
    return (resource != NULL) ? resource->gd32_instance : 0U;
}

static int ADC_FillConfig(const ADC_HandleTypeDef *hadc,
                          GD32_HAL_ADCConfig *config)
{
    static const uint8_t clockDividers[] = {2U, 4U, 6U, 8U};
    static const uint8_t resolutions[] = {12U, 10U, 8U, 6U};
    const ADC_InitTypeDef *init = &hadc->Init;

    if ((init->ClockPrescaler > ADC_CLOCK_SYNC_PCLK_DIV8) ||
        (init->Resolution > ADC_RESOLUTION_6B) ||
        ((init->DataAlign != ADC_DATAALIGN_RIGHT) &&
         (init->DataAlign != ADC_DATAALIGN_LEFT)) ||
        ((init->ScanConvMode != DISABLE) && (init->ScanConvMode != ENABLE)) ||
        ((init->ContinuousConvMode != DISABLE) &&
         (init->ContinuousConvMode != ENABLE)) ||
        ((init->DiscontinuousConvMode != DISABLE) &&
         (init->DiscontinuousConvMode != ENABLE)) ||
        ((init->DMAContinuousRequests != DISABLE) &&
         (init->DMAContinuousRequests != ENABLE)) ||
        (init->NbrOfConversion == 0U) || (init->NbrOfConversion > 16U) ||
        ((init->ScanConvMode == DISABLE) && (init->NbrOfConversion != 1U)) ||
        ((init->ContinuousConvMode == ENABLE) &&
         (init->DiscontinuousConvMode == ENABLE)) ||
        ((init->DiscontinuousConvMode == ENABLE) &&
         ((init->NbrOfDiscConversion == 0U) ||
          (init->NbrOfDiscConversion > 8U))) ||
        (init->EOCSelection == ADC_EOC_SINGLE_SEQ_CONV) ||
        ((init->EOCSelection == ADC_EOC_SINGLE_CONV) &&
         (init->NbrOfConversion != 1U)) ||
        ((init->EOCSelection != ADC_EOC_SEQ_CONV) &&
         (init->EOCSelection != ADC_EOC_SINGLE_CONV)))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_INVALID_CONFIG,
                           init->NbrOfConversion);
        return 0;
    }
    {
        const int trigger_status = GD32_HAL_ADC_MapSTM32Trigger(
            init->ExternalTrigConv,
            init->ExternalTrigConvEdge,
            &config->trigger);
        if (trigger_status == -1)
        {
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_TRIGGER_UNMAPPABLE,
                               init->ExternalTrigConv);
            return 0;
        }
        if (trigger_status == -2)
        {
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_TRIGGER_EDGE_UNSUPPORTED,
                               init->ExternalTrigConvEdge);
            return 0;
        }
    }

    config->clock_divider = clockDividers[init->ClockPrescaler];
    config->resolution_bits = resolutions[init->Resolution];
    config->align_left = (init->DataAlign == ADC_DATAALIGN_LEFT) ? 1U : 0U;
    config->scan = (init->ScanConvMode == ENABLE) ? 1U : 0U;
    config->continuous = (init->ContinuousConvMode == ENABLE) ? 1U : 0U;
    config->discontinuous = (init->DiscontinuousConvMode == ENABLE) ? 1U : 0U;
    config->discontinuous_count = config->discontinuous ?
                                  (uint8_t)init->NbrOfDiscConversion : 1U;
    config->sequence_length = (uint8_t)init->NbrOfConversion;
    return 1;
}

static HAL_StatusTypeDef ADC_PrepareStart(ADC_HandleTypeDef *hadc,
                                          int interrupt)
{
    if ((hadc == NULL) || (hadc->Instance == NULL) ||
        (GD32_HAL_ADC_IsInstance(ADC_Address(hadc)) == 0))
    {
        return HAL_ERROR;
    }
    if ((hadc->State & HAL_ADC_STATE_REG_BUSY) != 0U)
    {
        return HAL_BUSY;
    }

    __HAL_LOCK(hadc);
    if (GD32_HAL_ADC_EnableAndCalibrate(ADC_Address(hadc)) != 0)
    {
        hadc->State |= HAL_ADC_STATE_ERROR_INTERNAL;
        hadc->ErrorCode |= HAL_ADC_ERROR_INTERNAL;
        __HAL_UNLOCK(hadc);
        return HAL_ERROR;
    }
    GD32_HAL_ADC_ClearFlag(ADC_Address(hadc), GD32_HAL_ADC_FLAG_EOC);
    GD32_HAL_ADC_SetInterrupt(ADC_Address(hadc),
                              GD32_HAL_ADC_INTERRUPT_EOC, interrupt);
    hadc->State &= ~(HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC |
                     HAL_ADC_STATE_TIMEOUT | HAL_ADC_STATE_REG_OVR);
    hadc->State |= HAL_ADC_STATE_REG_BUSY;
    hadc->ErrorCode = HAL_ADC_ERROR_NONE;
    hadc->NbrOfCurrentConversionRank = 0U;
    __HAL_UNLOCK(hadc);

    if (hadc->Init.ExternalTrigConv == ADC_SOFTWARE_START)
    {
        GD32_HAL_ADC_StartSoftware(ADC_Address(hadc));
    }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef *hadc)
{
    GD32_HAL_ADCConfig config;
    const GD32_HAL_Resource *resource;

    if ((hadc == NULL) || (hadc->Instance == NULL))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE,
                           (hadc != NULL) ? ADC_Address(hadc) : 0U);
        return HAL_ERROR;
    }
    resource = GD32_HAL_ResolveInstance((uintptr_t)hadc->Instance,
                                        GD32_HAL_RESOURCE_ADC);
    if (resource == NULL)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE,
                           (uint32_t)(uintptr_t)hadc->Instance);
        return HAL_ERROR;
    }
    hadc->GD32_RESOURCE = resource;
    hadc->GD32_INSTANCE = resource->gd32_instance;
    hadc->GD32_IRQ_NUMBER = resource->gd32_irq;
    if (ADC_FillConfig(hadc, &config) == 0)
    {
        hadc->State |= HAL_ADC_STATE_ERROR_CONFIG;
        return HAL_ERROR;
    }
    if (hadc->State == HAL_ADC_STATE_RESET)
    {
        hadc->Lock = HAL_UNLOCKED;
        HAL_ADC_MspInit(hadc);
    }

    __HAL_LOCK(hadc);
    hadc->State = HAL_ADC_STATE_BUSY_INTERNAL;
    if (GD32_HAL_ADC_Configure(ADC_Address(hadc), &config) != 0)
    {
        hadc->State = HAL_ADC_STATE_ERROR_INTERNAL;
        hadc->ErrorCode = HAL_ADC_ERROR_INTERNAL;
        __HAL_UNLOCK(hadc);
        return HAL_ERROR;
    }
    hadc->NbrOfCurrentConversionRank = 0U;
    hadc->ErrorCode = HAL_ADC_ERROR_NONE;
    hadc->State = HAL_ADC_STATE_READY;
    __HAL_UNLOCK(hadc);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_ADC_DeInit(ADC_HandleTypeDef *hadc)
{
    if ((hadc == NULL) || (hadc->Instance == NULL) ||
        (GD32_HAL_ADC_IsInstance(ADC_Address(hadc)) == 0))
    {
        return HAL_ERROR;
    }
    __HAL_LOCK(hadc);
    GD32_HAL_ADC_SetInterrupt(ADC_Address(hadc),
                              GD32_HAL_ADC_INTERRUPT_EOC, 0);
    GD32_HAL_ADC_SetDMARequest(ADC_Address(hadc), 0);
    GD32_HAL_ADC_Disable(ADC_Address(hadc));
    GD32_HAL_ADC_DeInit(ADC_Address(hadc));
    HAL_ADC_MspDeInit(hadc);
    hadc->State = HAL_ADC_STATE_RESET;
    hadc->ErrorCode = HAL_ADC_ERROR_NONE;
    hadc->NbrOfCurrentConversionRank = 0U;
    hadc->GD32_INSTANCE = 0U;
    hadc->GD32_IRQ_NUMBER = -1;
    hadc->GD32_RESOURCE = NULL;
    __HAL_UNLOCK(hadc);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef *hadc,
                                        ADC_ChannelConfTypeDef *sConfig)
{
    uint8_t sample_time;
    if ((hadc == NULL) || (sConfig == NULL) || (hadc->Instance == NULL) ||
        (sConfig->Rank == 0U) || (sConfig->Rank > 16U) ||
        (sConfig->Rank > hadc->Init.NbrOfConversion) ||
        (sConfig->Channel > ADC_CHANNEL_17) || (sConfig->Offset != 0U))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_CHANNEL_UNSUPPORTED,
                           (sConfig != NULL) ? sConfig->Channel : 0U);
        return HAL_ERROR;
    }
    if ((hadc->State & HAL_ADC_STATE_REG_BUSY) != 0U)
    {
        return HAL_BUSY;
    }
    if (GD32_HAL_ADC_MapSTM32SampleTime(sConfig->SamplingTime,
                                        &sample_time) != 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_SAMPLE_TIME_UNMAPPABLE,
                           sConfig->SamplingTime);
        return HAL_ERROR;
    }
    __HAL_LOCK(hadc);
    if (GD32_HAL_ADC_ConfigChannel(ADC_Address(hadc),
                                   (uint8_t)(sConfig->Rank - 1U),
                                   (uint8_t)sConfig->Channel,
                                   sample_time) != 0)
    {
        hadc->State |= HAL_ADC_STATE_ERROR_CONFIG;
        __HAL_UNLOCK(hadc);
        return HAL_ERROR;
    }
    __HAL_UNLOCK(hadc);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *hadc)
{
    return ADC_PrepareStart(hadc, 0);
}

HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef *hadc)
{
    if ((hadc == NULL) || (hadc->Instance == NULL))
    {
        return HAL_ERROR;
    }
    __HAL_LOCK(hadc);
    GD32_HAL_ADC_SetInterrupt(ADC_Address(hadc),
                              GD32_HAL_ADC_INTERRUPT_EOC, 0);
    GD32_HAL_ADC_Disable(ADC_Address(hadc));
    hadc->State &= ~(HAL_ADC_STATE_REG_BUSY | HAL_ADC_STATE_TIMEOUT);
    hadc->State |= HAL_ADC_STATE_READY;
    __HAL_UNLOCK(hadc);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef *hadc,
                                            uint32_t Timeout)
{
    uint32_t tick_start;
    if ((hadc == NULL) || ((hadc->State & HAL_ADC_STATE_REG_BUSY) == 0U))
    {
        return HAL_ERROR;
    }
    tick_start = HAL_GetTick();
    while (GD32_HAL_ADC_GetFlag(ADC_Address(hadc),
                                GD32_HAL_ADC_FLAG_EOC) == 0U)
    {
        if ((Timeout != HAL_MAX_DELAY) &&
            ((Timeout == 0U) || ((HAL_GetTick() - tick_start) >= Timeout)))
        {
            hadc->State |= HAL_ADC_STATE_TIMEOUT;
            return HAL_TIMEOUT;
        }
    }
    GD32_HAL_ADC_ClearFlag(ADC_Address(hadc), GD32_HAL_ADC_FLAG_EOC);
    hadc->NbrOfCurrentConversionRank = hadc->Init.NbrOfConversion;
    hadc->State |= HAL_ADC_STATE_REG_EOC;
    if (hadc->Init.ContinuousConvMode == DISABLE)
    {
        hadc->State &= ~HAL_ADC_STATE_REG_BUSY;
        hadc->State |= HAL_ADC_STATE_READY;
    }
    return HAL_OK;
}

uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef *hadc)
{
    return ((hadc != NULL) && (hadc->Instance != NULL)) ?
           GD32_HAL_ADC_ReadData(ADC_Address(hadc)) : 0U;
}

HAL_StatusTypeDef HAL_ADC_Start_IT(ADC_HandleTypeDef *hadc)
{
    return ADC_PrepareStart(hadc, 1);
}

HAL_StatusTypeDef HAL_ADC_Stop_IT(ADC_HandleTypeDef *hadc)
{
    return HAL_ADC_Stop(hadc);
}

static int ADC_DMAConfigValid(const ADC_HandleTypeDef *hadc,
                              const void *data)
{
    const DMA_HandleTypeDef *hdma = hadc->DMA_Handle;
    uintptr_t memory_address;
    if ((hdma == NULL) || (hdma->Instance == NULL) || (hdma->Parent != hadc))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_DMA_LINK_INVALID, 0U);
        return 0;
    }
    memory_address = (uintptr_t)data;
    if ((GD32_HAL_ADC_IsDMAChannelValid(
             ADC_Address(hadc), GD32_HAL_DMA_MappedInstance(hdma),
             GD32_HAL_DMA_MappedRequest(hdma)) == 0) ||
        (hdma->Init.Direction != DMA_PERIPH_TO_MEMORY) ||
        (hdma->Init.PeriphInc != DMA_PINC_DISABLE) ||
        (hdma->Init.MemInc != DMA_MINC_ENABLE) ||
        (((hdma->Init.PeriphDataAlignment != DMA_PDATAALIGN_HALFWORD) ||
          (hdma->Init.MemDataAlignment != DMA_MDATAALIGN_HALFWORD)) &&
         ((hdma->Init.PeriphDataAlignment != DMA_PDATAALIGN_WORD) ||
          (hdma->Init.MemDataAlignment != DMA_MDATAALIGN_WORD))) ||
        ((hdma->Init.MemDataAlignment == DMA_MDATAALIGN_HALFWORD) &&
         ((memory_address & 1U) != 0U)) ||
        ((hdma->Init.MemDataAlignment == DMA_MDATAALIGN_WORD) &&
         ((memory_address & 3U) != 0U)) ||
        ((hadc->Init.DMAContinuousRequests == ENABLE) &&
         (hdma->Init.Mode != DMA_CIRCULAR)) ||
        ((hadc->Init.DMAContinuousRequests == DISABLE) &&
         (hdma->Init.Mode != DMA_NORMAL)) ||
        ((hadc->Init.ContinuousConvMode == ENABLE) &&
         (hadc->Init.DMAContinuousRequests != ENABLE)))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_DMA_CONFIG_MISMATCH,
                           GD32_HAL_DMA_MappedRequest(hdma));
        return 0;
    }
    return 1;
}

HAL_StatusTypeDef HAL_ADC_Start_DMA(ADC_HandleTypeDef *hadc,
                                    uint32_t *pData,
                                    uint32_t Length)
{
    HAL_StatusTypeDef status;
    DMA_HandleTypeDef *hdma;
    if ((hadc == NULL) || (pData == NULL) || (Length == 0U) ||
        ((hadc->Init.NbrOfConversion > 1U) &&
         ((Length % hadc->Init.NbrOfConversion) != 0U)))
    {
        return HAL_ERROR;
    }
    if ((hadc->State & HAL_ADC_STATE_REG_BUSY) != 0U)
    {
        return HAL_BUSY;
    }
    if (ADC_DMAConfigValid(hadc, pData) == 0)
    {
        return HAL_ERROR;
    }
    hdma = hadc->DMA_Handle;

    __HAL_LOCK(hadc);
    if (GD32_HAL_ADC_EnableAndCalibrate(ADC_Address(hadc)) != 0)
    {
        hadc->State |= HAL_ADC_STATE_ERROR_INTERNAL;
        hadc->ErrorCode |= HAL_ADC_ERROR_INTERNAL;
        __HAL_UNLOCK(hadc);
        return HAL_ERROR;
    }
    GD32_HAL_ADC_ClearFlag(ADC_Address(hadc), GD32_HAL_ADC_FLAG_EOC);
    hdma->XferCpltCallback = ADC_DMAConvCplt;
    hdma->XferHalfCpltCallback = ADC_DMAHalfConvCplt;
    hdma->XferErrorCallback = ADC_DMAError;
    hadc->State &= ~(HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC |
                     HAL_ADC_STATE_TIMEOUT | HAL_ADC_STATE_REG_OVR);
    hadc->State |= HAL_ADC_STATE_REG_BUSY;
    hadc->ErrorCode = HAL_ADC_ERROR_NONE;
    hadc->NbrOfCurrentConversionRank = 0U;
    status = HAL_DMA_Start_IT(hdma, GD32_HAL_ADC_GetDataAddress(ADC_Address(hadc)),
                              (uint32_t)(uintptr_t)pData, Length);
    if (status != HAL_OK)
    {
        hadc->State &= ~HAL_ADC_STATE_REG_BUSY;
        hadc->State |= HAL_ADC_STATE_READY | HAL_ADC_STATE_ERROR_DMA;
        hadc->ErrorCode |= HAL_ADC_ERROR_DMA;
        GD32_HAL_ADC_Disable(ADC_Address(hadc));
        __HAL_UNLOCK(hadc);
        return status;
    }
    GD32_HAL_ADC_SetDMARequest(ADC_Address(hadc), 1);
    __HAL_UNLOCK(hadc);
    if (hadc->Init.ExternalTrigConv == ADC_SOFTWARE_START)
    {
        GD32_HAL_ADC_StartSoftware(ADC_Address(hadc));
    }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_ADC_Stop_DMA(ADC_HandleTypeDef *hadc)
{
    HAL_StatusTypeDef status = HAL_OK;
    if ((hadc == NULL) || (hadc->DMA_Handle == NULL))
    {
        return HAL_ERROR;
    }
    __HAL_LOCK(hadc);
    GD32_HAL_ADC_SetDMARequest(ADC_Address(hadc), 0);
    if (hadc->DMA_Handle->State == HAL_DMA_STATE_BUSY)
    {
        status = HAL_DMA_Abort(hadc->DMA_Handle);
    }
    GD32_HAL_ADC_Disable(ADC_Address(hadc));
    hadc->State &= ~(HAL_ADC_STATE_REG_BUSY | HAL_ADC_STATE_TIMEOUT);
    hadc->State |= HAL_ADC_STATE_READY;
    if (status != HAL_OK)
    {
        hadc->State |= HAL_ADC_STATE_ERROR_DMA;
        hadc->ErrorCode |= HAL_ADC_ERROR_DMA;
    }
    __HAL_UNLOCK(hadc);
    return status;
}

void HAL_ADC_IRQHandler(ADC_HandleTypeDef *hadc)
{
    if ((hadc == NULL) || (hadc->Instance == NULL))
    {
        return;
    }
    if ((GD32_HAL_ADC_GetFlag(ADC_Address(hadc), GD32_HAL_ADC_FLAG_EOC) != 0U) &&
        ((GD32_HAL_ADC_GetInterrupts(ADC_Address(hadc)) &
          GD32_HAL_ADC_INTERRUPT_EOC) != 0U))
    {
        GD32_HAL_ADC_ClearFlag(ADC_Address(hadc), GD32_HAL_ADC_FLAG_EOC);
        hadc->NbrOfCurrentConversionRank = hadc->Init.NbrOfConversion;
        hadc->State |= HAL_ADC_STATE_REG_EOC;
        if (hadc->Init.ContinuousConvMode == DISABLE)
        {
            GD32_HAL_ADC_SetInterrupt(ADC_Address(hadc),
                                      GD32_HAL_ADC_INTERRUPT_EOC, 0);
            hadc->State &= ~HAL_ADC_STATE_REG_BUSY;
            hadc->State |= HAL_ADC_STATE_READY;
        }
        HAL_ADC_ConvCpltCallback(hadc);
    }
}

static void ADC_DMAConvCplt(DMA_HandleTypeDef *hdma)
{
    ADC_HandleTypeDef *hadc = (ADC_HandleTypeDef *)hdma->Parent;
    if (hadc == NULL)
    {
        return;
    }
    hadc->NbrOfCurrentConversionRank = hadc->Init.NbrOfConversion;
    hadc->State |= HAL_ADC_STATE_REG_EOC;
    if (hadc->Init.DMAContinuousRequests == DISABLE)
    {
        GD32_HAL_ADC_SetDMARequest(ADC_Address(hadc), 0);
        hadc->State &= ~HAL_ADC_STATE_REG_BUSY;
        hadc->State |= HAL_ADC_STATE_READY;
    }
    HAL_ADC_ConvCpltCallback(hadc);
}

static void ADC_DMAHalfConvCplt(DMA_HandleTypeDef *hdma)
{
    ADC_HandleTypeDef *hadc = (ADC_HandleTypeDef *)hdma->Parent;
    if (hadc != NULL)
    {
        HAL_ADC_ConvHalfCpltCallback(hadc);
    }
}

static void ADC_DMAError(DMA_HandleTypeDef *hdma)
{
    ADC_HandleTypeDef *hadc = (ADC_HandleTypeDef *)hdma->Parent;
    if (hadc == NULL)
    {
        return;
    }
    GD32_HAL_ADC_SetDMARequest(ADC_Address(hadc), 0);
    hadc->State &= ~HAL_ADC_STATE_REG_BUSY;
    hadc->State |= HAL_ADC_STATE_READY | HAL_ADC_STATE_ERROR_DMA;
    hadc->ErrorCode |= HAL_ADC_ERROR_DMA;
    HAL_ADC_ErrorCallback(hadc);
}

uint32_t HAL_ADC_GetState(const ADC_HandleTypeDef *hadc)
{
    return (hadc != NULL) ? hadc->State : HAL_ADC_STATE_RESET;
}

uint32_t HAL_ADC_GetError(const ADC_HandleTypeDef *hadc)
{
    return (hadc != NULL) ? hadc->ErrorCode : HAL_ADC_ERROR_INTERNAL;
}
