#include "gd32_hal_port.h"
#include "gd32f403_dma.h"
#include "gd32f403_exti.h"
#include "gd32f403_rcu.h"
#include <stddef.h>

#define GD32_HAL_UID_BASE 0x1FFFF7E8UL

int GD32_HAL_PortInit(void)
{
    /* EXTI 路由和外设重映射均依赖 AFIO 时钟。 */
    rcu_periph_clock_enable(RCU_AF);
    return 0;
}

int GD32_HAL_PortDeInit(void)
{
    /* 只复位兼容层实际映射的资源，不干扰产品使用的 GD32 原生外设。 */
    static const rcu_periph_reset_enum resets[] = {
        RCU_AFRST,
        RCU_GPIOARST, RCU_GPIOBRST, RCU_GPIOCRST, RCU_GPIODRST,
        RCU_USART0RST, RCU_USART1RST, RCU_USART2RST,
        RCU_TIMER0RST, RCU_TIMER1RST, RCU_TIMER2RST, RCU_TIMER3RST,
        RCU_TIMER4RST, RCU_TIMER8RST, RCU_TIMER9RST, RCU_TIMER10RST,
        RCU_ADC0RST,
        RCU_I2C0RST, RCU_I2C1RST,
        RCU_SPI0RST, RCU_SPI1RST, RCU_SPI2RST
    };
    size_t index;
    uint32_t channel;

    GD32_HAL_ADC_InvalidateCalibration(GD32_HAL_ADC0_ADDRESS);
    for (index = 0U; index < (sizeof(resets) / sizeof(resets[0])); ++index)
    {
        rcu_periph_reset_enable(resets[index]);
        rcu_periph_reset_disable(resets[index]);
    }

    /* DMA 没有独立的 RCU 复位位，需逐通道恢复。 */
    rcu_periph_clock_enable(RCU_DMA0);
    rcu_periph_clock_enable(RCU_DMA1);
    for (channel = 0U; channel <= (uint32_t)DMA_CH6; ++channel)
    {
        dma_deinit(DMA0, (dma_channel_enum)channel);
    }
    for (channel = 0U; channel <= (uint32_t)DMA_CH4; ++channel)
    {
        dma_deinit(DMA1, (dma_channel_enum)channel);
    }
    GD32_HAL_DMA_ResetOwnership();
    rcu_periph_clock_disable(RCU_DMA0);
    rcu_periph_clock_disable(RCU_DMA1);
    exti_deinit();

    return 0;
}

uint32_t GD32_HAL_ReadUIDWord(uint8_t index)
{
    if (index > 2U)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE, index);
        return 0U;
    }
    return REG32(GD32_HAL_UID_BASE + (4UL * (uint32_t)index));
}
