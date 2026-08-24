#include "gd32_hal_port.h"
#include <stddef.h>

static uint32_t GD32_HAL_GPIO_ModeValue(GD32_HAL_GPIOMode mode)
{
    switch (mode)
    {
        case GD32_HAL_GPIO_MODE_ANALOG:
            return GPIO_MODE_AIN;
        case GD32_HAL_GPIO_MODE_INPUT_FLOATING:
            return GPIO_MODE_IN_FLOATING;
        case GD32_HAL_GPIO_MODE_INPUT_PULLUP:
            return GPIO_MODE_IPU;
        case GD32_HAL_GPIO_MODE_INPUT_PULLDOWN:
            return GPIO_MODE_IPD;
        case GD32_HAL_GPIO_MODE_OUTPUT_PP:
            return GPIO_MODE_OUT_PP;
        case GD32_HAL_GPIO_MODE_OUTPUT_OD:
            return GPIO_MODE_OUT_OD;
        case GD32_HAL_GPIO_MODE_AF_PP:
            return GPIO_MODE_AF_PP;
        case GD32_HAL_GPIO_MODE_AF_OD:
            return GPIO_MODE_AF_OD;
        default:
            return GPIO_MODE_IN_FLOATING;
    }
}

static uint32_t GD32_HAL_GPIO_SpeedValue(GD32_HAL_GPIOSpeed speed)
{
    switch (speed)
    {
        case GD32_HAL_GPIO_SPEED_2MHZ:
            return GPIO_OSPEED_2MHZ;
        case GD32_HAL_GPIO_SPEED_10MHZ:
            return GPIO_OSPEED_10MHZ;
        case GD32_HAL_GPIO_SPEED_50MHZ:
            return GPIO_OSPEED_50MHZ;
        case GD32_HAL_GPIO_SPEED_MAX:
        default:
            return GPIO_OSPEED_MAX;
    }
}

int GD32_HAL_PortInit(void)
{
    /* EXTI 路由和外设重映射均依赖 AFIO 时钟。 */
    rcu_periph_clock_enable(RCU_AF);
    return 0;
}

int GD32_HAL_PortDeInit(void)
{
    static const rcu_periph_reset_enum resets[] = {
        RCU_USBFSRST,
        RCU_TIMER1RST, RCU_TIMER2RST, RCU_TIMER3RST, RCU_TIMER4RST,
        RCU_TIMER5RST, RCU_TIMER6RST, RCU_TIMER11RST, RCU_TIMER12RST,
        RCU_TIMER13RST, RCU_WWDGTRST, RCU_SPI1RST, RCU_SPI2RST,
        RCU_USART1RST, RCU_USART2RST, RCU_UART3RST, RCU_UART4RST,
        RCU_I2C0RST, RCU_I2C1RST, RCU_CAN0RST, RCU_CAN1RST,
        RCU_BKPIRST, RCU_PMURST, RCU_DACRST, RCU_CTCRST,
        RCU_AFRST, RCU_GPIOARST, RCU_GPIOBRST, RCU_GPIOCRST,
        RCU_GPIODRST, RCU_GPIOERST, RCU_GPIOFRST, RCU_GPIOGRST,
        RCU_ADC0RST, RCU_ADC1RST, RCU_TIMER0RST, RCU_SPI0RST,
        RCU_TIMER7RST, RCU_USART0RST, RCU_ADC2RST, RCU_TIMER8RST,
        RCU_TIMER9RST, RCU_TIMER10RST
    };
    size_t index;
    uint32_t channel;

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

int GD32_HAL_GPIO_IsInstance(uint32_t gpio_address)
{
    return (gpio_address == GD32_HAL_GPIOA_ADDRESS) ||
           (gpio_address == GD32_HAL_GPIOB_ADDRESS) ||
           (gpio_address == GD32_HAL_GPIOC_ADDRESS) ||
           (gpio_address == GD32_HAL_GPIOD_ADDRESS);
}

int GD32_HAL_GPIO_ArePinsAvailable(uint32_t gpio_address, uint32_t pins)
{
    if ((pins == 0U) || ((pins & ~0xFFFFU) != 0U))
    {
        return 0;
    }

    if ((gpio_address == GD32_HAL_GPIOA_ADDRESS) ||
        (gpio_address == GD32_HAL_GPIOB_ADDRESS) ||
        (gpio_address == GD32_HAL_GPIOC_ADDRESS))
    {
        return 1;
    }

    /* LQFP64 的 PD0/PD1 与晶振脚复用，默认仅开放普通封装引脚 PD2。 */
    if (gpio_address == GD32_HAL_GPIOD_ADDRESS)
    {
        uint32_t available = (uint32_t)GPIO_PIN_2;
#if (GD32_HAL_ALLOW_PD01_GPIO == 1U)
        available |= (uint32_t)GPIO_PIN_0 | GPIO_PIN_1;
#endif
        return (pins & ~available) == 0U;
    }

    return 0;
}

int GD32_HAL_GPIO_GetPortSource(uint32_t gpio_address, uint8_t *port_source)
{
    if (port_source == NULL)
    {
        return -1;
    }

    switch (gpio_address)
    {
        case GD32_HAL_GPIOA_ADDRESS:
            *port_source = GPIO_PORT_SOURCE_GPIOA;
            return 0;
        case GD32_HAL_GPIOB_ADDRESS:
            *port_source = GPIO_PORT_SOURCE_GPIOB;
            return 0;
        case GD32_HAL_GPIOC_ADDRESS:
            *port_source = GPIO_PORT_SOURCE_GPIOC;
            return 0;
        case GD32_HAL_GPIOD_ADDRESS:
            *port_source = GPIO_PORT_SOURCE_GPIOD;
            return 0;
        default:
            return -1;
    }
}

void GD32_HAL_GPIO_InitPins(uint32_t gpio_address,
                            GD32_HAL_GPIOMode mode,
                            GD32_HAL_GPIOSpeed speed,
                            uint32_t pins)
{
    gpio_init(gpio_address,
              GD32_HAL_GPIO_ModeValue(mode),
              GD32_HAL_GPIO_SpeedValue(speed),
              pins);
}

void GD32_HAL_GPIO_DeInitPins(uint32_t gpio_address, uint32_t pins)
{
    gpio_init(gpio_address, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, pins);
    gpio_bit_reset(gpio_address, pins);

    EXTI_INTEN &= ~pins;
    EXTI_EVEN &= ~pins;
    EXTI_RTEN &= ~pins;
    EXTI_FTEN &= ~pins;
    EXTI_PD = pins;
}

uint32_t GD32_HAL_GPIO_ReadPins(uint32_t gpio_address, uint32_t pins)
{
    return ((uint32_t)gpio_input_port_get(gpio_address)) & pins;
}

void GD32_HAL_GPIO_WritePins(uint32_t gpio_address, uint32_t pins, int set)
{
    if (set != 0)
    {
        gpio_bit_set(gpio_address, pins);
    }
    else
    {
        gpio_bit_reset(gpio_address, pins);
    }
}

void GD32_HAL_GPIO_TogglePins(uint32_t gpio_address, uint32_t pins)
{
    const uint32_t output = GPIO_OCTL(gpio_address);
    const uint32_t set_mask = (~output) & pins & 0xFFFFU;
    const uint32_t reset_mask = output & pins & 0xFFFFU;

    /* BOP 的低 16 位置位、高 16 位复位，单次写入可保持原子性。 */
    GPIO_BOP(gpio_address) = set_mask | (reset_mask << 16U);
}

int GD32_HAL_GPIO_LockPins(uint32_t gpio_address, uint32_t pins)
{
    gpio_pin_lock(gpio_address, pins);
    return ((GPIO_LOCK(gpio_address) & GPIO_LOCK_LKK) != 0U) ? 0 : -1;
}

void GD32_HAL_GPIO_ConfigEXTI(uint32_t gpio_address,
                              uint32_t pins,
                              GD32_HAL_EXTIMode mode,
                              GD32_HAL_EXTITrigger trigger)
{
    uint8_t port_source;
    uint8_t position;
    exti_mode_enum gd_mode;
    exti_trig_type_enum gd_trigger;

    if (GD32_HAL_GPIO_GetPortSource(gpio_address, &port_source) != 0)
    {
        return;
    }

    for (position = 0U; position < 16U; ++position)
    {
        if ((pins & (1UL << position)) != 0U)
        {
            gpio_exti_source_select(port_source, position);
        }
    }

    gd_mode = (mode == GD32_HAL_EXTI_EVENT) ? EXTI_EVENT : EXTI_INTERRUPT;
    if (trigger == GD32_HAL_EXTI_FALLING)
    {
        gd_trigger = EXTI_TRIG_FALLING;
    }
    else if (trigger == GD32_HAL_EXTI_BOTH)
    {
        gd_trigger = EXTI_TRIG_BOTH;
    }
    else
    {
        gd_trigger = EXTI_TRIG_RISING;
    }

    exti_init((exti_line_enum)pins, gd_mode, gd_trigger);
}

uint32_t GD32_HAL_GPIO_EXTIPending(uint32_t pins)
{
    return EXTI_PD & pins;
}

void GD32_HAL_GPIO_EXTIClear(uint32_t pins)
{
    EXTI_PD = pins;
}

void GD32_HAL_GPIO_EXTIGenerate(uint32_t pins)
{
    exti_software_interrupt_enable((exti_line_enum)pins);
}
