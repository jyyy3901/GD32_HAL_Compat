#include "stm32f4xx_hal.h"

void RegisterTIMDMABurstMustFail(void)
{
    TIM1->DCR = 0U;
}
