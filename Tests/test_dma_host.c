#include "stm32f4xx_hal.h"
#include "gd32_hal_port.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static GD32_HAL_DMAConfig mock_config;
static uint32_t mock_configure_count;
static uint32_t mock_deinit_count;
static uint32_t mock_flags;
static uint32_t mock_interrupts;
static uint32_t mock_remaining;
static uint32_t mock_source;
static uint32_t mock_destination;
static uint32_t mock_length;
static GD32_HAL_DMADirection mock_direction;
static int mock_enabled;
static int mock_disable_stuck;
static const void *mock_owners[12];
static uint32_t mock_tick;
static int mock_tick_auto_increment;
static GD32_HAL_PortError mock_last_port_error;
static uint32_t mock_full_callback_count;
static uint32_t mock_half_callback_count;
static uint32_t mock_error_callback_count;
static uint32_t mock_abort_callback_count;
static uint8_t mock_events[16];
static uint32_t mock_event_count;

#define MOCK_EVENT_HALF  1U
#define MOCK_EVENT_FULL  2U
#define MOCK_EVENT_ERROR 3U
#define MOCK_EVENT_ABORT 4U

static int mock_instance_index(uint32_t address)
{
    uint32_t index;

    for (index = 0U; index <= 6U; ++index)
    {
        if (address == GD32_HAL_DMA_CHANNEL_ADDRESS(GD32_HAL_DMA0_ADDRESS, index))
        {
            return (int)index;
        }
    }
    for (index = 0U; index <= 4U; ++index)
    {
        if (address == GD32_HAL_DMA_CHANNEL_ADDRESS(GD32_HAL_DMA1_ADDRESS, index))
        {
            return (int)(7U + index);
        }
    }
    return -1;
}

static void mock_reset(void)
{
    memset(&mock_config, 0, sizeof(mock_config));
    memset(mock_owners, 0, sizeof(mock_owners));
    mock_configure_count = 0U;
    mock_deinit_count = 0U;
    mock_flags = 0U;
    mock_interrupts = 0U;
    mock_remaining = 0U;
    mock_source = 0U;
    mock_destination = 0U;
    mock_length = 0U;
    mock_direction = GD32_HAL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    mock_enabled = 0;
    mock_disable_stuck = 0;
    mock_tick = 0U;
    mock_tick_auto_increment = 0;
    mock_last_port_error = GD32_HAL_PORT_ERROR_NONE;
    mock_full_callback_count = 0U;
    mock_half_callback_count = 0U;
    mock_error_callback_count = 0U;
    mock_abort_callback_count = 0U;
    mock_event_count = 0U;
}

static DMA_HandleTypeDef mock_handle(DMA_Stream_TypeDef *instance,
                                     uint32_t request,
                                     uint32_t direction)
{
    DMA_HandleTypeDef hdma;

    memset(&hdma, 0, sizeof(hdma));
    hdma.Instance = instance;
    hdma.Init.Channel = request;
    hdma.Init.Direction = direction;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma.Init.Mode = DMA_NORMAL;
    hdma.Init.Priority = DMA_PRIORITY_HIGH;
    hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
    hdma.State = HAL_DMA_STATE_RESET;
    return hdma;
}

uint32_t HAL_GetTick(void)
{
    uint32_t value = mock_tick;
    if (mock_tick_auto_increment != 0)
    {
        ++mock_tick;
    }
    return value;
}

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    (void)detail;
    mock_last_port_error = error;
}

int GD32_HAL_DMA_IsInstance(uint32_t channel_address)
{
    return mock_instance_index(channel_address) >= 0;
}

int GD32_HAL_DMA_IsRequestValid(uint32_t channel_address,
                                uint32_t request,
                                GD32_HAL_DMADirection direction)
{
    int index = mock_instance_index(channel_address);
    uint32_t controller;
    uint32_t channel;

    if (index < 0)
    {
        return 0;
    }
    if (direction == GD32_HAL_DMA_DIRECTION_MEMORY_TO_MEMORY)
    {
        return request == GD32_DMA_REQUEST_MEMORY;
    }
    if ((request & 0x80000000UL) == 0U)
    {
        return 0;
    }
    controller = (index >= 7) ? 1U : 0U;
    channel = (index >= 7) ? (uint32_t)(index - 7) : (uint32_t)index;
    return (((request >> 16U) & 1U) == controller) &&
           (((request >> 8U) & 0xFFU) == channel) &&
           ((request & 0xFFU) != 0U);
}

uint32_t GD32_HAL_DMA_GetControllerAddress(uint32_t channel_address)
{
    int index = mock_instance_index(channel_address);
    assert(index >= 0);
    return (index >= 7) ? GD32_HAL_DMA1_ADDRESS : GD32_HAL_DMA0_ADDRESS;
}

uint32_t GD32_HAL_DMA_GetChannelIndex(uint32_t channel_address)
{
    int index = mock_instance_index(channel_address);
    assert(index >= 0);
    return (index >= 7) ? (uint32_t)(index - 7) : (uint32_t)index;
}

int GD32_HAL_DMA_Claim(uint32_t channel_address, const void *owner)
{
    int index = mock_instance_index(channel_address);
    assert(index >= 0);
    if ((mock_owners[index] != NULL) && (mock_owners[index] != owner))
    {
        return 0;
    }
    mock_owners[index] = owner;
    return 1;
}

void GD32_HAL_DMA_Release(uint32_t channel_address, const void *owner)
{
    int index = mock_instance_index(channel_address);
    assert(index >= 0);
    if (mock_owners[index] == owner)
    {
        mock_owners[index] = NULL;
    }
}

int GD32_HAL_DMA_Configure(uint32_t channel_address,
                           const GD32_HAL_DMAConfig *config)
{
    assert(GD32_HAL_DMA_IsInstance(channel_address));
    mock_config = *config;
    ++mock_configure_count;
    return 0;
}

void GD32_HAL_DMA_DeInit(uint32_t channel_address)
{
    assert(GD32_HAL_DMA_IsInstance(channel_address));
    ++mock_deinit_count;
}

void GD32_HAL_DMA_SetTransfer(uint32_t channel_address,
                              GD32_HAL_DMADirection direction,
                              uint32_t source,
                              uint32_t destination,
                              uint32_t count)
{
    assert(GD32_HAL_DMA_IsInstance(channel_address));
    mock_direction = direction;
    mock_source = source;
    mock_destination = destination;
    mock_length = count;
    mock_remaining = count;
}

void GD32_HAL_DMA_Enable(uint32_t channel_address)
{
    assert(GD32_HAL_DMA_IsInstance(channel_address));
    mock_enabled = 1;
}

void GD32_HAL_DMA_Disable(uint32_t channel_address)
{
    assert(GD32_HAL_DMA_IsInstance(channel_address));
    if (mock_disable_stuck == 0)
    {
        mock_enabled = 0;
    }
}

int GD32_HAL_DMA_IsEnabled(uint32_t channel_address)
{
    assert(GD32_HAL_DMA_IsInstance(channel_address));
    return mock_enabled;
}

uint32_t GD32_HAL_DMA_GetFlag(uint32_t channel_address, uint32_t flags)
{
    assert(GD32_HAL_DMA_IsInstance(channel_address));
    if ((flags & (GD32_HAL_DMA_FLAG_FIFO_UNSUPPORTED |
                  GD32_HAL_DMA_FLAG_DIRECT_UNSUPPORTED)) != 0U)
    {
        mock_last_port_error = GD32_HAL_PORT_ERROR_DMA_UNSUPPORTED_FLAG;
    }
    return mock_flags & flags;
}

void GD32_HAL_DMA_ClearFlag(uint32_t channel_address, uint32_t flags)
{
    assert(GD32_HAL_DMA_IsInstance(channel_address));
    mock_flags &= ~flags;
}

void GD32_HAL_DMA_SetInterrupt(uint32_t channel_address,
                               uint32_t interrupts,
                               int enable)
{
    assert(GD32_HAL_DMA_IsInstance(channel_address));
    if ((interrupts & (GD32_HAL_DMA_INTERRUPT_FIFO_UNSUPPORTED |
                       GD32_HAL_DMA_INTERRUPT_DIRECT_UNSUPPORTED)) != 0U)
    {
        mock_last_port_error = GD32_HAL_PORT_ERROR_DMA_UNSUPPORTED_FLAG;
    }
    if (enable != 0)
    {
        mock_interrupts |= interrupts &
                           (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE);
    }
    else
    {
        mock_interrupts &= ~interrupts;
    }
}

uint32_t GD32_HAL_DMA_GetInterrupts(uint32_t channel_address)
{
    assert(GD32_HAL_DMA_IsInstance(channel_address));
    return mock_interrupts;
}

void GD32_HAL_DMA_SetRemaining(uint32_t channel_address, uint32_t count)
{
    assert(GD32_HAL_DMA_IsInstance(channel_address));
    mock_remaining = count;
}

uint32_t GD32_HAL_DMA_GetRemaining(uint32_t channel_address)
{
    assert(GD32_HAL_DMA_IsInstance(channel_address));
    return mock_remaining;
}

static void mock_full_callback(DMA_HandleTypeDef *hdma)
{
    (void)hdma;
    ++mock_full_callback_count;
    mock_events[mock_event_count++] = MOCK_EVENT_FULL;
}

static void mock_half_callback(DMA_HandleTypeDef *hdma)
{
    (void)hdma;
    ++mock_half_callback_count;
    mock_events[mock_event_count++] = MOCK_EVENT_HALF;
}

static void mock_error_callback(DMA_HandleTypeDef *hdma)
{
    assert((hdma->ErrorCode & HAL_DMA_ERROR_TE) != 0U);
    ++mock_error_callback_count;
    mock_events[mock_event_count++] = MOCK_EVENT_ERROR;
}

static void mock_abort_callback(DMA_HandleTypeDef *hdma)
{
    assert(hdma->State == HAL_DMA_STATE_READY);
    ++mock_abort_callback_count;
    mock_events[mock_event_count++] = MOCK_EVENT_ABORT;
}

static void test_init_and_guards(void)
{
    DMA_HandleTypeDef hdma;

    mock_reset();
    hdma = mock_handle(GD32_DMA0_CHANNEL3,
                       GD32_DMA_REQUEST_USART0_TX,
                       DMA_MEMORY_TO_PERIPH);
    assert(HAL_DMA_Init(&hdma) == HAL_OK);
    assert(mock_configure_count == 1U);
    assert(mock_config.direction == GD32_HAL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    assert(mock_config.memory_increment == 1U);
    assert(mock_config.priority == 2U);
    assert(hdma.StreamBaseAddress == GD32_HAL_DMA0_ADDRESS);
    assert(hdma.StreamIndex == 3U);
    assert(hdma.State == HAL_DMA_STATE_READY);

    hdma.Init.Channel = DMA_CHANNEL_4;
    assert(HAL_DMA_Init(&hdma) == HAL_ERROR);
    assert((hdma.ErrorCode & HAL_DMA_ERROR_REQUEST) != 0U);
    assert(mock_last_port_error == GD32_HAL_PORT_ERROR_DMA_REQUEST_MISMATCH);

    hdma = mock_handle(GD32_DMA0_CHANNEL3,
                       GD32_DMA_REQUEST_USART0_TX,
                       DMA_MEMORY_TO_PERIPH);
    hdma.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    assert(HAL_DMA_Init(&hdma) == HAL_ERROR);
    assert((hdma.ErrorCode & HAL_DMA_ERROR_NOT_SUPPORTED) != 0U);

    hdma = mock_handle(GD32_DMA0_CHANNEL3,
                       GD32_DMA_REQUEST_USART0_TX,
                       DMA_MEMORY_TO_PERIPH);
    hdma.Init.Mode = DMA_PFCTRL;
    assert(HAL_DMA_Init(&hdma) == HAL_ERROR);
    assert((hdma.ErrorCode & HAL_DMA_ERROR_NOT_SUPPORTED) != 0U);

    hdma = mock_handle(GD32_DMA0_CHANNEL6,
                       GD32_DMA_REQUEST_MEMORY,
                       DMA_MEMORY_TO_MEMORY);
    hdma.Init.Mode = DMA_CIRCULAR;
    assert(HAL_DMA_Init(&hdma) == HAL_ERROR);
    assert((hdma.ErrorCode & HAL_DMA_ERROR_NOT_SUPPORTED) != 0U);

    hdma = mock_handle((DMA_Stream_TypeDef *)(uintptr_t)0x12345678U,
                       GD32_DMA_REQUEST_MEMORY,
                       DMA_MEMORY_TO_MEMORY);
    assert(HAL_DMA_Init(&hdma) == HAL_ERROR);
    assert(mock_last_port_error == GD32_HAL_PORT_ERROR_INVALID_INSTANCE);
}

static void test_polling(void)
{
    DMA_HandleTypeDef hdma;

    mock_reset();
    hdma = mock_handle(GD32_DMA0_CHANNEL3,
                       GD32_DMA_REQUEST_USART0_TX,
                       DMA_MEMORY_TO_PERIPH);
    assert(HAL_DMA_Init(&hdma) == HAL_OK);
    assert(HAL_DMA_Start(&hdma, 0x20000000U, 0x40013804U, 0U) == HAL_ERROR);
    assert(HAL_DMA_Start(&hdma, 0x20000000U, 0x40013804U, 8U) == HAL_OK);
    assert(hdma.State == HAL_DMA_STATE_BUSY);
    assert(hdma.Lock == HAL_LOCKED);
    assert(mock_source == 0x20000000U);
    assert(mock_destination == 0x40013804U);
    assert(mock_length == 8U);
    assert(HAL_DMA_Start(&hdma, 0x20000000U, 0x40013804U, 1U) == HAL_BUSY);

    mock_flags = GD32_HAL_DMA_FLAG_HALF;
    assert(HAL_DMA_PollForTransfer(&hdma, HAL_DMA_HALF_TRANSFER, 10U) == HAL_OK);
    assert(hdma.State == HAL_DMA_STATE_BUSY);
    assert(hdma.Lock == HAL_LOCKED);

    mock_flags = GD32_HAL_DMA_FLAG_FULL;
    assert(HAL_DMA_PollForTransfer(&hdma, HAL_DMA_FULL_TRANSFER, 10U) == HAL_OK);
    assert(hdma.State == HAL_DMA_STATE_READY);
    assert(hdma.Lock == HAL_UNLOCKED);
    assert(mock_enabled == 0);

    assert(HAL_DMA_Start(&hdma, 0x20000000U, 0x40013804U, 2U) == HAL_OK);
    mock_tick_auto_increment = 1;
    assert(HAL_DMA_PollForTransfer(&hdma, HAL_DMA_FULL_TRANSFER, 2U) == HAL_TIMEOUT);
    assert(hdma.ErrorCode == HAL_DMA_ERROR_TIMEOUT);
    assert(hdma.State == HAL_DMA_STATE_READY);
    mock_tick_auto_increment = 0;

    assert(HAL_DMA_Start(&hdma, 0x20000000U, 0x40013804U, 2U) == HAL_OK);
    mock_flags = GD32_HAL_DMA_FLAG_ERROR;
    assert(HAL_DMA_PollForTransfer(&hdma, HAL_DMA_FULL_TRANSFER, 10U) == HAL_ERROR);
    assert((hdma.ErrorCode & HAL_DMA_ERROR_TE) != 0U);
    assert(hdma.State == HAL_DMA_STATE_READY);

    assert(HAL_DMA_Start(&hdma, 0x20000000U, 0x40013804U, 2U) == HAL_OK);
    mock_disable_stuck = 1;
    mock_tick_auto_increment = 1;
    assert(HAL_DMA_Abort(&hdma) == HAL_TIMEOUT);
    assert(hdma.State == HAL_DMA_STATE_TIMEOUT);
    assert(hdma.ErrorCode == HAL_DMA_ERROR_TIMEOUT);
    mock_tick_auto_increment = 0;
    mock_disable_stuck = 0;
    assert(HAL_DMA_DeInit(&hdma) == HAL_OK);
}

static void test_interrupt_and_circular(void)
{
    DMA_HandleTypeDef hdma;

    mock_reset();
    hdma = mock_handle(GD32_DMA1_CHANNEL4,
                       GD32_DMA_REQUEST_UART3_TX,
                       DMA_MEMORY_TO_PERIPH);
    assert(HAL_DMA_Init(&hdma) == HAL_OK);
    assert(HAL_DMA_RegisterCallback(&hdma,
                                    HAL_DMA_XFER_CPLT_CB_ID,
                                    mock_full_callback) == HAL_OK);
    assert(HAL_DMA_RegisterCallback(&hdma,
                                    HAL_DMA_XFER_HALFCPLT_CB_ID,
                                    mock_half_callback) == HAL_OK);
    assert(HAL_DMA_RegisterCallback(&hdma,
                                    HAL_DMA_XFER_ERROR_CB_ID,
                                    mock_error_callback) == HAL_OK);
    assert(HAL_DMA_RegisterCallback(&hdma,
                                    HAL_DMA_XFER_ABORT_CB_ID,
                                    mock_abort_callback) == HAL_OK);

    assert(HAL_DMA_Start_IT(&hdma, 0x20000100U, 0x40004C04U, 16U) == HAL_OK);
    assert((mock_interrupts & (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE)) ==
           (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE));
    mock_flags = GD32_HAL_DMA_FLAG_HALF;
    HAL_DMA_IRQHandler(&hdma);
    assert(mock_half_callback_count == 1U);
    assert(hdma.State == HAL_DMA_STATE_BUSY);
    mock_flags = GD32_HAL_DMA_FLAG_FULL;
    HAL_DMA_IRQHandler(&hdma);
    assert(mock_full_callback_count == 1U);
    assert(hdma.State == HAL_DMA_STATE_READY);
    assert(mock_event_count == 2U);
    assert(mock_events[0] == MOCK_EVENT_HALF);
    assert(mock_events[1] == MOCK_EVENT_FULL);

    assert(HAL_DMA_Start_IT(&hdma, 0x20000100U, 0x40004C04U, 4U) == HAL_OK);
    mock_flags = GD32_HAL_DMA_FLAG_ERROR;
    HAL_DMA_IRQHandler(&hdma);
    assert(mock_error_callback_count == 1U);
    assert(hdma.State == HAL_DMA_STATE_READY);

    hdma.Init.Mode = DMA_CIRCULAR;
    assert(HAL_DMA_Init(&hdma) == HAL_OK);
    assert(HAL_DMA_Start_IT(&hdma, 0x20000100U, 0x40004C04U, 4U) == HAL_OK);
    mock_flags = GD32_HAL_DMA_FLAG_FULL;
    HAL_DMA_IRQHandler(&hdma);
    mock_flags = GD32_HAL_DMA_FLAG_FULL;
    HAL_DMA_IRQHandler(&hdma);
    assert(mock_full_callback_count == 3U);
    assert(hdma.State == HAL_DMA_STATE_BUSY);
    assert(HAL_DMA_Abort_IT(&hdma) == HAL_OK);
    assert(mock_abort_callback_count == 1U);
    assert(hdma.State == HAL_DMA_STATE_READY);
}

static void test_directions_conflict_and_macros(void)
{
    typedef struct
    {
        DMA_HandleTypeDef *hdma;
    } ParentType;
    DMA_HandleTypeDef first;
    DMA_HandleTypeDef second;
    DMA_HandleTypeDef memory;
    ParentType parent = {0};

    mock_reset();
    first = mock_handle(GD32_DMA0_CHANNEL0,
                        GD32_DMA_REQUEST_ADC0,
                        DMA_PERIPH_TO_MEMORY);
    first.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    first.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    assert(HAL_DMA_Init(&first) == HAL_OK);
    assert(mock_config.direction == GD32_HAL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    assert(mock_config.periph_width == 2U);
    assert(mock_config.memory_width == 2U);

    second = first;
    second.Lock = HAL_UNLOCKED;
    second.State = HAL_DMA_STATE_RESET;
    second.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    second.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    assert(HAL_DMA_Init(&second) == HAL_OK);
    assert(HAL_DMA_Start(&first, 0x4001244DU, 0x20000200U, 4U) == HAL_ERROR);
    assert(first.State == HAL_DMA_STATE_READY);
    assert(HAL_DMA_Start(&first, 0x4001244CU, 0x20000200U, 4U) == HAL_OK);
    assert(mock_config.periph_width == 2U);
    assert(mock_config.memory_width == 2U);
    assert(HAL_DMA_Start(&second, 0x4001244CU, 0x20000300U, 4U) == HAL_BUSY);
    assert((second.ErrorCode & HAL_DMA_ERROR_CHANNEL_CONFLICT) != 0U);
    assert(HAL_DMA_Abort(&first) == HAL_OK);

    memory = mock_handle(GD32_DMA0_CHANNEL6,
                         GD32_DMA_REQUEST_MEMORY,
                         DMA_MEMORY_TO_MEMORY);
    memory.Init.PeriphInc = DMA_PINC_ENABLE;
    assert(HAL_DMA_Init(&memory) == HAL_OK);
    assert(mock_config.direction == GD32_HAL_DMA_DIRECTION_MEMORY_TO_MEMORY);
    assert(HAL_DMA_Start(&memory, 0x20001000U, 0x20002000U, 32U) == HAL_OK);
    assert(mock_direction == GD32_HAL_DMA_DIRECTION_MEMORY_TO_MEMORY);
    assert(HAL_DMA_Abort(&memory) == HAL_OK);

    __HAL_DMA_SET_COUNTER(&memory, 12U);
    assert(__HAL_DMA_GET_COUNTER(&memory) == 12U);
    mock_flags = GD32_HAL_DMA_FLAG_FULL;
    assert(__HAL_DMA_GET_FLAG(&memory, __HAL_DMA_GET_TC_FLAG_INDEX(&memory)) != 0U);
    __HAL_DMA_CLEAR_FLAG(&memory, GD32_HAL_DMA_FLAG_FULL);
    assert(mock_flags == 0U);
    __HAL_DMA_ENABLE_IT(&memory, DMA_IT_FE);
    assert(mock_last_port_error == GD32_HAL_PORT_ERROR_DMA_UNSUPPORTED_FLAG);

    __HAL_LINKDMA(&parent, hdma, memory);
    assert(parent.hdma == &memory);
    assert(memory.Parent == &parent);
}

static void test_deinit_and_callback_guards(void)
{
    DMA_HandleTypeDef hdma;

    mock_reset();
    hdma = mock_handle(GD32_DMA0_CHANNEL5,
                       GD32_DMA_REQUEST_USART1_RX,
                       DMA_PERIPH_TO_MEMORY);
    assert(HAL_DMA_Init(&hdma) == HAL_OK);
    assert(HAL_DMA_RegisterCallback(&hdma,
                                    HAL_DMA_XFER_M1CPLT_CB_ID,
                                    mock_full_callback) == HAL_ERROR);
    assert((hdma.ErrorCode & HAL_DMA_ERROR_NOT_SUPPORTED) != 0U);
    assert(HAL_DMA_CleanCallbacks(&hdma) == HAL_OK);
    assert(HAL_DMA_DeInit(&hdma) == HAL_OK);
    assert(mock_deinit_count == 1U);
    assert(hdma.State == HAL_DMA_STATE_RESET);
    assert(hdma.XferCpltCallback == NULL);
}

int main(void)
{
    test_init_and_guards();
    test_polling();
    test_interrupt_and_circular();
    test_directions_conflict_and_macros();
    test_deinit_and_callback_guards();
    puts("DMA polling, interrupt and conflict host tests: PASS");
    return 0;
}
