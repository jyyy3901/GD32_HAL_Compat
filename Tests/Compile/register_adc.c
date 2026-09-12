#include "stm32f4xx_hal.h"

STM32_COMPAT_STATIC_ASSERT(ADC1_BASE == 0x40012400UL, adc1_base_test);
STM32_COMPAT_STATIC_ASSERT(offsetof(ADC_TypeDef, DR) == 0x4CU, adc_dr_test);
STM32_COMPAT_STATIC_ASSERT(ADC_CR2_ADON == 0x1UL, adc_adon_test);

void RegisterADCConfigurationPositive(void)
{
    ADC1->CR1 |= ADC_CR1_SCAN;
    ADC1->CR2 |= ADC_CR2_ADON;
    ADC1->SQR1 = 0U;
    (void)ADC1->DR;
}
