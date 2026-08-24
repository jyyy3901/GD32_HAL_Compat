#include "stm32f4xx_hal.h"

__IO GD32_HAL_PortError gd32HalLastPortError;
__IO uint32_t gd32HalLastPortErrorDetail;

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    gd32HalLastPortError = error;
    gd32HalLastPortErrorDetail = detail;
}

void TargetSmoke(void)
{
    static TIM_HandleTypeDef htim;
    static TIM_OC_InitTypeDef output;
    static TIM_IC_InitTypeDef input;
    static TIM_MasterConfigTypeDef master;
    static TIM_SlaveConfigTypeDef slave;

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
