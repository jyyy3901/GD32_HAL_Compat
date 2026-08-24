#include "gd32_hal_port.h"
#include "gd32f403_spi.h"
#include <stddef.h>

static uint32_t SPI_TransferMode(uint8_t direction)
{
    if (direction == 1U)
    {
        return SPI_TRANSMODE_RECEIVEONLY;
    }
    if (direction == 2U)
    {
        return SPI_TRANSMODE_BDRECEIVE;
    }
    if (direction == 3U)
    {
        return SPI_TRANSMODE_BDTRANSMIT;
    }
    return SPI_TRANSMODE_FULLDUPLEX;
}

static uint32_t SPI_Prescaler(uint16_t prescaler)
{
    static const uint32_t values[] =
    {
        SPI_PSC_2, SPI_PSC_4, SPI_PSC_8, SPI_PSC_16,
        SPI_PSC_32, SPI_PSC_64, SPI_PSC_128, SPI_PSC_256
    };
    uint16_t value = 2U;
    uint32_t index = 0U;
    while ((value < prescaler) && (index < 7U))
    {
        value = (uint16_t)(value * 2U);
        ++index;
    }
    return values[index];
}

int GD32_HAL_SPI_IsInstance(uint32_t spi_address)
{
    return (spi_address == GD32_HAL_SPI0_ADDRESS) ||
           (spi_address == GD32_HAL_SPI1_ADDRESS) ||
           (spi_address == GD32_HAL_SPI2_ADDRESS);
}

int GD32_HAL_SPI_Configure(uint32_t spi_address,
                           const GD32_HAL_SPIConfig *config)
{
    spi_parameter_struct init;
    uint32_t polarity_phase;

    if ((GD32_HAL_SPI_IsInstance(spi_address) == 0) || (config == NULL) ||
        ((config->data_bits != 8U) && (config->data_bits != 16U)) ||
        (config->direction > 3U) || (config->nss > 2U) ||
        (config->crc_polynomial == 0U) ||
        ((config->prescaler != 2U) && (config->prescaler != 4U) &&
         (config->prescaler != 8U) && (config->prescaler != 16U) &&
         (config->prescaler != 32U) && (config->prescaler != 64U) &&
         (config->prescaler != 128U) && (config->prescaler != 256U)))
    {
        return -1;
    }

    spi_i2s_deinit(spi_address);
    spi_struct_para_init(&init);
    init.device_mode = (config->master != 0U) ? SPI_MASTER : SPI_SLAVE;
    init.trans_mode = SPI_TransferMode(config->direction);
    init.frame_size = (config->data_bits == 16U) ?
                      SPI_FRAMESIZE_16BIT : SPI_FRAMESIZE_8BIT;
    init.nss = (config->nss == 0U) ? SPI_NSS_SOFT : SPI_NSS_HARD;
    init.endian = (config->lsb_first != 0U) ? SPI_ENDIAN_LSB : SPI_ENDIAN_MSB;
    if (config->clock_polarity_high != 0U)
    {
        polarity_phase = (config->clock_phase_second != 0U) ?
                         SPI_CK_PL_HIGH_PH_2EDGE : SPI_CK_PL_HIGH_PH_1EDGE;
    }
    else
    {
        polarity_phase = (config->clock_phase_second != 0U) ?
                         SPI_CK_PL_LOW_PH_2EDGE : SPI_CK_PL_LOW_PH_1EDGE;
    }
    init.clock_polarity_phase = polarity_phase;
    init.prescale = SPI_Prescaler(config->prescaler);
    spi_init(spi_address, &init);

    if (config->nss == 0U)
    {
        spi_nss_internal_high(spi_address);
    }
    else if ((config->nss == 2U) && (config->master != 0U))
    {
        spi_nss_output_enable(spi_address);
    }
    else
    {
        spi_nss_output_disable(spi_address);
    }
    if (config->ti_mode != 0U)
    {
        spi_ti_mode_enable(spi_address);
    }
    else
    {
        spi_ti_mode_disable(spi_address);
    }
    spi_crc_polynomial_set(spi_address, config->crc_polynomial);
    if (config->crc_enable != 0U)
    {
        spi_crc_on(spi_address);
    }
    else
    {
        spi_crc_off(spi_address);
    }
    spi_enable(spi_address);
    return 0;
}

void GD32_HAL_SPI_DeInit(uint32_t spi_address)
{
    spi_i2s_deinit(spi_address);
}

void GD32_HAL_SPI_Enable(uint32_t spi_address)
{
    spi_enable(spi_address);
}

void GD32_HAL_SPI_Disable(uint32_t spi_address)
{
    spi_disable(spi_address);
}

void GD32_HAL_SPI_SetDirection(uint32_t spi_address, uint8_t direction)
{
    if (direction == 2U)
    {
        spi_bidirectional_transfer_config(spi_address,
                                          SPI_BIDIRECTIONAL_RECEIVE);
    }
    else if (direction == 3U)
    {
        spi_bidirectional_transfer_config(spi_address,
                                          SPI_BIDIRECTIONAL_TRANSMIT);
    }
}

void GD32_HAL_SPI_WriteData(uint32_t spi_address, uint16_t data)
{
    spi_i2s_data_transmit(spi_address, data);
}

uint16_t GD32_HAL_SPI_ReadData(uint32_t spi_address)
{
    return spi_i2s_data_receive(spi_address);
}

uint32_t GD32_HAL_SPI_GetFlags(uint32_t spi_address)
{
    uint32_t flags = 0U;
    if (spi_i2s_flag_get(spi_address, SPI_FLAG_RBNE) == SET)
    {
        flags |= GD32_HAL_SPI_FLAG_RX_NOT_EMPTY;
    }
    if (spi_i2s_flag_get(spi_address, SPI_FLAG_TBE) == SET)
    {
        flags |= GD32_HAL_SPI_FLAG_TX_EMPTY;
    }
    if (spi_i2s_flag_get(spi_address, SPI_FLAG_TRANS) == SET)
    {
        flags |= GD32_HAL_SPI_FLAG_BUSY;
    }
    return flags;
}

uint32_t GD32_HAL_SPI_GetErrors(uint32_t spi_address)
{
    uint32_t errors = 0U;
    if (spi_i2s_flag_get(spi_address, SPI_FLAG_CONFERR) == SET)
    {
        errors |= GD32_HAL_SPI_ERROR_MODE;
    }
    if (spi_i2s_flag_get(spi_address, SPI_FLAG_CRCERR) == SET)
    {
        errors |= GD32_HAL_SPI_ERROR_CRC;
    }
    if (spi_i2s_flag_get(spi_address, SPI_FLAG_RXORERR) == SET)
    {
        errors |= GD32_HAL_SPI_ERROR_OVERRUN;
    }
    if (spi_i2s_flag_get(spi_address, SPI_FLAG_FERR) == SET)
    {
        errors |= GD32_HAL_SPI_ERROR_FRAME;
    }
    return errors;
}

void GD32_HAL_SPI_ClearErrors(uint32_t spi_address, uint32_t errors)
{
    volatile uint32_t discard;
    if ((errors & GD32_HAL_SPI_ERROR_CRC) != 0U)
    {
        spi_crc_error_clear(spi_address);
    }
    if ((errors & GD32_HAL_SPI_ERROR_OVERRUN) != 0U)
    {
        discard = SPI_DATA(spi_address);
        discard = SPI_STAT(spi_address);
        (void)discard;
    }
    if ((errors & GD32_HAL_SPI_ERROR_FRAME) != 0U)
    {
        spi_i2s_format_error_clear(spi_address, SPI_FLAG_FERR);
    }
    if ((errors & GD32_HAL_SPI_ERROR_MODE) != 0U)
    {
        discard = SPI_STAT(spi_address);
        SPI_CTL0(spi_address) = SPI_CTL0(spi_address);
        (void)discard;
    }
}

void GD32_HAL_SPI_SetInterrupts(uint32_t spi_address,
                                uint32_t interrupts,
                                int enable)
{
    if ((interrupts & GD32_HAL_SPI_INTERRUPT_TX) != 0U)
    {
        (enable != 0) ? spi_i2s_interrupt_enable(spi_address, SPI_I2S_INT_TBE) :
                        spi_i2s_interrupt_disable(spi_address, SPI_I2S_INT_TBE);
    }
    if ((interrupts & GD32_HAL_SPI_INTERRUPT_RX) != 0U)
    {
        (enable != 0) ? spi_i2s_interrupt_enable(spi_address, SPI_I2S_INT_RBNE) :
                        spi_i2s_interrupt_disable(spi_address, SPI_I2S_INT_RBNE);
    }
    if ((interrupts & GD32_HAL_SPI_INTERRUPT_ERROR) != 0U)
    {
        (enable != 0) ? spi_i2s_interrupt_enable(spi_address, SPI_I2S_INT_ERR) :
                        spi_i2s_interrupt_disable(spi_address, SPI_I2S_INT_ERR);
    }
}

uint32_t GD32_HAL_SPI_GetInterrupts(uint32_t spi_address)
{
    uint32_t interrupts = 0U;
    if ((SPI_CTL1(spi_address) & SPI_CTL1_TBEIE) != 0U)
    {
        interrupts |= GD32_HAL_SPI_INTERRUPT_TX;
    }
    if ((SPI_CTL1(spi_address) & SPI_CTL1_RBNEIE) != 0U)
    {
        interrupts |= GD32_HAL_SPI_INTERRUPT_RX;
    }
    if ((SPI_CTL1(spi_address) & SPI_CTL1_ERRIE) != 0U)
    {
        interrupts |= GD32_HAL_SPI_INTERRUPT_ERROR;
    }
    return interrupts;
}

void GD32_HAL_SPI_SetDMARequest(uint32_t spi_address,
                                int transmit,
                                int enable)
{
    uint8_t request = (transmit != 0) ? SPI_DMA_TRANSMIT : SPI_DMA_RECEIVE;
    if (enable != 0)
    {
        spi_dma_enable(spi_address, request);
    }
    else
    {
        spi_dma_disable(spi_address, request);
    }
}

uint32_t GD32_HAL_SPI_GetDataAddress(uint32_t spi_address)
{
    return spi_address + 0x0CU;
}

int GD32_HAL_SPI_IsDMAChannelValid(uint32_t spi_address,
                                   int transmit,
                                   uint32_t channel_address,
                                   uint32_t request_token)
{
    if (spi_address == GD32_HAL_SPI0_ADDRESS)
    {
        return (transmit != 0) ?
            ((channel_address == GD32_HAL_DMA0_CHANNEL2_ADDRESS) &&
             (request_token == GD32_HAL_DMA_REQUEST_ENCODE(0U, 2U, 0U))) :
            ((channel_address == GD32_HAL_DMA0_CHANNEL1_ADDRESS) &&
             (request_token == GD32_HAL_DMA_REQUEST_ENCODE(0U, 1U, 0U)));
    }
    if (spi_address == GD32_HAL_SPI1_ADDRESS)
    {
        return (transmit != 0) ?
            ((channel_address == GD32_HAL_DMA0_CHANNEL4_ADDRESS) &&
             (request_token == GD32_HAL_DMA_REQUEST_ENCODE(0U, 4U, 0U))) :
            ((channel_address == GD32_HAL_DMA0_CHANNEL3_ADDRESS) &&
             (request_token == GD32_HAL_DMA_REQUEST_ENCODE(0U, 3U, 0U)));
    }
    if (spi_address == GD32_HAL_SPI2_ADDRESS)
    {
        return (transmit != 0) ?
            ((channel_address == GD32_HAL_DMA1_CHANNEL1_ADDRESS) &&
             (request_token == GD32_HAL_DMA_REQUEST_ENCODE(1U, 1U, 0U))) :
            ((channel_address == GD32_HAL_DMA1_CHANNEL0_ADDRESS) &&
             (request_token == GD32_HAL_DMA_REQUEST_ENCODE(1U, 0U, 0U)));
    }
    return 0;
}
