#include "stm32f4xx_hal.h"

uint32_t UnsupportedSTM32OnlyBitMustNotCompile(void)
{
    return ADC1->SR & ADC_SR_OVR;
}
