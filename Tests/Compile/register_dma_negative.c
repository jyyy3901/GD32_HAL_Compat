#include "stm32f4xx_hal.h"

uint32_t RegisterDMAMustNotCompile(void)
{
    return DMA2_Stream0->CR;
}
