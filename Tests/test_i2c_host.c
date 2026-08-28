#include "stm32f4xx_hal.h"
#include "gd32_hal_port.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static GD32_HAL_I2CConfig mock_config;
static uint32_t mock_flags;
static uint32_t mock_errors;
static uint32_t mock_interrupts;
static uint8_t mock_tx[32];
static uint32_t mock_tx_count;
static uint8_t mock_rx[32];
static uint32_t mock_rx_count;
static uint32_t mock_rx_pos;
static uint16_t mock_addresses[16];
static int mock_directions[16];
static uint32_t mock_address_count;
static uint32_t mock_start_count;
static uint32_t mock_stop_count;
static uint32_t mock_reset_count;
static uint32_t mock_config_count;
static uint32_t mock_tick;
static int mock_tick_increment;
static int mock_auto_address;
static int mock_ready_after_trial;
static int mock_trial;
static int mock_ten_stage;
static uint32_t mock_tx_callback;
static uint32_t mock_rx_callback;
static uint32_t mock_error_callback;
static uint32_t mock_msp_init;
static uint32_t mock_msp_deinit;
static GD32_HAL_PortError mock_port_error;

static void MockReset(void)
{
    memset(&mock_config, 0, sizeof(mock_config));
    mock_flags = GD32_HAL_I2C_FLAG_TX_EMPTY |
                 GD32_HAL_I2C_FLAG_BYTE_TRANSFER;
    mock_errors = 0U;
    mock_interrupts = 0U;
    mock_tx_count = 0U;
    mock_rx_count = 0U;
    mock_rx_pos = 0U;
    mock_address_count = 0U;
    mock_start_count = 0U;
    mock_stop_count = 0U;
    mock_reset_count = 0U;
    mock_config_count = 0U;
    mock_tick = 0U;
    mock_tick_increment = 0;
    mock_auto_address = 1;
    mock_ready_after_trial = 0;
    mock_trial = 0;
    mock_ten_stage = 0;
    mock_tx_callback = 0U;
    mock_rx_callback = 0U;
    mock_error_callback = 0U;
    mock_msp_init = 0U;
    mock_msp_deinit = 0U;
    mock_port_error = GD32_HAL_PORT_ERROR_NONE;
}

static I2C_HandleTypeDef MakeI2C(void)
{
    I2C_HandleTypeDef hi2c;
    memset(&hi2c, 0, sizeof(hi2c));
    hi2c.Instance = I2C1;
    hi2c.Init.ClockSpeed = 400000U;
    hi2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c.Init.OwnAddress1 = 0x42U;
    hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    hi2c.State = HAL_I2C_STATE_RESET;
    return hi2c;
}

uint32_t HAL_GetTick(void)
{
    uint32_t tick = mock_tick;
    if (mock_tick_increment != 0)
    {
        ++mock_tick;
    }
    return tick;
}

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    (void)detail;
    mock_port_error = error;
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    assert(hi2c != NULL);
    ++mock_msp_init;
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    assert(hi2c != NULL);
    ++mock_msp_deinit;
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    assert(hi2c != NULL);
    ++mock_tx_callback;
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    assert(hi2c != NULL);
    ++mock_rx_callback;
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    assert(hi2c != NULL);
    ++mock_error_callback;
}

int GD32_HAL_I2C_IsInstance(uint32_t address)
{
    return address == GD32_HAL_I2C0_ADDRESS;
}

int GD32_HAL_I2C_Configure(uint32_t address,
                           const GD32_HAL_I2CConfig *config)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
    mock_config = *config;
    ++mock_config_count;
    return 0;
}

void GD32_HAL_I2C_DeInit(uint32_t address)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
}

void GD32_HAL_I2C_Enable(uint32_t address) { (void)address; }
void GD32_HAL_I2C_Disable(uint32_t address) { (void)address; }

void GD32_HAL_I2C_Start(uint32_t address)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
    if ((mock_flags & GD32_HAL_I2C_FLAG_BUSY) == 0U)
    {
        mock_ten_stage = 0;
    }
    ++mock_start_count;
    mock_flags |= GD32_HAL_I2C_FLAG_START | GD32_HAL_I2C_FLAG_BUSY;
}

void GD32_HAL_I2C_Stop(uint32_t address)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
    ++mock_stop_count;
    mock_flags &= ~GD32_HAL_I2C_FLAG_BUSY;
}

void GD32_HAL_I2C_SetAck(uint32_t address, int enable)
{
    (void)address;
    (void)enable;
}

void GD32_HAL_I2C_SetAckNext(uint32_t address, int next)
{
    (void)address;
    (void)next;
}

void GD32_HAL_I2C_SendAddress(uint32_t address,
                              uint16_t device,
                              int receive)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
    mock_addresses[mock_address_count] = device;
    mock_directions[mock_address_count] = receive;
    ++mock_address_count;
    if ((mock_ready_after_trial > 0) && (mock_trial < mock_ready_after_trial))
    {
        ++mock_trial;
        mock_errors |= GD32_HAL_I2C_ERROR_ACK;
        mock_flags &= ~GD32_HAL_I2C_FLAG_ADDRESS;
    }
    else if (mock_auto_address != 0)
    {
        mock_flags |= GD32_HAL_I2C_FLAG_ADDRESS;
    }
}

void GD32_HAL_I2C_WriteData(uint32_t address, uint8_t data)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
    mock_tx[mock_tx_count++] = data;
    if (((data & 0xF8U) == 0xF0U) && (mock_ten_stage != 1))
    {
        if (mock_ten_stage == 0)
        {
            mock_flags |= GD32_HAL_I2C_FLAG_ADDRESS10;
            mock_ten_stage = 1;
        }
        else
        {
            mock_flags |= GD32_HAL_I2C_FLAG_ADDRESS;
        }
    }
    else if (mock_ten_stage == 1)
    {
        mock_flags |= GD32_HAL_I2C_FLAG_ADDRESS;
        mock_ten_stage = 2;
    }
    mock_flags |= GD32_HAL_I2C_FLAG_TX_EMPTY |
                  GD32_HAL_I2C_FLAG_BYTE_TRANSFER;
}

uint8_t GD32_HAL_I2C_ReadData(uint32_t address)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
    assert(mock_rx_pos < mock_rx_count);
    return mock_rx[mock_rx_pos++];
}

uint32_t GD32_HAL_I2C_GetFlags(uint32_t address)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
    if (mock_rx_pos < mock_rx_count)
    {
        mock_flags |= GD32_HAL_I2C_FLAG_RX_NOT_EMPTY |
                      GD32_HAL_I2C_FLAG_BYTE_TRANSFER;
    }
    return mock_flags;
}

void GD32_HAL_I2C_ClearAddress(uint32_t address)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
    mock_flags &= ~GD32_HAL_I2C_FLAG_ADDRESS;
}

uint32_t GD32_HAL_I2C_GetErrors(uint32_t address)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
    return mock_errors;
}

void GD32_HAL_I2C_ClearErrors(uint32_t address, uint32_t errors)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
    mock_errors &= ~errors;
}

void GD32_HAL_I2C_SetInterrupts(uint32_t address,
                                uint32_t interrupts,
                                int enable)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
    if (enable != 0)
    {
        mock_interrupts |= interrupts;
    }
    else
    {
        mock_interrupts &= ~interrupts;
    }
}

uint32_t GD32_HAL_I2C_GetInterrupts(uint32_t address)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
    return mock_interrupts;
}

void GD32_HAL_I2C_SoftwareReset(uint32_t address)
{
    assert(address == GD32_HAL_I2C0_ADDRESS);
    ++mock_reset_count;
    mock_flags = GD32_HAL_I2C_FLAG_TX_EMPTY |
                 GD32_HAL_I2C_FLAG_BYTE_TRANSFER;
    mock_errors = 0U;
}

static void FeedRx(const uint8_t *data, uint32_t count)
{
    memcpy(mock_rx, data, count);
    mock_rx_count = count;
    mock_rx_pos = 0U;
}

static void TestPollingAndMem(void)
{
    I2C_HandleTypeDef hi2c = MakeI2C();
    uint8_t tx[] = {0x11U, 0x22U};
    uint8_t rx[3] = {0U};
    uint8_t incoming[] = {0xA1U, 0xB2U, 0xC3U};

    assert(HAL_I2C_Init(&hi2c) == HAL_OK);
    assert(mock_config_count == 1U);
    assert(mock_config.clock_speed == 400000U);
    assert(mock_msp_init == 1U);
    assert(HAL_I2C_Master_Transmit(&hi2c, 0xA0U, tx, 2U, 10U) == HAL_OK);
    assert(mock_address_count == 1U);
    assert(mock_addresses[0] == 0xA0U && mock_directions[0] == 0);
    assert(mock_tx_count == 2U);
    assert(memcmp(mock_tx, tx, 2U) == 0);

    FeedRx(incoming, 3U);
    assert(HAL_I2C_Master_Receive(&hi2c, 0xA0U, rx, 3U, 10U) == HAL_OK);
    assert(memcmp(rx, incoming, 3U) == 0);

    mock_tx_count = 0U;
    mock_address_count = 0U;
    mock_start_count = 0U;
    FeedRx(incoming, 2U);
    assert(HAL_I2C_Mem_Read(&hi2c, 0xA0U, 0x1234U,
                            I2C_MEMADD_SIZE_16BIT, rx, 2U, 10U) == HAL_OK);
    assert(mock_start_count == 2U);
    assert(mock_address_count == 2U);
    assert(mock_directions[0] == 0 && mock_directions[1] == 1);
    assert(mock_tx_count == 2U);
    assert(mock_tx[0] == 0x12U && mock_tx[1] == 0x34U);
    assert(rx[0] == 0xA1U && rx[1] == 0xB2U);
}

static void TestITAndError(void)
{
    I2C_HandleTypeDef hi2c = MakeI2C();
    uint8_t tx[] = {0x31U, 0x32U};
    uint8_t rx[2] = {0U};
    uint8_t incoming[] = {0x61U, 0x62U};

    assert(HAL_I2C_Init(&hi2c) == HAL_OK);
    assert(HAL_I2C_Master_Transmit_IT(&hi2c, 0xA0U, tx, 2U) == HAL_OK);
    HAL_I2C_EV_IRQHandler(&hi2c);
    HAL_I2C_EV_IRQHandler(&hi2c);
    HAL_I2C_EV_IRQHandler(&hi2c);
    HAL_I2C_EV_IRQHandler(&hi2c);
    assert(mock_tx_callback == 1U);
    assert(hi2c.State == HAL_I2C_STATE_READY);

    FeedRx(incoming, 2U);
    assert(HAL_I2C_Master_Receive_IT(&hi2c, 0xA0U, rx, 2U) == HAL_OK);
    HAL_I2C_IRQHandler(&hi2c);
    HAL_I2C_EV_IRQHandler(&hi2c);
    HAL_I2C_EV_IRQHandler(&hi2c);
    assert(mock_rx_callback == 1U);
    assert(memcmp(rx, incoming, 2U) == 0);

    assert(HAL_I2C_Master_Transmit_IT(&hi2c, 0xA0U, tx, 1U) == HAL_OK);
    mock_errors = GD32_HAL_I2C_ERROR_ARBITRATION;
    HAL_I2C_ER_IRQHandler(&hi2c);
    assert(mock_error_callback == 1U);
    assert((hi2c.ErrorCode & HAL_I2C_ERROR_ARLO) != 0U);
    assert(mock_reset_count == 1U);
}

static void TestTenBitAddress(void)
{
    I2C_HandleTypeDef hi2c = MakeI2C();
    uint8_t tx = 0x5AU;
    uint8_t rx = 0U;
    uint8_t incoming = 0xC7U;

    hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_10BIT;
    hi2c.Init.OwnAddress1 = 0x155U;
    assert(HAL_I2C_Init(&hi2c) == HAL_OK);
    assert(HAL_I2C_Master_Transmit(&hi2c, 0x2AAU, &tx, 1U, 10U) == HAL_OK);
    assert(mock_tx_count == 3U);
    assert(mock_tx[0] == 0xF4U && mock_tx[1] == 0xAAU);
    assert(mock_tx[2] == 0x5AU);

    mock_tx_count = 0U;
    FeedRx(&incoming, 1U);
    assert(HAL_I2C_Master_Receive(&hi2c, 0x2AAU, &rx, 1U, 10U) == HAL_OK);
    assert(mock_tx_count == 3U);
    assert(mock_tx[0] == 0xF4U && mock_tx[1] == 0xAAU &&
           mock_tx[2] == 0xF5U);
    assert(rx == 0xC7U);
}

static void TestReadyAndTimeoutRecovery(void)
{
    I2C_HandleTypeDef hi2c = MakeI2C();
    uint8_t tx = 0x55U;
    assert(HAL_I2C_Init(&hi2c) == HAL_OK);
    mock_ready_after_trial = 1;
    assert(HAL_I2C_IsDeviceReady(&hi2c, 0xA0U, 3U, 10U) == HAL_OK);
    assert(mock_address_count == 2U);

    mock_flags = GD32_HAL_I2C_FLAG_BUSY;
    mock_tick_increment = 1;
    assert(HAL_I2C_Master_Transmit(&hi2c, 0xA0U, &tx, 1U, 2U) == HAL_BUSY);
    assert(mock_reset_count == 1U);
    assert(mock_port_error == GD32_HAL_PORT_ERROR_I2C_TIMEOUT);
}

int main(void)
{
    MockReset();
    TestPollingAndMem();
    MockReset();
    TestITAndError();
    MockReset();
    TestReadyAndTimeoutRecovery();
    MockReset();
    TestTenBitAddress();
    puts("I2C host tests: PASS");
    return 0;
}
