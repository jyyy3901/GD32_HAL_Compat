#include "gd32_hal_port.h"
#include "gd32f403_timer.h"
#include <assert.h>
#include <stdio.h>

static uint16_t last_source;
static uint32_t enable_calls;
static uint32_t disable_calls;
static GD32_HAL_PortError last_error;
static uint32_t last_detail;

void timer_dma_enable(uint32_t timer_periph, uint16_t dma)
{
    assert(timer_periph == GD32_HAL_TIMER0_ADDRESS);
    last_source = dma;
    ++enable_calls;
}

void timer_dma_disable(uint32_t timer_periph, uint16_t dma)
{
    assert(timer_periph == GD32_HAL_TIMER0_ADDRESS);
    last_source = dma;
    ++disable_calls;
}

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    last_error = error;
    last_detail = detail;
}

int main(void)
{
    static const struct
    {
        GD32_HAL_TIMERDMARequest request;
        uint16_t source;
    } cases[] =
    {
        {GD32_HAL_TIMER_DMA_UPDATE, TIMER_DMA_UPD},
        {GD32_HAL_TIMER_DMA_CC1, TIMER_DMA_CH0D},
        {GD32_HAL_TIMER_DMA_CC2, TIMER_DMA_CH1D},
        {GD32_HAL_TIMER_DMA_CC3, TIMER_DMA_CH2D},
        {GD32_HAL_TIMER_DMA_CC4, TIMER_DMA_CH3D},
        {GD32_HAL_TIMER_DMA_COM, TIMER_DMA_CMTD},
        {GD32_HAL_TIMER_DMA_TRIGGER, TIMER_DMA_TRGD}
    };
    uint32_t i;

    for (i = 0U; i < (uint32_t)(sizeof(cases) / sizeof(cases[0])); ++i)
    {
        GD32_HAL_TIMER_SetDMARequest(GD32_HAL_TIMER0_ADDRESS,
                                     cases[i].request, 1);
        assert(enable_calls == (i + 1U));
        assert(last_source == cases[i].source);
        assert((last_source & 0x00FFU) == 0U);

        GD32_HAL_TIMER_SetDMARequest(GD32_HAL_TIMER0_ADDRESS,
                                     cases[i].request, 0);
        assert(disable_calls == (i + 1U));
        assert(last_source == cases[i].source);
    }

    GD32_HAL_TIMER_SetDMARequest(
        GD32_HAL_TIMER0_ADDRESS, (GD32_HAL_TIMERDMARequest)7U, 1);
    assert(enable_calls == (uint32_t)(sizeof(cases) / sizeof(cases[0])));
    assert(last_error == GD32_HAL_PORT_ERROR_TIMER_DMA_UNSUPPORTED);
    assert(last_detail == 7U);

    puts("TIM DMA request vendor Port tests: PASS");
    return 0;
}
