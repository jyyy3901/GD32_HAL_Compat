#include "stm32f4xx_hal.h"
#include "gd32_hal_port.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

__IO GD32_HAL_PortError gd32HalLastPortError;
__IO uint32_t gd32HalLastPortErrorDetail;

static GD32_HAL_ADCConfig configured;
static int configure_count;
static int msp_init_count;
static int msp_deinit_count;
static int calibration_count;
static int disable_count;
static int adc_enabled;
static int calibration_valid;
static int calibration_status;
static int software_start_count;
static uint32_t adc_flags;
static uint32_t adc_interrupts;
static uint32_t adc_value;
static int dma_request_enabled;
static int dma_abort_count;
static uint32_t dma_source;
static uint32_t dma_destination;
static uint32_t dma_length;
static HAL_StatusTypeDef dma_start_status;
static HAL_StatusTypeDef dma_abort_status;
static int conv_count;
static int half_count;
static int error_count;
static uint32_t mock_tick;
static int tick_advances;
static uint8_t last_rank;
static uint8_t last_channel;
static uint8_t last_sample_time;

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    gd32HalLastPortError = error;
    gd32HalLastPortErrorDetail = detail;
}

uint32_t HAL_GetTick(void)
{
    uint32_t result = mock_tick;
    if (tick_advances != 0)
    {
        ++mock_tick;
    }
    return result;
}

int GD32_HAL_ADC_IsInstance(uint32_t adc_address)
{
    return adc_address == GD32_HAL_ADC0_ADDRESS;
}

int GD32_HAL_ADC_MapSTM32Trigger(uint32_t source,
                                 uint32_t edge,
                                 GD32_HAL_ADCTrigger *trigger)
{
    if (trigger == NULL)
    {
        return -1;
    }
    if (source == ADC_SOFTWARE_START)
        *trigger = GD32_HAL_ADC_TRIGGER_SOFTWARE;
    else if (source == ADC_EXTERNALTRIGCONV_T1_CC1)
        *trigger = GD32_HAL_ADC_TRIGGER_TIMER0_CH0;
    else if (source == ADC_EXTERNALTRIGCONV_T1_CC2)
        *trigger = GD32_HAL_ADC_TRIGGER_TIMER0_CH1;
    else if (source == ADC_EXTERNALTRIGCONV_T1_CC3)
        *trigger = GD32_HAL_ADC_TRIGGER_TIMER0_CH2;
    else if (source == ADC_EXTERNALTRIGCONV_T2_CC2)
        *trigger = GD32_HAL_ADC_TRIGGER_TIMER1_CH1;
    else if (source == ADC_EXTERNALTRIGCONV_T3_TRGO)
        *trigger = GD32_HAL_ADC_TRIGGER_TIMER2_TRGO;
    else if (source == ADC_EXTERNALTRIGCONV_T4_CC4)
        *trigger = GD32_HAL_ADC_TRIGGER_TIMER3_CH3;
    else if (source == ADC_EXTERNALTRIGCONV_Ext_IT11)
        *trigger = GD32_HAL_ADC_TRIGGER_EXTI11;
    else
        return -1;

    if (((*trigger == GD32_HAL_ADC_TRIGGER_SOFTWARE) &&
         (edge != ADC_EXTERNALTRIGCONVEDGE_NONE)) ||
        ((*trigger != GD32_HAL_ADC_TRIGGER_SOFTWARE) &&
         (edge != ADC_EXTERNALTRIGCONVEDGE_RISING)))
    {
        return -2;
    }
    return 0;
}

int GD32_HAL_ADC_MapSTM32SampleTime(uint32_t source, uint8_t *target)
{
    static const uint8_t map[] = {1U, 3U, 3U, 6U, 7U, 7U, 7U};

    if ((target == NULL) || (source >= ADC_SAMPLETIME_480CYCLES))
    {
        return -1;
    }
    *target = map[source];
    return 0;
}

int GD32_HAL_ADC_Configure(uint32_t adc_address,
                           const GD32_HAL_ADCConfig *config)
{
    assert(adc_address == GD32_HAL_ADC0_ADDRESS);
    configured = *config;
    ++configure_count;
    adc_enabled = 0;
    calibration_valid = 0;
    return 0;
}

void GD32_HAL_ADC_DeInit(uint32_t adc_address)
{
    assert(adc_address == GD32_HAL_ADC0_ADDRESS);
    adc_enabled = 0;
    calibration_valid = 0;
}

int GD32_HAL_ADC_ConfigChannel(uint32_t adc_address,
                               uint8_t rank,
                               uint8_t channel,
                               uint8_t sample_time)
{
    assert(adc_address == GD32_HAL_ADC0_ADDRESS);
    last_rank = rank;
    last_channel = channel;
    last_sample_time = sample_time;
    return 0;
}

int GD32_HAL_ADC_EnableAndCalibrate(uint32_t adc_address)
{
    assert(adc_address == GD32_HAL_ADC0_ADDRESS);
    if (calibration_status != 0)
    {
        return calibration_status;
    }
    if (adc_enabled == 0)
    {
        adc_enabled = 1;
    }
    if (calibration_valid == 0)
    {
        ++calibration_count;
        calibration_valid = 1;
    }
    return 0;
}

void GD32_HAL_ADC_Disable(uint32_t adc_address)
{
    assert(adc_address == GD32_HAL_ADC0_ADDRESS);
    adc_enabled = 0;
    calibration_valid = 0;
    ++disable_count;
}

int GD32_HAL_ADC_IsEnabled(uint32_t adc_address)
{
    return (adc_address == GD32_HAL_ADC0_ADDRESS) && (adc_enabled != 0);
}

void GD32_HAL_ADC_StartSoftware(uint32_t adc_address)
{
    assert(adc_address == GD32_HAL_ADC0_ADDRESS);
    ++software_start_count;
}

uint32_t GD32_HAL_ADC_GetFlag(uint32_t adc_address, uint32_t flags)
{
    assert(adc_address == GD32_HAL_ADC0_ADDRESS);
    return adc_flags & flags;
}

void GD32_HAL_ADC_ClearFlag(uint32_t adc_address, uint32_t flags)
{
    assert(adc_address == GD32_HAL_ADC0_ADDRESS);
    adc_flags &= ~flags;
}

void GD32_HAL_ADC_SetInterrupt(uint32_t adc_address,
                               uint32_t interrupts,
                               int enable)
{
    assert(adc_address == GD32_HAL_ADC0_ADDRESS);
    if (enable != 0)
    {
        adc_interrupts |= interrupts;
    }
    else
    {
        adc_interrupts &= ~interrupts;
    }
}

uint32_t GD32_HAL_ADC_GetInterrupts(uint32_t adc_address)
{
    assert(adc_address == GD32_HAL_ADC0_ADDRESS);
    return adc_interrupts;
}

uint32_t GD32_HAL_ADC_ReadData(uint32_t adc_address)
{
    assert(adc_address == GD32_HAL_ADC0_ADDRESS);
    return adc_value;
}

uint32_t GD32_HAL_ADC_GetDataAddress(uint32_t adc_address)
{
    assert(adc_address == GD32_HAL_ADC0_ADDRESS);
    return adc_address + 0x4CU;
}

void GD32_HAL_ADC_SetDMARequest(uint32_t adc_address, int enable)
{
    assert(adc_address == GD32_HAL_ADC0_ADDRESS);
    dma_request_enabled = enable;
}

int GD32_HAL_ADC_IsDMAChannelValid(uint32_t adc_address,
                                   uint32_t channel_address,
                                   uint32_t request_token)
{
    return (adc_address == GD32_HAL_ADC0_ADDRESS) &&
           (channel_address == GD32_HAL_DMA0_CHANNEL0_ADDRESS) &&
           (request_token == GD32_DMA_REQUEST_ADC0);
}

HAL_StatusTypeDef HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma,
                                   uint32_t SrcAddress,
                                   uint32_t DstAddress,
                                   uint32_t DataLength)
{
    dma_source = SrcAddress;
    dma_destination = DstAddress;
    dma_length = DataLength;
    if (dma_start_status == HAL_OK)
    {
        hdma->State = HAL_DMA_STATE_BUSY;
    }
    return dma_start_status;
}

HAL_StatusTypeDef HAL_DMA_Abort(DMA_HandleTypeDef *hdma)
{
    ++dma_abort_count;
    if (dma_abort_status == HAL_OK)
    {
        hdma->State = HAL_DMA_STATE_READY;
    }
    return dma_abort_status;
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    assert(hadc != NULL);
    ++msp_init_count;
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
    assert(hadc != NULL);
    ++msp_deinit_count;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    assert(hadc != NULL);
    ++conv_count;
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    assert(hadc != NULL);
    ++half_count;
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    assert(hadc != NULL);
    ++error_count;
}

static ADC_HandleTypeDef MakeADC(void)
{
    ADC_HandleTypeDef hadc;
    memset(&hadc, 0, sizeof(hadc));
    hadc.Instance = ADC1;
    hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.ScanConvMode = DISABLE;
    hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.NbrOfConversion = 1U;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.NbrOfDiscConversion = 1U;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.DMAContinuousRequests = DISABLE;
    hadc.State = HAL_ADC_STATE_RESET;
    return hadc;
}

static DMA_HandleTypeDef MakeADCDMA(ADC_HandleTypeDef *hadc, uint32_t mode)
{
    DMA_HandleTypeDef hdma;
    memset(&hdma, 0, sizeof(hdma));
    hdma.Instance = GD32_DMA0_CHANNEL0;
    hdma.Init.Channel = GD32_DMA_REQUEST_ADC0;
    hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode = mode;
    hdma.State = HAL_DMA_STATE_READY;
    hdma.Parent = hadc;
    return hdma;
}

static void ResetMocks(void)
{
    memset(&configured, 0, sizeof(configured));
    configure_count = 0;
    msp_init_count = 0;
    msp_deinit_count = 0;
    calibration_count = 0;
    disable_count = 0;
    adc_enabled = 0;
    calibration_valid = 0;
    calibration_status = 0;
    software_start_count = 0;
    adc_flags = 0U;
    adc_interrupts = 0U;
    adc_value = 0U;
    dma_request_enabled = 0;
    dma_abort_count = 0;
    dma_source = 0U;
    dma_destination = 0U;
    dma_length = 0U;
    dma_start_status = HAL_OK;
    dma_abort_status = HAL_OK;
    conv_count = 0;
    half_count = 0;
    error_count = 0;
    mock_tick = 0U;
    tick_advances = 0;
    last_rank = 0U;
    last_channel = 0U;
    last_sample_time = 0U;
    gd32HalLastPortError = GD32_HAL_PORT_ERROR_NONE;
    gd32HalLastPortErrorDetail = 0U;
}

static void TestInitChannelPolling(void)
{
    ADC_HandleTypeDef hadc = MakeADC();
    ADC_ChannelConfTypeDef channel = {ADC_CHANNEL_5, ADC_REGULAR_RANK_1,
                                      ADC_SAMPLETIME_3CYCLES, 0U};

    assert(HAL_ADC_Init(&hadc) == HAL_OK);
    assert(configure_count == 1);
    assert(msp_init_count == 1);
    assert(configured.clock_divider == 4U);
    assert(configured.resolution_bits == 12U);
    assert(configured.trigger == GD32_HAL_ADC_TRIGGER_SOFTWARE);
    assert(calibration_count == 0);

    assert(HAL_ADC_ConfigChannel(&hadc, &channel) == HAL_OK);
    assert(last_rank == 0U);
    assert(last_channel == 5U);
    assert(last_sample_time == 1U);
    channel.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    assert(HAL_ADC_ConfigChannel(&hadc, &channel) == HAL_OK);
    assert(last_sample_time == 6U);
    channel.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    assert(HAL_ADC_ConfigChannel(&hadc, &channel) == HAL_ERROR);
    assert(gd32HalLastPortError ==
           GD32_HAL_PORT_ERROR_ADC_SAMPLE_TIME_UNMAPPABLE);
    channel.SamplingTime = ADC_SAMPLETIME_28CYCLES;
    channel.Channel = ADC_CHANNEL_VBAT;
    assert(HAL_ADC_ConfigChannel(&hadc, &channel) == HAL_ERROR);

    /* 直接寄存器语义先 enable 也不能让 HAL 跳过 calibration。 */
    adc_enabled = 1;
    calibration_valid = 0;
    assert(HAL_ADC_Start(&hadc) == HAL_OK);
    assert(calibration_count == 1);
    assert(software_start_count == 1);
    assert((hadc.State & HAL_ADC_STATE_REG_BUSY) != 0U);
    adc_value = 0x0ABCU;
    adc_flags = GD32_HAL_ADC_FLAG_EOC;
    assert(HAL_ADC_PollForConversion(&hadc, 10U) == HAL_OK);
    assert(HAL_ADC_GetValue(&hadc) == 0x0ABCU);
    assert((hadc.State & HAL_ADC_STATE_READY) != 0U);
    assert((hadc.State & HAL_ADC_STATE_REG_EOC) != 0U);
    assert(HAL_ADC_Stop(&hadc) == HAL_OK);
    assert(HAL_ADC_Start(&hadc) == HAL_OK);
    assert(calibration_count == 2);
    tick_advances = 1;
    assert(HAL_ADC_PollForConversion(&hadc, 2U) == HAL_TIMEOUT);
    assert((hadc.State & HAL_ADC_STATE_TIMEOUT) != 0U);
    assert(HAL_ADC_Stop(&hadc) == HAL_OK);
    assert(HAL_ADC_DeInit(&hadc) == HAL_OK);
    assert(msp_deinit_count == 1);
}

static void TestTriggerAndIT(void)
{
    ADC_HandleTypeDef hadc = MakeADC();
    int software_before;

    hadc.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    assert(HAL_ADC_Init(&hadc) == HAL_OK);
    assert(configured.trigger == GD32_HAL_ADC_TRIGGER_TIMER2_TRGO);
    software_before = software_start_count;
    assert(HAL_ADC_Start_IT(&hadc) == HAL_OK);
    assert(calibration_count == 1);
    assert(software_start_count == software_before);
    assert((adc_interrupts & GD32_HAL_ADC_INTERRUPT_EOC) != 0U);
    adc_flags = GD32_HAL_ADC_FLAG_EOC;
    HAL_ADC_IRQHandler(&hadc);
    assert(conv_count == 1);
    assert((hadc.State & HAL_ADC_STATE_READY) != 0U);
    assert((adc_interrupts & GD32_HAL_ADC_INTERRUPT_EOC) == 0U);

    assert(HAL_ADC_Stop_IT(&hadc) == HAL_OK);
    hadc.Init.ContinuousConvMode = ENABLE;
    assert(HAL_ADC_Init(&hadc) == HAL_OK);
    assert(HAL_ADC_Start_IT(&hadc) == HAL_OK);
    adc_flags = GD32_HAL_ADC_FLAG_EOC;
    HAL_ADC_IRQHandler(&hadc);
    assert(conv_count == 2);
    assert((hadc.State & HAL_ADC_STATE_REG_BUSY) != 0U);
    assert((adc_interrupts & GD32_HAL_ADC_INTERRUPT_EOC) != 0U);
    assert(HAL_ADC_Stop_IT(&hadc) == HAL_OK);

    hadc.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    assert(HAL_ADC_Init(&hadc) == HAL_ERROR);
    assert(gd32HalLastPortError == GD32_HAL_PORT_ERROR_ADC_TRIGGER_UNMAPPABLE);
    hadc = MakeADC();
    hadc.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_FALLING;
    assert(HAL_ADC_Init(&hadc) == HAL_ERROR);
    assert(gd32HalLastPortError ==
           GD32_HAL_PORT_ERROR_ADC_TRIGGER_EDGE_UNSUPPORTED);
    hadc = MakeADC();
    hadc.Init.ScanConvMode = ENABLE;
    hadc.Init.NbrOfConversion = 2U;
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    assert(HAL_ADC_Init(&hadc) == HAL_ERROR);
}

static void TestDMA(void)
{
    ADC_HandleTypeDef hadc = MakeADC();
    DMA_HandleTypeDef hdma;
    uint32_t samples[4] = {0U};
    uint16_t half_samples[4] = {0U};
    uint32_t alignment_storage[3] = {0U};

    hadc.Init.ScanConvMode = ENABLE;
    hadc.Init.NbrOfConversion = 2U;
    assert(HAL_ADC_Init(&hadc) == HAL_OK);
    hdma = MakeADCDMA(&hadc, DMA_NORMAL);
    hadc.DMA_Handle = &hdma;
    assert(HAL_ADC_Start_DMA(&hadc, samples, 2U) == HAL_OK);
    assert(calibration_count == 1);
    assert(dma_source == GD32_HAL_ADC0_ADDRESS + 0x4CU);
    assert(dma_destination == (uint32_t)(uintptr_t)samples);
    assert(dma_length == 2U);
    assert(dma_request_enabled == 1);
    assert(hdma.XferHalfCpltCallback != NULL);
    assert(hdma.XferCpltCallback != NULL);
    hdma.XferHalfCpltCallback(&hdma);
    assert(half_count == 1);
    hdma.State = HAL_DMA_STATE_READY;
    hdma.XferCpltCallback(&hdma);
    assert(conv_count == 1);
    assert(dma_request_enabled == 0);
    assert((hadc.State & HAL_ADC_STATE_READY) != 0U);

    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    assert(HAL_ADC_Start_DMA(&hadc,
                             (uint32_t *)(void *)half_samples, 2U) == HAL_OK);
    assert(dma_length == 2U);
    hdma.State = HAL_DMA_STATE_READY;
    hdma.XferCpltCallback(&hdma);

    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    assert(HAL_ADC_Start_DMA(&hadc, samples, 2U) == HAL_ERROR);
    assert(gd32HalLastPortError ==
           GD32_HAL_PORT_ERROR_ADC_DMA_CONFIG_MISMATCH);
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    assert(HAL_ADC_Start_DMA(
               &hadc,
               (uint32_t *)(void *)((uint8_t *)alignment_storage + 1U),
               2U) == HAL_ERROR);
    assert(gd32HalLastPortError ==
           GD32_HAL_PORT_ERROR_ADC_DMA_CONFIG_MISMATCH);
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;

    assert(HAL_ADC_Start_DMA(&hadc, samples, 3U) == HAL_ERROR);
    assert(HAL_ADC_Start_DMA(&hadc, samples, 2U) == HAL_OK);
    assert(HAL_ADC_Stop_DMA(&hadc) == HAL_OK);
    assert(dma_abort_count == 1);

    hadc.Init.ContinuousConvMode = ENABLE;
    hadc.Init.DMAContinuousRequests = ENABLE;
    assert(HAL_ADC_Init(&hadc) == HAL_OK);
    hdma = MakeADCDMA(&hadc, DMA_CIRCULAR);
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hadc.DMA_Handle = &hdma;
    assert(HAL_ADC_Start_DMA(&hadc,
                             (uint32_t *)(void *)half_samples, 4U) == HAL_OK);
    hdma.XferCpltCallback(&hdma);
    assert(conv_count == 3);
    assert(dma_request_enabled == 1);
    assert((hadc.State & HAL_ADC_STATE_REG_BUSY) != 0U);
    hdma.XferErrorCallback(&hdma);
    assert(error_count == 1);
    assert(dma_request_enabled == 0);
    assert((hadc.State & HAL_ADC_STATE_ERROR_DMA) != 0U);
    assert((hadc.ErrorCode & HAL_ADC_ERROR_DMA) != 0U);

    assert(HAL_ADC_Stop_DMA(&hadc) == HAL_OK);
    hdma.Parent = NULL;
    hadc.DMA_Handle = &hdma;
    assert(HAL_ADC_Start_DMA(&hadc, samples, 4U) == HAL_ERROR);
    assert(gd32HalLastPortError == GD32_HAL_PORT_ERROR_ADC_DMA_LINK_INVALID);
}

static void TestCalibrationFailure(void)
{
    ADC_HandleTypeDef hadc = MakeADC();
    DMA_HandleTypeDef hdma;
    uint32_t sample = 0U;

    assert(HAL_ADC_Init(&hadc) == HAL_OK);
    calibration_status = -1;
    assert(HAL_ADC_Start(&hadc) == HAL_ERROR);
    assert((hadc.ErrorCode & HAL_ADC_ERROR_INTERNAL) != 0U);
    assert((hadc.State & HAL_ADC_STATE_ERROR_INTERNAL) != 0U);
    assert(software_start_count == 0);

    hadc.State = HAL_ADC_STATE_READY;
    hadc.ErrorCode = HAL_ADC_ERROR_NONE;
    assert(HAL_ADC_Start_IT(&hadc) == HAL_ERROR);
    assert(adc_interrupts == 0U);

    hadc.State = HAL_ADC_STATE_READY;
    hadc.ErrorCode = HAL_ADC_ERROR_NONE;
    hdma = MakeADCDMA(&hadc, DMA_NORMAL);
    hadc.DMA_Handle = &hdma;
    assert(HAL_ADC_Start_DMA(&hadc, &sample, 1U) == HAL_ERROR);
    assert(dma_request_enabled == 0);
}

int main(void)
{
    ResetMocks();
    TestInitChannelPolling();
    ResetMocks();
    TestTriggerAndIT();
    ResetMocks();
    TestDMA();
    ResetMocks();
    TestCalibrationFailure();
    puts("ADC host tests: PASS");
    return 0;
}
