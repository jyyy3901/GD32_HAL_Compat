#include "stm32f4xx_hal.h"

static uint32_t TIM_Address(const TIM_HandleTypeDef *htim)
{
    return (uint32_t)(uintptr_t)htim->Instance;
}

static int TIM_ChannelIndex(uint32_t channel, uint8_t *index)
{
    if (!IS_TIM_CHANNELS(channel))
    {
        return 0;
    }
    *index = (uint8_t)(channel >> 2U);
    return 1;
}

static HAL_TIM_ActiveChannel TIM_ActiveFromIndex(uint8_t index)
{
    return (HAL_TIM_ActiveChannel)(1UL << index);
}

static int TIM_BaseConfig(const TIM_HandleTypeDef *htim,
                          GD32_HAL_TIMERBaseConfig *config)
{
    if ((htim->Init.Prescaler > 0xFFFFU) ||
        (htim->Init.Period > 0xFFFFU) ||
        (htim->Init.RepetitionCounter > 0xFFU))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_16BIT_RANGE,
                           (htim->Init.Period > 0xFFFFU) ?
                           htim->Init.Period :
                           (htim->Init.Prescaler > 0xFFFFU) ?
                           htim->Init.Prescaler : htim->Init.RepetitionCounter);
        return 0;
    }

    config->prescaler = (uint16_t)htim->Init.Prescaler;
    config->period = (uint16_t)htim->Init.Period;
    config->repetition = (uint8_t)htim->Init.RepetitionCounter;
    config->auto_reload_preload =
        (htim->Init.AutoReloadPreload == TIM_AUTORELOAD_PRELOAD_ENABLE) ? 1U : 0U;

    if (htim->Init.CounterMode == TIM_COUNTERMODE_UP)
    {
        config->alignment = GD32_HAL_TIMER_ALIGNMENT_EDGE;
        config->direction = GD32_HAL_TIMER_DIRECTION_UP;
    }
    else if (htim->Init.CounterMode == TIM_COUNTERMODE_DOWN)
    {
        config->alignment = GD32_HAL_TIMER_ALIGNMENT_EDGE;
        config->direction = GD32_HAL_TIMER_DIRECTION_DOWN;
    }
    else if (htim->Init.CounterMode == TIM_COUNTERMODE_CENTERALIGNED1)
    {
        config->alignment = GD32_HAL_TIMER_ALIGNMENT_CENTER_DOWN;
        config->direction = GD32_HAL_TIMER_DIRECTION_UP;
    }
    else if (htim->Init.CounterMode == TIM_COUNTERMODE_CENTERALIGNED2)
    {
        config->alignment = GD32_HAL_TIMER_ALIGNMENT_CENTER_UP;
        config->direction = GD32_HAL_TIMER_DIRECTION_UP;
    }
    else if (htim->Init.CounterMode == TIM_COUNTERMODE_CENTERALIGNED3)
    {
        config->alignment = GD32_HAL_TIMER_ALIGNMENT_CENTER_BOTH;
        config->direction = GD32_HAL_TIMER_DIRECTION_UP;
    }
    else
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_INVALID_CONFIG,
                           htim->Init.CounterMode);
        return 0;
    }

    if (htim->Init.ClockDivision == TIM_CLOCKDIVISION_DIV1)
    {
        config->clock_division = 1U;
    }
    else if (htim->Init.ClockDivision == TIM_CLOCKDIVISION_DIV2)
    {
        config->clock_division = 2U;
    }
    else if (htim->Init.ClockDivision == TIM_CLOCKDIVISION_DIV4)
    {
        config->clock_division = 4U;
    }
    else
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_INVALID_CONFIG,
                           htim->Init.ClockDivision);
        return 0;
    }
    if ((htim->Init.AutoReloadPreload != TIM_AUTORELOAD_PRELOAD_DISABLE) &&
        (htim->Init.AutoReloadPreload != TIM_AUTORELOAD_PRELOAD_ENABLE))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_INVALID_CONFIG,
                           htim->Init.AutoReloadPreload);
        return 0;
    }
    return 1;
}

static void TIM_SetChannelStates(TIM_HandleTypeDef *htim,
                                 HAL_TIM_ChannelStateTypeDef state)
{
    uint8_t channel;
    for (channel = 0U; channel < 4U; ++channel)
    {
        htim->ChannelState[channel] = state;
        htim->ChannelNState[channel] = state;
    }
}

static HAL_StatusTypeDef TIM_Init(TIM_HandleTypeDef *htim,
                                  void (*msp_init)(TIM_HandleTypeDef *))
{
    GD32_HAL_TIMERBaseConfig config;
    const HAL_TIM_StateTypeDef previous_state =
        (htim != NULL) ? htim->State : HAL_TIM_STATE_RESET;

    if ((htim == NULL) || (htim->Instance == NULL))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE, 0U);
        return HAL_ERROR;
    }
    if (GD32_HAL_TIMER_IsInstance(TIM_Address(htim)) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE, TIM_Address(htim));
        return HAL_ERROR;
    }
    if (TIM_BaseConfig(htim, &config) == 0)
    {
        return HAL_ERROR;
    }
    if (htim->State == HAL_TIM_STATE_RESET)
    {
        htim->Lock = HAL_UNLOCKED;
        msp_init(htim);
    }
    htim->State = HAL_TIM_STATE_BUSY;
    if (GD32_HAL_TIMER_ConfigureBase(TIM_Address(htim), &config) != 0)
    {
        htim->State = previous_state;
        return HAL_ERROR;
    }
    htim->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
    TIM_SetChannelStates(htim, HAL_TIM_CHANNEL_STATE_READY);
    htim->DMABurstState = HAL_DMA_BURST_STATE_READY;
    htim->State = HAL_TIM_STATE_READY;
    return HAL_OK;
}

static HAL_StatusTypeDef TIM_DeInit(TIM_HandleTypeDef *htim,
                                    void (*msp_deinit)(TIM_HandleTypeDef *))
{
    if ((htim == NULL) || (htim->Instance == NULL) ||
        (GD32_HAL_TIMER_IsInstance(TIM_Address(htim)) == 0))
    {
        return HAL_ERROR;
    }
    htim->State = HAL_TIM_STATE_BUSY;
    GD32_HAL_TIMER_Disable(TIM_Address(htim));
    GD32_HAL_TIMER_DeInit(TIM_Address(htim));
    msp_deinit(htim);
    TIM_SetChannelStates(htim, HAL_TIM_CHANNEL_STATE_RESET);
    htim->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
    htim->DMABurstState = HAL_DMA_BURST_STATE_RESET;
    htim->State = HAL_TIM_STATE_RESET;
    htim->Lock = HAL_UNLOCKED;
    return HAL_OK;
}

static int TIM_OCConfig(const TIM_OC_InitTypeDef *source,
                        GD32_HAL_TIMEROCConfig *destination,
                        int pwm)
{
    if ((source == NULL) || (source->Pulse > 0xFFFFU) ||
        ((source->OCPolarity != TIM_OCPOLARITY_HIGH) &&
         (source->OCPolarity != TIM_OCPOLARITY_LOW)) ||
        ((source->OCFastMode != TIM_OCFAST_DISABLE) &&
         (source->OCFastMode != TIM_OCFAST_ENABLE)) ||
        ((source->OCIdleState != TIM_OCIDLESTATE_RESET) &&
         (source->OCIdleState != TIM_OCIDLESTATE_SET)) ||
        (source->OCNPolarity != TIM_OCNPOLARITY_HIGH) ||
        (source->OCNIdleState != TIM_OCNIDLESTATE_RESET))
    {
        return 0;
    }

    if (source->OCMode == TIM_OCMODE_TIMING) { destination->mode = GD32_HAL_TIMER_OC_TIMING; }
    else if (source->OCMode == TIM_OCMODE_ACTIVE) { destination->mode = GD32_HAL_TIMER_OC_ACTIVE; }
    else if (source->OCMode == TIM_OCMODE_INACTIVE) { destination->mode = GD32_HAL_TIMER_OC_INACTIVE; }
    else if (source->OCMode == TIM_OCMODE_TOGGLE) { destination->mode = GD32_HAL_TIMER_OC_TOGGLE; }
    else if (source->OCMode == TIM_OCMODE_FORCED_INACTIVE) { destination->mode = GD32_HAL_TIMER_OC_FORCED_INACTIVE; }
    else if (source->OCMode == TIM_OCMODE_FORCED_ACTIVE) { destination->mode = GD32_HAL_TIMER_OC_FORCED_ACTIVE; }
    else if (source->OCMode == TIM_OCMODE_PWM1) { destination->mode = GD32_HAL_TIMER_OC_PWM1; }
    else if (source->OCMode == TIM_OCMODE_PWM2) { destination->mode = GD32_HAL_TIMER_OC_PWM2; }
    else { return 0; }

    if ((pwm != 0) &&
        (source->OCMode != TIM_OCMODE_PWM1) &&
        (source->OCMode != TIM_OCMODE_PWM2))
    {
        return 0;
    }
    destination->pulse = (uint16_t)source->Pulse;
    destination->polarity = (source->OCPolarity == TIM_OCPOLARITY_LOW) ?
                            GD32_HAL_TIMER_POLARITY_FALLING_LOW :
                            GD32_HAL_TIMER_POLARITY_RISING_HIGH;
    destination->fast = (source->OCFastMode == TIM_OCFAST_ENABLE) ? 1U : 0U;
    destination->idle_high = (source->OCIdleState == TIM_OCIDLESTATE_SET) ? 1U : 0U;
    destination->preload = (pwm != 0) ? 1U : 0U;
    return 1;
}

static HAL_StatusTypeDef TIM_ConfigOC(TIM_HandleTypeDef *htim,
                                      const TIM_OC_InitTypeDef *sConfig,
                                      uint32_t channel,
                                      int pwm)
{
    GD32_HAL_TIMEROCConfig config;
    uint8_t index = 0U;

    if ((htim == NULL) || (TIM_ChannelIndex(channel, &index) == 0) ||
        (GD32_HAL_TIMER_IsChannelValid(TIM_Address(htim), index) == 0) ||
        (TIM_OCConfig(sConfig, &config, pwm) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_INVALID_CONFIG,
                           (htim != NULL) ? TIM_Address(htim) | index : 0U);
        return HAL_ERROR;
    }
    __HAL_LOCK(htim);
    if (GD32_HAL_TIMER_ConfigureOutput(TIM_Address(htim), index, &config) != 0)
    {
        __HAL_UNLOCK(htim);
        return HAL_ERROR;
    }
    htim->ChannelState[index] = HAL_TIM_CHANNEL_STATE_READY;
    __HAL_UNLOCK(htim);
    return HAL_OK;
}

static HAL_StatusTypeDef TIM_StartChannel(TIM_HandleTypeDef *htim,
                                          uint32_t channel,
                                          int interrupt)
{
    uint8_t index = 0U;
    if ((htim == NULL) || (TIM_ChannelIndex(channel, &index) == 0) ||
        (GD32_HAL_TIMER_IsChannelValid(TIM_Address(htim), index) == 0))
    {
        return HAL_ERROR;
    }
    if (htim->ChannelState[index] == HAL_TIM_CHANNEL_STATE_BUSY)
    {
        return HAL_BUSY;
    }
    if (htim->ChannelState[index] != HAL_TIM_CHANNEL_STATE_READY)
    {
        return HAL_ERROR;
    }
    htim->ChannelState[index] = HAL_TIM_CHANNEL_STATE_BUSY;
    if (interrupt != 0)
    {
        __HAL_TIM_ENABLE_IT(htim, (uint32_t)TIM_IT_CC1 << index);
    }
    GD32_HAL_TIMER_SetChannel(TIM_Address(htim), index, 1);
    if (GD32_HAL_TIMER_IsTriggerSlaveMode(TIM_Address(htim)) == 0)
    {
        GD32_HAL_TIMER_Enable(TIM_Address(htim));
    }
    return HAL_OK;
}

static HAL_StatusTypeDef TIM_StopChannel(TIM_HandleTypeDef *htim,
                                         uint32_t channel,
                                         int interrupt)
{
    uint8_t index = 0U;
    if ((htim == NULL) || (TIM_ChannelIndex(channel, &index) == 0) ||
        (GD32_HAL_TIMER_IsChannelValid(TIM_Address(htim), index) == 0))
    {
        return HAL_ERROR;
    }
    if (interrupt != 0)
    {
        __HAL_TIM_DISABLE_IT(htim, (uint32_t)TIM_IT_CC1 << index);
    }
    GD32_HAL_TIMER_SetChannel(TIM_Address(htim), index, 0);
    GD32_HAL_TIMER_DisableIfIdle(TIM_Address(htim));
    htim->ChannelState[index] = HAL_TIM_CHANNEL_STATE_READY;
    return HAL_OK;
}

static int TIM_ICConfig(const TIM_IC_InitTypeDef *source,
                        GD32_HAL_TIMERICConfig *destination)
{
    if ((source == NULL) || (source->ICFilter > 15U))
    {
        return 0;
    }
    if (source->ICPolarity == TIM_ICPOLARITY_RISING)
    {
        destination->polarity = GD32_HAL_TIMER_POLARITY_RISING_HIGH;
    }
    else if (source->ICPolarity == TIM_ICPOLARITY_FALLING)
    {
        destination->polarity = GD32_HAL_TIMER_POLARITY_FALLING_LOW;
    }
    else if (source->ICPolarity == TIM_ICPOLARITY_BOTHEDGE)
    {
        destination->polarity = GD32_HAL_TIMER_POLARITY_BOTH;
    }
    else { return 0; }

    if (source->ICSelection == TIM_ICSELECTION_DIRECTTI)
    {
        destination->selection = GD32_HAL_TIMER_IC_DIRECT;
    }
    else if (source->ICSelection == TIM_ICSELECTION_INDIRECTTI)
    {
        destination->selection = GD32_HAL_TIMER_IC_INDIRECT;
    }
    else if (source->ICSelection == TIM_ICSELECTION_TRC)
    {
        destination->selection = GD32_HAL_TIMER_IC_TRIGGER;
    }
    else { return 0; }

    if (source->ICPrescaler == TIM_ICPSC_DIV1) { destination->prescaler = 0U; }
    else if (source->ICPrescaler == TIM_ICPSC_DIV2) { destination->prescaler = 1U; }
    else if (source->ICPrescaler == TIM_ICPSC_DIV4) { destination->prescaler = 2U; }
    else if (source->ICPrescaler == TIM_ICPSC_DIV8) { destination->prescaler = 3U; }
    else { return 0; }
    destination->filter = (uint8_t)source->ICFilter;
    return 1;
}

static int TIM_DMAIndex(const TIM_HandleTypeDef *htim,
                        const DMA_HandleTypeDef *hdma,
                        uint8_t *index)
{
    uint8_t i;
    for (i = 0U; i <= TIM_DMA_ID_TRIGGER; ++i)
    {
        if (htim->hdma[i] == hdma)
        {
            *index = i;
            return 1;
        }
    }
    return 0;
}

static int TIM_DMAConfigValid(const TIM_HandleTypeDef *htim,
                              const DMA_HandleTypeDef *hdma,
                              uint8_t dma_id,
                              int capture)
{
    const uint32_t expected_direction =
        (capture != 0) ? DMA_PERIPH_TO_MEMORY : DMA_MEMORY_TO_PERIPH;

    if ((hdma == NULL) || (hdma->Instance == NULL) || (hdma->Parent != htim))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_DMA_LINK_INVALID,
                           TIM_Address(htim));
        return 0;
    }
    if ((hdma->Init.Direction != expected_direction) ||
        (hdma->Init.PeriphInc != DMA_PINC_DISABLE) ||
        (hdma->Init.MemInc != DMA_MINC_ENABLE) ||
        (hdma->Init.PeriphDataAlignment != DMA_PDATAALIGN_WORD) ||
        (hdma->Init.MemDataAlignment != DMA_MDATAALIGN_WORD) ||
        ((hdma->Init.Mode != DMA_NORMAL) && (hdma->Init.Mode != DMA_CIRCULAR)) ||
        (GD32_HAL_TIMER_IsDMAChannelValid(
             TIM_Address(htim),
             (GD32_HAL_TIMERDMARequest)dma_id,
             (uint32_t)(uintptr_t)hdma->Instance,
             hdma->Init.Channel) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_DMA_CONFIG_MISMATCH,
                           TIM_Address(htim) | dma_id);
        return 0;
    }
    return 1;
}

static int TIM_DMAOutputDataValid(const uint32_t *data, uint16_t length)
{
    uint16_t i;
    if ((data == NULL) || (length == 0U))
    {
        return 0;
    }
    for (i = 0U; i < length; ++i)
    {
        if (data[i] > 0xFFFFU)
        {
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_16BIT_RANGE, data[i]);
            return 0;
        }
    }
    return 1;
}

static void TIM_DMABaseCplt(DMA_HandleTypeDef *hdma)
{
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)hdma->Parent;
    if (htim == NULL) { return; }
    if (hdma->Init.Mode != DMA_CIRCULAR)
    {
        htim->State = HAL_TIM_STATE_READY;
    }
    HAL_TIM_PeriodElapsedCallback(htim);
}

static void TIM_DMABaseHalfCplt(DMA_HandleTypeDef *hdma)
{
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)hdma->Parent;
    if (htim != NULL) { HAL_TIM_PeriodElapsedHalfCpltCallback(htim); }
}

static void TIM_DMAChannelCplt(DMA_HandleTypeDef *hdma)
{
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)hdma->Parent;
    uint8_t dma_id = 0U;
    uint8_t index = 0U;
    if ((htim == NULL) || (TIM_DMAIndex(htim, hdma, &dma_id) == 0) ||
        (dma_id < TIM_DMA_ID_CC1) || (dma_id > TIM_DMA_ID_CC4))
    {
        return;
    }
    index = (uint8_t)(dma_id - TIM_DMA_ID_CC1);
    htim->Channel = TIM_ActiveFromIndex(index);
    if (hdma->Init.Mode != DMA_CIRCULAR)
    {
        htim->ChannelState[index] = HAL_TIM_CHANNEL_STATE_READY;
    }
    if (GD32_HAL_TIMER_IsChannelInput(TIM_Address(htim), index) != 0)
    {
        HAL_TIM_IC_CaptureCallback(htim);
    }
    else
    {
        HAL_TIM_PWM_PulseFinishedCallback(htim);
    }
    htim->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
}

static void TIM_DMAChannelHalfCplt(DMA_HandleTypeDef *hdma)
{
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)hdma->Parent;
    uint8_t dma_id = 0U;
    if ((htim == NULL) || (TIM_DMAIndex(htim, hdma, &dma_id) == 0) ||
        (dma_id < TIM_DMA_ID_CC1) || (dma_id > TIM_DMA_ID_CC4))
    {
        return;
    }
    htim->Channel = TIM_ActiveFromIndex((uint8_t)(dma_id - TIM_DMA_ID_CC1));
    if (GD32_HAL_TIMER_IsChannelInput(TIM_Address(htim),
                                      (uint8_t)(dma_id - TIM_DMA_ID_CC1)) != 0)
    {
        HAL_TIM_IC_CaptureHalfCpltCallback(htim);
    }
    else
    {
        HAL_TIM_PWM_PulseFinishedHalfCpltCallback(htim);
    }
    htim->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
}

static void TIM_DMAError(DMA_HandleTypeDef *hdma)
{
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)hdma->Parent;
    uint8_t dma_id = 0U;
    if (htim == NULL) { return; }
    if (TIM_DMAIndex(htim, hdma, &dma_id) != 0)
    {
        GD32_HAL_TIMER_SetDMARequest(TIM_Address(htim),
                                     (GD32_HAL_TIMERDMARequest)dma_id, 0);
        if ((dma_id >= TIM_DMA_ID_CC1) && (dma_id <= TIM_DMA_ID_CC4))
        {
            const uint8_t index = (uint8_t)(dma_id - TIM_DMA_ID_CC1);
            GD32_HAL_TIMER_SetChannel(TIM_Address(htim), index, 0);
            htim->ChannelState[index] = HAL_TIM_CHANNEL_STATE_READY;
        }
    }
    htim->State = HAL_TIM_STATE_READY;
    GD32_HAL_TIMER_DisableIfIdle(TIM_Address(htim));
    HAL_TIM_ErrorCallback(htim);
}

static HAL_StatusTypeDef TIM_StartChannelDMA(TIM_HandleTypeDef *htim,
                                             uint32_t channel,
                                             uint32_t *data,
                                             uint16_t length,
                                             int capture)
{
    DMA_HandleTypeDef *hdma;
    uint8_t index = 0U;
    uint8_t dma_id = 0U;
    uint32_t peripheral;
    HAL_StatusTypeDef status;

    if ((htim == NULL) || (TIM_ChannelIndex(channel, &index) == 0) ||
        (GD32_HAL_TIMER_IsChannelValid(TIM_Address(htim), index) == 0) ||
        (length == 0U) || (data == NULL))
    {
        return HAL_ERROR;
    }
    if ((capture == 0) && (TIM_DMAOutputDataValid(data, length) == 0))
    {
        return HAL_ERROR;
    }
    if (htim->ChannelState[index] == HAL_TIM_CHANNEL_STATE_BUSY)
    {
        return HAL_BUSY;
    }
    if (htim->ChannelState[index] != HAL_TIM_CHANNEL_STATE_READY)
    {
        return HAL_ERROR;
    }

    dma_id = (uint8_t)(TIM_DMA_ID_CC1 + index);
    hdma = htim->hdma[dma_id];
    if (TIM_DMAConfigValid(htim, hdma, dma_id, capture) == 0)
    {
        return HAL_ERROR;
    }
    htim->ChannelState[index] = HAL_TIM_CHANNEL_STATE_BUSY;
    hdma->XferCpltCallback = TIM_DMAChannelCplt;
    hdma->XferHalfCpltCallback = TIM_DMAChannelHalfCplt;
    hdma->XferErrorCallback = TIM_DMAError;
    peripheral = GD32_HAL_TIMER_GetDMADataAddress(
        TIM_Address(htim), (GD32_HAL_TIMERDMARequest)dma_id);
    if (capture != 0)
    {
        status = HAL_DMA_Start_IT(hdma,
                                  peripheral,
                                  (uint32_t)(uintptr_t)data,
                                  length);
    }
    else
    {
        status = HAL_DMA_Start_IT(hdma,
                                  (uint32_t)(uintptr_t)data,
                                  peripheral,
                                  length);
    }
    if (status != HAL_OK)
    {
        htim->ChannelState[index] = HAL_TIM_CHANNEL_STATE_READY;
        return status;
    }
    GD32_HAL_TIMER_SetDMARequest(TIM_Address(htim),
                                 (GD32_HAL_TIMERDMARequest)dma_id, 1);
    GD32_HAL_TIMER_SetChannel(TIM_Address(htim), index, 1);
    if (GD32_HAL_TIMER_IsTriggerSlaveMode(TIM_Address(htim)) == 0)
    {
        GD32_HAL_TIMER_Enable(TIM_Address(htim));
    }
    return HAL_OK;
}

static HAL_StatusTypeDef TIM_StopChannelDMA(TIM_HandleTypeDef *htim,
                                            uint32_t channel)
{
    uint8_t index = 0U;
    uint8_t dma_id = 0U;
    DMA_HandleTypeDef *hdma;
    HAL_StatusTypeDef status;
    if ((htim == NULL) || (TIM_ChannelIndex(channel, &index) == 0) ||
        (GD32_HAL_TIMER_IsChannelValid(TIM_Address(htim), index) == 0))
    {
        return HAL_ERROR;
    }
    dma_id = (uint8_t)(TIM_DMA_ID_CC1 + index);
    hdma = htim->hdma[dma_id];
    if ((hdma == NULL) || (hdma->Parent != htim))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_DMA_LINK_INVALID,
                           TIM_Address(htim));
        return HAL_ERROR;
    }
    GD32_HAL_TIMER_SetDMARequest(TIM_Address(htim),
                                 (GD32_HAL_TIMERDMARequest)dma_id, 0);
    status = (hdma->State == HAL_DMA_STATE_BUSY) ? HAL_DMA_Abort(hdma) : HAL_OK;
    GD32_HAL_TIMER_SetChannel(TIM_Address(htim), index, 0);
    GD32_HAL_TIMER_DisableIfIdle(TIM_Address(htim));
    htim->ChannelState[index] = HAL_TIM_CHANNEL_STATE_READY;
    return status;
}

HAL_StatusTypeDef HAL_TIM_Base_Init(TIM_HandleTypeDef *htim)
{
    return TIM_Init(htim, HAL_TIM_Base_MspInit);
}

HAL_StatusTypeDef HAL_TIM_Base_DeInit(TIM_HandleTypeDef *htim)
{
    return TIM_DeInit(htim, HAL_TIM_Base_MspDeInit);
}

HAL_StatusTypeDef HAL_TIM_Base_Start(TIM_HandleTypeDef *htim)
{
    if ((htim == NULL) || (htim->State != HAL_TIM_STATE_READY))
    {
        return (htim != NULL && htim->State == HAL_TIM_STATE_BUSY) ? HAL_BUSY : HAL_ERROR;
    }
    htim->State = HAL_TIM_STATE_BUSY;
    GD32_HAL_TIMER_Enable(TIM_Address(htim));
    return HAL_OK;
}

HAL_StatusTypeDef HAL_TIM_Base_Stop(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) { return HAL_ERROR; }
    GD32_HAL_TIMER_Disable(TIM_Address(htim));
    htim->State = HAL_TIM_STATE_READY;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_TIM_Base_Start_IT(TIM_HandleTypeDef *htim)
{
    if ((htim == NULL) || (htim->State != HAL_TIM_STATE_READY))
    {
        return (htim != NULL && htim->State == HAL_TIM_STATE_BUSY) ? HAL_BUSY : HAL_ERROR;
    }
    htim->State = HAL_TIM_STATE_BUSY;
    __HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
    GD32_HAL_TIMER_Enable(TIM_Address(htim));
    return HAL_OK;
}

HAL_StatusTypeDef HAL_TIM_Base_Stop_IT(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) { return HAL_ERROR; }
    __HAL_TIM_DISABLE_IT(htim, TIM_IT_UPDATE);
    return HAL_TIM_Base_Stop(htim);
}

HAL_StatusTypeDef HAL_TIM_Base_Start_DMA(TIM_HandleTypeDef *htim,
                                         const uint32_t *pData,
                                         uint16_t Length)
{
    DMA_HandleTypeDef *hdma;
    HAL_StatusTypeDef status;
    if (htim == NULL) { return HAL_ERROR; }
    if (htim->State == HAL_TIM_STATE_BUSY) { return HAL_BUSY; }
    if ((htim->State != HAL_TIM_STATE_READY) ||
        (TIM_DMAOutputDataValid(pData, Length) == 0)) { return HAL_ERROR; }
    hdma = htim->hdma[TIM_DMA_ID_UPDATE];
    if (TIM_DMAConfigValid(htim, hdma, TIM_DMA_ID_UPDATE, 0) == 0)
    {
        return HAL_ERROR;
    }
    htim->State = HAL_TIM_STATE_BUSY;
    hdma->XferCpltCallback = TIM_DMABaseCplt;
    hdma->XferHalfCpltCallback = TIM_DMABaseHalfCplt;
    hdma->XferErrorCallback = TIM_DMAError;
    status = HAL_DMA_Start_IT(
        hdma,
        (uint32_t)(uintptr_t)pData,
        GD32_HAL_TIMER_GetDMADataAddress(TIM_Address(htim), GD32_HAL_TIMER_DMA_UPDATE),
        Length);
    if (status != HAL_OK)
    {
        htim->State = HAL_TIM_STATE_READY;
        return status;
    }
    GD32_HAL_TIMER_SetDMARequest(TIM_Address(htim), GD32_HAL_TIMER_DMA_UPDATE, 1);
    GD32_HAL_TIMER_Enable(TIM_Address(htim));
    return HAL_OK;
}

HAL_StatusTypeDef HAL_TIM_Base_Stop_DMA(TIM_HandleTypeDef *htim)
{
    HAL_StatusTypeDef status;
    if ((htim == NULL) || (htim->hdma[TIM_DMA_ID_UPDATE] == NULL) ||
        (htim->hdma[TIM_DMA_ID_UPDATE]->Parent != htim))
    {
        return HAL_ERROR;
    }
    GD32_HAL_TIMER_SetDMARequest(TIM_Address(htim), GD32_HAL_TIMER_DMA_UPDATE, 0);
    status = (htim->hdma[TIM_DMA_ID_UPDATE]->State == HAL_DMA_STATE_BUSY) ?
             HAL_DMA_Abort(htim->hdma[TIM_DMA_ID_UPDATE]) : HAL_OK;
    GD32_HAL_TIMER_Disable(TIM_Address(htim));
    htim->State = HAL_TIM_STATE_READY;
    return status;
}

HAL_StatusTypeDef HAL_TIM_OC_Init(TIM_HandleTypeDef *htim) { return TIM_Init(htim, HAL_TIM_OC_MspInit); }
HAL_StatusTypeDef HAL_TIM_OC_DeInit(TIM_HandleTypeDef *htim) { return TIM_DeInit(htim, HAL_TIM_OC_MspDeInit); }
HAL_StatusTypeDef HAL_TIM_PWM_Init(TIM_HandleTypeDef *htim) { return TIM_Init(htim, HAL_TIM_PWM_MspInit); }
HAL_StatusTypeDef HAL_TIM_PWM_DeInit(TIM_HandleTypeDef *htim) { return TIM_DeInit(htim, HAL_TIM_PWM_MspDeInit); }
HAL_StatusTypeDef HAL_TIM_IC_Init(TIM_HandleTypeDef *htim) { return TIM_Init(htim, HAL_TIM_IC_MspInit); }
HAL_StatusTypeDef HAL_TIM_IC_DeInit(TIM_HandleTypeDef *htim) { return TIM_DeInit(htim, HAL_TIM_IC_MspDeInit); }

HAL_StatusTypeDef HAL_TIM_OC_ConfigChannel(TIM_HandleTypeDef *htim,
                                           const TIM_OC_InitTypeDef *sConfig,
                                           uint32_t Channel)
{
    return TIM_ConfigOC(htim, sConfig, Channel, 0);
}

HAL_StatusTypeDef HAL_TIM_PWM_ConfigChannel(TIM_HandleTypeDef *htim,
                                            const TIM_OC_InitTypeDef *sConfig,
                                            uint32_t Channel)
{
    return TIM_ConfigOC(htim, sConfig, Channel, 1);
}

HAL_StatusTypeDef HAL_TIM_OC_Start(TIM_HandleTypeDef *htim, uint32_t Channel) { return TIM_StartChannel(htim, Channel, 0); }
HAL_StatusTypeDef HAL_TIM_OC_Stop(TIM_HandleTypeDef *htim, uint32_t Channel) { return TIM_StopChannel(htim, Channel, 0); }
HAL_StatusTypeDef HAL_TIM_OC_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel) { return TIM_StartChannel(htim, Channel, 1); }
HAL_StatusTypeDef HAL_TIM_OC_Stop_IT(TIM_HandleTypeDef *htim, uint32_t Channel) { return TIM_StopChannel(htim, Channel, 1); }
HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t Channel) { return TIM_StartChannel(htim, Channel, 0); }
HAL_StatusTypeDef HAL_TIM_PWM_Stop(TIM_HandleTypeDef *htim, uint32_t Channel) { return TIM_StopChannel(htim, Channel, 0); }
HAL_StatusTypeDef HAL_TIM_PWM_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel) { return TIM_StartChannel(htim, Channel, 1); }
HAL_StatusTypeDef HAL_TIM_PWM_Stop_IT(TIM_HandleTypeDef *htim, uint32_t Channel) { return TIM_StopChannel(htim, Channel, 1); }

HAL_StatusTypeDef HAL_TIM_OC_Start_DMA(TIM_HandleTypeDef *htim,
                                       uint32_t Channel,
                                       const uint32_t *pData,
                                       uint16_t Length)
{
    return TIM_StartChannelDMA(htim, Channel, (uint32_t *)(uintptr_t)pData, Length, 0);
}

HAL_StatusTypeDef HAL_TIM_OC_Stop_DMA(TIM_HandleTypeDef *htim, uint32_t Channel)
{
    return TIM_StopChannelDMA(htim, Channel);
}

HAL_StatusTypeDef HAL_TIM_PWM_Start_DMA(TIM_HandleTypeDef *htim,
                                        uint32_t Channel,
                                        const uint32_t *pData,
                                        uint16_t Length)
{
    return TIM_StartChannelDMA(htim, Channel, (uint32_t *)(uintptr_t)pData, Length, 0);
}

HAL_StatusTypeDef HAL_TIM_PWM_Stop_DMA(TIM_HandleTypeDef *htim, uint32_t Channel)
{
    return TIM_StopChannelDMA(htim, Channel);
}

HAL_StatusTypeDef HAL_TIM_IC_ConfigChannel(TIM_HandleTypeDef *htim,
                                           const TIM_IC_InitTypeDef *sConfig,
                                           uint32_t Channel)
{
    GD32_HAL_TIMERICConfig config;
    uint8_t index = 0U;
    if ((htim == NULL) || (TIM_ChannelIndex(Channel, &index) == 0) ||
        (GD32_HAL_TIMER_IsChannelValid(TIM_Address(htim), index) == 0) ||
        (TIM_ICConfig(sConfig, &config) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_INVALID_CONFIG,
                           (htim != NULL) ? TIM_Address(htim) : 0U);
        return HAL_ERROR;
    }
    __HAL_LOCK(htim);
    if (GD32_HAL_TIMER_ConfigureInput(TIM_Address(htim), index, &config) != 0)
    {
        __HAL_UNLOCK(htim);
        return HAL_ERROR;
    }
    htim->ChannelState[index] = HAL_TIM_CHANNEL_STATE_READY;
    __HAL_UNLOCK(htim);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_TIM_IC_Start(TIM_HandleTypeDef *htim, uint32_t Channel) { return TIM_StartChannel(htim, Channel, 0); }
HAL_StatusTypeDef HAL_TIM_IC_Stop(TIM_HandleTypeDef *htim, uint32_t Channel) { return TIM_StopChannel(htim, Channel, 0); }
HAL_StatusTypeDef HAL_TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel) { return TIM_StartChannel(htim, Channel, 1); }
HAL_StatusTypeDef HAL_TIM_IC_Stop_IT(TIM_HandleTypeDef *htim, uint32_t Channel) { return TIM_StopChannel(htim, Channel, 1); }

HAL_StatusTypeDef HAL_TIM_IC_Start_DMA(TIM_HandleTypeDef *htim,
                                       uint32_t Channel,
                                       uint32_t *pData,
                                       uint16_t Length)
{
    return TIM_StartChannelDMA(htim, Channel, pData, Length, 1);
}

HAL_StatusTypeDef HAL_TIM_IC_Stop_DMA(TIM_HandleTypeDef *htim, uint32_t Channel)
{
    return TIM_StopChannelDMA(htim, Channel);
}

HAL_StatusTypeDef HAL_TIM_OnePulse_Init(TIM_HandleTypeDef *htim, uint32_t OnePulseMode)
{
    HAL_StatusTypeDef status;
    if ((OnePulseMode != TIM_OPMODE_SINGLE) &&
        (OnePulseMode != TIM_OPMODE_REPETITIVE))
    {
        return HAL_ERROR;
    }
    status = TIM_Init(htim, HAL_TIM_OnePulse_MspInit);
    if (status != HAL_OK) { return status; }
    if (GD32_HAL_TIMER_IsOnePulseCapable(TIM_Address(htim)) == 0)
    {
        return HAL_ERROR;
    }
    GD32_HAL_TIMER_SetOnePulse(TIM_Address(htim),
                               OnePulseMode == TIM_OPMODE_SINGLE);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_TIM_OnePulse_DeInit(TIM_HandleTypeDef *htim)
{
    return TIM_DeInit(htim, HAL_TIM_OnePulse_MspDeInit);
}

HAL_StatusTypeDef HAL_TIM_OnePulse_ConfigChannel(TIM_HandleTypeDef *htim,
                                                 TIM_OnePulse_InitTypeDef *sConfig,
                                                 uint32_t OutputChannel,
                                                 uint32_t InputChannel)
{
    TIM_OC_InitTypeDef output;
    TIM_IC_InitTypeDef input;
    GD32_HAL_TIMERSlaveConfig slave;
    HAL_StatusTypeDef status;

    if ((htim == NULL) || (sConfig == NULL) ||
        !(((OutputChannel == TIM_CHANNEL_1) && (InputChannel == TIM_CHANNEL_2)) ||
          ((OutputChannel == TIM_CHANNEL_2) && (InputChannel == TIM_CHANNEL_1))))
    {
        return HAL_ERROR;
    }
    output.OCMode = sConfig->OCMode;
    output.Pulse = sConfig->Pulse;
    output.OCPolarity = sConfig->OCPolarity;
    output.OCNPolarity = sConfig->OCNPolarity;
    output.OCFastMode = TIM_OCFAST_DISABLE;
    output.OCIdleState = sConfig->OCIdleState;
    output.OCNIdleState = sConfig->OCNIdleState;
    status = TIM_ConfigOC(htim, &output, OutputChannel, 0);
    if (status != HAL_OK) { return status; }

    input.ICPolarity = sConfig->ICPolarity;
    input.ICSelection = sConfig->ICSelection;
    input.ICPrescaler = TIM_ICPSC_DIV1;
    input.ICFilter = sConfig->ICFilter;
    status = HAL_TIM_IC_ConfigChannel(htim, &input, InputChannel);
    if (status != HAL_OK) { return status; }

    slave.slave_mode = 3U;
    slave.input_trigger = (InputChannel == TIM_CHANNEL_1) ? 5U : 6U;
    slave.trigger_polarity = (sConfig->ICPolarity == TIM_ICPOLARITY_FALLING) ?
                             GD32_HAL_TIMER_POLARITY_FALLING_LOW :
                             GD32_HAL_TIMER_POLARITY_RISING_HIGH;
    slave.trigger_prescaler = 0U;
    slave.trigger_filter = (uint8_t)sConfig->ICFilter;
    return (GD32_HAL_TIMER_ConfigureSlave(TIM_Address(htim), &slave) == 0) ?
           HAL_OK : HAL_ERROR;
}

static HAL_StatusTypeDef TIM_OnePulseStartStop(TIM_HandleTypeDef *htim,
                                               uint32_t output_channel,
                                               int start,
                                               int interrupt)
{
    uint8_t output = 0U;
    uint8_t input;
    if ((htim == NULL) || (TIM_ChannelIndex(output_channel, &output) == 0) ||
        (output > 1U))
    {
        return HAL_ERROR;
    }
    input = (uint8_t)(1U - output);
    if (start != 0)
    {
        if ((htim->ChannelState[output] != HAL_TIM_CHANNEL_STATE_READY) ||
            (htim->ChannelState[input] != HAL_TIM_CHANNEL_STATE_READY))
        {
            return HAL_BUSY;
        }
        htim->ChannelState[output] = HAL_TIM_CHANNEL_STATE_BUSY;
        htim->ChannelState[input] = HAL_TIM_CHANNEL_STATE_BUSY;
        if (interrupt != 0)
        {
            __HAL_TIM_ENABLE_IT(htim, ((uint32_t)TIM_IT_CC1 << output) |
                                      ((uint32_t)TIM_IT_CC1 << input));
        }
        GD32_HAL_TIMER_SetChannel(TIM_Address(htim), output, 1);
        GD32_HAL_TIMER_SetChannel(TIM_Address(htim), input, 1);
    }
    else
    {
        if (interrupt != 0)
        {
            __HAL_TIM_DISABLE_IT(htim, ((uint32_t)TIM_IT_CC1 << output) |
                                       ((uint32_t)TIM_IT_CC1 << input));
        }
        GD32_HAL_TIMER_SetChannel(TIM_Address(htim), output, 0);
        GD32_HAL_TIMER_SetChannel(TIM_Address(htim), input, 0);
        GD32_HAL_TIMER_Disable(TIM_Address(htim));
        htim->ChannelState[output] = HAL_TIM_CHANNEL_STATE_READY;
        htim->ChannelState[input] = HAL_TIM_CHANNEL_STATE_READY;
    }
    return HAL_OK;
}

HAL_StatusTypeDef HAL_TIM_OnePulse_Start(TIM_HandleTypeDef *htim, uint32_t OutputChannel)
{
    return TIM_OnePulseStartStop(htim, OutputChannel, 1, 0);
}

HAL_StatusTypeDef HAL_TIM_OnePulse_Stop(TIM_HandleTypeDef *htim, uint32_t OutputChannel)
{
    return TIM_OnePulseStartStop(htim, OutputChannel, 0, 0);
}

HAL_StatusTypeDef HAL_TIM_OnePulse_Start_IT(TIM_HandleTypeDef *htim, uint32_t OutputChannel)
{
    return TIM_OnePulseStartStop(htim, OutputChannel, 1, 1);
}

HAL_StatusTypeDef HAL_TIM_OnePulse_Stop_IT(TIM_HandleTypeDef *htim, uint32_t OutputChannel)
{
    return TIM_OnePulseStartStop(htim, OutputChannel, 0, 1);
}

HAL_StatusTypeDef HAL_TIM_ConfigClockSource(
    TIM_HandleTypeDef *htim,
    const TIM_ClockConfigTypeDef *sClockSourceConfig)
{
    if ((htim == NULL) || (sClockSourceConfig == NULL) ||
        (sClockSourceConfig->ClockSource != TIM_CLOCKSOURCE_INTERNAL))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_INVALID_CONFIG,
                           (sClockSourceConfig != NULL) ?
                           sClockSourceConfig->ClockSource : 0U);
        return HAL_ERROR;
    }
    GD32_HAL_TIMER_SetInternalClock(TIM_Address(htim));
    return HAL_OK;
}

HAL_StatusTypeDef HAL_TIM_SlaveConfigSynchro(
    TIM_HandleTypeDef *htim,
    const TIM_SlaveConfigTypeDef *sSlaveConfig)
{
    GD32_HAL_TIMERSlaveConfig config;
    uint32_t trigger;
    if ((htim == NULL) || (sSlaveConfig == NULL)) { return HAL_ERROR; }

    if (sSlaveConfig->SlaveMode == TIM_SLAVEMODE_DISABLE) { config.slave_mode = 0U; }
    else if (sSlaveConfig->SlaveMode == TIM_SLAVEMODE_RESET) { config.slave_mode = 1U; }
    else if (sSlaveConfig->SlaveMode == TIM_SLAVEMODE_GATED) { config.slave_mode = 2U; }
    else if (sSlaveConfig->SlaveMode == TIM_SLAVEMODE_TRIGGER) { config.slave_mode = 3U; }
    else if (sSlaveConfig->SlaveMode == TIM_SLAVEMODE_EXTERNAL1) { config.slave_mode = 4U; }
    else { return HAL_ERROR; }

    if ((sSlaveConfig->InputTrigger == TIM_TS_NONE) ||
        ((sSlaveConfig->InputTrigger & ~0x70U) != 0U))
    {
        return HAL_ERROR;
    }
    trigger = sSlaveConfig->InputTrigger >> 4U;
    config.input_trigger = (uint8_t)trigger;
    if ((sSlaveConfig->TriggerPolarity == TIM_TRIGGERPOLARITY_BOTHEDGE))
    {
        config.trigger_polarity = GD32_HAL_TIMER_POLARITY_BOTH;
    }
    else if ((sSlaveConfig->TriggerPolarity == TIM_TRIGGERPOLARITY_FALLING) ||
             (sSlaveConfig->TriggerPolarity == TIM_TRIGGERPOLARITY_INVERTED))
    {
        config.trigger_polarity = GD32_HAL_TIMER_POLARITY_FALLING_LOW;
    }
    else
    {
        config.trigger_polarity = GD32_HAL_TIMER_POLARITY_RISING_HIGH;
    }
    if ((sSlaveConfig->TriggerPrescaler & ~0x3000U) != 0U) { return HAL_ERROR; }
    config.trigger_prescaler = (uint8_t)(sSlaveConfig->TriggerPrescaler >> 12U);
    if (sSlaveConfig->TriggerFilter > 15U) { return HAL_ERROR; }
    config.trigger_filter = (uint8_t)sSlaveConfig->TriggerFilter;
    return (GD32_HAL_TIMER_ConfigureSlave(TIM_Address(htim), &config) == 0) ?
           HAL_OK : HAL_ERROR;
}

HAL_StatusTypeDef HAL_TIM_SlaveConfigSynchro_IT(
    TIM_HandleTypeDef *htim,
    const TIM_SlaveConfigTypeDef *sSlaveConfig)
{
    HAL_StatusTypeDef status = HAL_TIM_SlaveConfigSynchro(htim, sSlaveConfig);
    if (status == HAL_OK) { __HAL_TIM_ENABLE_IT(htim, TIM_IT_TRIGGER); }
    return status;
}

void HAL_TIM_IRQHandler(TIM_HandleTypeDef *htim)
{
    uint8_t index = 0U;
    if (htim == NULL) { return; }
    for (index = 0U; index < 4U; ++index)
    {
        const uint32_t interrupt = (uint32_t)TIM_IT_CC1 << index;
        const uint32_t flag = (uint32_t)TIM_FLAG_CC1 << index;
        if ((__HAL_TIM_GET_FLAG(htim, flag) != 0U) &&
            (__HAL_TIM_GET_IT_SOURCE(htim, interrupt) != 0U))
        {
            __HAL_TIM_CLEAR_IT(htim, flag);
            htim->Channel = TIM_ActiveFromIndex(index);
            if (GD32_HAL_TIMER_IsChannelInput(TIM_Address(htim), index) != 0)
            {
                HAL_TIM_IC_CaptureCallback(htim);
            }
            else
            {
                HAL_TIM_OC_DelayElapsedCallback(htim);
                HAL_TIM_PWM_PulseFinishedCallback(htim);
            }
            htim->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
        }
    }
    if ((__HAL_TIM_GET_FLAG(htim, TIM_FLAG_UPDATE) != 0U) &&
        (__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_UPDATE) != 0U))
    {
        __HAL_TIM_CLEAR_IT(htim, TIM_FLAG_UPDATE);
        HAL_TIM_PeriodElapsedCallback(htim);
    }
    if ((__HAL_TIM_GET_FLAG(htim, TIM_FLAG_TRIGGER) != 0U) &&
        (__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_TRIGGER) != 0U))
    {
        __HAL_TIM_CLEAR_IT(htim, TIM_FLAG_TRIGGER);
        HAL_TIM_TriggerCallback(htim);
    }
}

uint32_t HAL_TIM_ReadCapturedValue(const TIM_HandleTypeDef *htim, uint32_t Channel)
{
    uint8_t index = 0U;
    if ((htim == NULL) || (TIM_ChannelIndex(Channel, &index) == 0)) { return 0U; }
    return GD32_HAL_TIMER_GetCompare(TIM_Address(htim), index);
}

HAL_TIM_StateTypeDef HAL_TIM_Base_GetState(const TIM_HandleTypeDef *htim) { return htim->State; }
HAL_TIM_StateTypeDef HAL_TIM_OC_GetState(const TIM_HandleTypeDef *htim) { return htim->State; }
HAL_TIM_StateTypeDef HAL_TIM_PWM_GetState(const TIM_HandleTypeDef *htim) { return htim->State; }
HAL_TIM_StateTypeDef HAL_TIM_IC_GetState(const TIM_HandleTypeDef *htim) { return htim->State; }
HAL_TIM_StateTypeDef HAL_TIM_OnePulse_GetState(const TIM_HandleTypeDef *htim) { return htim->State; }
HAL_TIM_ActiveChannel HAL_TIM_GetActiveChannel(const TIM_HandleTypeDef *htim) { return htim->Channel; }

HAL_TIM_ChannelStateTypeDef HAL_TIM_GetChannelState(const TIM_HandleTypeDef *htim,
                                                     uint32_t Channel)
{
    uint8_t index = 0U;
    if ((htim == NULL) || (TIM_ChannelIndex(Channel, &index) == 0))
    {
        return HAL_TIM_CHANNEL_STATE_RESET;
    }
    return htim->ChannelState[index];
}
