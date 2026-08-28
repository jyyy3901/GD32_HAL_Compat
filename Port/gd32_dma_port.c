#include "gd32_hal_port.h"
#include <stddef.h>

#define GD32_HAL_DMA_CHANNEL_COUNT 12U

static const void *gd32HalDMAOwners[GD32_HAL_DMA_CHANNEL_COUNT];

static int GD32_HAL_DMA_Resolve(uint32_t channel_address,
                                uint32_t *controller,
                                dma_channel_enum *channel,
                                uint32_t *linear_index)
{
    uint32_t index;

    for (index = 0U; index <= 6U; ++index)
    {
        if (channel_address == GD32_HAL_DMA_CHANNEL_ADDRESS(GD32_HAL_DMA0_ADDRESS, index))
        {
            if (controller != NULL)
            {
                *controller = DMA0;
            }
            if (channel != NULL)
            {
                *channel = (dma_channel_enum)index;
            }
            if (linear_index != NULL)
            {
                *linear_index = index;
            }
            return 1;
        }
    }

    for (index = 0U; index <= 4U; ++index)
    {
        if (channel_address == GD32_HAL_DMA_CHANNEL_ADDRESS(GD32_HAL_DMA1_ADDRESS, index))
        {
            if (controller != NULL)
            {
                *controller = DMA1;
            }
            if (channel != NULL)
            {
                *channel = (dma_channel_enum)index;
            }
            if (linear_index != NULL)
            {
                *linear_index = 7U + index;
            }
            return 1;
        }
    }
    return 0;
}

int GD32_HAL_DMA_IsInstance(uint32_t channel_address)
{
    return GD32_HAL_DMA_Resolve(channel_address, NULL, NULL, NULL);
}

int GD32_HAL_DMA_IsRequestValid(uint32_t channel_address,
                                uint32_t request,
                                GD32_HAL_DMADirection direction)
{
    static const uint8_t dma0_slots[7] = {3U, 5U, 5U, 7U, 6U, 5U, 5U};
    static const uint8_t dma1_slots[5] = {5U, 6U, 4U, 4U, 4U};
    uint32_t controller;
    dma_channel_enum channel;
    uint32_t request_controller;
    uint32_t request_channel;
    uint32_t request_slot;

    if (GD32_HAL_DMA_Resolve(channel_address, &controller, &channel, NULL) == 0)
    {
        return 0;
    }
    if (direction == GD32_HAL_DMA_DIRECTION_MEMORY_TO_MEMORY)
    {
        return request == GD32_HAL_DMA_REQUEST_MEMORY;
    }
    if ((request & 0x80000000UL) == 0U)
    {
        return 0;
    }
    if ((request & 0x7FFE0000UL) != 0U)
    {
        return 0;
    }

    request_controller = (request >> 16U) & 0x01U;
    request_channel = (request >> 8U) & 0xFFU;
    request_slot = request & 0xFFU;
    if ((request_slot == 0U) ||
        (request_controller != ((controller == DMA0) ? 0U : 1U)) ||
        (request_channel != (uint32_t)channel))
    {
        return 0;
    }

    if (controller == DMA0)
    {
        return request_slot <= dma0_slots[(uint32_t)channel];
    }
    return request_slot <= dma1_slots[(uint32_t)channel];
}

uint32_t GD32_HAL_DMA_GetControllerAddress(uint32_t channel_address)
{
    uint32_t controller = 0U;

    (void)GD32_HAL_DMA_Resolve(channel_address, &controller, NULL, NULL);
    return controller;
}

uint32_t GD32_HAL_DMA_GetChannelIndex(uint32_t channel_address)
{
    dma_channel_enum channel = DMA_CH0;

    (void)GD32_HAL_DMA_Resolve(channel_address, NULL, &channel, NULL);
    return (uint32_t)channel;
}

int GD32_HAL_DMA_Claim(uint32_t channel_address, const void *owner)
{
    uint32_t linear_index;
    uint32_t interrupt_state;
    int claimed = 0;

    if ((owner == NULL) ||
        (GD32_HAL_DMA_Resolve(channel_address, NULL, NULL, &linear_index) == 0))
    {
        return 0;
    }

    interrupt_state = __get_PRIMASK();
    __disable_irq();
    if ((gd32HalDMAOwners[linear_index] == NULL) ||
        (gd32HalDMAOwners[linear_index] == owner))
    {
        gd32HalDMAOwners[linear_index] = owner;
        claimed = 1;
    }
    if (interrupt_state == 0U)
    {
        __enable_irq();
    }
    return claimed;
}

void GD32_HAL_DMA_Release(uint32_t channel_address, const void *owner)
{
    uint32_t linear_index;
    uint32_t interrupt_state;

    if ((owner == NULL) ||
        (GD32_HAL_DMA_Resolve(channel_address, NULL, NULL, &linear_index) == 0))
    {
        return;
    }

    interrupt_state = __get_PRIMASK();
    __disable_irq();
    if (gd32HalDMAOwners[linear_index] == owner)
    {
        gd32HalDMAOwners[linear_index] = NULL;
    }
    if (interrupt_state == 0U)
    {
        __enable_irq();
    }
}

void GD32_HAL_DMA_ResetOwnership(void)
{
    uint32_t index;
    uint32_t interrupt_state = __get_PRIMASK();

    __disable_irq();
    for (index = 0U; index < GD32_HAL_DMA_CHANNEL_COUNT; ++index)
    {
        gd32HalDMAOwners[index] = NULL;
    }
    if (interrupt_state == 0U)
    {
        __enable_irq();
    }
}

static uint32_t GD32_HAL_DMA_PeriphWidth(uint8_t width)
{
    if (width == 2U)
    {
        return DMA_PERIPHERAL_WIDTH_16BIT;
    }
    if (width == 4U)
    {
        return DMA_PERIPHERAL_WIDTH_32BIT;
    }
    return DMA_PERIPHERAL_WIDTH_8BIT;
}

static uint32_t GD32_HAL_DMA_MemoryWidth(uint8_t width)
{
    if (width == 2U)
    {
        return DMA_MEMORY_WIDTH_16BIT;
    }
    if (width == 4U)
    {
        return DMA_MEMORY_WIDTH_32BIT;
    }
    return DMA_MEMORY_WIDTH_8BIT;
}

static uint32_t GD32_HAL_DMA_Priority(uint8_t priority)
{
    if (priority == 1U)
    {
        return DMA_PRIORITY_MEDIUM;
    }
    if (priority == 2U)
    {
        return DMA_PRIORITY_HIGH;
    }
    if (priority == 3U)
    {
        return DMA_PRIORITY_ULTRA_HIGH;
    }
    return DMA_PRIORITY_LOW;
}

int GD32_HAL_DMA_Configure(uint32_t channel_address,
                           const GD32_HAL_DMAConfig *config)
{
    dma_parameter_struct init;
    uint32_t controller;
    dma_channel_enum channel;

    if ((config == NULL) ||
        (GD32_HAL_DMA_Resolve(channel_address, &controller, &channel, NULL) == 0))
    {
        return -1;
    }

    dma_deinit(controller, channel);
    dma_struct_para_init(&init);
    init.periph_width = GD32_HAL_DMA_PeriphWidth(config->periph_width);
    init.memory_width = GD32_HAL_DMA_MemoryWidth(config->memory_width);
    init.periph_inc = (config->periph_increment != 0U) ?
                      DMA_PERIPH_INCREASE_ENABLE : DMA_PERIPH_INCREASE_DISABLE;
    init.memory_inc = (config->memory_increment != 0U) ?
                      DMA_MEMORY_INCREASE_ENABLE : DMA_MEMORY_INCREASE_DISABLE;
    init.direction = (config->direction == GD32_HAL_DMA_DIRECTION_PERIPH_TO_MEMORY) ?
                     DMA_PERIPHERAL_TO_MEMORY : DMA_MEMORY_TO_PERIPHERAL;
    init.priority = GD32_HAL_DMA_Priority(config->priority);
    dma_init(controller, channel, &init);

    if (config->circular != 0U)
    {
        dma_circulation_enable(controller, channel);
    }
    else
    {
        dma_circulation_disable(controller, channel);
    }
    if (config->direction == GD32_HAL_DMA_DIRECTION_MEMORY_TO_MEMORY)
    {
        dma_memory_to_memory_enable(controller, channel);
    }
    else
    {
        dma_memory_to_memory_disable(controller, channel);
    }
    return 0;
}

void GD32_HAL_DMA_DeInit(uint32_t channel_address)
{
    uint32_t controller;
    dma_channel_enum channel;

    if (GD32_HAL_DMA_Resolve(channel_address, &controller, &channel, NULL) != 0)
    {
        dma_deinit(controller, channel);
    }
}

void GD32_HAL_DMA_SetTransfer(uint32_t channel_address,
                              GD32_HAL_DMADirection direction,
                              uint32_t source,
                              uint32_t destination,
                              uint32_t count)
{
    uint32_t controller;
    dma_channel_enum channel;

    if (GD32_HAL_DMA_Resolve(channel_address, &controller, &channel, NULL) == 0)
    {
        return;
    }

    if (direction == GD32_HAL_DMA_DIRECTION_PERIPH_TO_MEMORY)
    {
        dma_periph_address_config(controller, channel, source);
        dma_memory_address_config(controller, channel, destination);
    }
    else
    {
        dma_memory_address_config(controller, channel, source);
        dma_periph_address_config(controller, channel, destination);
    }
    dma_transfer_number_config(controller, channel, count);
}

void GD32_HAL_DMA_Enable(uint32_t channel_address)
{
    uint32_t controller;
    dma_channel_enum channel;

    if (GD32_HAL_DMA_Resolve(channel_address, &controller, &channel, NULL) != 0)
    {
        dma_channel_enable(controller, channel);
    }
}

void GD32_HAL_DMA_Disable(uint32_t channel_address)
{
    uint32_t controller;
    dma_channel_enum channel;

    if (GD32_HAL_DMA_Resolve(channel_address, &controller, &channel, NULL) != 0)
    {
        dma_channel_disable(controller, channel);
    }
}

int GD32_HAL_DMA_IsEnabled(uint32_t channel_address)
{
    uint32_t controller;
    dma_channel_enum channel;

    if (GD32_HAL_DMA_Resolve(channel_address, &controller, &channel, NULL) == 0)
    {
        return 0;
    }
    return (DMA_CHCTL(controller, channel) & DMA_CHXCTL_CHEN) != 0U;
}

static uint32_t GD32_HAL_DMA_FlagValue(uint32_t flags)
{
    uint32_t value = 0U;

    if ((flags & GD32_HAL_DMA_FLAG_FULL) != 0U)
    {
        value |= DMA_FLAG_FTF;
    }
    if ((flags & GD32_HAL_DMA_FLAG_HALF) != 0U)
    {
        value |= DMA_FLAG_HTF;
    }
    if ((flags & GD32_HAL_DMA_FLAG_ERROR) != 0U)
    {
        value |= DMA_FLAG_ERR;
    }
    return value;
}

uint32_t GD32_HAL_DMA_GetFlag(uint32_t channel_address, uint32_t flags)
{
    uint32_t controller;
    dma_channel_enum channel;
    uint32_t value;

    if ((flags & (GD32_HAL_DMA_FLAG_FIFO_UNSUPPORTED |
                  GD32_HAL_DMA_FLAG_DIRECT_UNSUPPORTED)) != 0U)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_DMA_UNSUPPORTED_FLAG, flags);
    }
    if (GD32_HAL_DMA_Resolve(channel_address, &controller, &channel, NULL) == 0)
    {
        return 0U;
    }
    value = GD32_HAL_DMA_FlagValue(flags);
    return (value != 0U) && (dma_flag_get(controller, channel, value) != RESET);
}

void GD32_HAL_DMA_ClearFlag(uint32_t channel_address, uint32_t flags)
{
    uint32_t controller;
    dma_channel_enum channel;
    uint32_t value;

    if ((flags & (GD32_HAL_DMA_FLAG_FIFO_UNSUPPORTED |
                  GD32_HAL_DMA_FLAG_DIRECT_UNSUPPORTED)) != 0U)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_DMA_UNSUPPORTED_FLAG, flags);
    }
    if (GD32_HAL_DMA_Resolve(channel_address, &controller, &channel, NULL) == 0)
    {
        return;
    }
    value = GD32_HAL_DMA_FlagValue(flags);
    if (value != 0U)
    {
        dma_flag_clear(controller, channel, value);
    }
}

void GD32_HAL_DMA_SetInterrupt(uint32_t channel_address,
                               uint32_t interrupts,
                               int enable)
{
    uint32_t controller;
    dma_channel_enum channel;
    uint32_t supported = interrupts &
                         (GD32_HAL_DMA_INTERRUPT_FULL |
                          GD32_HAL_DMA_INTERRUPT_HALF |
                          GD32_HAL_DMA_INTERRUPT_ERROR);

    if ((interrupts & (GD32_HAL_DMA_INTERRUPT_FIFO_UNSUPPORTED |
                       GD32_HAL_DMA_INTERRUPT_DIRECT_UNSUPPORTED)) != 0U)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_DMA_UNSUPPORTED_FLAG, interrupts);
    }
    if (GD32_HAL_DMA_Resolve(channel_address, &controller, &channel, NULL) == 0)
    {
        return;
    }

    if ((supported & GD32_HAL_DMA_INTERRUPT_FULL) != 0U)
    {
        if (enable != 0)
        {
            dma_interrupt_enable(controller, channel, DMA_INT_FTF);
        }
        else
        {
            dma_interrupt_disable(controller, channel, DMA_INT_FTF);
        }
    }
    if ((supported & GD32_HAL_DMA_INTERRUPT_HALF) != 0U)
    {
        if (enable != 0)
        {
            dma_interrupt_enable(controller, channel, DMA_INT_HTF);
        }
        else
        {
            dma_interrupt_disable(controller, channel, DMA_INT_HTF);
        }
    }
    if ((supported & GD32_HAL_DMA_INTERRUPT_ERROR) != 0U)
    {
        if (enable != 0)
        {
            dma_interrupt_enable(controller, channel, DMA_INT_ERR);
        }
        else
        {
            dma_interrupt_disable(controller, channel, DMA_INT_ERR);
        }
    }
}

uint32_t GD32_HAL_DMA_GetInterrupts(uint32_t channel_address)
{
    uint32_t controller;
    dma_channel_enum channel;
    uint32_t value = 0U;

    if (GD32_HAL_DMA_Resolve(channel_address, &controller, &channel, NULL) == 0)
    {
        return 0U;
    }
    if ((DMA_CHCTL(controller, channel) & DMA_CHXCTL_FTFIE) != 0U)
    {
        value |= GD32_HAL_DMA_INTERRUPT_FULL;
    }
    if ((DMA_CHCTL(controller, channel) & DMA_CHXCTL_HTFIE) != 0U)
    {
        value |= GD32_HAL_DMA_INTERRUPT_HALF;
    }
    if ((DMA_CHCTL(controller, channel) & DMA_CHXCTL_ERRIE) != 0U)
    {
        value |= GD32_HAL_DMA_INTERRUPT_ERROR;
    }
    return value;
}

void GD32_HAL_DMA_SetRemaining(uint32_t channel_address, uint32_t count)
{
    uint32_t controller;
    dma_channel_enum channel;

    if (GD32_HAL_DMA_Resolve(channel_address, &controller, &channel, NULL) != 0)
    {
        dma_transfer_number_config(controller, channel, count);
    }
}

uint32_t GD32_HAL_DMA_GetRemaining(uint32_t channel_address)
{
    uint32_t controller;
    dma_channel_enum channel;

    if (GD32_HAL_DMA_Resolve(channel_address, &controller, &channel, NULL) == 0)
    {
        return 0U;
    }
    return dma_transfer_number_get(controller, channel);
}
