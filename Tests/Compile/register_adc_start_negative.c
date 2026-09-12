#include "stm32f4xx_hal.h"

void RegisterADCStartMustNotCompile(void)
{
    ADC1->CR2 |= ADC_CR2_SWSTART;
}
