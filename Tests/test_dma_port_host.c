#include "gd32_hal_port.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

uint32_t mock_primask;

static volatile uint32_t mock_ctl[2][7];
static uint32_t mock_flags[2][7];
static uint32_t mock_periph_address[2][7];
static uint32_t mock_memory_address[2][7];
static uint32_t mock_count[2][7];
static int mock_circular[2][7];
static int mock_memory_to_memory[2][7];
static dma_parameter_struct mock_init;
static uint32_t mock_init_controller;
static dma_channel_enum mock_init_channel;
static uint32_t mock_deinit_count;
static GD32_HAL_PortError mock_last_port_error;

static uint32_t mock_controller_index(uint32_t controller)
{
    assert((controller == DMA0) || (controller == DMA1));
    return (controller == DMA0) ? 0U : 1U;
}

static void mock_reset(void)
{
    memset((void *)mock_ctl, 0, sizeof(mock_ctl));
    memset(mock_flags, 0, sizeof(mock_flags));
    memset(mock_periph_address, 0, sizeof(mock_periph_address));
    memset(mock_memory_address, 0, sizeof(mock_memory_address));
    memset(mock_count, 0, sizeof(mock_count));
    memset(mock_circular, 0, sizeof(mock_circular));
    memset(mock_memory_to_memory, 0, sizeof(mock_memory_to_memory));
    memset(&mock_init, 0, sizeof(mock_init));
    mock_init_controller = 0U;
    mock_init_channel = DMA_CH0;
    mock_deinit_count = 0U;
    mock_last_port_error = GD32_HAL_PORT_ERROR_NONE;
    mock_primask = 0U;
    GD32_HAL_DMA_ResetOwnership();
}

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    (void)detail;
    mock_last_port_error = error;
}

volatile uint32_t *mock_dma_ctl_address(uint32_t controller,
                                         dma_channel_enum channel)
{
    assert((uint32_t)channel <= 6U);
    return &mock_ctl[mock_controller_index(controller)][(uint32_t)channel];
}

void dma_deinit(uint32_t controller, dma_channel_enum channel)
{
    uint32_t index = mock_controller_index(controller);
    mock_ctl[index][(uint32_t)channel] = 0U;
    mock_flags[index][(uint32_t)channel] = 0U;
    ++mock_deinit_count;
}

void dma_struct_para_init(dma_parameter_struct *init_struct)
{
    memset(init_struct, 0, sizeof(*init_struct));
}

void dma_init(uint32_t controller,
              dma_channel_enum channel,
              dma_parameter_struct *init_struct)
{
    mock_init_controller = controller;
    mock_init_channel = channel;
    mock_init = *init_struct;
}

void dma_circulation_enable(uint32_t controller, dma_channel_enum channel)
{
    mock_circular[mock_controller_index(controller)][(uint32_t)channel] = 1;
}

void dma_circulation_disable(uint32_t controller, dma_channel_enum channel)
{
    mock_circular[mock_controller_index(controller)][(uint32_t)channel] = 0;
}

void dma_memory_to_memory_enable(uint32_t controller, dma_channel_enum channel)
{
    mock_memory_to_memory[mock_controller_index(controller)][(uint32_t)channel] = 1;
}

void dma_memory_to_memory_disable(uint32_t controller, dma_channel_enum channel)
{
    mock_memory_to_memory[mock_controller_index(controller)][(uint32_t)channel] = 0;
}

void dma_channel_enable(uint32_t controller, dma_channel_enum channel)
{
    mock_ctl[mock_controller_index(controller)][(uint32_t)channel] |= DMA_CHXCTL_CHEN;
}

void dma_channel_disable(uint32_t controller, dma_channel_enum channel)
{
    mock_ctl[mock_controller_index(controller)][(uint32_t)channel] &= ~DMA_CHXCTL_CHEN;
}

void dma_periph_address_config(uint32_t controller,
                               dma_channel_enum channel,
                               uint32_t address)
{
    mock_periph_address[mock_controller_index(controller)][(uint32_t)channel] = address;
}

void dma_memory_address_config(uint32_t controller,
                               dma_channel_enum channel,
                               uint32_t address)
{
    mock_memory_address[mock_controller_index(controller)][(uint32_t)channel] = address;
}

void dma_transfer_number_config(uint32_t controller,
                                dma_channel_enum channel,
                                uint32_t number)
{
    mock_count[mock_controller_index(controller)][(uint32_t)channel] = number;
}

uint32_t dma_transfer_number_get(uint32_t controller, dma_channel_enum channel)
{
    return mock_count[mock_controller_index(controller)][(uint32_t)channel];
}

FlagStatus dma_flag_get(uint32_t controller,
                        dma_channel_enum channel,
                        uint32_t flag)
{
    return ((mock_flags[mock_controller_index(controller)][(uint32_t)channel] & flag) != 0U) ?
           SET : RESET;
}

void dma_flag_clear(uint32_t controller,
                    dma_channel_enum channel,
                    uint32_t flag)
{
    mock_flags[mock_controller_index(controller)][(uint32_t)channel] &= ~flag;
}

void dma_interrupt_enable(uint32_t controller,
                          dma_channel_enum channel,
                          uint32_t source)
{
    volatile uint32_t *ctl = mock_dma_ctl_address(controller, channel);
    if ((source & DMA_INT_FTF) != 0U)
    {
        *ctl |= DMA_CHXCTL_FTFIE;
    }
    if ((source & DMA_INT_HTF) != 0U)
    {
        *ctl |= DMA_CHXCTL_HTFIE;
    }
    if ((source & DMA_INT_ERR) != 0U)
    {
        *ctl |= DMA_CHXCTL_ERRIE;
    }
}

void dma_interrupt_disable(uint32_t controller,
                           dma_channel_enum channel,
                           uint32_t source)
{
    volatile uint32_t *ctl = mock_dma_ctl_address(controller, channel);
    if ((source & DMA_INT_FTF) != 0U)
    {
        *ctl &= ~DMA_CHXCTL_FTFIE;
    }
    if ((source & DMA_INT_HTF) != 0U)
    {
        *ctl &= ~DMA_CHXCTL_HTFIE;
    }
    if ((source & DMA_INT_ERR) != 0U)
    {
        *ctl &= ~DMA_CHXCTL_ERRIE;
    }
}

static void test_instances_and_requests(void)
{
    assert(GD32_HAL_DMA_IsInstance(GD32_HAL_DMA0_CHANNEL0_ADDRESS));
    assert(GD32_HAL_DMA_IsInstance(GD32_HAL_DMA0_CHANNEL6_ADDRESS));
    assert(GD32_HAL_DMA_IsInstance(GD32_HAL_DMA1_CHANNEL4_ADDRESS));
    assert(!GD32_HAL_DMA_IsInstance(GD32_HAL_DMA1_CHANNEL4_ADDRESS + 0x14U));

    assert(GD32_HAL_DMA_IsRequestValid(
        GD32_HAL_DMA0_CHANNEL3_ADDRESS,
        GD32_HAL_DMA_REQUEST_ENCODE(0U, 3U, 1U),
        GD32_HAL_DMA_DIRECTION_MEMORY_TO_PERIPH));
    assert(!GD32_HAL_DMA_IsRequestValid(
        GD32_HAL_DMA0_CHANNEL4_ADDRESS,
        GD32_HAL_DMA_REQUEST_ENCODE(0U, 3U, 1U),
        GD32_HAL_DMA_DIRECTION_MEMORY_TO_PERIPH));
    assert(!GD32_HAL_DMA_IsRequestValid(
        GD32_HAL_DMA0_CHANNEL0_ADDRESS,
        GD32_HAL_DMA_REQUEST_ENCODE(0U, 0U, 3U),
        GD32_HAL_DMA_DIRECTION_PERIPH_TO_MEMORY));
    assert(!GD32_HAL_DMA_IsRequestValid(
        GD32_HAL_DMA0_CHANNEL0_ADDRESS,
        GD32_HAL_DMA_REQUEST_ENCODE(0U, 0U, 0U) | 0x00020000UL,
        GD32_HAL_DMA_DIRECTION_PERIPH_TO_MEMORY));
    assert(GD32_HAL_DMA_IsRequestValid(
        GD32_HAL_DMA1_CHANNEL4_ADDRESS,
        GD32_HAL_DMA_REQUEST_ENCODE(1U, 4U, 3U),
        GD32_HAL_DMA_DIRECTION_PERIPH_TO_MEMORY));
    assert(GD32_HAL_DMA_IsRequestValid(
        GD32_HAL_DMA0_CHANNEL6_ADDRESS,
        GD32_HAL_DMA_REQUEST_MEMORY,
        GD32_HAL_DMA_DIRECTION_MEMORY_TO_MEMORY));
    assert(!GD32_HAL_DMA_IsRequestValid(
        GD32_HAL_DMA0_CHANNEL6_ADDRESS,
        GD32_HAL_DMA_REQUEST_MEMORY,
        GD32_HAL_DMA_DIRECTION_MEMORY_TO_PERIPH));
}

static void test_configuration_and_transfer(void)
{
    GD32_HAL_DMAConfig config = {
        GD32_HAL_DMA_DIRECTION_MEMORY_TO_MEMORY,
        1U,
        1U,
        2U,
        4U,
        0U,
        3U
    };

    assert(GD32_HAL_DMA_Configure(GD32_HAL_DMA1_CHANNEL2_ADDRESS, &config) == 0);
    assert(mock_deinit_count == 1U);
    assert(mock_init_controller == DMA1);
    assert(mock_init_channel == DMA_CH2);
    assert(mock_init.periph_width == DMA_PERIPHERAL_WIDTH_16BIT);
    assert(mock_init.memory_width == DMA_MEMORY_WIDTH_32BIT);
    assert(mock_init.periph_inc == DMA_PERIPH_INCREASE_ENABLE);
    assert(mock_init.memory_inc == DMA_MEMORY_INCREASE_ENABLE);
    assert(mock_init.direction == DMA_MEMORY_TO_PERIPHERAL);
    assert(mock_init.priority == DMA_PRIORITY_ULTRA_HIGH);
    assert(mock_memory_to_memory[1][2] == 1);

    GD32_HAL_DMA_SetTransfer(GD32_HAL_DMA1_CHANNEL2_ADDRESS,
                             GD32_HAL_DMA_DIRECTION_MEMORY_TO_MEMORY,
                             0x20000000U,
                             0x20001000U,
                             32U);
    assert(mock_memory_address[1][2] == 0x20000000U);
    assert(mock_periph_address[1][2] == 0x20001000U);
    assert(mock_count[1][2] == 32U);

    GD32_HAL_DMA_SetTransfer(GD32_HAL_DMA0_CHANNEL0_ADDRESS,
                             GD32_HAL_DMA_DIRECTION_PERIPH_TO_MEMORY,
                             0x4001244CU,
                             0x20002000U,
                             8U);
    assert(mock_periph_address[0][0] == 0x4001244CU);
    assert(mock_memory_address[0][0] == 0x20002000U);
}

static void test_flags_interrupts_counter_and_owner(void)
{
    const uint32_t address = GD32_HAL_DMA0_CHANNEL5_ADDRESS;
    int first_owner;
    int second_owner;

    GD32_HAL_DMA_Enable(address);
    assert(GD32_HAL_DMA_IsEnabled(address));
    GD32_HAL_DMA_Disable(address);
    assert(!GD32_HAL_DMA_IsEnabled(address));

    mock_flags[0][5] = DMA_FLAG_FTF | DMA_FLAG_HTF | DMA_FLAG_ERR;
    assert(GD32_HAL_DMA_GetFlag(address, GD32_HAL_DMA_FLAG_FULL));
    GD32_HAL_DMA_ClearFlag(address,
                           GD32_HAL_DMA_FLAG_FULL | GD32_HAL_DMA_FLAG_HALF);
    assert(mock_flags[0][5] == DMA_FLAG_ERR);

    GD32_HAL_DMA_SetInterrupt(address,
                              GD32_HAL_DMA_INTERRUPT_FULL |
                              GD32_HAL_DMA_INTERRUPT_HALF |
                              GD32_HAL_DMA_INTERRUPT_ERROR,
                              1);
    assert(GD32_HAL_DMA_GetInterrupts(address) ==
           (GD32_HAL_DMA_INTERRUPT_FULL |
            GD32_HAL_DMA_INTERRUPT_HALF |
            GD32_HAL_DMA_INTERRUPT_ERROR));
    GD32_HAL_DMA_SetInterrupt(address, GD32_HAL_DMA_INTERRUPT_HALF, 0);
    assert((GD32_HAL_DMA_GetInterrupts(address) &
            GD32_HAL_DMA_INTERRUPT_HALF) == 0U);

    GD32_HAL_DMA_SetRemaining(address, 123U);
    assert(GD32_HAL_DMA_GetRemaining(address) == 123U);

    assert(GD32_HAL_DMA_Claim(address, &first_owner));
    assert(!GD32_HAL_DMA_Claim(address, &second_owner));
    assert(GD32_HAL_DMA_Claim(address, &first_owner));
    GD32_HAL_DMA_Release(address, &second_owner);
    assert(!GD32_HAL_DMA_Claim(address, &second_owner));
    GD32_HAL_DMA_Release(address, &first_owner);
    assert(GD32_HAL_DMA_Claim(address, &second_owner));
    GD32_HAL_DMA_ResetOwnership();
    assert(GD32_HAL_DMA_Claim(address, &first_owner));
    GD32_HAL_DMA_Release(address, &first_owner);

    (void)GD32_HAL_DMA_GetFlag(address, GD32_HAL_DMA_FLAG_FIFO_UNSUPPORTED);
    assert(mock_last_port_error == GD32_HAL_PORT_ERROR_DMA_UNSUPPORTED_FLAG);
}

int main(void)
{
    mock_reset();
    test_instances_and_requests();
    test_configuration_and_transfer();
    test_flags_interrupts_counter_and_owner();
    puts("DMA GD32 Port mapping host tests: PASS");
    return 0;
}
