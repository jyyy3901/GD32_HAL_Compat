#include "stm32f4xx_hal.h"

void DMAStreamInitSyntaxPositive(DMA_HandleTypeDef *hdma)
{
    hdma->Instance = DMA2_Stream7;
    hdma->Init.Channel = DMA_CHANNEL_4;
    hdma->Init.Direction = DMA_MEMORY_TO_PERIPH;
}
