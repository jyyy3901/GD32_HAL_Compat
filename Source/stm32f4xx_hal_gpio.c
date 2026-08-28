#include "stm32f4xx_hal.h"

static uint32_t GD32_HAL_GPIO_Address(GPIO_TypeDef *GPIOx)
{
    const GD32_HAL_Resource *resource =
        GD32_HAL_ResolveInstance((uintptr_t)GPIOx, GD32_HAL_RESOURCE_GPIO);

    return (resource != NULL) ? resource->gd32_instance : 0U;
}

static GD32_HAL_GPIOSpeed GD32_HAL_GPIO_MapSpeed(uint32_t speed)
{
    switch (speed)
    {
        case GPIO_SPEED_FREQ_LOW:
            return GD32_HAL_GPIO_SPEED_2MHZ;
        case GPIO_SPEED_FREQ_MEDIUM:
            return GD32_HAL_GPIO_SPEED_10MHZ;
        case GPIO_SPEED_FREQ_HIGH:
            return GD32_HAL_GPIO_SPEED_50MHZ;
        case GPIO_SPEED_FREQ_VERY_HIGH:
        default:
            return GD32_HAL_GPIO_SPEED_MAX;
    }
}

static GD32_HAL_GPIOMode GD32_HAL_GPIO_MapInputPull(uint32_t pull)
{
    if (pull == GPIO_PULLUP)
    {
        return GD32_HAL_GPIO_MODE_INPUT_PULLUP;
    }
    if (pull == GPIO_PULLDOWN)
    {
        return GD32_HAL_GPIO_MODE_INPUT_PULLDOWN;
    }
    return GD32_HAL_GPIO_MODE_INPUT_FLOATING;
}

void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
    uint32_t gpio_address;
    GD32_HAL_GPIOMode gd_mode;
    GD32_HAL_GPIOSpeed gd_speed;

    if ((GPIOx == NULL) || (GPIO_Init == NULL))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE, 0U);
        return;
    }

    gpio_address = GD32_HAL_GPIO_Address(GPIOx);
    if (GD32_HAL_GPIO_IsInstance(gpio_address) == 0)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE, gpio_address);
        return;
    }
    if ((IS_GPIO_PIN(GPIO_Init->Pin) == 0U) ||
        (GD32_HAL_GPIO_ArePinsAvailable(gpio_address, GPIO_Init->Pin) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_PIN, GPIO_Init->Pin);
        return;
    }
    if ((IS_GPIO_MODE(GPIO_Init->Mode) == 0U) ||
        (IS_GPIO_PULL(GPIO_Init->Pull) == 0U) ||
        (IS_GPIO_SPEED(GPIO_Init->Speed) == 0U) ||
        (((GPIO_Init->Mode & GPIO_MODE) == MODE_AF) &&
         (IS_GPIO_AF(GPIO_Init->Alternate) == 0U)))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_PIN, GPIO_Init->Pin);
        return;
    }

    gd_speed = GD32_HAL_GPIO_MapSpeed(GPIO_Init->Speed);
    switch (GPIO_Init->Mode)
    {
        case GPIO_MODE_ANALOG:
            gd_mode = GD32_HAL_GPIO_MODE_ANALOG;
            break;
        case GPIO_MODE_OUTPUT_PP:
            gd_mode = GD32_HAL_GPIO_MODE_OUTPUT_PP;
            break;
        case GPIO_MODE_OUTPUT_OD:
            gd_mode = GD32_HAL_GPIO_MODE_OUTPUT_OD;
            break;
        case GPIO_MODE_AF_PP:
            gd_mode = GD32_HAL_GPIO_MODE_AF_PP;
            break;
        case GPIO_MODE_AF_OD:
            gd_mode = GD32_HAL_GPIO_MODE_AF_OD;
            break;
        default:
            gd_mode = GD32_HAL_GPIO_MapInputPull(GPIO_Init->Pull);
            break;
    }

    if ((((GPIO_Init->Mode & GPIO_MODE) == MODE_OUTPUT) ||
         ((GPIO_Init->Mode & GPIO_MODE) == MODE_AF)) &&
        (GPIO_Init->Pull != GPIO_NOPULL))
    {
        /* GD32F403 的 F1 风格 GPIO 在输出/AF 模式下没有独立 PUPDR。 */
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_OUTPUT_PULL_UNSUPPORTED, GPIO_Init->Pin);
        return;
    }

    /* Alternate 字段仅保留接口兼容；GD32 AFIO remap 必须由 MSP 原生配置。 */
    GD32_HAL_GPIO_InitPins(gpio_address, gd_mode, gd_speed, GPIO_Init->Pin);

    if ((GPIO_Init->Mode & EXTI_MODE) != 0U)
    {
        GD32_HAL_EXTIMode exti_mode;
        GD32_HAL_EXTITrigger trigger;

        exti_mode = ((GPIO_Init->Mode & EXTI_MODE) == EXTI_EVT) ?
                    GD32_HAL_EXTI_EVENT : GD32_HAL_EXTI_INTERRUPT;

        if ((GPIO_Init->Mode & TRIGGER_MODE) == (TRIGGER_RISING | TRIGGER_FALLING))
        {
            trigger = GD32_HAL_EXTI_BOTH;
        }
        else if ((GPIO_Init->Mode & TRIGGER_MODE) == TRIGGER_FALLING)
        {
            trigger = GD32_HAL_EXTI_FALLING;
        }
        else
        {
            trigger = GD32_HAL_EXTI_RISING;
        }

        GD32_HAL_GPIO_ConfigEXTI(gpio_address, GPIO_Init->Pin, exti_mode, trigger);
    }
}

void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin)
{
    const uint32_t gpio_address = GD32_HAL_GPIO_Address(GPIOx);

    if ((GD32_HAL_GPIO_IsInstance(gpio_address) == 0) ||
        (GD32_HAL_GPIO_ArePinsAvailable(gpio_address, GPIO_Pin) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_PIN, GPIO_Pin);
        return;
    }

    GD32_HAL_GPIO_DeInitPins(gpio_address, GPIO_Pin);
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    const uint32_t gpio_address = GD32_HAL_GPIO_Address(GPIOx);

    if ((GD32_HAL_GPIO_IsInstance(gpio_address) == 0) ||
        (GD32_HAL_GPIO_ArePinsAvailable(gpio_address, GPIO_Pin) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_PIN, GPIO_Pin);
        return GPIO_PIN_RESET;
    }

    return (GD32_HAL_GPIO_ReadPins(gpio_address, GPIO_Pin) != 0U) ?
           GPIO_PIN_SET : GPIO_PIN_RESET;
}

void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    const uint32_t gpio_address = GD32_HAL_GPIO_Address(GPIOx);

    if ((GD32_HAL_GPIO_IsInstance(gpio_address) == 0) ||
        (GD32_HAL_GPIO_ArePinsAvailable(gpio_address, GPIO_Pin) == 0) ||
        (IS_GPIO_PIN_ACTION(PinState) == 0U))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_PIN, GPIO_Pin);
        return;
    }

    GD32_HAL_GPIO_WritePins(gpio_address, GPIO_Pin, PinState == GPIO_PIN_SET);
}

void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    const uint32_t gpio_address = GD32_HAL_GPIO_Address(GPIOx);

    if ((GD32_HAL_GPIO_IsInstance(gpio_address) == 0) ||
        (GD32_HAL_GPIO_ArePinsAvailable(gpio_address, GPIO_Pin) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_PIN, GPIO_Pin);
        return;
    }

    GD32_HAL_GPIO_TogglePins(gpio_address, GPIO_Pin);
}

HAL_StatusTypeDef HAL_GPIO_LockPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    const uint32_t gpio_address = GD32_HAL_GPIO_Address(GPIOx);

    if ((GD32_HAL_GPIO_IsInstance(gpio_address) == 0) ||
        (GD32_HAL_GPIO_ArePinsAvailable(gpio_address, GPIO_Pin) == 0))
    {
        return HAL_ERROR;
    }

    return (GD32_HAL_GPIO_LockPins(gpio_address, GPIO_Pin) == 0) ? HAL_OK : HAL_ERROR;
}

void HAL_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin)
{
    if (GD32_HAL_GPIO_EXTIPending(GPIO_Pin) != 0U)
    {
        GD32_HAL_GPIO_EXTIClear(GPIO_Pin);
        HAL_GPIO_EXTI_Callback(GPIO_Pin);
    }
}

__weak void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    UNUSED(GPIO_Pin);
}
