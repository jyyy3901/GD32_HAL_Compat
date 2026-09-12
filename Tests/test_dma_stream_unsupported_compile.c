#include "stm32f4xx_hal.h"

uint32_t legacy_stream_register_access(void)
{
    return DMA1_Stream0->CR;
}
