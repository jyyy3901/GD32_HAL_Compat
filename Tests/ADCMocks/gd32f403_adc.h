#ifndef GD32F403_ADC_H
#define GD32F403_ADC_H

#include "gd32f403.h"

extern volatile uint32_t mock_adc_ctl0;
extern volatile uint32_t mock_adc_ctl1;
void mock_adc_nop(void);

#define __NOP() mock_adc_nop()
#define ADC_CTL0(adc) \
    (*(volatile uint32_t *)((void)(adc), (void *)&mock_adc_ctl0))
#define ADC_CTL1(adc) \
    (*(volatile uint32_t *)((void)(adc), (void *)&mock_adc_ctl1))
#define ADC_CTL0_EOCIE (1UL << 5U)
#define ADC_CTL1_ADCON (1UL << 0U)
#define ADC_CTL1_CLB   (1UL << 2U)
#define ADC_CTL1_RSTCLB (1UL << 3U)

#define ADC_RESOLUTION_12B 0U
#define ADC_RESOLUTION_10B 1U
#define ADC_RESOLUTION_8B  2U
#define ADC_RESOLUTION_6B  3U
#define ADC_DATAALIGN_RIGHT 0U
#define ADC_DATAALIGN_LEFT  1U
#define ADC_SCAN_MODE 0U
#define ADC_CONTINUOUS_MODE 1U
#define ADC_ROUTINE_CHANNEL 0U
#define ADC_CHANNEL_DISCON_DISABLE 1U
#define ADC_FLAG_EOC 1U
#define ADC_INT_EOC 1U
#define ADC_SAMPLETIME_1POINT5 0U
#define ADC_SAMPLETIME_7POINT5 1U
#define ADC_SAMPLETIME_13POINT5 2U
#define ADC_SAMPLETIME_28POINT5 3U
#define ADC_SAMPLETIME_41POINT5 4U
#define ADC_SAMPLETIME_55POINT5 5U
#define ADC_SAMPLETIME_71POINT5 6U
#define ADC_SAMPLETIME_239POINT5 7U
#define ADC0_1_2_EXTTRIG_ROUTINE_NONE 0U
#define ADC0_1_EXTTRIG_ROUTINE_T0_CH0 1U
#define ADC0_1_EXTTRIG_ROUTINE_T0_CH1 2U
#define ADC0_1_EXTTRIG_ROUTINE_T0_CH2 3U
#define ADC0_1_EXTTRIG_ROUTINE_T1_CH1 4U
#define ADC0_1_EXTTRIG_ROUTINE_T2_TRGO 5U
#define ADC0_1_EXTTRIG_ROUTINE_T3_CH3 6U
#define ADC0_1_EXTTRIG_ROUTINE_T7_TRGO 7U
#define ADC0_1_EXTTRIG_ROUTINE_EXTI_11 8U

void adc_deinit(uint32_t adc);
void adc_resolution_config(uint32_t adc, uint32_t value);
void adc_data_alignment_config(uint32_t adc, uint32_t value);
void adc_special_function_config(uint32_t adc, uint32_t function,
                                 ControlStatus enable);
void adc_channel_length_config(uint32_t adc, uint8_t group, uint32_t length);
void adc_discontinuous_mode_config(uint32_t adc, uint8_t group, uint8_t length);
void adc_external_trigger_source_config(uint32_t adc, uint8_t group,
                                        uint32_t source);
void adc_external_trigger_config(uint32_t adc, uint8_t group,
                                 ControlStatus enable);
void adc_flag_clear(uint32_t adc, uint32_t flag);
void adc_dma_mode_disable(uint32_t adc);
void adc_dma_mode_enable(uint32_t adc);
void adc_tempsensor_vrefint_disable(void);
void adc_tempsensor_vrefint_enable(void);
void adc_routine_channel_config(uint32_t adc, uint8_t rank, uint8_t channel,
                                uint32_t sample);
void adc_enable(uint32_t adc);
void adc_disable(uint32_t adc);
void adc_software_trigger_enable(uint32_t adc, uint8_t group);
FlagStatus adc_flag_get(uint32_t adc, uint32_t flag);
void adc_interrupt_enable(uint32_t adc, uint32_t interrupt);
void adc_interrupt_disable(uint32_t adc, uint32_t interrupt);
uint16_t adc_routine_data_read(uint32_t adc);

#endif
