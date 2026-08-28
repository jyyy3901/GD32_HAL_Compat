#include "stm32f4xx_hal.h"
#include "gd32_hal_port.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static GD32_HAL_UARTConfig mock_config;
static uint32_t mock_configure_count;
static uint32_t mock_deinit_count;
static uint32_t mock_flags;
static uint32_t mock_error_flags;
static uint32_t mock_interrupts;
static uint16_t mock_tx_data[32];
static uint32_t mock_tx_count;
static uint16_t mock_rx_data[32];
static uint32_t mock_rx_count;
static uint32_t mock_rx_position;
static uint32_t mock_clear_error_count;
static int mock_dma_tx_active;
static int mock_dma_rx_active;
static uint32_t mock_tick;
static int mock_tick_auto_increment;
static GD32_HAL_PortError mock_last_port_error;
static uint32_t mock_msp_init_count;
static uint32_t mock_msp_deinit_count;
static uint32_t mock_tx_callback_count;
static uint32_t mock_tx_half_callback_count;
static uint32_t mock_rx_callback_count;
static uint32_t mock_rx_half_callback_count;
static uint32_t mock_error_callback_count;
static uint32_t mock_error_callback_last_code;
static uint32_t mock_abort_callback_count;
static uint32_t mock_abort_tx_callback_count;
static uint32_t mock_abort_rx_callback_count;
static uint8_t mock_callback_events[16];
static uint32_t mock_callback_event_count;
static uint32_t mock_dma_start_count;
static uint32_t mock_dma_abort_count;
static uint32_t mock_dma_source;
static uint32_t mock_dma_destination;
static uint32_t mock_dma_length;

#define MOCK_EVENT_TX    1U
#define MOCK_EVENT_RX    2U
#define MOCK_EVENT_ERROR 3U
#define MOCK_EVENT_ABORT 4U

static void mock_reset(void)
{
    memset(&mock_config, 0, sizeof(mock_config));
    mock_configure_count = 0U;
    mock_deinit_count = 0U;
    mock_flags = GD32_HAL_UART_FLAG_TX_EMPTY |
                 GD32_HAL_UART_FLAG_TX_COMPLETE;
    mock_error_flags = 0U;
    mock_interrupts = 0U;
    mock_tx_count = 0U;
    mock_rx_count = 0U;
    mock_rx_position = 0U;
    mock_clear_error_count = 0U;
    mock_dma_tx_active = 0;
    mock_dma_rx_active = 0;
    mock_tick = 0U;
    mock_tick_auto_increment = 0;
    mock_last_port_error = GD32_HAL_PORT_ERROR_NONE;
    mock_msp_init_count = 0U;
    mock_msp_deinit_count = 0U;
    mock_tx_callback_count = 0U;
    mock_tx_half_callback_count = 0U;
    mock_rx_callback_count = 0U;
    mock_rx_half_callback_count = 0U;
    mock_error_callback_count = 0U;
    mock_error_callback_last_code = HAL_UART_ERROR_NONE;
    mock_abort_callback_count = 0U;
    mock_abort_tx_callback_count = 0U;
    mock_abort_rx_callback_count = 0U;
    mock_callback_event_count = 0U;
    mock_dma_start_count = 0U;
    mock_dma_abort_count = 0U;
    mock_dma_source = 0U;
    mock_dma_destination = 0U;
    mock_dma_length = 0U;
}

static void mock_feed_rx(const uint16_t *data, uint32_t count)
{
    uint32_t index;

    assert(count <= (sizeof(mock_rx_data) / sizeof(mock_rx_data[0])));
    for (index = 0U; index < count; ++index)
    {
        mock_rx_data[index] = data[index];
    }
    mock_rx_count = count;
    mock_rx_position = 0U;
}

static UART_HandleTypeDef mock_handle(USART_TypeDef *instance)
{
    UART_HandleTypeDef huart;

    memset(&huart, 0, sizeof(huart));
    huart.Instance = instance;
    huart.Init.BaudRate = 115200U;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    huart.gState = HAL_UART_STATE_RESET;
    huart.RxState = HAL_UART_STATE_RESET;
    return huart;
}

static DMA_HandleTypeDef mock_dma_handle(DMA_Stream_TypeDef *instance,
                                         uint32_t request,
                                         uint32_t direction,
                                         uint32_t mode)
{
    DMA_HandleTypeDef hdma;
    const GD32_HAL_Resource *resource;

    memset(&hdma, 0, sizeof(hdma));
    hdma.Instance = instance;
    resource = GD32_HAL_ResolveInstance((uintptr_t)instance,
                                        GD32_HAL_RESOURCE_DMA);
    assert(resource != NULL);
    hdma.GD32_RESOURCE = resource;
    hdma.GD32_INSTANCE = resource->gd32_instance;
    hdma.gd32_dma_periph = resource->gd32_periph;
    hdma.gd32_dma_channel = resource->gd32_channel;
    hdma.GD32_IRQ_NUMBER = resource->gd32_irq;
    hdma.Init.Channel = request;
    hdma.Init.Direction = direction;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma.Init.Mode = mode;
    hdma.Init.Priority = DMA_PRIORITY_LOW;
    hdma.State = HAL_DMA_STATE_READY;
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

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    (void)huart;
    ++mock_msp_init_count;
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    (void)huart;
    ++mock_msp_deinit_count;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
    ++mock_tx_callback_count;
    mock_callback_events[mock_callback_event_count++] = MOCK_EVENT_TX;
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
    ++mock_tx_half_callback_count;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
    ++mock_rx_callback_count;
    mock_callback_events[mock_callback_event_count++] = MOCK_EVENT_RX;
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
    ++mock_rx_half_callback_count;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    ++mock_error_callback_count;
    mock_error_callback_last_code = huart->ErrorCode;
    mock_callback_events[mock_callback_event_count++] = MOCK_EVENT_ERROR;
}

void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
    ++mock_abort_callback_count;
    mock_callback_events[mock_callback_event_count++] = MOCK_EVENT_ABORT;
}

void HAL_UART_AbortTransmitCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
    ++mock_abort_tx_callback_count;
}

void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
    ++mock_abort_rx_callback_count;
}

int GD32_HAL_UART_IsInstance(uint32_t uart_address)
{
    return (uart_address == GD32_HAL_USART0_ADDRESS) ||
           (uart_address == GD32_HAL_USART1_ADDRESS) ||
           (uart_address == GD32_HAL_USART2_ADDRESS) ||
           (uart_address == GD32_HAL_UART3_ADDRESS) ||
           (uart_address == GD32_HAL_UART4_ADDRESS);
}

int GD32_HAL_UART_IsFlowControlCapable(uint32_t uart_address)
{
    return (uart_address == GD32_HAL_USART0_ADDRESS) ||
           (uart_address == GD32_HAL_USART1_ADDRESS) ||
           (uart_address == GD32_HAL_USART2_ADDRESS);
}

int GD32_HAL_UART_IsDMACapable(uint32_t uart_address)
{
    return (uart_address == GD32_HAL_USART0_ADDRESS) ||
           (uart_address == GD32_HAL_USART1_ADDRESS) ||
           (uart_address == GD32_HAL_USART2_ADDRESS) ||
           (uart_address == GD32_HAL_UART3_ADDRESS);
}

int GD32_HAL_UART_IsDMAChannelValid(uint32_t uart_address,
                                    int transmit,
                                    uint32_t channel_address,
                                    uint32_t request)
{
    if (uart_address == GD32_HAL_USART0_ADDRESS)
    {
        return (transmit != 0) ?
               ((channel_address == GD32_HAL_DMA0_CHANNEL3_ADDRESS) &&
                (request == GD32_DMA_REQUEST_USART0_TX)) :
               ((channel_address == GD32_HAL_DMA0_CHANNEL4_ADDRESS) &&
                (request == GD32_DMA_REQUEST_USART0_RX));
    }
    if (uart_address == GD32_HAL_USART1_ADDRESS)
    {
        return (transmit != 0) ?
               ((channel_address == GD32_HAL_DMA0_CHANNEL6_ADDRESS) &&
                (request == GD32_DMA_REQUEST_USART1_TX)) :
               ((channel_address == GD32_HAL_DMA0_CHANNEL5_ADDRESS) &&
                (request == GD32_DMA_REQUEST_USART1_RX));
    }
    if (uart_address == GD32_HAL_USART2_ADDRESS)
    {
        return (transmit != 0) ?
               ((channel_address == GD32_HAL_DMA0_CHANNEL1_ADDRESS) &&
                (request == GD32_DMA_REQUEST_USART2_TX)) :
               ((channel_address == GD32_HAL_DMA0_CHANNEL2_ADDRESS) &&
                (request == GD32_DMA_REQUEST_USART2_RX));
    }
    if (uart_address == GD32_HAL_UART3_ADDRESS)
    {
        return (transmit != 0) ?
               ((channel_address == GD32_HAL_DMA1_CHANNEL4_ADDRESS) &&
                (request == GD32_DMA_REQUEST_UART3_TX)) :
               ((channel_address == GD32_HAL_DMA1_CHANNEL2_ADDRESS) &&
                (request == GD32_DMA_REQUEST_UART3_RX));
    }
    return 0;
}

uint32_t GD32_HAL_UART_GetDataAddress(uint32_t uart_address)
{
    assert(GD32_HAL_UART_IsInstance(uart_address));
    return uart_address + 0x04U;
}

int GD32_HAL_UART_IsDMARequestActive(uint32_t uart_address, int transmit)
{
    (void)uart_address;
    return (transmit != 0) ? mock_dma_tx_active : mock_dma_rx_active;
}

void GD32_HAL_UART_EnableDMARequest(uint32_t uart_address, int transmit)
{
    assert(GD32_HAL_UART_IsDMACapable(uart_address));
    if (transmit != 0)
    {
        mock_dma_tx_active = 1;
    }
    else
    {
        mock_dma_rx_active = 1;
    }
}

int GD32_HAL_UART_Configure(uint32_t uart_address, const GD32_HAL_UARTConfig *config)
{
    assert(GD32_HAL_UART_IsInstance(uart_address));
    mock_config = *config;
    ++mock_configure_count;
    return 0;
}

void GD32_HAL_UART_DeInit(uint32_t uart_address)
{
    assert(GD32_HAL_UART_IsInstance(uart_address));
    ++mock_deinit_count;
}

void GD32_HAL_UART_Enable(uint32_t uart_address)
{
    assert(GD32_HAL_UART_IsInstance(uart_address));
}

void GD32_HAL_UART_Disable(uint32_t uart_address)
{
    assert(GD32_HAL_UART_IsInstance(uart_address));
}

uint32_t GD32_HAL_UART_GetFlag(uint32_t uart_address, GD32_HAL_UARTFlag flag)
{
    assert(GD32_HAL_UART_IsInstance(uart_address));
    if ((flag == GD32_HAL_UART_FLAG_RX_NOT_EMPTY) &&
        (mock_rx_position < mock_rx_count))
    {
        return (uint32_t)flag;
    }
    return mock_flags & (uint32_t)flag;
}

void GD32_HAL_UART_ClearFlag(uint32_t uart_address, GD32_HAL_UARTFlag flag)
{
    assert(GD32_HAL_UART_IsInstance(uart_address));
    mock_flags &= ~(uint32_t)flag;
}

uint32_t GD32_HAL_UART_GetErrorFlags(uint32_t uart_address)
{
    assert(GD32_HAL_UART_IsInstance(uart_address));
    return mock_error_flags;
}

void GD32_HAL_UART_ClearErrors(uint32_t uart_address)
{
    assert(GD32_HAL_UART_IsInstance(uart_address));
    mock_error_flags = 0U;
    ++mock_clear_error_count;
}

void GD32_HAL_UART_WriteData(uint32_t uart_address, uint16_t data)
{
    assert(GD32_HAL_UART_IsInstance(uart_address));
    assert(mock_tx_count < (sizeof(mock_tx_data) / sizeof(mock_tx_data[0])));
    mock_tx_data[mock_tx_count++] = data;
}

uint16_t GD32_HAL_UART_ReadData(uint32_t uart_address)
{
    uint16_t data;

    assert(GD32_HAL_UART_IsInstance(uart_address));
    assert(mock_rx_position < mock_rx_count);
    data = mock_rx_data[mock_rx_position++];
    mock_error_flags = 0U;
    return data;
}

void GD32_HAL_UART_SetInterrupt(uint32_t uart_address,
                               GD32_HAL_UARTInterrupt interrupt,
                               int enable)
{
    const uint32_t mask = 1UL << (uint32_t)interrupt;
    assert(GD32_HAL_UART_IsInstance(uart_address));
    if (enable != 0)
    {
        mock_interrupts |= mask;
    }
    else
    {
        mock_interrupts &= ~mask;
    }
}

int GD32_HAL_UART_IsInterruptEnabled(uint32_t uart_address,
                                    GD32_HAL_UARTInterrupt interrupt)
{
    assert(GD32_HAL_UART_IsInstance(uart_address));
    return (mock_interrupts & (1UL << (uint32_t)interrupt)) != 0U;
}

void GD32_HAL_UART_DisableDMARequest(uint32_t uart_address, int transmit)
{
    assert(GD32_HAL_UART_IsInstance(uart_address));
    if (transmit != 0)
    {
        mock_dma_tx_active = 0;
    }
    else
    {
        mock_dma_rx_active = 0;
    }
}

HAL_StatusTypeDef HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma,
                                   uint32_t SrcAddress,
                                   uint32_t DstAddress,
                                   uint32_t DataLength)
{
    if ((hdma == NULL) || (hdma->State != HAL_DMA_STATE_READY))
    {
        return (hdma == NULL) ? HAL_ERROR : HAL_BUSY;
    }
    hdma->State = HAL_DMA_STATE_BUSY;
    mock_dma_source = SrcAddress;
    mock_dma_destination = DstAddress;
    mock_dma_length = DataLength;
    ++mock_dma_start_count;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_DMA_Abort(DMA_HandleTypeDef *hdma)
{
    if ((hdma == NULL) || (hdma->State != HAL_DMA_STATE_BUSY))
    {
        return HAL_ERROR;
    }
    hdma->State = HAL_DMA_STATE_READY;
    ++mock_dma_abort_count;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_DMA_Abort_IT(DMA_HandleTypeDef *hdma)
{
    void (*callback)(DMA_HandleTypeDef *hdma);

    if ((hdma == NULL) || (hdma->State != HAL_DMA_STATE_BUSY))
    {
        return HAL_ERROR;
    }
    callback = hdma->XferAbortCallback;
    hdma->State = HAL_DMA_STATE_READY;
    ++mock_dma_abort_count;
    if (callback != NULL)
    {
        callback(hdma);
    }
    return HAL_OK;
}

HAL_DMA_StateTypeDef HAL_DMA_GetState(const DMA_HandleTypeDef *hdma)
{
    return (hdma == NULL) ? HAL_DMA_STATE_RESET : hdma->State;
}

static void test_init_and_guards(void)
{
    UART_HandleTypeDef huart;

    mock_reset();
    assert((int)USART1_IRQn == STM32_HAL_IRQ_USART1);
    assert((int)USART2_IRQn == STM32_HAL_IRQ_USART2);
    assert((int)USART6_IRQn == STM32_HAL_IRQ_USART6);
    assert((uintptr_t)USART1 == STM32_UART_INSTANCE_1);
    assert((uintptr_t)USART6 == STM32_UART_INSTANCE_6);
    assert((uint32_t)(uintptr_t)USART1 != GD32_HAL_USART0_ADDRESS);

    huart = mock_handle(USART1);
    assert(HAL_UART_Init(&huart) == HAL_OK);
    assert(huart.GD32_INSTANCE == GD32_HAL_USART0_ADDRESS);
    assert(huart.GD32_IRQ_NUMBER == STM32_HAL_IRQ_USART1);
    assert(huart.GD32_RESOURCE != NULL);
    assert(mock_configure_count == 1U);
    assert(mock_config.baud_rate == 115200U);
    assert(mock_config.word_length == 8U);
    assert(mock_config.mode == (GD32_HAL_UART_MODE_TX | GD32_HAL_UART_MODE_RX));
    assert(huart.gState == HAL_UART_STATE_READY);
    assert(huart.RxState == HAL_UART_STATE_READY);
    assert(mock_msp_init_count == 1U);

    huart.Init.OverSampling = UART_OVERSAMPLING_8;
    assert(HAL_UART_Init(&huart) == HAL_ERROR);
    assert(mock_last_port_error == GD32_HAL_PORT_ERROR_UART_OVERSAMPLING_UNSUPPORTED);

    huart = mock_handle((USART_TypeDef *)(uintptr_t)
                        STM32_HAL_INSTANCE_TOKEN(0x02U, 0x04U));
    assert(HAL_UART_Init(&huart) == HAL_ERROR);
    assert(mock_last_port_error == GD32_HAL_PORT_ERROR_INVALID_INSTANCE);
}

static void test_polling(void)
{
    UART_HandleTypeDef huart;
    const uint8_t tx[] = {0x12U, 0x34U, 0x56U};
    const uint16_t rx_source[] = {0xA1U, 0xB2U, 0xC3U};
    uint8_t rx[3] = {0U};

    mock_reset();
    huart = mock_handle(USART2);
    assert(HAL_UART_Init(&huart) == HAL_OK);
    huart.pRxBuffPtr = (uint8_t *)(uintptr_t)0x1234U;
    assert(HAL_UART_Transmit(&huart, tx, 0U, 10U) == HAL_ERROR);
    assert(HAL_UART_Transmit(&huart, tx, 3U, 10U) == HAL_OK);
    assert(mock_tx_count == 3U);
    assert(mock_tx_data[0] == 0x12U);
    assert(mock_tx_data[2] == 0x56U);
    assert(huart.gState == HAL_UART_STATE_READY);

    mock_feed_rx(rx_source, 3U);
    assert(HAL_UART_Receive(&huart, rx, 3U, 10U) == HAL_OK);
    assert(rx[0] == 0xA1U);
    assert(rx[1] == 0xB2U);
    assert(rx[2] == 0xC3U);
    assert(huart.RxState == HAL_UART_STATE_READY);
    assert(huart.pRxBuffPtr == (uint8_t *)(uintptr_t)0x1234U);

    mock_flags = GD32_HAL_UART_FLAG_TX_COMPLETE;
    mock_tick_auto_increment = 1;
    assert(HAL_UART_Transmit(&huart, tx, 1U, 2U) == HAL_TIMEOUT);
    assert(huart.gState == HAL_UART_STATE_READY);

    mock_flags = GD32_HAL_UART_FLAG_TX_EMPTY |
                 GD32_HAL_UART_FLAG_TX_COMPLETE;
    mock_tick_auto_increment = 0;
    mock_feed_rx(rx_source, 1U);
    mock_error_flags = GD32_HAL_UART_ERROR_FE;
    assert(HAL_UART_Receive(&huart, rx, 1U, 10U) == HAL_ERROR);
    assert((huart.ErrorCode & HAL_UART_ERROR_FE) != 0U);
    assert(huart.RxState == HAL_UART_STATE_READY);
}

static void test_nine_bit_data_path(void)
{
    UART_HandleTypeDef huart;
    const uint16_t tx[] = {0x01A5U, 0x005AU};
    const uint16_t rx_source[] = {0x0101U, 0x00FEU};
    uint16_t rx[2] = {0U};

    mock_reset();
    huart = mock_handle(USART1);
    huart.Init.WordLength = UART_WORDLENGTH_9B;
    assert(HAL_UART_Init(&huart) == HAL_OK);
    assert(HAL_UART_Transmit(&huart,
                             (const uint8_t *)(const void *)tx,
                             2U,
                             10U) == HAL_OK);
    assert(mock_tx_data[0] == 0x01A5U);
    assert(mock_tx_data[1] == 0x005AU);

    mock_feed_rx(rx_source, 2U);
    assert(HAL_UART_Receive(&huart, (uint8_t *)(void *)rx, 2U, 10U) == HAL_OK);
    assert(rx[0] == 0x0101U);
    assert(rx[1] == 0x00FEU);
}

static void test_parity_data_mask(void)
{
    UART_HandleTypeDef huart;
    const uint16_t rx_source[] = {0x00FFU};
    uint8_t rx = 0U;

    mock_reset();
    huart = mock_handle(USART1);
    huart.Init.Parity = UART_PARITY_EVEN;
    assert(HAL_UART_Init(&huart) == HAL_OK);
    mock_feed_rx(rx_source, 1U);
    assert(HAL_UART_Receive(&huart, &rx, 1U, 10U) == HAL_OK);
    assert(rx == 0x7FU);
}

static void test_interrupt_transfers(void)
{
    UART_HandleTypeDef huart;
    const uint8_t tx[] = {0x41U, 0x42U};
    const uint16_t rx_source[] = {0x61U, 0x62U};
    uint8_t rx[2] = {0U};

    mock_reset();
    huart = mock_handle(USART6);
    assert(HAL_UART_Init(&huart) == HAL_OK);

    assert(HAL_UART_Transmit_IT(&huart, tx, 2U) == HAL_OK);
    assert(HAL_UART_Transmit_IT(&huart, tx, 1U) == HAL_BUSY);
    assert(__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_TXE));
    HAL_UART_IRQHandler(&huart);
    assert(huart.TxXferCount == 1U);
    HAL_UART_IRQHandler(&huart);
    assert(huart.TxXferCount == 0U);
    assert(!__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_TXE));
    assert(__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_TC));
    HAL_UART_IRQHandler(&huart);
    assert(huart.gState == HAL_UART_STATE_READY);
    assert(!__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_TC));
    assert(mock_tx_count == 2U);
    assert(mock_tx_callback_count == 1U);

    mock_feed_rx(rx_source, 2U);
    assert(HAL_UART_Receive_IT(&huart, rx, 2U) == HAL_OK);
    assert(HAL_UART_Receive_IT(&huart, rx, 1U) == HAL_BUSY);
    assert(__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_RXNE));
    HAL_UART_IRQHandler(&huart);
    HAL_UART_IRQHandler(&huart);
    assert(rx[0] == 0x61U);
    assert(rx[1] == 0x62U);
    assert(huart.RxState == HAL_UART_STATE_READY);
    assert(!__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_RXNE));
    assert(mock_rx_callback_count == 1U);
}

static void test_interrupt_full_duplex(void)
{
    UART_HandleTypeDef huart;
    const uint8_t tx = 0x35U;
    const uint16_t rx_source[] = {0x53U};
    uint8_t rx = 0U;

    mock_reset();
    huart = mock_handle(USART1);
    assert(HAL_UART_Init(&huart) == HAL_OK);
    mock_feed_rx(rx_source, 1U);

    assert(HAL_UART_Transmit_IT(&huart, &tx, 1U) == HAL_OK);
    assert(HAL_UART_Receive_IT(&huart, &rx, 1U) == HAL_OK);
    assert(huart.gState == HAL_UART_STATE_BUSY_TX);
    assert(huart.RxState == HAL_UART_STATE_BUSY_RX);

    HAL_UART_IRQHandler(&huart);
    assert(rx == 0x53U);
    assert(huart.RxState == HAL_UART_STATE_READY);
    assert(huart.gState == HAL_UART_STATE_BUSY_TX);

    HAL_UART_IRQHandler(&huart);
    HAL_UART_IRQHandler(&huart);
    assert(mock_tx_data[0] == 0x35U);
    assert(huart.gState == HAL_UART_STATE_READY);
    assert(mock_tx_callback_count == 1U);
    assert(mock_rx_callback_count == 1U);
}

static void test_interrupt_error_mapping(void)
{
    static const uint32_t port_errors[] = {
        GD32_HAL_UART_ERROR_PE,
        GD32_HAL_UART_ERROR_NE,
        GD32_HAL_UART_ERROR_FE
    };
    static const uint32_t hal_errors[] = {
        HAL_UART_ERROR_PE,
        HAL_UART_ERROR_NE,
        HAL_UART_ERROR_FE
    };
    UART_HandleTypeDef huart;
    uint8_t rx[2] = {0U};
    uint32_t index;

    mock_reset();
    huart = mock_handle(USART2);
    huart.Init.Parity = UART_PARITY_EVEN;
    assert(HAL_UART_Init(&huart) == HAL_OK);
    assert(HAL_UART_Receive_IT(&huart, rx, 2U) == HAL_OK);

    for (index = 0U; index < 3U; ++index)
    {
        mock_error_flags = port_errors[index];
        HAL_UART_IRQHandler(&huart);
        assert(mock_error_callback_last_code == hal_errors[index]);
        assert(huart.ErrorCode == HAL_UART_ERROR_NONE);
        assert(huart.RxState == HAL_UART_STATE_BUSY_RX);
    }

    assert(HAL_UART_AbortReceive(&huart) == HAL_OK);
}

static void test_interrupt_errors_and_abort(void)
{
    UART_HandleTypeDef huart;
    uint8_t rx[2] = {0U};

    mock_reset();
    huart = mock_handle(USART1);
    assert(HAL_UART_Init(&huart) == HAL_OK);
    {
        const uint16_t error_data[] = {0x55U};
        mock_feed_rx(error_data, 1U);
    }
    assert(HAL_UART_Receive_IT(&huart, rx, 1U) == HAL_OK);
    mock_callback_event_count = 0U;
    mock_error_flags = GD32_HAL_UART_ERROR_FE;
    HAL_UART_IRQHandler(&huart);
    assert(mock_callback_event_count == 2U);
    assert(mock_callback_events[0] == MOCK_EVENT_RX);
    assert(mock_callback_events[1] == MOCK_EVENT_ERROR);
    assert(mock_rx_callback_count == 1U);
    assert(mock_error_callback_count == 1U);
    assert(mock_error_callback_last_code == HAL_UART_ERROR_FE);
    assert(huart.ErrorCode == HAL_UART_ERROR_NONE);
    assert(huart.RxState == HAL_UART_STATE_READY);

    assert(HAL_UART_Receive_IT(&huart, rx, 2U) == HAL_OK);
    mock_error_flags = GD32_HAL_UART_ERROR_ORE;
    HAL_UART_IRQHandler(&huart);
    assert(mock_clear_error_count == 1U);
    assert(mock_error_callback_count == 2U);
    assert((huart.ErrorCode & HAL_UART_ERROR_ORE) != 0U);
    assert(huart.RxState == HAL_UART_STATE_READY);
    assert(!__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_RXNE));

    huart.gState = HAL_UART_STATE_BUSY_TX;
    huart.TxXferCount = 5U;
    assert(HAL_UART_AbortTransmit_IT(&huart) == HAL_OK);
    assert(huart.gState == HAL_UART_STATE_READY);
    assert(huart.TxXferCount == 0U);
    assert(mock_abort_tx_callback_count == 1U);

    huart.gState = HAL_UART_STATE_BUSY_TX;
    huart.TxXferCount = 4U;
    mock_dma_tx_active = 1;
    assert(HAL_UART_AbortTransmit(&huart) == HAL_ERROR);
    assert(huart.gState == HAL_UART_STATE_READY);
    assert((huart.ErrorCode & HAL_UART_ERROR_DMA) != 0U);
    assert(mock_dma_tx_active == 0);
    assert(mock_last_port_error == GD32_HAL_PORT_ERROR_UART_DMA_LINK_INVALID);
}

static void test_dma_normal_transfers(void)
{
    UART_HandleTypeDef huart;
    DMA_HandleTypeDef hdmatx;
    DMA_HandleTypeDef hdmarx;
    const uint8_t tx[] = {0x11U, 0x22U, 0x33U, 0x44U};
    uint8_t rx[4] = {0U};

    mock_reset();
    huart = mock_handle(USART1);
    assert(HAL_UART_Init(&huart) == HAL_OK);
    hdmatx = mock_dma_handle(GD32_DMA0_CHANNEL3,
                             GD32_DMA_REQUEST_USART0_TX,
                             DMA_MEMORY_TO_PERIPH,
                             DMA_NORMAL);
    hdmarx = mock_dma_handle(GD32_DMA0_CHANNEL4,
                             GD32_DMA_REQUEST_USART0_RX,
                             DMA_PERIPH_TO_MEMORY,
                             DMA_NORMAL);
    __HAL_LINKDMA(&huart, hdmatx, hdmatx);
    __HAL_LINKDMA(&huart, hdmarx, hdmarx);

    assert(HAL_UART_Transmit_DMA(&huart, tx, 4U) == HAL_OK);
    assert(HAL_UART_Transmit_DMA(&huart, tx, 4U) == HAL_BUSY);
    assert(mock_dma_start_count == 1U);
    assert(mock_dma_source == (uint32_t)(uintptr_t)tx);
    assert(mock_dma_destination == GD32_HAL_USART0_ADDRESS + 0x04U);
    assert(mock_dma_length == 4U);
    assert(mock_dma_tx_active != 0);
    assert(huart.gState == HAL_UART_STATE_BUSY_TX);

    hdmatx.XferHalfCpltCallback(&hdmatx);
    assert(mock_tx_half_callback_count == 1U);
    hdmatx.State = HAL_DMA_STATE_READY;
    hdmatx.XferCpltCallback(&hdmatx);
    assert(mock_dma_tx_active == 0);
    assert(huart.TxXferCount == 0U);
    assert(huart.gState == HAL_UART_STATE_BUSY_TX);
    assert(__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_TC));
    assert(mock_tx_callback_count == 0U);
    mock_flags |= GD32_HAL_UART_FLAG_TX_COMPLETE;
    HAL_UART_IRQHandler(&huart);
    assert(huart.gState == HAL_UART_STATE_READY);
    assert(mock_tx_callback_count == 1U);

    hdmatx.State = HAL_DMA_STATE_READY;
    assert(HAL_UART_Transmit_DMA(&huart, tx, 4U) == HAL_OK);
    hdmatx.State = HAL_DMA_STATE_READY;
    hdmatx.XferCpltCallback(&hdmatx);
    assert(__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_TC));
    assert(HAL_UART_DMAStop(&huart) == HAL_OK);
    assert(huart.gState == HAL_UART_STATE_READY);
    assert(!__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_TC));
    assert(mock_tx_callback_count == 1U);

    assert(HAL_UART_Receive_DMA(&huart, rx, 4U) == HAL_OK);
    assert(HAL_UART_Receive_DMA(&huart, rx, 4U) == HAL_BUSY);
    assert(mock_dma_start_count == 3U);
    assert(mock_dma_source == GD32_HAL_USART0_ADDRESS + 0x04U);
    assert(mock_dma_destination == (uint32_t)(uintptr_t)rx);
    assert(mock_dma_rx_active != 0);
    assert(huart.RxState == HAL_UART_STATE_BUSY_RX);
    assert(__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_ERR));

    hdmarx.XferHalfCpltCallback(&hdmarx);
    assert(mock_rx_half_callback_count == 1U);
    assert(huart.RxEventType == HAL_UART_RXEVENT_HT);
    hdmarx.State = HAL_DMA_STATE_READY;
    hdmarx.XferCpltCallback(&hdmarx);
    assert(mock_dma_rx_active == 0);
    assert(huart.RxXferCount == 0U);
    assert(huart.RxState == HAL_UART_STATE_READY);
    assert(huart.RxEventType == HAL_UART_RXEVENT_TC);
    assert(mock_rx_callback_count == 1U);
}

static void test_dma_circular_pause_resume_stop(void)
{
    UART_HandleTypeDef huart;
    DMA_HandleTypeDef hdmatx;
    DMA_HandleTypeDef hdmarx;
    uint8_t tx[8] = {0U};
    uint8_t rx[8] = {0U};

    mock_reset();
    huart = mock_handle(USART1);
    assert(HAL_UART_Init(&huart) == HAL_OK);
    hdmatx = mock_dma_handle(GD32_DMA0_CHANNEL3,
                             GD32_DMA_REQUEST_USART0_TX,
                             DMA_MEMORY_TO_PERIPH,
                             DMA_CIRCULAR);
    hdmarx = mock_dma_handle(GD32_DMA0_CHANNEL4,
                             GD32_DMA_REQUEST_USART0_RX,
                             DMA_PERIPH_TO_MEMORY,
                             DMA_CIRCULAR);
    __HAL_LINKDMA(&huart, hdmatx, hdmatx);
    __HAL_LINKDMA(&huart, hdmarx, hdmarx);

    assert(HAL_UART_Transmit_DMA(&huart, tx, 8U) == HAL_OK);
    assert(HAL_UART_Receive_DMA(&huart, rx, 8U) == HAL_OK);
    hdmatx.XferCpltCallback(&hdmatx);
    hdmarx.XferCpltCallback(&hdmarx);
    assert(mock_tx_callback_count == 1U);
    assert(mock_rx_callback_count == 1U);
    assert(huart.gState == HAL_UART_STATE_BUSY_TX);
    assert(huart.RxState == HAL_UART_STATE_BUSY_RX);
    assert(mock_dma_tx_active != 0);
    assert(mock_dma_rx_active != 0);

    assert(HAL_UART_DMAPause(&huart) == HAL_OK);
    assert(mock_dma_tx_active == 0);
    assert(mock_dma_rx_active == 0);
    assert(!__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_ERR));
    assert(HAL_UART_DMAResume(&huart) == HAL_OK);
    assert(mock_dma_tx_active != 0);
    assert(mock_dma_rx_active != 0);
    assert(__HAL_UART_GET_IT_SOURCE(&huart, UART_IT_ERR));

    assert(HAL_UART_DMAPause(&huart) == HAL_OK);
    assert(HAL_UART_DMAStop(&huart) == HAL_OK);
    assert(mock_dma_abort_count == 2U);
    assert(hdmatx.State == HAL_DMA_STATE_READY);
    assert(hdmarx.State == HAL_DMA_STATE_READY);
    assert(huart.gState == HAL_UART_STATE_READY);
    assert(huart.RxState == HAL_UART_STATE_READY);

    assert(HAL_UART_Transmit_DMA(&huart, tx, 8U) == HAL_OK);
    assert(HAL_UART_Receive_DMA(&huart, rx, 8U) == HAL_OK);
    assert(HAL_UART_DMAPause(&huart) == HAL_OK);
    assert(HAL_UART_Abort_IT(&huart) == HAL_OK);
    assert(mock_dma_abort_count == 4U);
    assert(mock_abort_callback_count == 1U);
    assert(huart.gState == HAL_UART_STATE_READY);
    assert(huart.RxState == HAL_UART_STATE_READY);

    assert(HAL_UART_Transmit_DMA(&huart, tx, 8U) == HAL_OK);
    assert(HAL_UART_Receive_DMA(&huart, rx, 8U) == HAL_OK);
    hdmarx.State = HAL_DMA_STATE_READY;
    hdmarx.XferErrorCallback(&hdmarx);
    assert(mock_dma_abort_count == 5U);
    assert(hdmatx.State == HAL_DMA_STATE_READY);
    assert(mock_dma_tx_active == 0);
    assert(mock_dma_rx_active == 0);
    assert(huart.gState == HAL_UART_STATE_READY);
    assert(huart.RxState == HAL_UART_STATE_READY);
    assert((huart.ErrorCode & HAL_UART_ERROR_DMA) != 0U);
}

static void test_dma_errors_abort_and_guards(void)
{
    UART_HandleTypeDef huart;
    DMA_HandleTypeDef hdmatx;
    DMA_HandleTypeDef hdmarx;
    uint8_t data[4] = {0U};

    mock_reset();
    huart = mock_handle(USART2);
    assert(HAL_UART_Init(&huart) == HAL_OK);
    hdmatx = mock_dma_handle(GD32_DMA0_CHANNEL6,
                             GD32_DMA_REQUEST_USART1_TX,
                             DMA_MEMORY_TO_PERIPH,
                             DMA_NORMAL);
    hdmarx = mock_dma_handle(GD32_DMA0_CHANNEL5,
                             GD32_DMA_REQUEST_USART1_RX,
                             DMA_PERIPH_TO_MEMORY,
                             DMA_NORMAL);
    __HAL_LINKDMA(&huart, hdmatx, hdmatx);
    __HAL_LINKDMA(&huart, hdmarx, hdmarx);

    assert(HAL_UART_Receive_DMA(&huart, data, 4U) == HAL_OK);
    mock_error_flags = GD32_HAL_UART_ERROR_FE;
    HAL_UART_IRQHandler(&huart);
    assert(mock_dma_abort_count == 1U);
    assert(mock_dma_rx_active == 0);
    assert(huart.RxState == HAL_UART_STATE_READY);
    assert(mock_error_callback_count == 1U);
    assert((mock_error_callback_last_code & HAL_UART_ERROR_FE) != 0U);

    hdmarx.State = HAL_DMA_STATE_READY;
    assert(HAL_UART_Receive_DMA(&huart, data, 4U) == HAL_OK);
    hdmarx.State = HAL_DMA_STATE_ERROR;
    hdmarx.XferErrorCallback(&hdmarx);
    assert(mock_dma_rx_active == 0);
    assert(huart.RxState == HAL_UART_STATE_READY);
    assert((huart.ErrorCode & HAL_UART_ERROR_DMA) != 0U);
    assert(mock_error_callback_count == 2U);

    huart.ErrorCode = HAL_UART_ERROR_NONE;
    hdmatx.State = HAL_DMA_STATE_READY;
    assert(HAL_UART_Transmit_DMA(&huart, data, 4U) == HAL_OK);
    assert(HAL_UART_AbortTransmit_IT(&huart) == HAL_OK);
    assert(mock_abort_tx_callback_count == 1U);
    assert(mock_dma_tx_active == 0);
    assert(huart.gState == HAL_UART_STATE_READY);

    huart = mock_handle(USART1);
    assert(HAL_UART_Init(&huart) == HAL_OK);
    hdmatx = mock_dma_handle(GD32_DMA0_CHANNEL4,
                             GD32_DMA_REQUEST_USART0_RX,
                             DMA_MEMORY_TO_PERIPH,
                             DMA_NORMAL);
    __HAL_LINKDMA(&huart, hdmatx, hdmatx);
    assert(HAL_UART_Transmit_DMA(&huart, data, 4U) == HAL_ERROR);
    assert(mock_last_port_error == GD32_HAL_PORT_ERROR_UART_DMA_CONFIG_MISMATCH);

    hdmatx = mock_dma_handle(GD32_DMA0_CHANNEL3,
                             GD32_DMA_REQUEST_USART0_TX,
                             DMA_MEMORY_TO_PERIPH,
                             DMA_NORMAL);
    huart.hdmatx = &hdmatx;
    assert(HAL_UART_Transmit_DMA(&huart, data, 4U) == HAL_ERROR);
    assert(mock_last_port_error == GD32_HAL_PORT_ERROR_UART_DMA_LINK_INVALID);
}

static void test_deinit(void)
{
    UART_HandleTypeDef huart;

    mock_reset();
    huart = mock_handle(USART6);
    assert(HAL_UART_Init(&huart) == HAL_OK);
    assert(HAL_UART_DeInit(&huart) == HAL_OK);
    assert(mock_deinit_count == 1U);
    assert(mock_msp_deinit_count == 1U);
    assert(huart.gState == HAL_UART_STATE_RESET);
    assert(huart.RxState == HAL_UART_STATE_RESET);
}

int main(void)
{
    test_init_and_guards();
    test_polling();
    test_nine_bit_data_path();
    test_parity_data_mask();
    test_interrupt_transfers();
    test_interrupt_full_duplex();
    test_interrupt_error_mapping();
    test_interrupt_errors_and_abort();
    test_dma_normal_transfers();
    test_dma_circular_pause_resume_stop();
    test_dma_errors_abort_and_guards();
    test_deinit();
    puts("UART polling, interrupt and DMA host tests: PASS");
    return 0;
}
