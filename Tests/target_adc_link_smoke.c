#include "stm32f4xx_hal.h"

uint32_t SystemCoreClock = 120000000U;

static ADC_HandleTypeDef targetADC;
static DMA_HandleTypeDef targetADCDMA;
static uint32_t targetSamples[2];

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        __HAL_RCC_ADC1_CLK_ENABLE();
    }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
    UNUSED(hadc);
}

void TargetSmoke(void)
{
    ADC_ChannelConfTypeDef channel;

    targetADC.Instance = ADC1;
    targetADC.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    targetADC.Init.Resolution = ADC_RESOLUTION_12B;
    targetADC.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    targetADC.Init.ScanConvMode = ENABLE;
    targetADC.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    targetADC.Init.ContinuousConvMode = DISABLE;
    targetADC.Init.NbrOfConversion = 2U;
    targetADC.Init.DiscontinuousConvMode = DISABLE;
    targetADC.Init.NbrOfDiscConversion = 1U;
    targetADC.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    targetADC.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    targetADC.Init.DMAContinuousRequests = DISABLE;
    targetADC.State = HAL_ADC_STATE_RESET;

    targetADCDMA.Instance = GD32_DMA0_CHANNEL0;
    targetADCDMA.Init.Channel = GD32_DMA_REQUEST_ADC0;
    targetADCDMA.Init.Direction = DMA_PERIPH_TO_MEMORY;
    targetADCDMA.Init.PeriphInc = DMA_PINC_DISABLE;
    targetADCDMA.Init.MemInc = DMA_MINC_ENABLE;
    targetADCDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    targetADCDMA.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    targetADCDMA.Init.Mode = DMA_NORMAL;
    targetADCDMA.Init.Priority = DMA_PRIORITY_HIGH;
    targetADCDMA.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    targetADCDMA.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    targetADCDMA.Init.MemBurst = DMA_MBURST_SINGLE;
    targetADCDMA.Init.PeriphBurst = DMA_PBURST_SINGLE;
    (void)HAL_DMA_Init(&targetADCDMA);
    __HAL_LINKDMA(&targetADC, DMA_Handle, targetADCDMA);

    channel.Channel = ADC_CHANNEL_0;
    channel.Rank = ADC_REGULAR_RANK_1;
    channel.SamplingTime = ADC_SAMPLETIME_28CYCLES;
    channel.Offset = 0U;
    (void)HAL_ADC_Init(&targetADC);
    (void)HAL_ADC_ConfigChannel(&targetADC, &channel);
    channel.Channel = ADC_CHANNEL_1;
    channel.Rank = ADC_REGULAR_RANK_2;
    (void)HAL_ADC_ConfigChannel(&targetADC, &channel);
    (void)HAL_ADC_Start(&targetADC);
    (void)HAL_ADC_PollForConversion(&targetADC, 1U);
    targetSamples[0] = HAL_ADC_GetValue(&targetADC);
    (void)HAL_ADC_Stop(&targetADC);
    (void)HAL_ADC_Start_IT(&targetADC);
    HAL_ADC_IRQHandler(&targetADC);
    (void)HAL_ADC_Stop_IT(&targetADC);
    (void)HAL_ADC_Start_DMA(&targetADC, targetSamples, 2U);
    (void)HAL_ADC_Stop_DMA(&targetADC);
    (void)HAL_ADC_DeInit(&targetADC);
}
