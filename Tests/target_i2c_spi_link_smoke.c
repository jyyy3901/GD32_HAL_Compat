#include "stm32f4xx_hal.h"

uint32_t SystemCoreClock = 120000000U;

static I2C_HandleTypeDef targetI2C;
static SPI_HandleTypeDef targetSPI;
static DMA_HandleTypeDef targetSPITxDMA;
static DMA_HandleTypeDef targetSPIRxDMA;
static uint8_t targetTx[2] = {0x5AU, 0xA5U};
static uint8_t targetRx[2];

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        __HAL_RCC_I2C1_CLK_ENABLE();
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        __HAL_RCC_SPI1_CLK_ENABLE();
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    UNUSED(hspi);
}

static void ConfigureSPIDMA(void)
{
    targetSPITxDMA.Instance = GD32_DMA0_CHANNEL2;
    targetSPITxDMA.Init.Channel = GD32_DMA_REQUEST_SPI0_TX;
    targetSPITxDMA.Init.Direction = DMA_MEMORY_TO_PERIPH;
    targetSPITxDMA.Init.PeriphInc = DMA_PINC_DISABLE;
    targetSPITxDMA.Init.MemInc = DMA_MINC_ENABLE;
    targetSPITxDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    targetSPITxDMA.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    targetSPITxDMA.Init.Mode = DMA_NORMAL;
    targetSPITxDMA.Init.Priority = DMA_PRIORITY_HIGH;
    targetSPITxDMA.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    targetSPITxDMA.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    targetSPITxDMA.Init.MemBurst = DMA_MBURST_SINGLE;
    targetSPITxDMA.Init.PeriphBurst = DMA_PBURST_SINGLE;
    (void)HAL_DMA_Init(&targetSPITxDMA);
    __HAL_LINKDMA(&targetSPI, hdmatx, targetSPITxDMA);

    targetSPIRxDMA.Instance = GD32_DMA0_CHANNEL1;
    targetSPIRxDMA.Init.Channel = GD32_DMA_REQUEST_SPI0_RX;
    targetSPIRxDMA.Init.Direction = DMA_PERIPH_TO_MEMORY;
    targetSPIRxDMA.Init.PeriphInc = DMA_PINC_DISABLE;
    targetSPIRxDMA.Init.MemInc = DMA_MINC_ENABLE;
    targetSPIRxDMA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    targetSPIRxDMA.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    targetSPIRxDMA.Init.Mode = DMA_NORMAL;
    targetSPIRxDMA.Init.Priority = DMA_PRIORITY_HIGH;
    targetSPIRxDMA.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    targetSPIRxDMA.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    targetSPIRxDMA.Init.MemBurst = DMA_MBURST_SINGLE;
    targetSPIRxDMA.Init.PeriphBurst = DMA_PBURST_SINGLE;
    (void)HAL_DMA_Init(&targetSPIRxDMA);
    __HAL_LINKDMA(&targetSPI, hdmarx, targetSPIRxDMA);
}

void TargetSmoke(void)
{
    targetI2C.Instance = I2C1;
    targetI2C.Init.ClockSpeed = 400000U;
    targetI2C.Init.DutyCycle = I2C_DUTYCYCLE_2;
    targetI2C.Init.OwnAddress1 = 0x42U;
    targetI2C.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    targetI2C.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    targetI2C.Init.OwnAddress2 = 0U;
    targetI2C.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    targetI2C.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    targetI2C.State = HAL_I2C_STATE_RESET;
    (void)HAL_I2C_Init(&targetI2C);
    (void)HAL_I2C_Master_Transmit(&targetI2C, 0xA0U, targetTx, 2U, 1U);
    (void)HAL_I2C_Master_Receive(&targetI2C, 0xA0U, targetRx, 2U, 1U);
    (void)HAL_I2C_Master_Transmit_IT(&targetI2C, 0xA0U, targetTx, 2U);
    HAL_I2C_EV_IRQHandler(&targetI2C);
    HAL_I2C_ER_IRQHandler(&targetI2C);
    (void)HAL_I2C_Mem_Write(&targetI2C, 0xA0U, 0x10U,
                            I2C_MEMADD_SIZE_8BIT, targetTx, 2U, 1U);
    (void)HAL_I2C_Mem_Read(&targetI2C, 0xA0U, 0x10U,
                           I2C_MEMADD_SIZE_8BIT, targetRx, 2U, 1U);
    (void)HAL_I2C_IsDeviceReady(&targetI2C, 0xA0U, 1U, 1U);

    targetSPI.Instance = SPI1;
    targetSPI.Init.Mode = SPI_MODE_MASTER;
    targetSPI.Init.Direction = SPI_DIRECTION_2LINES;
    targetSPI.Init.DataSize = SPI_DATASIZE_8BIT;
    targetSPI.Init.CLKPolarity = SPI_POLARITY_LOW;
    targetSPI.Init.CLKPhase = SPI_PHASE_1EDGE;
    targetSPI.Init.NSS = SPI_NSS_SOFT;
    targetSPI.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    targetSPI.Init.FirstBit = SPI_FIRSTBIT_MSB;
    targetSPI.Init.TIMode = SPI_TIMODE_DISABLE;
    targetSPI.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    targetSPI.Init.CRCPolynomial = 7U;
    targetSPI.State = HAL_SPI_STATE_RESET;
    (void)HAL_SPI_Init(&targetSPI);
    ConfigureSPIDMA();
    (void)HAL_SPI_Transmit(&targetSPI, targetTx, 2U, 1U);
    (void)HAL_SPI_Receive(&targetSPI, targetRx, 2U, 1U);
    (void)HAL_SPI_TransmitReceive(&targetSPI, targetTx, targetRx, 2U, 1U);
    (void)HAL_SPI_Transmit_IT(&targetSPI, targetTx, 2U);
    HAL_SPI_IRQHandler(&targetSPI);
    (void)HAL_SPI_Transmit_DMA(&targetSPI, targetTx, 2U);
    (void)HAL_SPI_Receive_DMA(&targetSPI, targetRx, 2U);
    (void)HAL_SPI_TransmitReceive_DMA(&targetSPI, targetTx, targetRx, 2U);
}
