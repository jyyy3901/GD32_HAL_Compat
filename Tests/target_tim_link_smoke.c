#include "stm32f4xx_hal.h"

void TargetSmoke(void)
{
    static TIM_HandleTypeDef htim;
    static TIM_OC_InitTypeDef output;
    static TIM_IC_InitTypeDef input;
    static TIM_MasterConfigTypeDef master;
    static TIM_SlaveConfigTypeDef slave;
    static DMA_HandleTypeDef dma;
    static uint16_t pulses[2] = {100U, 200U};

    htim.Instance = TIM2;
    htim.Init.Prescaler = 79U;
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim.Init.Period = 999U;
    htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    (void)HAL_TIM_Base_Init(&htim);

    output.OCMode = TIM_OCMODE_PWM1;
    output.Pulse = 500U;
    output.OCPolarity = TIM_OCPOLARITY_HIGH;
    output.OCFastMode = TIM_OCFAST_DISABLE;
    (void)HAL_TIM_PWM_ConfigChannel(&htim, &output, TIM_CHANNEL_1);

    dma.Instance = DMA1_Stream5;
    dma.Init.Channel = DMA_CHANNEL_3;
    dma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma.Init.PeriphInc = DMA_PINC_DISABLE;
    dma.Init.MemInc = DMA_MINC_ENABLE;
    dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    dma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    dma.Init.Mode = DMA_NORMAL;
    dma.Init.Priority = DMA_PRIORITY_HIGH;
    dma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dma.Init.MemBurst = DMA_MBURST_SINGLE;
    dma.Init.PeriphBurst = DMA_PBURST_SINGLE;
    (void)HAL_DMA_Init(&dma);
    __HAL_LINKDMA(&htim, hdma[TIM_DMA_ID_CC1], dma);
    (void)HAL_TIM_PWM_Start_DMA(
        &htim, TIM_CHANNEL_1,
        (const uint32_t *)(const void *)pulses, 2U);
    (void)HAL_TIM_PWM_Stop_DMA(&htim, TIM_CHANNEL_1);

    input.ICPolarity = TIM_ICPOLARITY_RISING;
    input.ICSelection = TIM_ICSELECTION_DIRECTTI;
    input.ICPrescaler = TIM_ICPSC_DIV1;
    (void)HAL_TIM_IC_ConfigChannel(&htim, &input, TIM_CHANNEL_2);

    master.MasterOutputTrigger = TIM_TRGO_UPDATE;
    master.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
    (void)HAL_TIMEx_MasterConfigSynchronization(&htim, &master);

    slave.SlaveMode = TIM_SLAVEMODE_TRIGGER;
    slave.InputTrigger = TIM_TS_ITR0;
    slave.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
    slave.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
    (void)HAL_TIM_SlaveConfigSynchro(&htim, &slave);
    HAL_TIM_IRQHandler(&htim);
}
