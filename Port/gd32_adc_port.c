#include "gd32_hal_port.h"
#include "gd32f403_adc.h"
#include "gd32f403_gpio.h"
#include "gd32f403_rcu.h"
#include <stddef.h>

/* STM32F401 HAL public encodings consumed only at the Port translation edge. */
#define STM32_ADC_TRIG_T1_CC1    0x00000000U
#define STM32_ADC_TRIG_T1_CC2    0x00000001U
#define STM32_ADC_TRIG_T1_CC3    0x00000002U
#define STM32_ADC_TRIG_T2_CC2    0x00000003U
#define STM32_ADC_TRIG_T3_TRGO   0x00000008U
#define STM32_ADC_TRIG_T4_CC4    0x00000009U
#define STM32_ADC_TRIG_EXTI11    0x0000000FU
#define STM32_ADC_TRIG_SOFTWARE  0x00000010U
#define STM32_ADC_EDGE_NONE      0x00000000U
#define STM32_ADC_EDGE_RISING    0x00000001U

static uint32_t ADC_CalibrationTimeoutLoops(void)
{
    uint32_t loops = SystemCoreClock / 100U;
    return (loops < 1000U) ? 1000U : loops;
}

static int ADC_WaitControlBitClear(uint32_t adc_address, uint32_t bit)
{
    uint32_t timeout = ADC_CalibrationTimeoutLoops();
    while ((ADC_CTL1(adc_address) & bit) != 0U)
    {
        if (timeout == 0U)
        {
            return -1;
        }
        --timeout;
    }
    return 0;
}

static uint32_t ADC_Resolution(uint8_t bits)
{
    if (bits == 12U)
    {
        return ADC_RESOLUTION_12B;
    }
    if (bits == 10U)
    {
        return ADC_RESOLUTION_10B;
    }
    if (bits == 8U)
    {
        return ADC_RESOLUTION_8B;
    }
    return ADC_RESOLUTION_6B;
}

static uint32_t ADC_ClockDivider(uint8_t divider)
{
    if (divider == 2U)
    {
        return RCU_CKADC_CKAPB2_DIV2;
    }
    if (divider == 4U)
    {
        return RCU_CKADC_CKAPB2_DIV4;
    }
    if (divider == 6U)
    {
        return RCU_CKADC_CKAPB2_DIV6;
    }
    return RCU_CKADC_CKAPB2_DIV8;
}

static uint32_t ADC_TriggerSource(GD32_HAL_ADCTrigger trigger)
{
    static const uint32_t sources[] =
    {
        ADC0_1_2_EXTTRIG_ROUTINE_NONE,
        ADC0_1_EXTTRIG_ROUTINE_T0_CH0,
        ADC0_1_EXTTRIG_ROUTINE_T0_CH1,
        ADC0_1_EXTTRIG_ROUTINE_T0_CH2,
        ADC0_1_EXTTRIG_ROUTINE_T1_CH1,
        ADC0_1_EXTTRIG_ROUTINE_T2_TRGO,
        ADC0_1_EXTTRIG_ROUTINE_T3_CH3,
        ADC0_1_EXTTRIG_ROUTINE_T7_TRGO,
        ADC0_1_EXTTRIG_ROUTINE_EXTI_11
    };
    return sources[(uint32_t)trigger];
}

int GD32_HAL_ADC_MapSTM32Trigger(uint32_t stm32_source,
                                 uint32_t stm32_edge,
                                 GD32_HAL_ADCTrigger *trigger)
{
    if (trigger == NULL)
    {
        return -1;
    }
    switch (stm32_source)
    {
        case STM32_ADC_TRIG_T1_CC1:
            *trigger = GD32_HAL_ADC_TRIGGER_TIMER0_CH0;
            break;
        case STM32_ADC_TRIG_T1_CC2:
            *trigger = GD32_HAL_ADC_TRIGGER_TIMER0_CH1;
            break;
        case STM32_ADC_TRIG_T1_CC3:
            *trigger = GD32_HAL_ADC_TRIGGER_TIMER0_CH2;
            break;
        case STM32_ADC_TRIG_T2_CC2:
            *trigger = GD32_HAL_ADC_TRIGGER_TIMER1_CH1;
            break;
        case STM32_ADC_TRIG_T3_TRGO:
            *trigger = GD32_HAL_ADC_TRIGGER_TIMER2_TRGO;
            break;
        case STM32_ADC_TRIG_T4_CC4:
            *trigger = GD32_HAL_ADC_TRIGGER_TIMER3_CH3;
            break;
        case STM32_ADC_TRIG_EXTI11:
            *trigger = GD32_HAL_ADC_TRIGGER_EXTI11;
            break;
        case STM32_ADC_TRIG_SOFTWARE:
            *trigger = GD32_HAL_ADC_TRIGGER_SOFTWARE;
            break;
        default:
            return -1;
    }

    if (((*trigger == GD32_HAL_ADC_TRIGGER_SOFTWARE) &&
         (stm32_edge != STM32_ADC_EDGE_NONE)) ||
        ((*trigger != GD32_HAL_ADC_TRIGGER_SOFTWARE) &&
         (stm32_edge != STM32_ADC_EDGE_RISING)))
    {
        return -2;
    }
    return 0;
}

int GD32_HAL_ADC_MapSTM32SampleTime(uint32_t stm32_sample_time,
                                    uint8_t *gd32_sample_index)
{
    static const uint8_t conservative_map[] = {1U, 3U, 3U, 6U, 7U, 7U, 7U};

    if ((gd32_sample_index == NULL) || (stm32_sample_time >= 7U))
    {
        return -1;
    }
    *gd32_sample_index = conservative_map[stm32_sample_time];
    return 0;
}

int GD32_HAL_ADC_IsInstance(uint32_t adc_address)
{
    return adc_address == GD32_HAL_ADC0_ADDRESS;
}

int GD32_HAL_ADC_Configure(uint32_t adc_address,
                           const GD32_HAL_ADCConfig *config)
{
    uint32_t apb2_clock;
    uint32_t adc_clock;
    if ((GD32_HAL_ADC_IsInstance(adc_address) == 0) || (config == NULL) ||
        ((config->clock_divider != 2U) && (config->clock_divider != 4U) &&
         (config->clock_divider != 6U) && (config->clock_divider != 8U)) ||
        ((config->resolution_bits != 12U) && (config->resolution_bits != 10U) &&
         (config->resolution_bits != 8U) && (config->resolution_bits != 6U)) ||
        (config->sequence_length == 0U) || (config->sequence_length > 16U) ||
        (config->discontinuous_count == 0U) ||
        (config->discontinuous_count > 8U) ||
        (config->trigger > GD32_HAL_ADC_TRIGGER_EXTI11))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_INVALID_CONFIG, adc_address);
        return -1;
    }

    apb2_clock = rcu_clock_freq_get(CK_APB2);
    adc_clock = (apb2_clock != 0U) ?
                (apb2_clock / config->clock_divider) : 0U;
    if ((adc_clock < GD32_HAL_ADC_MIN_CLOCK_HZ) ||
        (adc_clock > GD32_HAL_ADC_MAX_CLOCK_HZ))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_CLOCK_UNSUPPORTED,
                           adc_clock);
        return -1;
    }

    GD32_HAL_ADC_InvalidateCalibration(adc_address);
    adc_deinit(adc_address);
    rcu_adc_clock_config(ADC_ClockDivider(config->clock_divider));
    adc_resolution_config(adc_address, ADC_Resolution(config->resolution_bits));
    adc_data_alignment_config(adc_address,
                              (config->align_left != 0U) ?
                              ADC_DATAALIGN_LEFT : ADC_DATAALIGN_RIGHT);
    adc_special_function_config(adc_address, ADC_SCAN_MODE,
                                (config->scan != 0U) ? ENABLE : DISABLE);
    adc_special_function_config(adc_address, ADC_CONTINUOUS_MODE,
                                (config->continuous != 0U) ? ENABLE : DISABLE);
    adc_channel_length_config(adc_address, ADC_ROUTINE_CHANNEL,
                              config->sequence_length);
    if (config->discontinuous != 0U)
    {
        adc_discontinuous_mode_config(adc_address, ADC_ROUTINE_CHANNEL,
                                      config->discontinuous_count);
    }
    else
    {
        adc_discontinuous_mode_config(adc_address,
                                      ADC_CHANNEL_DISCON_DISABLE, 1U);
    }

    if (config->trigger == GD32_HAL_ADC_TRIGGER_TIMER7_TRGO)
    {
        rcu_periph_clock_enable(RCU_AF);
        gpio_pin_remap_config(GPIO_ADC0_ETRGROU_REMAP, ENABLE);
    }
    else if (config->trigger == GD32_HAL_ADC_TRIGGER_EXTI11)
    {
        rcu_periph_clock_enable(RCU_AF);
        gpio_pin_remap_config(GPIO_ADC0_ETRGROU_REMAP, DISABLE);
    }
    adc_external_trigger_source_config(adc_address, ADC_ROUTINE_CHANNEL,
                                       ADC_TriggerSource(config->trigger));
    adc_external_trigger_config(adc_address, ADC_ROUTINE_CHANNEL,
                                (config->trigger == GD32_HAL_ADC_TRIGGER_SOFTWARE) ?
                                DISABLE : ENABLE);
    adc_flag_clear(adc_address, ADC_FLAG_EOC);
    return 0;
}

void GD32_HAL_ADC_DeInit(uint32_t adc_address)
{
    if (GD32_HAL_ADC_IsInstance(adc_address) != 0)
    {
        GD32_HAL_ADC_InvalidateCalibration(adc_address);
        adc_dma_mode_disable(adc_address);
        adc_tempsensor_vrefint_disable();
        adc_deinit(adc_address);
    }
}

int GD32_HAL_ADC_ConfigChannel(uint32_t adc_address,
                               uint8_t rank,
                               uint8_t channel,
                               uint8_t sample_time)
{
    static const uint32_t sampleTimes[] =
    {
        ADC_SAMPLETIME_1POINT5,
        ADC_SAMPLETIME_7POINT5,
        ADC_SAMPLETIME_13POINT5,
        ADC_SAMPLETIME_28POINT5,
        ADC_SAMPLETIME_41POINT5,
        ADC_SAMPLETIME_55POINT5,
        ADC_SAMPLETIME_71POINT5,
        ADC_SAMPLETIME_239POINT5
    };
    if ((GD32_HAL_ADC_IsInstance(adc_address) == 0) || (rank >= 16U) ||
        (channel > 17U) || (sample_time >= 8U))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_CHANNEL_UNSUPPORTED,
                           channel);
        return -1;
    }
    if (channel >= 16U)
    {
        adc_tempsensor_vrefint_enable();
    }
    adc_routine_channel_config(adc_address, rank, channel,
                               sampleTimes[sample_time]);
    return 0;
}

int GD32_HAL_ADC_EnableAndCalibrate(uint32_t adc_address)
{
    uint32_t apb2_clock;
    uint32_t adc_divider_code;
    uint32_t divider;
    uint32_t loops;
    volatile uint32_t index;

    if (GD32_HAL_ADC_IsInstance(adc_address) == 0)
    {
        return -1;
    }
    if ((GD32_HAL_ADC_IsEnabled(adc_address) != 0) &&
        (GD32_HAL_ADC_IsCalibrationValid(adc_address) != 0))
    {
        return 0;
    }

    if (GD32_HAL_ADC_IsEnabled(adc_address) == 0)
    {
        adc_enable(adc_address);
    }
    apb2_clock = rcu_clock_freq_get(CK_APB2);
    adc_divider_code = (RCU_CFG0 & RCU_CFG0_ADCPSC) >> 14U;
    divider = (adc_divider_code == 0U) ? 2U :
              (adc_divider_code == 1U) ? 4U :
              (adc_divider_code == 2U) ? 6U : 8U;
    if (apb2_clock == 0U)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_INVALID_CONFIG, 0U);
        adc_disable(adc_address);
        return -1;
    }
    loops = (SystemCoreClock / apb2_clock) +
            (((SystemCoreClock % apb2_clock) != 0U) ? 1U : 0U);
    loops *= 14U * divider;
    if (loops < 16U)
    {
        loops = 16U;
    }
    for (index = 0U; index < loops; ++index)
    {
        __NOP();
    }
    ADC_CTL1(adc_address) |= ADC_CTL1_RSTCLB;
    if (ADC_WaitControlBitClear(adc_address, ADC_CTL1_RSTCLB) != 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_CALIBRATION_TIMEOUT,
                           ADC_CTL1_RSTCLB);
        GD32_HAL_ADC_Disable(adc_address);
        return -1;
    }
    ADC_CTL1(adc_address) |= ADC_CTL1_CLB;
    if (ADC_WaitControlBitClear(adc_address, ADC_CTL1_CLB) != 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_CALIBRATION_TIMEOUT,
                           ADC_CTL1_CLB);
        GD32_HAL_ADC_Disable(adc_address);
        return -1;
    }
    GD32_HAL_ADC_SetCalibrationValid(adc_address);
    return 0;
}

void GD32_HAL_ADC_Disable(uint32_t adc_address)
{
    if (GD32_HAL_ADC_IsInstance(adc_address) != 0)
    {
        GD32_HAL_ADC_InvalidateCalibration(adc_address);
        adc_disable(adc_address);
    }
}

int GD32_HAL_ADC_IsEnabled(uint32_t adc_address)
{
    return (GD32_HAL_ADC_IsInstance(adc_address) != 0) &&
           ((ADC_CTL1(adc_address) & ADC_CTL1_ADCON) != 0U);
}


void GD32_HAL_ADC_StartSoftware(uint32_t adc_address)
{
    if (GD32_HAL_ADC_IsInstance(adc_address) != 0)
    {
        adc_software_trigger_enable(adc_address, ADC_ROUTINE_CHANNEL);
    }
}

uint32_t GD32_HAL_ADC_GetFlag(uint32_t adc_address, uint32_t flags)
{
    uint32_t result = 0U;
    if ((GD32_HAL_ADC_IsInstance(adc_address) != 0) &&
        ((flags & GD32_HAL_ADC_FLAG_EOC) != 0U) &&
        (adc_flag_get(adc_address, ADC_FLAG_EOC) == SET))
    {
        result |= GD32_HAL_ADC_FLAG_EOC;
    }
    return result;
}

void GD32_HAL_ADC_ClearFlag(uint32_t adc_address, uint32_t flags)
{
    if ((GD32_HAL_ADC_IsInstance(adc_address) != 0) &&
        ((flags & GD32_HAL_ADC_FLAG_EOC) != 0U))
    {
        adc_flag_clear(adc_address, ADC_FLAG_EOC);
    }
}

void GD32_HAL_ADC_SetInterrupt(uint32_t adc_address,
                               uint32_t interrupts,
                               int enable)
{
    if (GD32_HAL_ADC_IsInstance(adc_address) == 0)
    {
        return;
    }
    if ((interrupts & GD32_HAL_ADC_INTERRUPT_EOC) != 0U)
    {
        if (enable != 0)
        {
            adc_interrupt_enable(adc_address, ADC_INT_EOC);
        }
        else
        {
            adc_interrupt_disable(adc_address, ADC_INT_EOC);
        }
    }
    if (((interrupts & GD32_HAL_ADC_INTERRUPT_OVERRUN_UNAVAILABLE) != 0U) &&
        (enable != 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_ADC_OVERRUN_UNAVAILABLE,
                           adc_address);
    }
}

uint32_t GD32_HAL_ADC_GetInterrupts(uint32_t adc_address)
{
    if ((GD32_HAL_ADC_IsInstance(adc_address) != 0) &&
        ((ADC_CTL0(adc_address) & ADC_CTL0_EOCIE) != 0U))
    {
        return GD32_HAL_ADC_INTERRUPT_EOC;
    }
    return 0U;
}

uint32_t GD32_HAL_ADC_ReadData(uint32_t adc_address)
{
    return (GD32_HAL_ADC_IsInstance(adc_address) != 0) ?
           (uint32_t)adc_routine_data_read(adc_address) : 0U;
}

uint32_t GD32_HAL_ADC_GetDataAddress(uint32_t adc_address)
{
    return (GD32_HAL_ADC_IsInstance(adc_address) != 0) ?
           (adc_address + 0x4CU) : 0U;
}

void GD32_HAL_ADC_SetDMARequest(uint32_t adc_address, int enable)
{
    if (GD32_HAL_ADC_IsInstance(adc_address) != 0)
    {
        if (enable != 0)
        {
            adc_dma_mode_enable(adc_address);
        }
        else
        {
            adc_dma_mode_disable(adc_address);
        }
    }
}

int GD32_HAL_ADC_IsDMAChannelValid(uint32_t adc_address,
                                   uint32_t channel_address,
                                   uint32_t request_token)
{
    return (adc_address == GD32_HAL_ADC0_ADDRESS) &&
           (channel_address == GD32_HAL_DMA0_CHANNEL0_ADDRESS) &&
           (request_token == GD32_HAL_DMA_REQUEST_ENCODE(0U, 0U, 0U));
}
