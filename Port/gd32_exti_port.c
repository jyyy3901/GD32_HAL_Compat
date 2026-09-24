#include "gd32_hal_port.h"
#include "gd32f403_exti.h"
#include "gd32f403_gpio.h"
#include "gd32f403_rcu.h"
#include <stddef.h>

static uint32_t GD32_HAL_EXTI_Mask(uint8_t line)
{
    return (line < 19U) ? (1UL << line) : 0U;
}

static exti_trig_type_enum GD32_HAL_EXTI_MapTrigger(uint32_t trigger)
{
    if (trigger == 3U)
    {
        return EXTI_TRIG_BOTH;
    }
    if (trigger == 2U)
    {
        return EXTI_TRIG_FALLING;
    }
    if (trigger == 1U)
    {
        return EXTI_TRIG_RISING;
    }
    return EXTI_TRIG_NONE;
}

int GD32_HAL_EXTI_Configure(uint8_t line,
                            uint32_t mode,
                            uint32_t trigger,
                            uint32_t gpio_source)
{
    const uint32_t mask = GD32_HAL_EXTI_Mask(line);
    exti_mode_enum primary_mode;

    if ((mask == 0U) || ((mode & ~3U) != 0U) || (mode == 0U) ||
        ((trigger & ~3U) != 0U))
    {
        return -1;
    }

    if (line < 16U)
    {
        if (gpio_source > GPIO_PORT_SOURCE_GPIOD)
        {
            return -1;
        }
        rcu_periph_clock_enable(RCU_AF);
        gpio_exti_source_select((uint8_t)gpio_source, line);
    }

    primary_mode = ((mode & 1U) != 0U) ? EXTI_INTERRUPT : EXTI_EVENT;
    exti_init((exti_line_enum)mask,
              primary_mode,
              GD32_HAL_EXTI_MapTrigger(trigger));

    if ((mode & 1U) != 0U)
    {
        exti_interrupt_enable((exti_line_enum)mask);
    }
    else
    {
        exti_interrupt_disable((exti_line_enum)mask);
    }
    if ((mode & 2U) != 0U)
    {
        exti_event_enable((exti_line_enum)mask);
    }
    else
    {
        exti_event_disable((exti_line_enum)mask);
    }

    return 0;
}

void GD32_HAL_EXTI_GetConfig(uint8_t line,
                             uint32_t *mode,
                             uint32_t *trigger,
                             uint32_t *gpio_source)
{
    const uint32_t mask = GD32_HAL_EXTI_Mask(line);
    uint32_t mode_value = 0U;
    uint32_t trigger_value = 0U;

    if ((mode == NULL) || (trigger == NULL) || (gpio_source == NULL) ||
        (mask == 0U))
    {
        return;
    }

    if ((EXTI_INTEN & mask) != 0U) mode_value |= 1U;
    if ((EXTI_EVEN & mask) != 0U) mode_value |= 2U;
    if ((EXTI_RTEN & mask) != 0U) trigger_value |= 1U;
    if ((EXTI_FTEN & mask) != 0U) trigger_value |= 2U;

    *mode = mode_value;
    *trigger = trigger_value;
    if (line < 16U)
    {
        const uint32_t shift = ((uint32_t)line & 3U) * 4U;
        const uint32_t reg = REG32(AFIO + 0x08U + (((uint32_t)line >> 2U) * 4U));
        *gpio_source = (reg >> shift) & 0x0FU;
    }
    else
    {
        *gpio_source = 0U;
    }
}

void GD32_HAL_EXTI_ClearConfig(uint8_t line)
{
    const uint32_t mask = GD32_HAL_EXTI_Mask(line);

    if (mask == 0U)
    {
        return;
    }

    exti_interrupt_disable((exti_line_enum)mask);
    exti_event_disable((exti_line_enum)mask);
    EXTI_RTEN &= ~mask;
    EXTI_FTEN &= ~mask;
    exti_flag_clear((exti_line_enum)mask);

    if (line < 16U)
    {
        gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, line);
    }
}

uint32_t GD32_HAL_EXTI_GetPending(uint8_t line)
{
    const uint32_t mask = GD32_HAL_EXTI_Mask(line);
    return ((mask != 0U) && (exti_flag_get((exti_line_enum)mask) != RESET)) ? 1U : 0U;
}

void GD32_HAL_EXTI_ClearPending(uint8_t line)
{
    const uint32_t mask = GD32_HAL_EXTI_Mask(line);
    if (mask != 0U)
    {
        exti_flag_clear((exti_line_enum)mask);
    }
}

void GD32_HAL_EXTI_GenerateSWI(uint8_t line)
{
    const uint32_t mask = GD32_HAL_EXTI_Mask(line);
    if (mask != 0U)
    {
        exti_software_interrupt_enable((exti_line_enum)mask);
    }
}
