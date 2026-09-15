#include "stm32f4xx_hal.h"
#include "gd32_hal_port.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static GD32_HAL_TIMERBaseConfig mock_base;
static GD32_HAL_TIMEROCConfig mock_output[4];
static GD32_HAL_TIMERICConfig mock_input[4];
static GD32_HAL_TIMERSlaveConfig mock_slave;
static uint32_t mock_interrupts;
static uint32_t mock_flags;
static uint32_t mock_dma_requests;
static uint32_t mock_counter;
static uint32_t mock_autoreload;
static uint32_t mock_prescaler;
static uint32_t mock_compare[4];
static uint8_t mock_channel_enabled[4];
static uint8_t mock_channel_input[4];
static uint8_t mock_master_trigger;
static int mock_master_slave;
static int mock_enabled;
static int mock_one_pulse;
static int mock_trigger_slave;
static uint32_t mock_base_config_count;
static uint32_t mock_dma_start_count;
static uint32_t mock_dma_abort_count;
static uint32_t mock_dma_source;
static uint32_t mock_dma_destination;
static uint32_t mock_dma_length;
static GD32_HAL_PortError mock_error;
static uint32_t mock_period_callback_count;
static uint32_t mock_period_half_callback_count;
static uint32_t mock_oc_callback_count;
static uint32_t mock_pwm_callback_count;
static uint32_t mock_pwm_half_callback_count;
static uint32_t mock_ic_callback_count;
static uint32_t mock_ic_half_callback_count;
static uint32_t mock_trigger_callback_count;
static uint32_t mock_error_callback_count;
static const GD32_HAL_Resource mock_dma_ch1_resource =
    {0U, 0U, GD32_HAL_DMA0_CHANNEL1_ADDRESS, GD32_HAL_DMA0_ADDRESS,
     0U, 12, GD32_HAL_CAP_DMA, 0U, 1U};
static const GD32_HAL_Resource mock_dma_ch2_resource =
    {0U, 0U, GD32_HAL_DMA0_CHANNEL2_ADDRESS, GD32_HAL_DMA0_ADDRESS,
     0U, 13, GD32_HAL_CAP_DMA, 0U, 2U};
static const GD32_HAL_Resource mock_dma_ch4_resource =
    {0U, 0U, GD32_HAL_DMA0_CHANNEL4_ADDRESS, GD32_HAL_DMA0_ADDRESS,
     0U, 15, GD32_HAL_CAP_DMA, 0U, 4U};
static const GD32_HAL_Resource mock_dma_ch5_resource =
    {0U, 0U, GD32_HAL_DMA0_CHANNEL5_ADDRESS, GD32_HAL_DMA0_ADDRESS,
     0U, 16, GD32_HAL_CAP_DMA, 0U, 5U};
static HAL_TIM_ActiveChannel mock_callback_channel;

static void mock_reset(void)
{
    memset(&mock_base, 0, sizeof(mock_base));
    memset(mock_output, 0, sizeof(mock_output));
    memset(mock_input, 0, sizeof(mock_input));
    memset(&mock_slave, 0, sizeof(mock_slave));
    memset(mock_compare, 0, sizeof(mock_compare));
    memset(mock_channel_enabled, 0, sizeof(mock_channel_enabled));
    memset(mock_channel_input, 0, sizeof(mock_channel_input));
    mock_interrupts = 0U;
    mock_flags = 0U;
    mock_dma_requests = 0U;
    mock_counter = 0U;
    mock_autoreload = 0U;
    mock_prescaler = 0U;
    mock_master_trigger = 0U;
    mock_master_slave = 0;
    mock_enabled = 0;
    mock_one_pulse = 0;
    mock_trigger_slave = 0;
    mock_base_config_count = 0U;
    mock_dma_start_count = 0U;
    mock_dma_abort_count = 0U;
    mock_dma_source = 0U;
    mock_dma_destination = 0U;
    mock_dma_length = 0U;
    mock_error = GD32_HAL_PORT_ERROR_NONE;
    mock_period_callback_count = 0U;
    mock_period_half_callback_count = 0U;
    mock_oc_callback_count = 0U;
    mock_pwm_callback_count = 0U;
    mock_pwm_half_callback_count = 0U;
    mock_ic_callback_count = 0U;
    mock_ic_half_callback_count = 0U;
    mock_trigger_callback_count = 0U;
    mock_error_callback_count = 0U;
    mock_callback_channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
}

static TIM_HandleTypeDef mock_handle(TIM_TypeDef *instance)
{
    TIM_HandleTypeDef htim;
    memset(&htim, 0, sizeof(htim));
    htim.Instance = instance;
    htim.Init.Prescaler = 79U;
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim.Init.Period = 999U;
    htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim.Init.RepetitionCounter = 0U;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    htim.State = HAL_TIM_STATE_RESET;
    return htim;
}

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    (void)detail;
    mock_error = error;
}

int GD32_HAL_TIMER_IsInstance(uint32_t timer_address)
{
    return (timer_address >= GD32_HAL_TIMER1_ADDRESS) &&
           (timer_address <= GD32_HAL_TIMER0_ADDRESS);
}

int GD32_HAL_TIMER_IsChannelValid(uint32_t timer_address, uint8_t channel)
{
    (void)timer_address;
    return channel < 4U;
}

int GD32_HAL_TIMER_IsOnePulseCapable(uint32_t timer_address)
{
    (void)timer_address;
    return 1;
}

int GD32_HAL_TIMER_IsMasterSlaveCapable(uint32_t timer_address)
{
    (void)timer_address;
    return 1;
}

int GD32_HAL_TIMER_ConfigureBase(uint32_t timer_address,
                                 const GD32_HAL_TIMERBaseConfig *config)
{
    (void)timer_address;
    mock_base = *config;
    mock_autoreload = config->period;
    mock_prescaler = config->prescaler;
    ++mock_base_config_count;
    return 0;
}

void GD32_HAL_TIMER_DeInit(uint32_t timer_address) { (void)timer_address; }
void GD32_HAL_TIMER_Enable(uint32_t timer_address) { (void)timer_address; mock_enabled = 1; }
void GD32_HAL_TIMER_Disable(uint32_t timer_address) { (void)timer_address; mock_enabled = 0; }

void GD32_HAL_TIMER_DisableIfIdle(uint32_t timer_address)
{
    uint8_t i;
    (void)timer_address;
    for (i = 0U; i < 4U; ++i) { if (mock_channel_enabled[i] != 0U) { return; } }
    mock_enabled = 0;
}

int GD32_HAL_TIMER_IsEnabled(uint32_t timer_address) { (void)timer_address; return mock_enabled; }
int GD32_HAL_TIMER_IsTriggerSlaveMode(uint32_t timer_address) { (void)timer_address; return mock_trigger_slave; }
void GD32_HAL_TIMER_SetInternalClock(uint32_t timer_address) { (void)timer_address; mock_trigger_slave = 0; }

int GD32_HAL_TIMER_ConfigureOutput(uint32_t timer_address,
                                   uint8_t channel,
                                   const GD32_HAL_TIMEROCConfig *config)
{
    (void)timer_address;
    mock_output[channel] = *config;
    mock_channel_input[channel] = 0U;
    mock_compare[channel] = config->pulse;
    return 0;
}

int GD32_HAL_TIMER_ConfigureInput(uint32_t timer_address,
                                  uint8_t channel,
                                  const GD32_HAL_TIMERICConfig *config)
{
    (void)timer_address;
    mock_input[channel] = *config;
    mock_channel_input[channel] = 1U;
    return 0;
}

void GD32_HAL_TIMER_SetChannel(uint32_t timer_address, uint8_t channel, int enable)
{
    (void)timer_address;
    mock_channel_enabled[channel] = (uint8_t)(enable != 0);
}

int GD32_HAL_TIMER_IsChannelInput(uint32_t timer_address, uint8_t channel)
{
    (void)timer_address;
    return mock_channel_input[channel] != 0U;
}

void GD32_HAL_TIMER_SetOnePulse(uint32_t timer_address, int single)
{
    (void)timer_address;
    mock_one_pulse = single;
}

int GD32_HAL_TIMER_ConfigureMaster(uint32_t timer_address,
                                   uint8_t trigger_output,
                                   int master_slave_enable)
{
    (void)timer_address;
    mock_master_trigger = trigger_output;
    mock_master_slave = master_slave_enable;
    return 0;
}

int GD32_HAL_TIMER_ConfigureSlave(uint32_t timer_address,
                                  const GD32_HAL_TIMERSlaveConfig *config)
{
    (void)timer_address;
    mock_slave = *config;
    mock_trigger_slave = config->slave_mode == 3U;
    return 0;
}

void GD32_HAL_TIMER_SetInterrupt(uint32_t timer_address, uint32_t interrupts, int enable)
{
    (void)timer_address;
    if (enable != 0) { mock_interrupts |= interrupts; }
    else { mock_interrupts &= ~interrupts; }
}

uint32_t GD32_HAL_TIMER_GetInterrupts(uint32_t timer_address) { (void)timer_address; return mock_interrupts; }
uint32_t GD32_HAL_TIMER_GetFlag(uint32_t timer_address, uint32_t flags) { (void)timer_address; return mock_flags & flags; }
void GD32_HAL_TIMER_ClearFlag(uint32_t timer_address, uint32_t flags) { (void)timer_address; mock_flags &= ~flags; }

void GD32_HAL_TIMER_SetDMARequest(uint32_t timer_address,
                                  GD32_HAL_TIMERDMARequest request,
                                  int enable)
{
    (void)timer_address;
    if (enable != 0) { mock_dma_requests |= 1UL << (uint32_t)request; }
    else { mock_dma_requests &= ~(1UL << (uint32_t)request); }
}

int GD32_HAL_TIMER_IsDMAChannelValid(uint32_t timer_address,
                                     GD32_HAL_TIMERDMARequest request,
                                     uint32_t channel_address,
                                     uint32_t request_token)
{
    (void)timer_address;
    return ((timer_address == GD32_HAL_TIMER1_ADDRESS) &&
            (request == GD32_HAL_TIMER_DMA_CC1) &&
            (channel_address == GD32_HAL_DMA0_CHANNEL4_ADDRESS) &&
            (request_token == GD32_DMA_REQUEST_TIMER1_CH0)) ||
           ((timer_address == GD32_HAL_TIMER1_ADDRESS) &&
            (request == GD32_HAL_TIMER_DMA_UPDATE) &&
            (channel_address == GD32_HAL_DMA0_CHANNEL1_ADDRESS) &&
            (request_token == GD32_DMA_REQUEST_TIMER1_UP)) ||
           ((timer_address == GD32_HAL_TIMER0_ADDRESS) &&
            (request == GD32_HAL_TIMER_DMA_CC1) &&
            (channel_address == GD32_HAL_DMA0_CHANNEL1_ADDRESS) &&
            (request_token == GD32_DMA_REQUEST_TIMER0_CH0)) ||
           ((timer_address == GD32_HAL_TIMER0_ADDRESS) &&
            (request == GD32_HAL_TIMER_DMA_CC2) &&
            (channel_address == GD32_HAL_DMA0_CHANNEL2_ADDRESS) &&
            (request_token == GD32_DMA_REQUEST_TIMER0_CH1)) ||
           ((timer_address == GD32_HAL_TIMER0_ADDRESS) &&
            (request == GD32_HAL_TIMER_DMA_CC3) &&
            (channel_address == GD32_HAL_DMA0_CHANNEL5_ADDRESS) &&
            (request_token == GD32_DMA_REQUEST_TIMER0_CH2));
}

HAL_StatusTypeDef GD32_HAL_DMA_ResolveForTimer(
    DMA_HandleTypeDef *hdma,
    uint32_t timer_address,
    GD32_HAL_TIMERDMARequest request)
{
    if (hdma->GD32_MAPPING_ORIGIN != GD32_HAL_DMA_MAPPING_STM32_TIMER)
    {
        return HAL_OK;
    }
    if (hdma->State != HAL_DMA_STATE_READY)
    {
        return HAL_BUSY;
    }
    if ((timer_address == GD32_HAL_TIMER1_ADDRESS) &&
        (request == GD32_HAL_TIMER_DMA_CC1) &&
        (hdma->Instance == DMA1_Stream5) &&
        (hdma->Init.Channel == DMA_CHANNEL_3))
    {
        hdma->GD32_RESOURCE = &mock_dma_ch4_resource;
        hdma->GD32_INSTANCE = GD32_HAL_DMA0_CHANNEL4_ADDRESS;
        hdma->gd32_dma_periph = GD32_HAL_DMA0_ADDRESS;
        hdma->gd32_dma_channel = 4U;
        hdma->GD32_REQUEST = GD32_DMA_REQUEST_TIMER1_CH0;
    }
    else if ((timer_address == GD32_HAL_TIMER1_ADDRESS) &&
             (request == GD32_HAL_TIMER_DMA_UPDATE) &&
             (hdma->Instance == DMA1_Stream1) &&
             (hdma->Init.Channel == DMA_CHANNEL_3))
    {
        hdma->GD32_RESOURCE = &mock_dma_ch1_resource;
        hdma->GD32_INSTANCE = GD32_HAL_DMA0_CHANNEL1_ADDRESS;
        hdma->gd32_dma_periph = GD32_HAL_DMA0_ADDRESS;
        hdma->gd32_dma_channel = 1U;
        hdma->GD32_REQUEST = GD32_DMA_REQUEST_TIMER1_UP;
    }
    else if ((timer_address == GD32_HAL_TIMER0_ADDRESS) &&
             (hdma->Instance == DMA2_Stream6) &&
             (hdma->Init.Channel == DMA_CHANNEL_0) &&
             (request == GD32_HAL_TIMER_DMA_CC1))
    {
        hdma->GD32_RESOURCE = &mock_dma_ch1_resource;
        hdma->GD32_INSTANCE = GD32_HAL_DMA0_CHANNEL1_ADDRESS;
        hdma->gd32_dma_periph = GD32_HAL_DMA0_ADDRESS;
        hdma->gd32_dma_channel = 1U;
        hdma->GD32_REQUEST = GD32_DMA_REQUEST_TIMER0_CH0;
    }
    else if ((timer_address == GD32_HAL_TIMER0_ADDRESS) &&
             (hdma->Instance == DMA2_Stream6) &&
             (hdma->Init.Channel == DMA_CHANNEL_0) &&
             (request == GD32_HAL_TIMER_DMA_CC2))
    {
        hdma->GD32_RESOURCE = &mock_dma_ch2_resource;
        hdma->GD32_INSTANCE = GD32_HAL_DMA0_CHANNEL2_ADDRESS;
        hdma->gd32_dma_periph = GD32_HAL_DMA0_ADDRESS;
        hdma->gd32_dma_channel = 2U;
        hdma->GD32_REQUEST = GD32_DMA_REQUEST_TIMER0_CH1;
    }
    else if ((timer_address == GD32_HAL_TIMER0_ADDRESS) &&
             (hdma->Instance == DMA2_Stream6) &&
             (hdma->Init.Channel == DMA_CHANNEL_0) &&
             (request == GD32_HAL_TIMER_DMA_CC3))
    {
        hdma->GD32_RESOURCE = &mock_dma_ch5_resource;
        hdma->GD32_INSTANCE = GD32_HAL_DMA0_CHANNEL5_ADDRESS;
        hdma->gd32_dma_periph = GD32_HAL_DMA0_ADDRESS;
        hdma->gd32_dma_channel = 5U;
        hdma->GD32_REQUEST = GD32_DMA_REQUEST_TIMER0_CH2;
    }
    else
    {
        hdma->ErrorCode = HAL_DMA_ERROR_REQUEST;
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_DMA_UNSUPPORTED,
                           timer_address | (uint32_t)request);
        return HAL_ERROR;
    }
    hdma->GD32_RESOLVED_FROM = (uintptr_t)hdma->Instance;
    hdma->GD32_DEFERRED = 0U;
    return HAL_OK;
}

uint32_t GD32_HAL_TIMER_GetDMADataAddress(uint32_t timer_address,
                                          GD32_HAL_TIMERDMARequest request)
{
    return timer_address + ((request == GD32_HAL_TIMER_DMA_UPDATE) ? 0x2CU :
                            (0x30U + (4U * (uint32_t)request)));
}

void GD32_HAL_TIMER_SetCounter(uint32_t timer_address, uint32_t value) { (void)timer_address; mock_counter = value; }
uint32_t GD32_HAL_TIMER_GetCounter(uint32_t timer_address) { (void)timer_address; return mock_counter; }
void GD32_HAL_TIMER_SetAutoReload(uint32_t timer_address, uint32_t value) { (void)timer_address; mock_autoreload = value; }
uint32_t GD32_HAL_TIMER_GetAutoReload(uint32_t timer_address) { (void)timer_address; return mock_autoreload; }
void GD32_HAL_TIMER_SetPrescaler(uint32_t timer_address, uint32_t value) { (void)timer_address; mock_prescaler = value; }
uint32_t GD32_HAL_TIMER_GetPrescaler(uint32_t timer_address) { (void)timer_address; return mock_prescaler; }
void GD32_HAL_TIMER_SetCompare(uint32_t timer_address, uint8_t channel, uint32_t value) { (void)timer_address; mock_compare[channel] = value; }
uint32_t GD32_HAL_TIMER_GetCompare(uint32_t timer_address, uint8_t channel) { (void)timer_address; return mock_compare[channel]; }

HAL_StatusTypeDef HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma,
                                   uint32_t SrcAddress,
                                   uint32_t DstAddress,
                                   uint32_t DataLength)
{
    ++mock_dma_start_count;
    mock_dma_source = SrcAddress;
    mock_dma_destination = DstAddress;
    mock_dma_length = DataLength;
    hdma->State = HAL_DMA_STATE_BUSY;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_DMA_Init(DMA_HandleTypeDef *hdma)
{
    if ((hdma == NULL) || (hdma->Instance == NULL))
    {
        return HAL_ERROR;
    }
    hdma->GD32_DEFERRED = 1U;
    hdma->GD32_MAPPING_ORIGIN = GD32_HAL_DMA_MAPPING_STM32_TIMER;
    hdma->GD32_ACTIVE_TIM_DMA_ID = GD32_HAL_DMA_ACTIVE_TIM_NONE;
    hdma->GD32_RESOURCE = NULL;
    hdma->GD32_INSTANCE = 0U;
    hdma->State = HAL_DMA_STATE_READY;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_DMA_Abort(DMA_HandleTypeDef *hdma)
{
    ++mock_dma_abort_count;
    hdma->State = HAL_DMA_STATE_READY;
    return HAL_OK;
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim) { (void)htim; }
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim) { (void)htim; }
void HAL_TIM_OC_MspInit(TIM_HandleTypeDef *htim) { (void)htim; }
void HAL_TIM_OC_MspDeInit(TIM_HandleTypeDef *htim) { (void)htim; }
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim) { (void)htim; }
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef *htim) { (void)htim; }
void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *htim) { (void)htim; }
void HAL_TIM_IC_MspDeInit(TIM_HandleTypeDef *htim) { (void)htim; }
void HAL_TIM_OnePulse_MspInit(TIM_HandleTypeDef *htim) { (void)htim; }
void HAL_TIM_OnePulse_MspDeInit(TIM_HandleTypeDef *htim) { (void)htim; }

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    ++mock_period_callback_count;
    mock_callback_channel = htim->Channel;
}
void HAL_TIM_PeriodElapsedHalfCpltCallback(TIM_HandleTypeDef *htim)
{
    ++mock_period_half_callback_count;
    mock_callback_channel = htim->Channel;
}
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) { ++mock_oc_callback_count; mock_callback_channel = htim->Channel; }
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) { ++mock_ic_callback_count; mock_callback_channel = htim->Channel; }
void HAL_TIM_IC_CaptureHalfCpltCallback(TIM_HandleTypeDef *htim)
{
    ++mock_ic_half_callback_count;
    mock_callback_channel = htim->Channel;
}
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) { ++mock_pwm_callback_count; mock_callback_channel = htim->Channel; }
void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim)
{
    ++mock_pwm_half_callback_count;
    mock_callback_channel = htim->Channel;
}
void HAL_TIM_TriggerCallback(TIM_HandleTypeDef *htim) { (void)htim; ++mock_trigger_callback_count; }
void HAL_TIM_TriggerHalfCpltCallback(TIM_HandleTypeDef *htim) { (void)htim; }
void HAL_TIM_ErrorCallback(TIM_HandleTypeDef *htim)
{
    (void)htim;
    ++mock_error_callback_count;
}

static void test_base_and_16bit_boundary(void)
{
    TIM_HandleTypeDef htim = mock_handle(TIM2);
    assert(HAL_TIM_Base_Init(&htim) == HAL_OK);
    assert(mock_base_config_count == 1U);
    assert(mock_base.prescaler == 79U);
    assert(mock_base.period == 999U);
    assert(HAL_TIM_Base_Start(&htim) == HAL_OK);
    assert(mock_enabled != 0);
    assert(HAL_TIM_Base_Stop(&htim) == HAL_OK);
    assert(mock_enabled == 0);

    htim.State = HAL_TIM_STATE_RESET;
    htim.Init.Period = 0x10000U;
    assert(HAL_TIM_Base_Init(&htim) == HAL_ERROR);
    assert(mock_error == GD32_HAL_PORT_ERROR_TIMER_16BIT_RANGE);
}

static void test_pwm_oc_ic_and_irq(void)
{
    TIM_HandleTypeDef htim = mock_handle(TIM3);
    TIM_OC_InitTypeDef oc = {0};
    TIM_IC_InitTypeDef ic = {0};

    assert(HAL_TIM_PWM_Init(&htim) == HAL_OK);
    oc.OCMode = TIM_OCMODE_PWM1;
    oc.Pulse = 250U;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    assert(HAL_TIM_PWM_ConfigChannel(&htim, &oc, TIM_CHANNEL_1) == HAL_OK);
    assert(mock_output[0].mode == GD32_HAL_TIMER_OC_PWM1);
    assert(mock_output[0].preload == 1U);
    assert(HAL_TIM_PWM_Start(&htim, TIM_CHANNEL_1) == HAL_OK);
    assert(mock_channel_enabled[0] == 1U);
    assert(HAL_TIM_PWM_Stop(&htim, TIM_CHANNEL_1) == HAL_OK);

    oc.OCMode = TIM_OCMODE_TOGGLE;
    oc.Pulse = 500U;
    assert(HAL_TIM_OC_ConfigChannel(&htim, &oc, TIM_CHANNEL_2) == HAL_OK);
    assert(HAL_TIM_OC_Start_IT(&htim, TIM_CHANNEL_2) == HAL_OK);
    mock_flags |= TIM_FLAG_CC2;
    HAL_TIM_IRQHandler(&htim);
    assert(mock_oc_callback_count == 1U);
    assert(mock_pwm_callback_count == 1U);
    assert(mock_callback_channel == HAL_TIM_ACTIVE_CHANNEL_2);

    ic.ICPolarity = TIM_ICPOLARITY_FALLING;
    ic.ICSelection = TIM_ICSELECTION_DIRECTTI;
    ic.ICPrescaler = TIM_ICPSC_DIV4;
    ic.ICFilter = 3U;
    assert(HAL_TIM_IC_ConfigChannel(&htim, &ic, TIM_CHANNEL_3) == HAL_OK);
    assert(HAL_TIM_IC_Start_IT(&htim, TIM_CHANNEL_3) == HAL_OK);
    mock_compare[2] = 777U;
    mock_flags |= TIM_FLAG_CC3;
    HAL_TIM_IRQHandler(&htim);
    assert(mock_ic_callback_count == 1U);
    assert(mock_callback_channel == HAL_TIM_ACTIVE_CHANNEL_3);
    assert(HAL_TIM_ReadCapturedValue(&htim, TIM_CHANNEL_3) == 777U);

    assert(HAL_TIM_Base_Stop(&htim) == HAL_OK);
    htim.State = HAL_TIM_STATE_READY;
    assert(HAL_TIM_Base_Start_IT(&htim) == HAL_OK);
    mock_flags |= TIM_FLAG_UPDATE;
    HAL_TIM_IRQHandler(&htim);
    assert(mock_period_callback_count == 1U);
    assert((mock_flags & TIM_FLAG_UPDATE) == 0U);
}

static void test_master_slave_and_one_pulse(void)
{
    TIM_HandleTypeDef htim = mock_handle(TIM2);
    TIM_MasterConfigTypeDef master;
    TIM_SlaveConfigTypeDef slave;
    TIM_OnePulse_InitTypeDef one_pulse = {0};

    assert(HAL_TIM_Base_Init(&htim) == HAL_OK);
    master.MasterOutputTrigger = TIM_TRGO_UPDATE;
    master.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
    assert(HAL_TIMEx_MasterConfigSynchronization(&htim, &master) == HAL_OK);
    assert(mock_master_trigger == 2U);
    assert(mock_master_slave != 0);

    slave.SlaveMode = TIM_SLAVEMODE_TRIGGER;
    slave.InputTrigger = TIM_TS_ITR0;
    slave.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
    slave.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
    slave.TriggerFilter = 0U;
    assert(HAL_TIM_SlaveConfigSynchro_IT(&htim, &slave) == HAL_OK);
    assert(mock_slave.slave_mode == 3U);
    assert(mock_slave.input_trigger == 0U);
    assert((mock_interrupts & TIM_IT_TRIGGER) != 0U);
    mock_flags |= TIM_FLAG_TRIGGER;
    HAL_TIM_IRQHandler(&htim);
    assert(mock_trigger_callback_count == 1U);

    htim.State = HAL_TIM_STATE_RESET;
    assert(HAL_TIM_OnePulse_Init(&htim, TIM_OPMODE_SINGLE) == HAL_OK);
    assert(mock_one_pulse != 0);
    one_pulse.OCMode = TIM_OCMODE_PWM1;
    one_pulse.Pulse = 100U;
    one_pulse.OCPolarity = TIM_OCPOLARITY_HIGH;
    one_pulse.ICPolarity = TIM_ICPOLARITY_RISING;
    one_pulse.ICSelection = TIM_ICSELECTION_DIRECTTI;
    one_pulse.ICFilter = 2U;
    assert(HAL_TIM_OnePulse_ConfigChannel(&htim,
                                          &one_pulse,
                                          TIM_CHANNEL_1,
                                          TIM_CHANNEL_2) == HAL_OK);
    assert(mock_slave.input_trigger == 6U);
    assert(HAL_TIM_OnePulse_Start_IT(&htim, TIM_CHANNEL_1) == HAL_OK);
    assert(mock_channel_enabled[0] == 1U);
    assert(mock_channel_enabled[1] == 1U);
    assert(HAL_TIM_OnePulse_Stop_IT(&htim, TIM_CHANNEL_1) == HAL_OK);
}

static void test_pwm_dma(void)
{
    TIM_HandleTypeDef htim = mock_handle(TIM2);
    DMA_HandleTypeDef hdma;
    TIM_OC_InitTypeDef oc = {0};
    uint16_t pulses[] = {100U, 200U, 300U};

    memset(&hdma, 0, sizeof(hdma));
    hdma.Instance = DMA1_Stream5;
    hdma.Init.Channel = DMA_CHANNEL_3;
    hdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma.Init.Mode = DMA_NORMAL;
    assert(HAL_DMA_Init(&hdma) == HAL_OK);

    assert(HAL_TIM_PWM_Init(&htim) == HAL_OK);
    oc.OCMode = TIM_OCMODE_PWM1;
    oc.Pulse = 100U;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    assert(HAL_TIM_PWM_ConfigChannel(&htim, &oc, TIM_CHANNEL_1) == HAL_OK);
    __HAL_LINKDMA(&htim, hdma[TIM_DMA_ID_CC1], hdma);
    assert(HAL_TIM_PWM_Start_DMA(&htim, TIM_CHANNEL_1,
                                 (const uint32_t *)(const void *)pulses,
                                 3U) == HAL_OK);
    assert(mock_dma_start_count == 1U);
    assert(mock_dma_length == 3U);
    assert(mock_dma_destination == GD32_HAL_TIMER1_ADDRESS + 0x34U);
    assert(hdma.GD32_INSTANCE == GD32_HAL_DMA0_CHANNEL4_ADDRESS);
    assert(hdma.gd32_dma_periph == GD32_HAL_DMA0_ADDRESS);
    assert(hdma.gd32_dma_channel == 4U);
    assert(hdma.GD32_REQUEST == GD32_DMA_REQUEST_TIMER1_CH0);
    assert((mock_dma_requests & (1UL << GD32_HAL_TIMER_DMA_CC1)) != 0U);
    assert(mock_interrupts == 0U);
    hdma.XferHalfCpltCallback(&hdma);
    assert(mock_pwm_half_callback_count == 1U);
    assert(mock_callback_channel == HAL_TIM_ACTIVE_CHANNEL_1);
    assert((mock_dma_requests & (1UL << GD32_HAL_TIMER_DMA_CC1)) != 0U);
    hdma.XferCpltCallback(&hdma);
    assert(HAL_TIM_GetChannelState(&htim, TIM_CHANNEL_1) == HAL_TIM_CHANNEL_STATE_READY);
    assert(mock_pwm_callback_count == 1U);
    assert(mock_dma_source == (uint32_t)(uintptr_t)pulses);
    assert(mock_channel_enabled[0] == 1U);
    assert((mock_dma_requests & (1UL << GD32_HAL_TIMER_DMA_CC1)) != 0U);
    hdma.State = HAL_DMA_STATE_READY;
    assert(HAL_TIM_PWM_Stop_DMA(&htim, TIM_CHANNEL_1) == HAL_OK);
    assert(mock_channel_enabled[0] == 0U);
    assert((mock_dma_requests & (1UL << GD32_HAL_TIMER_DMA_CC1)) == 0U);

    hdma.Init.Mode = DMA_CIRCULAR;
    assert(HAL_TIM_PWM_Start_DMA(&htim, TIM_CHANNEL_1,
                                 (const uint32_t *)(const void *)pulses,
                                 3U) == HAL_OK);
    hdma.XferCpltCallback(&hdma);
    assert(HAL_TIM_GetChannelState(&htim, TIM_CHANNEL_1) ==
           HAL_TIM_CHANNEL_STATE_BUSY);
    assert(hdma.GD32_ACTIVE_TIM_DMA_ID == TIM_DMA_ID_CC1);
    assert(HAL_TIM_PWM_Stop_DMA(&htim, TIM_CHANNEL_1) == HAL_OK);
}

static void test_oc_ic_dma_width_and_guards(void)
{
    TIM_HandleTypeDef htim = mock_handle(TIM2);
    DMA_HandleTypeDef hdma;
    TIM_OC_InitTypeDef oc = {0};
    TIM_IC_InitTypeDef ic = {0};
    uint32_t word_values[] = {100U, 0x10000U};
    uint16_t half_values[] = {10U, 20U};
    uint32_t alignment_storage[2] = {0U};
    DMA_HandleTypeDef unsupported;

    memset(&hdma, 0, sizeof(hdma));
    hdma.Instance = GD32_DMA0_CHANNEL4;
    hdma.Init.Channel = GD32_DMA_REQUEST_TIMER1_CH0;
    hdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode = DMA_NORMAL;
    hdma.State = HAL_DMA_STATE_READY;
    hdma.Parent = &htim;
    htim.hdma[TIM_DMA_ID_CC1] = &hdma;

    assert(HAL_TIM_OC_Init(&htim) == HAL_OK);
    oc.OCMode = TIM_OCMODE_TOGGLE;
    oc.Pulse = 10U;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    assert(HAL_TIM_OC_ConfigChannel(&htim, &oc, TIM_CHANNEL_1) == HAL_OK);
    assert(HAL_TIM_OC_Start_DMA(&htim, TIM_CHANNEL_1, word_values, 2U) ==
           HAL_ERROR);
    assert(mock_error == GD32_HAL_PORT_ERROR_TIMER_16BIT_RANGE);

    word_values[1] = 200U;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    assert(HAL_TIM_OC_Start_DMA(&htim, TIM_CHANNEL_1, word_values, 2U) ==
           HAL_ERROR);
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    assert(HAL_TIM_OC_Start_DMA(
               &htim, TIM_CHANNEL_1,
               (const uint32_t *)(const void *)((uint8_t *)alignment_storage + 1U),
               1U) == HAL_ERROR);

    assert(HAL_TIM_OC_Start_DMA(
               &htim, TIM_CHANNEL_1,
               (const uint32_t *)(const void *)half_values, 2U) == HAL_OK);
    assert((mock_dma_requests & (1UL << GD32_HAL_TIMER_DMA_CC1)) != 0U);
    assert(mock_interrupts == 0U);
    assert(HAL_TIM_OC_Stop_DMA(&htim, TIM_CHANNEL_1) == HAL_OK);
    assert(mock_dma_abort_count == 1U);
    assert((mock_dma_requests & (1UL << GD32_HAL_TIMER_DMA_CC1)) == 0U);

    memset(&hdma, 0, sizeof(hdma));
    hdma.Instance = DMA1_Stream5;
    hdma.Init.Channel = DMA_CHANNEL_3;
    hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma.Init.Mode = DMA_NORMAL;
    assert(HAL_DMA_Init(&hdma) == HAL_OK);
    assert(HAL_TIM_IC_Init(&htim) == HAL_OK);
    ic.ICPolarity = TIM_ICPOLARITY_RISING;
    ic.ICSelection = TIM_ICSELECTION_DIRECTTI;
    ic.ICPrescaler = TIM_ICPSC_DIV1;
    assert(HAL_TIM_IC_ConfigChannel(&htim, &ic, TIM_CHANNEL_1) == HAL_OK);
    __HAL_LINKDMA(&htim, hdma[TIM_DMA_ID_CC1], hdma);
    assert(HAL_TIM_IC_Start_DMA(&htim, TIM_CHANNEL_1,
                                (uint32_t *)(void *)half_values, 2U) == HAL_OK);
    assert(mock_dma_source == GD32_HAL_TIMER1_ADDRESS + 0x34U);
    assert(mock_dma_destination == (uint32_t)(uintptr_t)half_values);
    assert(hdma.GD32_INSTANCE == GD32_HAL_DMA0_CHANNEL4_ADDRESS);
    assert(hdma.GD32_REQUEST == GD32_DMA_REQUEST_TIMER1_CH0);
    assert(mock_interrupts == 0U);
    hdma.XferHalfCpltCallback(&hdma);
    assert(mock_ic_half_callback_count == 1U);
    assert(mock_callback_channel == HAL_TIM_ACTIVE_CHANNEL_1);
    assert((mock_dma_requests & (1UL << GD32_HAL_TIMER_DMA_CC1)) != 0U);
    hdma.XferCpltCallback(&hdma);
    assert(mock_ic_callback_count == 1U);
    assert(HAL_TIM_IC_Stop_DMA(&htim, TIM_CHANNEL_1) == HAL_OK);

    memset(&unsupported, 0, sizeof(unsupported));
    unsupported.Instance = DMA1_Stream1;
    unsupported.Init.Channel = DMA_CHANNEL_3;
    unsupported.Init.Direction = DMA_MEMORY_TO_PERIPH;
    unsupported.Init.PeriphInc = DMA_PINC_DISABLE;
    unsupported.Init.MemInc = DMA_MINC_ENABLE;
    unsupported.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    unsupported.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    unsupported.Init.Mode = DMA_NORMAL;
    assert(HAL_DMA_Init(&unsupported) == HAL_OK);
    __HAL_LINKDMA(&htim, hdma[TIM_DMA_ID_CC2], unsupported);
    assert(HAL_TIM_PWM_Start_DMA(
               &htim, TIM_CHANNEL_2,
               (const uint32_t *)(const void *)half_values, 2U) == HAL_ERROR);
    assert((unsupported.ErrorCode & HAL_DMA_ERROR_REQUEST) != 0U);
    assert(mock_error == GD32_HAL_PORT_ERROR_TIMER_DMA_UNSUPPORTED);
}

static void test_base_dma(void)
{
    TIM_HandleTypeDef htim = mock_handle(TIM2);
    DMA_HandleTypeDef hdma;
    uint32_t periods[] = {999U, 499U};

    memset(&hdma, 0, sizeof(hdma));
    hdma.Instance = DMA1_Stream1;
    hdma.Init.Channel = DMA_CHANNEL_3;
    hdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode = DMA_NORMAL;
    assert(HAL_DMA_Init(&hdma) == HAL_OK);

    assert(HAL_TIM_Base_Init(&htim) == HAL_OK);
    __HAL_LINKDMA(&htim, hdma[TIM_DMA_ID_UPDATE], hdma);
    assert(HAL_TIM_Base_Start_DMA(&htim, periods, 2U) == HAL_OK);
    assert(mock_dma_destination == GD32_HAL_TIMER1_ADDRESS + 0x2CU);
    assert(hdma.GD32_INSTANCE == GD32_HAL_DMA0_CHANNEL1_ADDRESS);
    assert(hdma.GD32_REQUEST == GD32_DMA_REQUEST_TIMER1_UP);
    assert((mock_dma_requests & (1UL << GD32_HAL_TIMER_DMA_UPDATE)) != 0U);
    assert(mock_interrupts == 0U);
    hdma.XferHalfCpltCallback(&hdma);
    assert(mock_period_half_callback_count == 1U);
    assert((mock_dma_requests & (1UL << GD32_HAL_TIMER_DMA_UPDATE)) != 0U);
    hdma.XferCpltCallback(&hdma);
    assert(htim.State == HAL_TIM_STATE_READY);
    assert(mock_period_callback_count == 1U);
    hdma.State = HAL_DMA_STATE_READY;
    assert(HAL_TIM_Base_Stop_DMA(&htim) == HAL_OK);
    assert((mock_dma_requests & (1UL << GD32_HAL_TIMER_DMA_UPDATE)) == 0U);
    assert(mock_enabled == 0);
}

static void test_tim1_shared_stream_dma_events(void)
{
    TIM_HandleTypeDef htim = mock_handle(TIM1);
    DMA_HandleTypeDef hdma;
    TIM_OC_InitTypeDef oc = {0};
    uint16_t pulses[] = {11U, 22U};
    uint32_t cc2_instance;
    uint32_t cc2_request;

    memset(&hdma, 0, sizeof(hdma));
    hdma.Instance = DMA2_Stream6;
    hdma.Init.Channel = DMA_CHANNEL_0;
    hdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma.Init.Mode = DMA_NORMAL;
    assert(HAL_DMA_Init(&hdma) == HAL_OK);
    assert(hdma.GD32_MAPPING_ORIGIN == GD32_HAL_DMA_MAPPING_STM32_TIMER);

    assert(HAL_TIM_PWM_Init(&htim) == HAL_OK);
    oc.OCMode = TIM_OCMODE_PWM1;
    oc.Pulse = 11U;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    assert(HAL_TIM_PWM_ConfigChannel(&htim, &oc, TIM_CHANNEL_1) == HAL_OK);
    assert(HAL_TIM_PWM_ConfigChannel(&htim, &oc, TIM_CHANNEL_2) == HAL_OK);
    assert(HAL_TIM_PWM_ConfigChannel(&htim, &oc, TIM_CHANNEL_3) == HAL_OK);
    __HAL_LINKDMA(&htim, hdma[TIM_DMA_ID_CC1], hdma);
    __HAL_LINKDMA(&htim, hdma[TIM_DMA_ID_CC2], hdma);
    __HAL_LINKDMA(&htim, hdma[TIM_DMA_ID_CC3], hdma);

    assert(HAL_TIM_PWM_Start_DMA(
               &htim, TIM_CHANNEL_1,
               (const uint32_t *)(const void *)pulses, 2U) == HAL_OK);
    assert(hdma.GD32_INSTANCE == GD32_HAL_DMA0_CHANNEL1_ADDRESS);
    assert(hdma.gd32_dma_periph == GD32_HAL_DMA0_ADDRESS);
    assert(hdma.gd32_dma_channel == 1U);
    assert(hdma.GD32_REQUEST == GD32_DMA_REQUEST_TIMER0_CH0);
    assert(mock_dma_destination == GD32_HAL_TIMER0_ADDRESS + 0x34U);
    assert(hdma.Init.Direction == DMA_MEMORY_TO_PERIPH);
    assert(HAL_TIM_PWM_Stop_DMA(&htim, TIM_CHANNEL_1) == HAL_OK);

    assert(HAL_TIM_PWM_Start_DMA(
               &htim, TIM_CHANNEL_1,
               (const uint32_t *)(const void *)pulses, 2U) == HAL_OK);
    assert(hdma.GD32_INSTANCE == GD32_HAL_DMA0_CHANNEL1_ADDRESS);
    assert(hdma.GD32_REQUEST == GD32_DMA_REQUEST_TIMER0_CH0);
    assert(HAL_TIM_PWM_Stop_DMA(&htim, TIM_CHANNEL_1) == HAL_OK);

    assert(HAL_TIM_PWM_Start_DMA(
               &htim, TIM_CHANNEL_2,
               (const uint32_t *)(const void *)pulses, 2U) == HAL_OK);
    assert(hdma.GD32_INSTANCE == GD32_HAL_DMA0_CHANNEL2_ADDRESS);
    assert(hdma.gd32_dma_periph == GD32_HAL_DMA0_ADDRESS);
    assert(hdma.gd32_dma_channel == 2U);
    assert(hdma.GD32_REQUEST == GD32_DMA_REQUEST_TIMER0_CH1);
    assert(mock_dma_destination == GD32_HAL_TIMER0_ADDRESS + 0x38U);
    assert(hdma.GD32_ACTIVE_TIM_DMA_ID == TIM_DMA_ID_CC2);
    cc2_instance = hdma.GD32_INSTANCE;
    cc2_request = hdma.GD32_REQUEST;
    assert(HAL_TIM_PWM_Start_DMA(
               &htim, TIM_CHANNEL_3,
               (const uint32_t *)(const void *)pulses, 2U) == HAL_BUSY);
    assert(hdma.GD32_INSTANCE == cc2_instance);
    assert(hdma.GD32_REQUEST == cc2_request);
    assert(hdma.GD32_ACTIVE_TIM_DMA_ID == TIM_DMA_ID_CC2);

    hdma.State = HAL_DMA_STATE_READY;
    hdma.XferCpltCallback(&hdma);
    assert(mock_callback_channel == HAL_TIM_ACTIVE_CHANNEL_2);
    assert(HAL_TIM_GetChannelState(&htim, TIM_CHANNEL_2) ==
           HAL_TIM_CHANNEL_STATE_READY);
    assert(hdma.GD32_ACTIVE_TIM_DMA_ID == GD32_HAL_DMA_ACTIVE_TIM_NONE);
    assert(HAL_TIM_PWM_Stop_DMA(&htim, TIM_CHANNEL_2) == HAL_OK);

    assert(HAL_TIM_PWM_Start_DMA(
               &htim, TIM_CHANNEL_3,
               (const uint32_t *)(const void *)pulses, 2U) == HAL_OK);
    assert(hdma.GD32_INSTANCE == GD32_HAL_DMA0_CHANNEL5_ADDRESS);
    assert(hdma.gd32_dma_periph == GD32_HAL_DMA0_ADDRESS);
    assert(hdma.gd32_dma_channel == 5U);
    assert(hdma.GD32_REQUEST == GD32_DMA_REQUEST_TIMER0_CH2);
    assert(mock_dma_destination == GD32_HAL_TIMER0_ADDRESS + 0x3CU);
    assert(hdma.Init.Direction == DMA_MEMORY_TO_PERIPH);
    assert(hdma.GD32_ACTIVE_TIM_DMA_ID == TIM_DMA_ID_CC3);
    hdma.State = HAL_DMA_STATE_READY;
    hdma.XferErrorCallback(&hdma);
    assert(mock_error_callback_count == 1U);
    assert((mock_dma_requests & (1UL << GD32_HAL_TIMER_DMA_CC3)) == 0U);
    assert(mock_channel_enabled[2] == 0U);
    assert(HAL_TIM_GetChannelState(&htim, TIM_CHANNEL_3) ==
           HAL_TIM_CHANNEL_STATE_READY);
    assert(hdma.GD32_ACTIVE_TIM_DMA_ID == GD32_HAL_DMA_ACTIVE_TIM_NONE);
}

int main(void)
{
    mock_reset();
    test_base_and_16bit_boundary();
    mock_reset();
    test_pwm_oc_ic_and_irq();
    mock_reset();
    test_master_slave_and_one_pulse();
    mock_reset();
    test_pwm_dma();
    mock_reset();
    test_oc_ic_dma_width_and_guards();
    mock_reset();
    test_base_dma();
    mock_reset();
    test_tim1_shared_stream_dma_events();
    puts("TIM host tests: PASS");
    return 0;
}
