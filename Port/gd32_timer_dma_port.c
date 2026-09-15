#include "gd32_hal_port.h"
#include "gd32f403_timer.h"
#include <stddef.h>

static int TIMER_DMASource(GD32_HAL_TIMERDMARequest request,
                           uint16_t *source)
{
    if (source == NULL)
    {
        return 0;
    }
    switch (request)
    {
        case GD32_HAL_TIMER_DMA_UPDATE:
            *source = TIMER_DMA_UPD;
            return 1;
        case GD32_HAL_TIMER_DMA_CC1:
            *source = TIMER_DMA_CH0D;
            return 1;
        case GD32_HAL_TIMER_DMA_CC2:
            *source = TIMER_DMA_CH1D;
            return 1;
        case GD32_HAL_TIMER_DMA_CC3:
            *source = TIMER_DMA_CH2D;
            return 1;
        case GD32_HAL_TIMER_DMA_CC4:
            *source = TIMER_DMA_CH3D;
            return 1;
        case GD32_HAL_TIMER_DMA_COM:
            *source = TIMER_DMA_CMTD;
            return 1;
        case GD32_HAL_TIMER_DMA_TRIGGER:
            *source = TIMER_DMA_TRGD;
            return 1;
        default:
            return 0;
    }
}

void GD32_HAL_TIMER_SetDMARequest(uint32_t timer_address,
                                  GD32_HAL_TIMERDMARequest request,
                                  int enable)
{
    uint16_t source;
    if (TIMER_DMASource(request, &source) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_TIMER_DMA_UNSUPPORTED,
                           (uint32_t)request);
        return;
    }
    if (enable != 0)
    {
        timer_dma_enable(timer_address, source);
    }
    else
    {
        timer_dma_disable(timer_address, source);
    }
}
