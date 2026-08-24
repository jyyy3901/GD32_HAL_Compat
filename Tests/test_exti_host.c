#include "stm32f4xx_hal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint32_t modes[19];
static uint32_t triggers[19];
static uint32_t sources[19];
static uint32_t pending[19];
static uint32_t callback_calls;
static uint32_t last_error;

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    (void)detail;
    last_error = (uint32_t)error;
}

int GD32_HAL_EXTI_Configure(uint8_t line,
                            uint32_t mode,
                            uint32_t trigger,
                            uint32_t gpio_source)
{
    if (line >= 19U) return -1;
    modes[line] = mode;
    triggers[line] = trigger;
    sources[line] = gpio_source;
    return 0;
}
void GD32_HAL_EXTI_GetConfig(uint8_t line,
                             uint32_t *mode,
                             uint32_t *trigger,
                             uint32_t *gpio_source)
{
    *mode = modes[line];
    *trigger = triggers[line];
    *gpio_source = sources[line];
}
void GD32_HAL_EXTI_ClearConfig(uint8_t line)
{
    modes[line] = 0U;
    triggers[line] = 0U;
    sources[line] = 0U;
    pending[line] = 0U;
}
uint32_t GD32_HAL_EXTI_GetPending(uint8_t line) { return pending[line]; }
void GD32_HAL_EXTI_ClearPending(uint8_t line) { pending[line] = 0U; }
void GD32_HAL_EXTI_GenerateSWI(uint8_t line) { pending[line] = 1U; }

static void pending_callback(void)
{
    ++callback_calls;
}

int main(void)
{
    EXTI_HandleTypeDef handle;
    EXTI_ConfigTypeDef config;
    EXTI_ConfigTypeDef readback;

    memset(&handle, 0, sizeof(handle));
    memset(&config, 0, sizeof(config));
    assert(HAL_EXTI_GetHandle(&handle, EXTI_LINE_2) == HAL_OK);
    assert(handle.PendingCallback == NULL);
    assert(HAL_EXTI_RegisterCallback(&handle,
                                     HAL_EXTI_COMMON_CB_ID,
                                     pending_callback) == HAL_OK);

    config.Line = EXTI_LINE_2;
    config.Mode = EXTI_MODE_INTERRUPT | EXTI_MODE_EVENT;
    config.Trigger = EXTI_TRIGGER_RISING_FALLING;
    config.GPIOSel = EXTI_GPIOD;
    assert(HAL_EXTI_SetConfigLine(&handle, &config) == HAL_OK);
    assert(modes[2] == (EXTI_MODE_INTERRUPT | EXTI_MODE_EVENT));
    assert(triggers[2] == EXTI_TRIGGER_RISING_FALLING);
    assert(HAL_EXTI_GetConfigLine(&handle, &readback) == HAL_OK);
    assert(readback.GPIOSel == EXTI_GPIOD);

    HAL_EXTI_GenerateSWI(&handle);
    assert(HAL_EXTI_GetPending(&handle, EXTI_TRIGGER_RISING) == 1U);
    HAL_EXTI_IRQHandler(&handle);
    assert(callback_calls == 1U);
    assert(pending[2] == 0U);

    config.Line = EXTI_LINE_3;
    config.GPIOSel = EXTI_GPIOD;
    assert(HAL_EXTI_SetConfigLine(&handle, &config) == HAL_ERROR);
    assert(last_error == GD32_HAL_PORT_ERROR_EXTI_INVALID_CONFIG);

    assert(HAL_EXTI_GetHandle(&handle, EXTI_LINE_18) == HAL_OK);
    config.Line = EXTI_LINE_18;
    config.Mode = EXTI_MODE_INTERRUPT;
    config.Trigger = EXTI_TRIGGER_RISING;
    config.GPIOSel = EXTI_GPIOA;
    assert(HAL_EXTI_SetConfigLine(&handle, &config) == HAL_OK);
    assert(HAL_EXTI_ClearConfigLine(&handle) == HAL_OK);
    assert(modes[18] == 0U);

    assert(HAL_EXTI_RegisterCallback(&handle,
                                     HAL_EXTI_COMMON_CB_ID,
                                     NULL) == HAL_ERROR);
    puts("EXTI host tests: PASS");
    return 0;
}
