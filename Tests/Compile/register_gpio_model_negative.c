#include "stm32f4xx_hal.h"

uint32_t RegisterGPIOModelMustNotCompile(void)
{
    return GPIOA->MODER;
}
