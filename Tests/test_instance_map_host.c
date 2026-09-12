#include "stm32f4xx_hal.h"
#include "gd32_hal_port.h"
#include <assert.h>
#include <stdio.h>

static uintptr_t clock_token;
static GD32_HAL_ResourceKind clock_kind;
static int clock_enable;
static GD32_HAL_RCCCommonClock common_clock;

int GD32_HAL_RCC_SetInstanceClock(uintptr_t token,
                                  GD32_HAL_ResourceKind kind,
                                  int enable)
{
    clock_token = token;
    clock_kind = kind;
    clock_enable = enable;
    return 0;
}

int GD32_HAL_RCC_SetCommonClock(GD32_HAL_RCCCommonClock clock, int enable)
{
    common_clock = clock;
    clock_enable = enable;
    return 0;
}

static void AssertMapping(uintptr_t semantic,
                          uintptr_t public_instance,
                          GD32_HAL_ResourceKind kind,
                          uint32_t target)
{
    const GD32_HAL_Resource *resource =
        GD32_HAL_ResolveInstance(public_instance, kind);

    assert(resource != NULL);
    assert(resource->semantic_id == semantic);
    assert(resource->stm32_instance == public_instance);
    assert(resource->gd32_instance == target);
}

int main(void)
{
    AssertMapping(STM32_GPIO_SEMANTIC_A, STM32_GPIO_INSTANCE_A,
                  GD32_HAL_RESOURCE_GPIO,
                  GD32_HAL_GPIOA_ADDRESS);
    AssertMapping(STM32_UART_SEMANTIC_1, STM32_UART_INSTANCE_1,
                  GD32_HAL_RESOURCE_UART,
                  GD32_HAL_USART0_ADDRESS);
    AssertMapping(STM32_UART_SEMANTIC_2, STM32_UART_INSTANCE_2,
                  GD32_HAL_RESOURCE_UART,
                  GD32_HAL_USART1_ADDRESS);
    AssertMapping(STM32_UART_SEMANTIC_6, STM32_UART_INSTANCE_6,
                  GD32_HAL_RESOURCE_UART,
                  GD32_HAL_USART2_ADDRESS);
    AssertMapping(STM32_TIM_SEMANTIC_1, STM32_TIM_INSTANCE_1,
                  GD32_HAL_RESOURCE_TIMER,
                  GD32_HAL_TIMER0_ADDRESS);
    AssertMapping(STM32_TIM_SEMANTIC_5, STM32_TIM_INSTANCE_5,
                  GD32_HAL_RESOURCE_TIMER,
                  GD32_HAL_TIMER4_ADDRESS);
    AssertMapping(STM32_TIM_SEMANTIC_9, STM32_TIM_INSTANCE_9,
                  GD32_HAL_RESOURCE_TIMER,
                  GD32_HAL_TIMER8_ADDRESS);
    AssertMapping(STM32_ADC_SEMANTIC_1, STM32_ADC_INSTANCE_1,
                  GD32_HAL_RESOURCE_ADC,
                  GD32_HAL_ADC0_ADDRESS);
    AssertMapping(STM32_DMA0_CHANNEL0, STM32_DMA0_CHANNEL0,
                  GD32_HAL_RESOURCE_DMA,
                  GD32_HAL_DMA0_CHANNEL0_ADDRESS);
    AssertMapping(STM32_SPI_SEMANTIC_1, STM32_SPI_INSTANCE_1,
                  GD32_HAL_RESOURCE_SPI,
                  GD32_HAL_SPI0_ADDRESS);

    {
        uint32_t request = 0U;
        const GD32_HAL_Resource *resource = GD32_HAL_ResolveDMAStream(
            STM32_DMA2_STREAM7, DMA_CHANNEL_4, DMA_MEMORY_TO_PERIPH, &request);
        assert(resource != NULL);
        assert(resource->gd32_instance == GD32_HAL_DMA0_CHANNEL3_ADDRESS);
        assert(request == GD32_DMA_REQUEST_USART0_TX);
    }

    assert(GD32_HAL_SPI0_ADDRESS == 0x40013000UL);
    assert(GD32_HAL_SPI0_ADDRESS != GD32_HAL_USART0_ADDRESS);
    assert(GD32_HAL_ResolveInstance(
               STM32_HAL_INSTANCE_TOKEN(0x02U, 0x03U),
               GD32_HAL_RESOURCE_UART) == NULL);
    assert(GD32_HAL_ResolveInstance(
               STM32_HAL_INSTANCE_TOKEN(0x03U, 0x08U),
               GD32_HAL_RESOURCE_TIMER) == NULL);
    assert(GD32_HAL_ResolveInstance(STM32_UART_INSTANCE_1,
                                    GD32_HAL_RESOURCE_TIMER) == NULL);

    __HAL_RCC_USART6_CLK_ENABLE();
    assert(clock_token == STM32_UART_INSTANCE_6);
    assert(clock_kind == GD32_HAL_RESOURCE_UART);
    assert(clock_enable == 1);
    __HAL_RCC_TIM9_CLK_DISABLE();
    assert(clock_token == STM32_TIM_INSTANCE_9);
    assert(clock_kind == GD32_HAL_RESOURCE_TIMER);
    assert(clock_enable == 0);
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    assert(common_clock == GD32_HAL_RCC_COMMON_AF);
    assert(clock_enable == 1);
    puts("Instance map host tests: PASS");
    return 0;
}
