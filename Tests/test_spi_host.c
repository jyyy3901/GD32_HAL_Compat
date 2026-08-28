#include "stm32f4xx_hal.h"
#include "gd32_hal_port.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static GD32_HAL_SPIConfig mock_config;
static uint32_t mock_flags;
static uint32_t mock_errors;
static uint32_t mock_interrupts;
static uint16_t mock_tx[64];
static uint32_t mock_tx_count;
static uint16_t mock_rx[64];
static uint32_t mock_rx_count;
static uint32_t mock_rx_pos;
static uint32_t mock_config_count;
static uint32_t mock_msp_init;
static uint32_t mock_msp_deinit;
static uint32_t mock_tx_callback;
static uint32_t mock_rx_callback;
static uint32_t mock_txrx_callback;
static uint32_t mock_tx_half;
static uint32_t mock_rx_half;
static uint32_t mock_txrx_half;
static uint32_t mock_error_callback;
static uint32_t mock_dma_requests;
static uint32_t mock_dma_starts;
static uint32_t mock_dma_aborts;
static uint32_t mock_dma_sources[4];
static uint32_t mock_dma_destinations[4];
static uint32_t mock_dma_lengths[4];
static uint32_t mock_tick;
static int mock_tick_increment;
static GD32_HAL_PortError mock_port_error;

static void MockReset(void)
{
    memset(&mock_config, 0, sizeof(mock_config));
    mock_flags = GD32_HAL_SPI_FLAG_TX_EMPTY;
    mock_errors = 0U;
    mock_interrupts = 0U;
    mock_tx_count = 0U;
    mock_rx_count = 0U;
    mock_rx_pos = 0U;
    mock_config_count = 0U;
    mock_msp_init = 0U;
    mock_msp_deinit = 0U;
    mock_tx_callback = 0U;
    mock_rx_callback = 0U;
    mock_txrx_callback = 0U;
    mock_tx_half = 0U;
    mock_rx_half = 0U;
    mock_txrx_half = 0U;
    mock_error_callback = 0U;
    mock_dma_requests = 0U;
    mock_dma_starts = 0U;
    mock_dma_aborts = 0U;
    memset(mock_dma_sources, 0, sizeof(mock_dma_sources));
    memset(mock_dma_destinations, 0, sizeof(mock_dma_destinations));
    memset(mock_dma_lengths, 0, sizeof(mock_dma_lengths));
    mock_tick = 0U;
    mock_tick_increment = 0;
    mock_port_error = GD32_HAL_PORT_ERROR_NONE;
}

static SPI_HandleTypeDef MakeSPI(uint32_t data_size)
{
    SPI_HandleTypeDef hspi;
    memset(&hspi, 0, sizeof(hspi));
    hspi.Instance = SPI1;
    hspi.Init.Mode = SPI_MODE_MASTER;
    hspi.Init.Direction = SPI_DIRECTION_2LINES;
    hspi.Init.DataSize = data_size;
    hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi.Init.NSS = SPI_NSS_SOFT;
    hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi.Init.CRCPolynomial = 7U;
    hspi.State = HAL_SPI_STATE_RESET;
    return hspi;
}

static DMA_HandleTypeDef MakeDMA(SPI_HandleTypeDef *hspi,
                                 DMA_Stream_TypeDef *instance,
                                 uint32_t request,
                                 uint32_t direction,
                                 uint32_t data_size,
                                 uint32_t mode)
{
    DMA_HandleTypeDef hdma;
    memset(&hdma, 0, sizeof(hdma));
    hdma.Instance = instance;
    hdma.Init.Channel = request;
    hdma.Init.Direction = direction;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment =
        (data_size == SPI_DATASIZE_16BIT) ?
        DMA_PDATAALIGN_HALFWORD : DMA_PDATAALIGN_BYTE;
    hdma.Init.MemDataAlignment =
        (data_size == SPI_DATASIZE_16BIT) ?
        DMA_MDATAALIGN_HALFWORD : DMA_MDATAALIGN_BYTE;
    hdma.Init.Mode = mode;
    hdma.State = HAL_DMA_STATE_READY;
    hdma.Parent = hspi;
    return hdma;
}

static void FeedRx(const uint16_t *data, uint32_t count)
{
    memcpy(mock_rx, data, count * sizeof(data[0]));
    mock_rx_count = count;
    mock_rx_pos = 0U;
}

uint32_t HAL_GetTick(void)
{
    uint32_t value = mock_tick;
    if (mock_tick_increment != 0)
    {
        ++mock_tick;
    }
    return value;
}

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    (void)detail;
    mock_port_error = error;
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    assert(hspi != NULL);
    ++mock_msp_init;
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    assert(hspi != NULL);
    ++mock_msp_deinit;
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    assert(hspi != NULL);
    ++mock_tx_callback;
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    assert(hspi != NULL);
    ++mock_rx_callback;
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    assert(hspi != NULL);
    ++mock_txrx_callback;
}

void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
    assert(hspi != NULL);
    ++mock_tx_half;
}

void HAL_SPI_RxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
    assert(hspi != NULL);
    ++mock_rx_half;
}

void HAL_SPI_TxRxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
    assert(hspi != NULL);
    ++mock_txrx_half;
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    assert(hspi != NULL);
    ++mock_error_callback;
}

int GD32_HAL_SPI_IsInstance(uint32_t address)
{
    return address == GD32_HAL_SPI0_ADDRESS;
}

int GD32_HAL_SPI_Configure(uint32_t address,
                           const GD32_HAL_SPIConfig *config)
{
    assert(address == GD32_HAL_SPI0_ADDRESS);
    mock_config = *config;
    ++mock_config_count;
    return 0;
}

void GD32_HAL_SPI_DeInit(uint32_t address) { (void)address; }
void GD32_HAL_SPI_Enable(uint32_t address) { (void)address; }
void GD32_HAL_SPI_Disable(uint32_t address) { (void)address; }
void GD32_HAL_SPI_SetDirection(uint32_t address, uint8_t direction)
{
    (void)address;
    (void)direction;
}

void GD32_HAL_SPI_WriteData(uint32_t address, uint16_t data)
{
    assert(address == GD32_HAL_SPI0_ADDRESS);
    mock_tx[mock_tx_count++] = data;
    if (mock_rx_pos < mock_rx_count)
    {
        mock_flags |= GD32_HAL_SPI_FLAG_RX_NOT_EMPTY;
    }
}

uint16_t GD32_HAL_SPI_ReadData(uint32_t address)
{
    uint16_t value;
    assert(address == GD32_HAL_SPI0_ADDRESS);
    assert(mock_rx_pos < mock_rx_count);
    value = mock_rx[mock_rx_pos++];
    if (mock_rx_pos >= mock_rx_count)
    {
        mock_flags &= ~GD32_HAL_SPI_FLAG_RX_NOT_EMPTY;
    }
    return value;
}

uint32_t GD32_HAL_SPI_GetFlags(uint32_t address)
{
    assert(address == GD32_HAL_SPI0_ADDRESS);
    return mock_flags;
}

uint32_t GD32_HAL_SPI_GetErrors(uint32_t address)
{
    assert(address == GD32_HAL_SPI0_ADDRESS);
    return mock_errors;
}

void GD32_HAL_SPI_ClearErrors(uint32_t address, uint32_t errors)
{
    assert(address == GD32_HAL_SPI0_ADDRESS);
    mock_errors &= ~errors;
}

void GD32_HAL_SPI_SetInterrupts(uint32_t address,
                                uint32_t interrupts,
                                int enable)
{
    assert(address == GD32_HAL_SPI0_ADDRESS);
    if (enable != 0)
    {
        mock_interrupts |= interrupts;
    }
    else
    {
        mock_interrupts &= ~interrupts;
    }
}

uint32_t GD32_HAL_SPI_GetInterrupts(uint32_t address)
{
    assert(address == GD32_HAL_SPI0_ADDRESS);
    return mock_interrupts;
}

void GD32_HAL_SPI_SetDMARequest(uint32_t address,
                                int transmit,
                                int enable)
{
    uint32_t bit = (transmit != 0) ? 1U : 2U;
    assert(address == GD32_HAL_SPI0_ADDRESS);
    if (enable != 0)
    {
        mock_dma_requests |= bit;
    }
    else
    {
        mock_dma_requests &= ~bit;
    }
}

uint32_t GD32_HAL_SPI_GetDataAddress(uint32_t address)
{
    return address + 0x0CU;
}

int GD32_HAL_SPI_IsDMAChannelValid(uint32_t address,
                                   int transmit,
                                   uint32_t channel,
                                   uint32_t request)
{
    if (address != GD32_HAL_SPI0_ADDRESS)
    {
        return 0;
    }
    return (transmit != 0) ?
        ((channel == GD32_HAL_DMA0_CHANNEL2_ADDRESS) &&
         (request == GD32_DMA_REQUEST_SPI0_TX)) :
        ((channel == GD32_HAL_DMA0_CHANNEL1_ADDRESS) &&
         (request == GD32_DMA_REQUEST_SPI0_RX));
}

HAL_StatusTypeDef HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma,
                                   uint32_t source,
                                   uint32_t destination,
                                   uint32_t length)
{
    uint32_t index = mock_dma_starts++;
    mock_dma_sources[index] = source;
    mock_dma_destinations[index] = destination;
    mock_dma_lengths[index] = length;
    hdma->State = HAL_DMA_STATE_BUSY;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_DMA_Abort(DMA_HandleTypeDef *hdma)
{
    ++mock_dma_aborts;
    hdma->State = HAL_DMA_STATE_READY;
    return HAL_OK;
}

static void TestPolling(void)
{
    SPI_HandleTypeDef hspi = MakeSPI(SPI_DATASIZE_8BIT);
    uint8_t tx[] = {0x11U, 0x22U};
    uint8_t rx[2] = {0U};
    uint16_t incoming[] = {0xA1U, 0xB2U};

    assert(HAL_SPI_Init(&hspi) == HAL_OK);
    assert(mock_config_count == 1U && mock_msp_init == 1U);
    assert(mock_config.master == 1U && mock_config.prescaler == 8U);
    FeedRx(incoming, 2U);
    assert(HAL_SPI_TransmitReceive(&hspi, tx, rx, 2U, 10U) == HAL_OK);
    assert(mock_tx_count == 2U);
    assert(mock_tx[0] == 0x11U && mock_tx[1] == 0x22U);
    assert(rx[0] == 0xA1U && rx[1] == 0xB2U);

    mock_tx_count = 0U;
    FeedRx(incoming, 2U);
    assert(HAL_SPI_Receive(&hspi, rx, 2U, 10U) == HAL_OK);
    assert(mock_tx[0] == 0xFFU && mock_tx[1] == 0xFFU);
}

static void Test16BitAndIT(void)
{
    SPI_HandleTypeDef hspi = MakeSPI(SPI_DATASIZE_16BIT);
    uint8_t tx[] = {0x34U, 0x12U, 0x78U, 0x56U};
    uint8_t rx[4] = {0U};
    uint16_t incoming[] = {0xABCDU, 0x1357U};

    assert(HAL_SPI_Init(&hspi) == HAL_OK);
    FeedRx(incoming, 2U);
    assert(HAL_SPI_TransmitReceive(&hspi, tx, rx, 2U, 10U) == HAL_OK);
    assert(mock_tx[0] == 0x1234U && mock_tx[1] == 0x5678U);
    assert(rx[0] == 0xCDU && rx[1] == 0xABU);
    assert(rx[2] == 0x57U && rx[3] == 0x13U);

    mock_tx_count = 0U;
    mock_rx_count = 0U;
    mock_rx_pos = 0U;
    mock_flags = GD32_HAL_SPI_FLAG_TX_EMPTY;
    assert(HAL_SPI_Transmit_IT(&hspi, tx, 2U) == HAL_OK);
    HAL_SPI_IRQHandler(&hspi);
    HAL_SPI_IRQHandler(&hspi);
    assert(mock_tx_callback == 1U);
    assert(hspi.State == HAL_SPI_STATE_READY);

    mock_tx_count = 0U;
    mock_flags = GD32_HAL_SPI_FLAG_TX_EMPTY;
    FeedRx(incoming, 2U);
    memset(rx, 0, sizeof(rx));
    assert(HAL_SPI_TransmitReceive_IT(&hspi, tx, rx, 2U) == HAL_OK);
    HAL_SPI_IRQHandler(&hspi);
    HAL_SPI_IRQHandler(&hspi);
    HAL_SPI_IRQHandler(&hspi);
    assert(mock_txrx_callback == 1U);
    assert(rx[0] == 0xCDU && rx[1] == 0xABU);

    mock_tx_count = 0U;
    mock_flags = GD32_HAL_SPI_FLAG_TX_EMPTY;
    FeedRx(incoming, 2U);
    memset(rx, 0, sizeof(rx));
    assert(HAL_SPI_Receive_IT(&hspi, rx, 2U) == HAL_OK);
    HAL_SPI_IRQHandler(&hspi);
    HAL_SPI_IRQHandler(&hspi);
    HAL_SPI_IRQHandler(&hspi);
    assert(mock_rx_callback == 1U);
    assert(mock_tx[0] == 0xFFFFU && mock_tx[1] == 0xFFFFU);
    assert(rx[0] == 0xCDU && rx[1] == 0xABU);

    assert(HAL_SPI_Transmit_IT(&hspi, tx, 1U) == HAL_OK);
    mock_errors = GD32_HAL_SPI_ERROR_MODE;
    HAL_SPI_IRQHandler(&hspi);
    assert(mock_error_callback == 1U);
    assert((hspi.ErrorCode & HAL_SPI_ERROR_MODF) != 0U);
}

static void TestDMA(void)
{
    SPI_HandleTypeDef hspi = MakeSPI(SPI_DATASIZE_8BIT);
    DMA_HandleTypeDef txdma;
    DMA_HandleTypeDef rxdma;
    uint8_t tx[] = {1U, 2U, 3U};
    uint8_t rx[3] = {0U};

    assert(HAL_SPI_Init(&hspi) == HAL_OK);
    txdma = MakeDMA(&hspi, GD32_DMA0_CHANNEL2,
                    GD32_DMA_REQUEST_SPI0_TX, DMA_MEMORY_TO_PERIPH,
                    SPI_DATASIZE_8BIT, DMA_NORMAL);
    rxdma = MakeDMA(&hspi, GD32_DMA0_CHANNEL1,
                    GD32_DMA_REQUEST_SPI0_RX, DMA_PERIPH_TO_MEMORY,
                    SPI_DATASIZE_8BIT, DMA_NORMAL);
    hspi.hdmatx = &txdma;
    hspi.hdmarx = &rxdma;

    assert(HAL_SPI_Transmit_DMA(&hspi, tx, 3U) == HAL_OK);
    assert(mock_dma_starts == 1U && mock_dma_lengths[0] == 3U);
    assert(mock_dma_requests == 1U);
    txdma.XferHalfCpltCallback(&txdma);
    txdma.XferCpltCallback(&txdma);
    assert(mock_tx_half == 1U && mock_tx_callback == 1U);
    assert(hspi.State == HAL_SPI_STATE_READY && mock_dma_requests == 0U);

    assert(HAL_SPI_TransmitReceive_DMA(&hspi, tx, rx, 3U) == HAL_OK);
    assert(mock_dma_starts == 3U);
    assert(mock_dma_requests == 3U);
    rxdma.XferHalfCpltCallback(&rxdma);
    rxdma.XferCpltCallback(&rxdma);
    assert(mock_txrx_half == 1U && mock_txrx_callback == 1U);
    assert(hspi.State == HAL_SPI_STATE_READY);

    assert(HAL_SPI_Receive_DMA(&hspi, rx, 3U) == HAL_OK);
    assert(hspi.State == HAL_SPI_STATE_BUSY_RX);
    rxdma.XferCpltCallback(&rxdma);
    assert(mock_rx_callback == 1U);

    rxdma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    assert(HAL_SPI_Receive_DMA(&hspi, rx, 3U) == HAL_ERROR);
    assert(mock_port_error == GD32_HAL_PORT_ERROR_SPI_DMA_CONFIG_MISMATCH);
}

int main(void)
{
    MockReset();
    TestPolling();
    MockReset();
    Test16BitAndIT();
    MockReset();
    TestDMA();
    puts("SPI host tests: PASS");
    return 0;
}
