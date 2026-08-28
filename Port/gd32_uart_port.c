#include "gd32_hal_port.h"
#include <stddef.h>

static uint32_t GD32_HAL_UART_ClockFrequency(uint32_t uart_address)
{
    return (uart_address == GD32_HAL_USART0_ADDRESS) ?
           rcu_clock_freq_get(CK_APB2) : rcu_clock_freq_get(CK_APB1);
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
                (request == GD32_HAL_DMA_REQUEST_ENCODE(0U, 3U, 1U))) :
               ((channel_address == GD32_HAL_DMA0_CHANNEL4_ADDRESS) &&
                (request == GD32_HAL_DMA_REQUEST_ENCODE(0U, 4U, 1U)));
    }
    if (uart_address == GD32_HAL_USART1_ADDRESS)
    {
        return (transmit != 0) ?
               ((channel_address == GD32_HAL_DMA0_CHANNEL6_ADDRESS) &&
                (request == GD32_HAL_DMA_REQUEST_ENCODE(0U, 6U, 0U))) :
               ((channel_address == GD32_HAL_DMA0_CHANNEL5_ADDRESS) &&
                (request == GD32_HAL_DMA_REQUEST_ENCODE(0U, 5U, 0U)));
    }
    if (uart_address == GD32_HAL_USART2_ADDRESS)
    {
        return (transmit != 0) ?
               ((channel_address == GD32_HAL_DMA0_CHANNEL1_ADDRESS) &&
                (request == GD32_HAL_DMA_REQUEST_ENCODE(0U, 1U, 1U))) :
               ((channel_address == GD32_HAL_DMA0_CHANNEL2_ADDRESS) &&
                (request == GD32_HAL_DMA_REQUEST_ENCODE(0U, 2U, 1U)));
    }
    if (uart_address == GD32_HAL_UART3_ADDRESS)
    {
        return (transmit != 0) ?
               ((channel_address == GD32_HAL_DMA1_CHANNEL4_ADDRESS) &&
                (request == GD32_HAL_DMA_REQUEST_ENCODE(1U, 4U, 1U))) :
               ((channel_address == GD32_HAL_DMA1_CHANNEL2_ADDRESS) &&
                (request == GD32_HAL_DMA_REQUEST_ENCODE(1U, 2U, 0U)));
    }
    return 0;
}

uint32_t GD32_HAL_UART_GetDataAddress(uint32_t uart_address)
{
    if (GD32_HAL_UART_IsInstance(uart_address) == 0)
    {
        return 0U;
    }
    return uart_address + 0x04U;
}

int GD32_HAL_UART_IsDMARequestActive(uint32_t uart_address, int transmit)
{
    const uint32_t mask = (transmit != 0) ? USART_CTL2_DENT : USART_CTL2_DENR;

    if (GD32_HAL_UART_IsInstance(uart_address) == 0)
    {
        return 0;
    }
    return (USART_CTL2(uart_address) & mask) != 0U;
}

void GD32_HAL_UART_EnableDMARequest(uint32_t uart_address, int transmit)
{
    if (GD32_HAL_UART_IsDMACapable(uart_address) == 0)
    {
        return;
    }
    if (transmit != 0)
    {
        usart_dma_transmit_config(uart_address, USART_TRANSMIT_DMA_ENABLE);
    }
    else
    {
        usart_dma_receive_config(uart_address, USART_RECEIVE_DMA_ENABLE);
    }
}

int GD32_HAL_UART_Configure(uint32_t uart_address, const GD32_HAL_UARTConfig *config)
{
    uint32_t clock;
    uint32_t divider;
    uint32_t word_length;
    uint32_t stop_bits;
    uint32_t parity;

    if ((config == NULL) || (GD32_HAL_UART_IsInstance(uart_address) == 0) ||
        (config->baud_rate == 0U))
    {
        return -1;
    }

    clock = GD32_HAL_UART_ClockFrequency(uart_address);
    divider = (clock + (config->baud_rate / 2U)) / config->baud_rate;
    if ((clock == 0U) || (config->baud_rate > (clock / 16U)) ||
        (divider < 16U) || (divider > 0xFFFFU))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_UART_BAUD_UNSUPPORTED,
                           config->baud_rate);
        return -1;
    }

    word_length = (config->word_length == 9U) ? USART_WL_9BIT : USART_WL_8BIT;
    stop_bits = (config->stop_bits == 2U) ? USART_STB_2BIT : USART_STB_1BIT;
    if (config->parity == GD32_HAL_UART_PARITY_EVEN)
    {
        parity = USART_PM_EVEN;
    }
    else if (config->parity == GD32_HAL_UART_PARITY_ODD)
    {
        parity = USART_PM_ODD;
    }
    else
    {
        parity = USART_PM_NONE;
    }

    /* 先复位外设，避免遗留同步、LIN、IrDA 或 DMA 请求位污染异步模式。 */
    usart_deinit(uart_address);
    usart_word_length_set(uart_address, word_length);
    usart_stop_bit_set(uart_address, stop_bits);
    usart_parity_config(uart_address, parity);
    usart_baudrate_set(uart_address, config->baud_rate);

    usart_transmit_config(uart_address,
                          ((config->mode & GD32_HAL_UART_MODE_TX) != 0U) ?
                          USART_TRANSMIT_ENABLE : USART_TRANSMIT_DISABLE);
    usart_receive_config(uart_address,
                         ((config->mode & GD32_HAL_UART_MODE_RX) != 0U) ?
                         USART_RECEIVE_ENABLE : USART_RECEIVE_DISABLE);

    if (GD32_HAL_UART_IsFlowControlCapable(uart_address) != 0)
    {
        usart_hardware_flow_rts_config(uart_address,
            ((config->flow_control & GD32_HAL_UART_FLOW_RTS) != 0U) ?
            USART_RTS_ENABLE : USART_RTS_DISABLE);
        usart_hardware_flow_cts_config(uart_address,
            ((config->flow_control & GD32_HAL_UART_FLOW_CTS) != 0U) ?
            USART_CTS_ENABLE : USART_CTS_DISABLE);
    }

    usart_enable(uart_address);
    return 0;
}

void GD32_HAL_UART_DeInit(uint32_t uart_address)
{
    if (GD32_HAL_UART_IsInstance(uart_address) != 0)
    {
        usart_deinit(uart_address);
    }
}

void GD32_HAL_UART_Enable(uint32_t uart_address)
{
    if (GD32_HAL_UART_IsInstance(uart_address) != 0)
    {
        usart_enable(uart_address);
    }
}

void GD32_HAL_UART_Disable(uint32_t uart_address)
{
    if (GD32_HAL_UART_IsInstance(uart_address) != 0)
    {
        usart_disable(uart_address);
    }
}

uint32_t GD32_HAL_UART_GetFlag(uint32_t uart_address, GD32_HAL_UARTFlag flag)
{
    if (GD32_HAL_UART_IsInstance(uart_address) == 0)
    {
        return 0U;
    }
    return USART_STAT0(uart_address) & (uint32_t)flag;
}

void GD32_HAL_UART_ClearFlag(uint32_t uart_address, GD32_HAL_UARTFlag flag)
{
    usart_flag_enum gd_flag;

    if (GD32_HAL_UART_IsInstance(uart_address) == 0)
    {
        return;
    }

    switch (flag)
    {
        case GD32_HAL_UART_FLAG_CTS:
            gd_flag = USART_FLAG_CTS;
            break;
        case GD32_HAL_UART_FLAG_LIN_BREAK:
            gd_flag = USART_FLAG_LBD;
            break;
        case GD32_HAL_UART_FLAG_TX_COMPLETE:
            gd_flag = USART_FLAG_TC;
            break;
        case GD32_HAL_UART_FLAG_RX_NOT_EMPTY:
            gd_flag = USART_FLAG_RBNE;
            break;
        case GD32_HAL_UART_FLAG_IDLE:
        case GD32_HAL_UART_FLAG_ORE:
        case GD32_HAL_UART_FLAG_NE:
        case GD32_HAL_UART_FLAG_FE:
        case GD32_HAL_UART_FLAG_PE:
            /* 这些标志必须使用 STAT0/DATA 读取序列。 */
            GD32_HAL_UART_ClearErrors(uart_address);
            return;
        case GD32_HAL_UART_FLAG_TX_EMPTY:
        default:
            /* TBE 是只读状态；未知标志也不得通过读取 DATA 产生副作用。 */
            return;
    }
    usart_flag_clear(uart_address, gd_flag);
}

uint32_t GD32_HAL_UART_GetErrorFlags(uint32_t uart_address)
{
    if (GD32_HAL_UART_IsInstance(uart_address) == 0)
    {
        return 0U;
    }
    return USART_STAT0(uart_address) &
           (USART_STAT0_PERR | USART_STAT0_FERR | USART_STAT0_NERR | USART_STAT0_ORERR);
}

void GD32_HAL_UART_ClearErrors(uint32_t uart_address)
{
    volatile uint32_t status;
    volatile uint32_t data;

    if (GD32_HAL_UART_IsInstance(uart_address) == 0)
    {
        return;
    }

    /* User Manual 要求依次读取 STAT0 和 DATA。 */
    status = USART_STAT0(uart_address);
    data = USART_DATA(uart_address);
    (void)status;
    (void)data;
}

void GD32_HAL_UART_WriteData(uint32_t uart_address, uint16_t data)
{
    usart_data_transmit(uart_address, data);
}

uint16_t GD32_HAL_UART_ReadData(uint32_t uart_address)
{
    return usart_data_receive(uart_address);
}

static usart_interrupt_enum GD32_HAL_UART_InterruptValue(GD32_HAL_UARTInterrupt interrupt)
{
    switch (interrupt)
    {
        case GD32_HAL_UART_INTERRUPT_PE:
            return USART_INT_PERR;
        case GD32_HAL_UART_INTERRUPT_TX_EMPTY:
            return USART_INT_TBE;
        case GD32_HAL_UART_INTERRUPT_TX_COMPLETE:
            return USART_INT_TC;
        case GD32_HAL_UART_INTERRUPT_RX_NOT_EMPTY:
            return USART_INT_RBNE;
        case GD32_HAL_UART_INTERRUPT_IDLE:
            return USART_INT_IDLE;
        case GD32_HAL_UART_INTERRUPT_LIN_BREAK:
            return USART_INT_LBD;
        case GD32_HAL_UART_INTERRUPT_CTS:
            return USART_INT_CTS;
        case GD32_HAL_UART_INTERRUPT_ERROR:
        default:
            return USART_INT_ERR;
    }
}

void GD32_HAL_UART_SetInterrupt(uint32_t uart_address,
                               GD32_HAL_UARTInterrupt interrupt,
                               int enable)
{
    const usart_interrupt_enum gd_interrupt = GD32_HAL_UART_InterruptValue(interrupt);

    if (GD32_HAL_UART_IsInstance(uart_address) == 0)
    {
        return;
    }
    if ((interrupt == GD32_HAL_UART_INTERRUPT_CTS) &&
        (GD32_HAL_UART_IsFlowControlCapable(uart_address) == 0))
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_UART_FLOW_CONTROL_UNSUPPORTED,
                           uart_address);
        return;
    }

    if (enable != 0)
    {
        usart_interrupt_enable(uart_address, gd_interrupt);
    }
    else
    {
        usart_interrupt_disable(uart_address, gd_interrupt);
    }
}

int GD32_HAL_UART_IsInterruptEnabled(uint32_t uart_address,
                                    GD32_HAL_UARTInterrupt interrupt)
{
    const usart_interrupt_enum gd_interrupt = GD32_HAL_UART_InterruptValue(interrupt);

    if (GD32_HAL_UART_IsInstance(uart_address) == 0)
    {
        return 0;
    }
    return (USART_REG_VAL(uart_address, gd_interrupt) &
            BIT(USART_BIT_POS(gd_interrupt))) != 0U;
}

void GD32_HAL_UART_DisableDMARequest(uint32_t uart_address, int transmit)
{
    if (GD32_HAL_UART_IsInstance(uart_address) == 0)
    {
        return;
    }
    if (transmit != 0)
    {
        usart_dma_transmit_config(uart_address, USART_TRANSMIT_DMA_DISABLE);
    }
    else
    {
        usart_dma_receive_config(uart_address, USART_RECEIVE_DMA_DISABLE);
    }
}
