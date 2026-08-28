#include "stm32f4xx_hal.h"
#include "gd32_hal_port.h"
#include <assert.h>
#include <stdio.h>

static uint32_t mock_output;
static uint32_t mock_input;
static uint32_t mock_exti_pending;
static uint32_t mock_error_count;
static GD32_HAL_PortError mock_last_error;
static GD32_HAL_GPIOMode mock_last_mode;
static GD32_HAL_GPIOSpeed mock_last_speed;
static uint32_t mock_last_init_pins;

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    (void)detail;
    mock_last_error = error;
    ++mock_error_count;
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
    if (gpio_address == GD32_HAL_GPIOD_ADDRESS)
    {
        uint32_t available = GPIO_PIN_2;
#if (GD32_HAL_ALLOW_PD01_GPIO == 1U)
        available |= GPIO_PIN_0 | GPIO_PIN_1;
#endif
        return (pins & ~available) == 0U;
    }
    return GD32_HAL_GPIO_IsInstance(gpio_address);
}

void GD32_HAL_GPIO_InitPins(uint32_t gpio_address,
                            GD32_HAL_GPIOMode mode,
                            GD32_HAL_GPIOSpeed speed,
                            uint32_t pins)
{
    (void)gpio_address;
    mock_last_mode = mode;
    mock_last_speed = speed;
    mock_last_init_pins = pins;
}

void GD32_HAL_GPIO_DeInitPins(uint32_t gpio_address, uint32_t pins)
{
    (void)gpio_address;
    mock_output &= ~pins;
}

uint32_t GD32_HAL_GPIO_ReadPins(uint32_t gpio_address, uint32_t pins)
{
    (void)gpio_address;
    return mock_input & pins;
}

void GD32_HAL_GPIO_WritePins(uint32_t gpio_address, uint32_t pins, int set)
{
    (void)gpio_address;
    if (set != 0)
    {
        mock_output |= pins;
    }
    else
    {
        mock_output &= ~pins;
    }
}

void GD32_HAL_GPIO_TogglePins(uint32_t gpio_address, uint32_t pins)
{
    (void)gpio_address;
    mock_output ^= pins;
}

int GD32_HAL_GPIO_LockPins(uint32_t gpio_address, uint32_t pins)
{
    (void)gpio_address;
    (void)pins;
    return 0;
}

void GD32_HAL_GPIO_ConfigEXTI(uint32_t gpio_address,
                              uint32_t pins,
                              GD32_HAL_EXTIMode mode,
                              GD32_HAL_EXTITrigger trigger)
{
    (void)gpio_address;
    (void)pins;
    (void)mode;
    (void)trigger;
}

uint32_t GD32_HAL_GPIO_EXTIPending(uint32_t pins)
{
    return mock_exti_pending & pins;
}

void GD32_HAL_GPIO_EXTIClear(uint32_t pins)
{
    mock_exti_pending &= ~pins;
}

void GD32_HAL_GPIO_EXTIGenerate(uint32_t pins)
{
    mock_exti_pending |= pins;
}

static void test_init_and_io(void)
{
    GPIO_InitTypeDef init = {0};

    init.Pin = GPIO_PIN_5;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &init);

    assert(mock_last_mode == GD32_HAL_GPIO_MODE_OUTPUT_PP);
    assert(mock_last_speed == GD32_HAL_GPIO_SPEED_50MHZ);
    assert(mock_last_init_pins == GPIO_PIN_5);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    assert((mock_output & GPIO_PIN_5) != 0U);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    assert((mock_output & GPIO_PIN_5) == 0U);
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    assert((mock_output & GPIO_PIN_5) != 0U);
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    assert((mock_output & GPIO_PIN_5) == 0U);

    mock_input = GPIO_PIN_3;
    assert(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_SET);
    assert(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET);
    assert(HAL_GPIO_LockPin(GPIOC, GPIO_PIN_2) == HAL_OK);
}

static void test_exti(void)
{
    mock_exti_pending = GPIO_PIN_7;
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    assert(mock_exti_pending == 0U);

    __HAL_GPIO_EXTI_GENERATE_SWIT(GPIO_PIN_6);
    assert(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_6) != 0U);
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_6);
    assert(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_6) == 0U);
}

static void test_package_and_feature_guards(void)
{
    GPIO_InitTypeDef init = {0};
    uint32_t before;

    init.Pin = GPIO_PIN_3;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;
    before = mock_error_count;
    HAL_GPIO_Init(GPIOD, &init);
    assert(mock_error_count == before + 1U);
    assert(mock_last_error == GD32_HAL_PORT_ERROR_INVALID_PIN);

    init.Pin = GPIO_PIN_0;
    before = mock_error_count;
    HAL_GPIO_Init(GPIOD, &init);
    assert(mock_error_count == before + 1U);
    assert(mock_last_error == GD32_HAL_PORT_ERROR_INVALID_PIN);

    init.Pin = GPIO_PIN_1;
    init.Pull = GPIO_PULLUP;
    before = mock_error_count;
    mock_last_init_pins = 0U;
    HAL_GPIO_Init(GPIOA, &init);
    assert(mock_error_count == before + 1U);
    assert(mock_last_error == GD32_HAL_PORT_ERROR_OUTPUT_PULL_UNSUPPORTED);
    assert(mock_last_init_pins == 0U);

    init.Mode = GPIO_MODE_AF_PP;
    init.Pull = GPIO_NOPULL;
    init.Alternate = 7U;
    before = mock_error_count;
    mock_last_init_pins = 0U;
    HAL_GPIO_Init(GPIOA, &init);
    assert(mock_error_count == before);
    assert(mock_last_mode == GD32_HAL_GPIO_MODE_AF_PP);
    assert(mock_last_init_pins == GPIO_PIN_1);

    init.Alternate = 16U;
    HAL_GPIO_Init(GPIOA, &init);
    assert(mock_error_count == before + 1U);
}

int main(void)
{
    test_init_and_io();
    test_exti();
    test_package_and_feature_guards();
    puts("GPIO host tests: PASS");
    return 0;
}
