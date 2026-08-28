#include "stm32f4xx_hal.h"

static int GD32_HAL_EXTI_IsLineValid(uint32_t line)
{
    const uint32_t position = line & EXTI_PIN_MASK;
    const uint32_t property = line & (EXTI_GPIO | EXTI_CONFIG);

    if (position > 18U)
    {
        return 0;
    }
    if (position < 16U)
    {
        return property == EXTI_GPIO;
    }
    return property == EXTI_CONFIG;
}

static int GD32_HAL_EXTI_IsGPIOAvailable(uint32_t gpio, uint32_t line)
{
    if (gpio <= EXTI_GPIOC)
    {
        return 1;
    }
    if (gpio != EXTI_GPIOD)
    {
        return 0;
    }
    if (line == 2U)
    {
        return 1;
    }
#if (GD32_HAL_ALLOW_PD01_GPIO == 1U)
    if (line <= 1U)
    {
        return 1;
    }
#endif
    return 0;
}

HAL_StatusTypeDef HAL_EXTI_SetConfigLine(EXTI_HandleTypeDef *hexti,
                                         EXTI_ConfigTypeDef *pExtiConfig)
{
    uint32_t position;

    if ((hexti == NULL) || (pExtiConfig == NULL) ||
        (GD32_HAL_EXTI_IsLineValid(pExtiConfig->Line) == 0) ||
        (pExtiConfig->Mode == EXTI_MODE_NONE) ||
        ((pExtiConfig->Mode & ~(EXTI_MODE_INTERRUPT | EXTI_MODE_EVENT)) != 0U) ||
        ((pExtiConfig->Trigger & ~EXTI_TRIGGER_RISING_FALLING) != 0U))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_EXTI_INVALID_CONFIG,
                           (pExtiConfig != NULL) ? pExtiConfig->Line : 0U);
        return HAL_ERROR;
    }

    position = pExtiConfig->Line & EXTI_PIN_MASK;
    if (((pExtiConfig->Line & EXTI_GPIO) == EXTI_GPIO) &&
        (GD32_HAL_EXTI_IsGPIOAvailable(pExtiConfig->GPIOSel, position) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_EXTI_INVALID_CONFIG,
                           pExtiConfig->GPIOSel);
        return HAL_ERROR;
    }

    if (GD32_HAL_EXTI_Configure((uint8_t)position,
                                pExtiConfig->Mode,
                                pExtiConfig->Trigger,
                                pExtiConfig->GPIOSel) != 0)
    {
        return HAL_ERROR;
    }

    hexti->Line = pExtiConfig->Line;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_EXTI_GetConfigLine(EXTI_HandleTypeDef *hexti,
                                         EXTI_ConfigTypeDef *pExtiConfig)
{
    uint32_t position;

    if ((hexti == NULL) || (pExtiConfig == NULL) ||
        (GD32_HAL_EXTI_IsLineValid(hexti->Line) == 0))
    {
        return HAL_ERROR;
    }

    position = hexti->Line & EXTI_PIN_MASK;
    pExtiConfig->Line = hexti->Line;
    GD32_HAL_EXTI_GetConfig((uint8_t)position,
                            &pExtiConfig->Mode,
                            &pExtiConfig->Trigger,
                            &pExtiConfig->GPIOSel);
    return HAL_OK;
}

HAL_StatusTypeDef HAL_EXTI_ClearConfigLine(EXTI_HandleTypeDef *hexti)
{
    if ((hexti == NULL) || (GD32_HAL_EXTI_IsLineValid(hexti->Line) == 0))
    {
        return HAL_ERROR;
    }
    GD32_HAL_EXTI_ClearConfig((uint8_t)(hexti->Line & EXTI_PIN_MASK));
    return HAL_OK;
}

HAL_StatusTypeDef HAL_EXTI_RegisterCallback(EXTI_HandleTypeDef *hexti,
                                            EXTI_CallbackIDTypeDef CallbackID,
                                            void (*pPendingCbfn)(void))
{
    if ((hexti == NULL) || (CallbackID != HAL_EXTI_COMMON_CB_ID) ||
        (pPendingCbfn == NULL))
    {
        return HAL_ERROR;
    }
    hexti->PendingCallback = pPendingCbfn;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_EXTI_GetHandle(EXTI_HandleTypeDef *hexti,
                                     uint32_t ExtiLine)
{
    if ((hexti == NULL) || (GD32_HAL_EXTI_IsLineValid(ExtiLine) == 0))
    {
        return HAL_ERROR;
    }
    hexti->Line = ExtiLine;
    hexti->PendingCallback = NULL;
    return HAL_OK;
}

void HAL_EXTI_IRQHandler(EXTI_HandleTypeDef *hexti)
{
    if ((hexti == NULL) || (GD32_HAL_EXTI_IsLineValid(hexti->Line) == 0))
    {
        return;
    }
    if (GD32_HAL_EXTI_GetPending((uint8_t)(hexti->Line & EXTI_PIN_MASK)) != 0U)
    {
        GD32_HAL_EXTI_ClearPending((uint8_t)(hexti->Line & EXTI_PIN_MASK));
        if (hexti->PendingCallback != NULL)
        {
            hexti->PendingCallback();
        }
    }
}

uint32_t HAL_EXTI_GetPending(EXTI_HandleTypeDef *hexti, uint32_t Edge)
{
    if ((hexti == NULL) || (GD32_HAL_EXTI_IsLineValid(hexti->Line) == 0) ||
        ((Edge != EXTI_TRIGGER_RISING) &&
         (Edge != EXTI_TRIGGER_FALLING) &&
         (Edge != EXTI_TRIGGER_RISING_FALLING)))
    {
        return 0U;
    }
    /* GD32F403 仅有单一 PD 位，不能区分由上升沿还是下降沿置位。 */
    return GD32_HAL_EXTI_GetPending((uint8_t)(hexti->Line & EXTI_PIN_MASK));
}

void HAL_EXTI_ClearPending(EXTI_HandleTypeDef *hexti, uint32_t Edge)
{
    UNUSED(Edge);
    if ((hexti != NULL) && (GD32_HAL_EXTI_IsLineValid(hexti->Line) != 0))
    {
        GD32_HAL_EXTI_ClearPending((uint8_t)(hexti->Line & EXTI_PIN_MASK));
    }
}

void HAL_EXTI_GenerateSWI(EXTI_HandleTypeDef *hexti)
{
    if ((hexti != NULL) && (GD32_HAL_EXTI_IsLineValid(hexti->Line) != 0))
    {
        GD32_HAL_EXTI_GenerateSWI((uint8_t)(hexti->Line & EXTI_PIN_MASK));
    }
}
