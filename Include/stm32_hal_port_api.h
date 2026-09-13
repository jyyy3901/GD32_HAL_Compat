#ifndef STM32_HAL_PORT_API_H
#define STM32_HAL_PORT_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal_conf.h"
#include <stdint.h>

/*
 * 本文件只描述 HAL compatible API 与目标 Port 的窄接口。
 * 它不包含 GD32 CMSIS/SPL 头文件，也不公开寄存器地址。
 */

#define GD32_HAL_DMA_REQUEST_MEMORY 0U
#define GD32_HAL_DMA_REQUEST_ENCODE(CONTROLLER, CHANNEL, SLOT) \
    (0x80000000UL | ((uint32_t)(CONTROLLER) << 16U) | \
     ((uint32_t)(CHANNEL) << 8U) | ((uint32_t)(SLOT) + 1U))


typedef enum
{
    GD32_HAL_RESOURCE_GPIO = 0,
    GD32_HAL_RESOURCE_UART,
    GD32_HAL_RESOURCE_DMA,
    GD32_HAL_RESOURCE_TIMER,
    GD32_HAL_RESOURCE_ADC,
    GD32_HAL_RESOURCE_I2C,
    GD32_HAL_RESOURCE_SPI
} GD32_HAL_ResourceKind;

typedef struct GD32_HAL_Resource
{
    uintptr_t semantic_id;
    uintptr_t stm32_instance;
    uint32_t gd32_instance;
    uint32_t gd32_periph;
    uint32_t gd32_clock;
    int32_t gd32_irq;
    uint32_t capabilities;
    uint8_t logical_index;
    uint8_t gd32_channel;
} GD32_HAL_Resource;

#define GD32_HAL_CAP_FLOW_CONTROL (1UL << 0U)
#define GD32_HAL_CAP_DMA          (1UL << 1U)
#define GD32_HAL_CAP_MASTER_SLAVE (1UL << 2U)
#define GD32_HAL_CAP_TRGO         (1UL << 3U)
#define GD32_HAL_CAP_ADVANCED     (1UL << 4U)
#define GD32_HAL_CAP_CHANNEL_1    (1UL << 8U)
#define GD32_HAL_CAP_CHANNEL_2    (1UL << 9U)
#define GD32_HAL_CAP_CHANNEL_3    (1UL << 10U)
#define GD32_HAL_CAP_CHANNEL_4    (1UL << 11U)

typedef enum
{
    GD32_HAL_GPIO_MODE_ANALOG = 0,
    GD32_HAL_GPIO_MODE_INPUT_FLOATING,
    GD32_HAL_GPIO_MODE_INPUT_PULLUP,
    GD32_HAL_GPIO_MODE_INPUT_PULLDOWN,
    GD32_HAL_GPIO_MODE_OUTPUT_PP,
    GD32_HAL_GPIO_MODE_OUTPUT_OD,
    GD32_HAL_GPIO_MODE_AF_PP,
    GD32_HAL_GPIO_MODE_AF_OD
} GD32_HAL_GPIOMode;

typedef enum
{
    GD32_HAL_GPIO_SPEED_2MHZ = 0,
    GD32_HAL_GPIO_SPEED_10MHZ,
    GD32_HAL_GPIO_SPEED_50MHZ,
    GD32_HAL_GPIO_SPEED_MAX
} GD32_HAL_GPIOSpeed;

typedef enum
{
    GD32_HAL_EXTI_INTERRUPT = 0,
    GD32_HAL_EXTI_EVENT
} GD32_HAL_EXTIMode;

typedef enum
{
    GD32_HAL_EXTI_RISING = 0,
    GD32_HAL_EXTI_FALLING,
    GD32_HAL_EXTI_BOTH
} GD32_HAL_EXTITrigger;

typedef enum
{
    GD32_HAL_RCC_CLOCK_SYS = 0,
    GD32_HAL_RCC_CLOCK_AHB,
    GD32_HAL_RCC_CLOCK_APB1,
    GD32_HAL_RCC_CLOCK_APB2
} GD32_HAL_RCCClock;

typedef enum
{
    GD32_HAL_RCC_COMMON_AF = 0,
    GD32_HAL_RCC_COMMON_PMU,
    GD32_HAL_RCC_COMMON_CRC
} GD32_HAL_RCCCommonClock;

typedef enum
{
    GD32_HAL_RCC_SOURCE_IRC8M = 0,
    GD32_HAL_RCC_SOURCE_HXTAL,
    GD32_HAL_RCC_SOURCE_PLL
} GD32_HAL_RCCSource;

#define GD32_HAL_RCC_HXTAL_OFF 0U
#define GD32_HAL_RCC_HXTAL_ON 1U
#define GD32_HAL_RCC_HXTAL_BYPASS 2U
#define GD32_HAL_RCC_OSC_IRC8M_ON (1UL << 0U)
#define GD32_HAL_RCC_OSC_HXTAL_ON (1UL << 1U)
#define GD32_HAL_RCC_OSC_HXTAL_BYPASS (1UL << 2U)
#define GD32_HAL_RCC_OSC_PLL_ON (1UL << 3U)

typedef enum
{
    GD32_HAL_FLASH_READY = 0,
    GD32_HAL_FLASH_BUSY,
    GD32_HAL_FLASH_PROGRAM_ERROR,
    GD32_HAL_FLASH_WRITE_PROTECT_ERROR,
    GD32_HAL_FLASH_TIMEOUT
} GD32_HAL_FLASHStatus;

typedef enum
{
    GD32_HAL_FLASH_IT_NONE = 0U,
    GD32_HAL_FLASH_IT_END = (1UL << 0U),
    GD32_HAL_FLASH_IT_PROGRAM_ERROR = (1UL << 1U),
    GD32_HAL_FLASH_IT_WRITE_PROTECT_ERROR = (1UL << 2U)
} GD32_HAL_FLASHInterrupt;

typedef enum
{
    GD32_HAL_PORT_ERROR_NONE = 0,
    GD32_HAL_PORT_ERROR_INVALID_INSTANCE,
    GD32_HAL_PORT_ERROR_INVALID_PIN,
    GD32_HAL_PORT_ERROR_OUTPUT_PULL_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_AF_SELECTION_REQUIRES_NATIVE_INIT,
    GD32_HAL_PORT_ERROR_TICK_RELOAD,
    GD32_HAL_PORT_ERROR_UART_INVALID_CONFIG,
    GD32_HAL_PORT_ERROR_UART_OVERSAMPLING_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_UART_FLOW_CONTROL_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_UART_BAUD_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_UART_DMA_PHASE_REQUIRED,
    GD32_HAL_PORT_ERROR_UART_DMA_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_UART_DMA_LINK_INVALID,
    GD32_HAL_PORT_ERROR_UART_DMA_CONFIG_MISMATCH,
    GD32_HAL_PORT_ERROR_DMA_INVALID_CONFIG,
    GD32_HAL_PORT_ERROR_DMA_UNSUPPORTED_CONFIG,
    GD32_HAL_PORT_ERROR_DMA_REQUEST_MISMATCH,
    GD32_HAL_PORT_ERROR_DMA_CHANNEL_CONFLICT,
    GD32_HAL_PORT_ERROR_DMA_UNSUPPORTED_FLAG,
    GD32_HAL_PORT_ERROR_TIMER_INVALID_CONFIG,
    GD32_HAL_PORT_ERROR_TIMER_16BIT_RANGE,
    GD32_HAL_PORT_ERROR_TIMER_COUNTER_MODE_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_TIMER_CHANNEL_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_TIMER_REPETITION_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_TIMER_MASTER_SLAVE_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_TIMER_ITR_UNMAPPABLE,
    GD32_HAL_PORT_ERROR_TIMER_DMA_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_TIMER_DMA_LINK_INVALID,
    GD32_HAL_PORT_ERROR_TIMER_DMA_CONFIG_MISMATCH,
    GD32_HAL_PORT_ERROR_ADC_INVALID_CONFIG,
    GD32_HAL_PORT_ERROR_ADC_CLOCK_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_ADC_TRIGGER_EDGE_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_ADC_TRIGGER_UNMAPPABLE,
    GD32_HAL_PORT_ERROR_ADC_SAMPLE_TIME_UNMAPPABLE,
    GD32_HAL_PORT_ERROR_ADC_CHANNEL_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_ADC_EOC_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_ADC_DMA_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_ADC_DMA_LINK_INVALID,
    GD32_HAL_PORT_ERROR_ADC_DMA_CONFIG_MISMATCH,
    GD32_HAL_PORT_ERROR_ADC_OVERRUN_UNAVAILABLE,
    GD32_HAL_PORT_ERROR_ADC_CALIBRATION_TIMEOUT,
    GD32_HAL_PORT_ERROR_I2C_INVALID_CONFIG,
    GD32_HAL_PORT_ERROR_I2C_TIMEOUT,
    GD32_HAL_PORT_ERROR_I2C_DMA_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_SPI_INVALID_CONFIG,
    GD32_HAL_PORT_ERROR_SPI_DMA_LINK_INVALID,
    GD32_HAL_PORT_ERROR_SPI_DMA_CONFIG_MISMATCH,
    GD32_HAL_PORT_ERROR_SPI_TIMEOUT,
    GD32_HAL_PORT_ERROR_RCC_INVALID_CONFIG,
    GD32_HAL_PORT_ERROR_RCC_PLL_MODEL_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_RCC_CLOCK_LIMIT,
    GD32_HAL_PORT_ERROR_EXTI_INVALID_CONFIG,
    GD32_HAL_PORT_ERROR_FLASH_INVALID_RANGE,
    GD32_HAL_PORT_ERROR_FLASH_WRITABLE_REGION_REQUIRED,
    GD32_HAL_PORT_ERROR_FLASH_PROGRAM_TYPE_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_FLASH_MASS_ERASE_UNSUPPORTED,
    GD32_HAL_PORT_ERROR_FLASH_OPERATION
} GD32_HAL_PortError;

typedef enum
{
    GD32_HAL_I2C_FLAG_START = (1UL << 0U),
    GD32_HAL_I2C_FLAG_ADDRESS = (1UL << 1U),
    GD32_HAL_I2C_FLAG_BYTE_TRANSFER = (1UL << 2U),
    GD32_HAL_I2C_FLAG_ADDRESS10 = (1UL << 3U),
    GD32_HAL_I2C_FLAG_RX_NOT_EMPTY = (1UL << 4U),
    GD32_HAL_I2C_FLAG_TX_EMPTY = (1UL << 5U),
    GD32_HAL_I2C_FLAG_BUSY = (1UL << 6U)
} GD32_HAL_I2CFlag;

#define GD32_HAL_I2C_ERROR_BUS         (1UL << 0U)
#define GD32_HAL_I2C_ERROR_ARBITRATION (1UL << 1U)
#define GD32_HAL_I2C_ERROR_ACK         (1UL << 2U)
#define GD32_HAL_I2C_ERROR_OVERRUN     (1UL << 3U)
#define GD32_HAL_I2C_ERROR_PEC         (1UL << 4U)
#define GD32_HAL_I2C_ERROR_TIMEOUT     (1UL << 5U)

#define GD32_HAL_I2C_INTERRUPT_ERROR (1UL << 0U)
#define GD32_HAL_I2C_INTERRUPT_EVENT (1UL << 1U)
#define GD32_HAL_I2C_INTERRUPT_BUFFER (1UL << 2U)

typedef struct
{
    uint32_t clock_speed;
    uint16_t own_address1;
    uint16_t own_address2;
    uint8_t duty_16_9;
    uint8_t address_10bit;
    uint8_t dual_address;
    uint8_t general_call;
    uint8_t no_stretch;
} GD32_HAL_I2CConfig;

typedef enum
{
    GD32_HAL_SPI_FLAG_RX_NOT_EMPTY = (1UL << 0U),
    GD32_HAL_SPI_FLAG_TX_EMPTY = (1UL << 1U),
    GD32_HAL_SPI_FLAG_BUSY = (1UL << 2U)
} GD32_HAL_SPIFlag;

#define GD32_HAL_SPI_ERROR_MODE    (1UL << 0U)
#define GD32_HAL_SPI_ERROR_CRC     (1UL << 1U)
#define GD32_HAL_SPI_ERROR_OVERRUN (1UL << 2U)
#define GD32_HAL_SPI_ERROR_FRAME   (1UL << 3U)

#define GD32_HAL_SPI_INTERRUPT_TX    (1UL << 0U)
#define GD32_HAL_SPI_INTERRUPT_RX    (1UL << 1U)
#define GD32_HAL_SPI_INTERRUPT_ERROR (1UL << 2U)

typedef struct
{
    uint8_t master;
    uint8_t direction;
    uint8_t data_bits;
    uint8_t clock_polarity_high;
    uint8_t clock_phase_second;
    uint8_t nss;
    uint16_t prescaler;
    uint8_t lsb_first;
    uint8_t ti_mode;
    uint8_t crc_enable;
    uint16_t crc_polynomial;
} GD32_HAL_SPIConfig;

typedef enum
{
    GD32_HAL_ADC_TRIGGER_SOFTWARE = 0,
    GD32_HAL_ADC_TRIGGER_TIMER0_CH0,
    GD32_HAL_ADC_TRIGGER_TIMER0_CH1,
    GD32_HAL_ADC_TRIGGER_TIMER0_CH2,
    GD32_HAL_ADC_TRIGGER_TIMER1_CH1,
    GD32_HAL_ADC_TRIGGER_TIMER2_TRGO,
    GD32_HAL_ADC_TRIGGER_TIMER3_CH3,
    GD32_HAL_ADC_TRIGGER_TIMER7_TRGO,
    GD32_HAL_ADC_TRIGGER_EXTI11
} GD32_HAL_ADCTrigger;

typedef enum
{
    GD32_HAL_ADC_FLAG_EOC = (1UL << 0U),
    GD32_HAL_ADC_FLAG_OVERRUN_UNAVAILABLE = (1UL << 1U)
} GD32_HAL_ADCFlag;

typedef enum
{
    GD32_HAL_ADC_INTERRUPT_EOC = (1UL << 0U),
    GD32_HAL_ADC_INTERRUPT_OVERRUN_UNAVAILABLE = (1UL << 1U)
} GD32_HAL_ADCInterrupt;

typedef struct
{
    uint8_t clock_divider;
    uint8_t resolution_bits;
    uint8_t align_left;
    uint8_t scan;
    uint8_t continuous;
    uint8_t discontinuous;
    uint8_t discontinuous_count;
    uint8_t sequence_length;
    GD32_HAL_ADCTrigger trigger;
} GD32_HAL_ADCConfig;

typedef enum
{
    GD32_HAL_TIMER_ALIGNMENT_EDGE = 0,
    GD32_HAL_TIMER_ALIGNMENT_CENTER_DOWN,
    GD32_HAL_TIMER_ALIGNMENT_CENTER_UP,
    GD32_HAL_TIMER_ALIGNMENT_CENTER_BOTH
} GD32_HAL_TIMERAlignment;

typedef enum
{
    GD32_HAL_TIMER_DIRECTION_UP = 0,
    GD32_HAL_TIMER_DIRECTION_DOWN
} GD32_HAL_TIMERDirection;

typedef enum
{
    GD32_HAL_TIMER_OC_TIMING = 0,
    GD32_HAL_TIMER_OC_ACTIVE,
    GD32_HAL_TIMER_OC_INACTIVE,
    GD32_HAL_TIMER_OC_TOGGLE,
    GD32_HAL_TIMER_OC_FORCED_INACTIVE,
    GD32_HAL_TIMER_OC_FORCED_ACTIVE,
    GD32_HAL_TIMER_OC_PWM1,
    GD32_HAL_TIMER_OC_PWM2
} GD32_HAL_TIMEROCMode;

typedef enum
{
    GD32_HAL_TIMER_POLARITY_RISING_HIGH = 0,
    GD32_HAL_TIMER_POLARITY_FALLING_LOW,
    GD32_HAL_TIMER_POLARITY_BOTH
} GD32_HAL_TIMERPolarity;

typedef enum
{
    GD32_HAL_TIMER_IC_DIRECT = 0,
    GD32_HAL_TIMER_IC_INDIRECT,
    GD32_HAL_TIMER_IC_TRIGGER
} GD32_HAL_TIMERICSelection;

typedef enum
{
    GD32_HAL_TIMER_INTERRUPT_UPDATE = (1UL << 0U),
    GD32_HAL_TIMER_INTERRUPT_CC1 = (1UL << 1U),
    GD32_HAL_TIMER_INTERRUPT_CC2 = (1UL << 2U),
    GD32_HAL_TIMER_INTERRUPT_CC3 = (1UL << 3U),
    GD32_HAL_TIMER_INTERRUPT_CC4 = (1UL << 4U),
    GD32_HAL_TIMER_INTERRUPT_COM = (1UL << 5U),
    GD32_HAL_TIMER_INTERRUPT_TRIGGER = (1UL << 6U),
    GD32_HAL_TIMER_INTERRUPT_BREAK = (1UL << 7U)
} GD32_HAL_TIMERInterrupt;

typedef enum
{
    GD32_HAL_TIMER_FLAG_UPDATE = (1UL << 0U),
    GD32_HAL_TIMER_FLAG_CC1 = (1UL << 1U),
    GD32_HAL_TIMER_FLAG_CC2 = (1UL << 2U),
    GD32_HAL_TIMER_FLAG_CC3 = (1UL << 3U),
    GD32_HAL_TIMER_FLAG_CC4 = (1UL << 4U),
    GD32_HAL_TIMER_FLAG_COM = (1UL << 5U),
    GD32_HAL_TIMER_FLAG_TRIGGER = (1UL << 6U),
    GD32_HAL_TIMER_FLAG_BREAK = (1UL << 7U),
    GD32_HAL_TIMER_FLAG_CC1_OVER = (1UL << 9U),
    GD32_HAL_TIMER_FLAG_CC2_OVER = (1UL << 10U),
    GD32_HAL_TIMER_FLAG_CC3_OVER = (1UL << 11U),
    GD32_HAL_TIMER_FLAG_CC4_OVER = (1UL << 12U)
} GD32_HAL_TIMERFlag;

typedef enum
{
    GD32_HAL_TIMER_DMA_UPDATE = 0,
    GD32_HAL_TIMER_DMA_CC1,
    GD32_HAL_TIMER_DMA_CC2,
    GD32_HAL_TIMER_DMA_CC3,
    GD32_HAL_TIMER_DMA_CC4,
    GD32_HAL_TIMER_DMA_COM,
    GD32_HAL_TIMER_DMA_TRIGGER
} GD32_HAL_TIMERDMARequest;

typedef struct
{
    uint16_t prescaler;
    uint16_t period;
    GD32_HAL_TIMERAlignment alignment;
    GD32_HAL_TIMERDirection direction;
    uint8_t clock_division;
    uint8_t repetition;
    uint8_t auto_reload_preload;
} GD32_HAL_TIMERBaseConfig;

typedef struct
{
    GD32_HAL_TIMEROCMode mode;
    uint16_t pulse;
    GD32_HAL_TIMERPolarity polarity;
    uint8_t fast;
    uint8_t idle_high;
    uint8_t preload;
} GD32_HAL_TIMEROCConfig;

typedef struct
{
    GD32_HAL_TIMERPolarity polarity;
    GD32_HAL_TIMERICSelection selection;
    uint8_t prescaler;
    uint8_t filter;
} GD32_HAL_TIMERICConfig;

typedef struct
{
    uintptr_t stm32_timer_instance;
    uint8_t slave_mode;
    uint8_t input_trigger;
    GD32_HAL_TIMERPolarity trigger_polarity;
    uint8_t trigger_prescaler;
    uint8_t trigger_filter;
} GD32_HAL_TIMERSlaveConfig;

typedef enum
{
    GD32_HAL_UART_PARITY_NONE = 0,
    GD32_HAL_UART_PARITY_EVEN,
    GD32_HAL_UART_PARITY_ODD
} GD32_HAL_UARTParity;

typedef enum
{
    GD32_HAL_UART_INTERRUPT_PE = 0,
    GD32_HAL_UART_INTERRUPT_TX_EMPTY,
    GD32_HAL_UART_INTERRUPT_TX_COMPLETE,
    GD32_HAL_UART_INTERRUPT_RX_NOT_EMPTY,
    GD32_HAL_UART_INTERRUPT_IDLE,
    GD32_HAL_UART_INTERRUPT_LIN_BREAK,
    GD32_HAL_UART_INTERRUPT_CTS,
    GD32_HAL_UART_INTERRUPT_ERROR
} GD32_HAL_UARTInterrupt;

typedef enum
{
    GD32_HAL_UART_FLAG_PE = (1UL << 0U),
    GD32_HAL_UART_FLAG_FE = (1UL << 1U),
    GD32_HAL_UART_FLAG_NE = (1UL << 2U),
    GD32_HAL_UART_FLAG_ORE = (1UL << 3U),
    GD32_HAL_UART_FLAG_IDLE = (1UL << 4U),
    GD32_HAL_UART_FLAG_RX_NOT_EMPTY = (1UL << 5U),
    GD32_HAL_UART_FLAG_TX_COMPLETE = (1UL << 6U),
    GD32_HAL_UART_FLAG_TX_EMPTY = (1UL << 7U),
    GD32_HAL_UART_FLAG_LIN_BREAK = (1UL << 8U),
    GD32_HAL_UART_FLAG_CTS = (1UL << 9U)
} GD32_HAL_UARTFlag;

#define GD32_HAL_UART_ERROR_PE   (1UL << 0U)
#define GD32_HAL_UART_ERROR_FE   (1UL << 1U)
#define GD32_HAL_UART_ERROR_NE   (1UL << 2U)
#define GD32_HAL_UART_ERROR_ORE  (1UL << 3U)

#define GD32_HAL_UART_MODE_TX    (1UL << 0U)
#define GD32_HAL_UART_MODE_RX    (1UL << 1U)
#define GD32_HAL_UART_FLOW_RTS   (1UL << 0U)
#define GD32_HAL_UART_FLOW_CTS   (1UL << 1U)

typedef struct
{
    uint32_t baud_rate;
    uint8_t word_length;
    uint8_t stop_bits;
    GD32_HAL_UARTParity parity;
    uint8_t mode;
    uint8_t flow_control;
} GD32_HAL_UARTConfig;

typedef enum
{
    GD32_HAL_DMA_DIRECTION_PERIPH_TO_MEMORY = 0,
    GD32_HAL_DMA_DIRECTION_MEMORY_TO_PERIPH,
    GD32_HAL_DMA_DIRECTION_MEMORY_TO_MEMORY
} GD32_HAL_DMADirection;

typedef enum
{
    GD32_HAL_DMA_FLAG_FULL = (1UL << 0U),
    GD32_HAL_DMA_FLAG_HALF = (1UL << 1U),
    GD32_HAL_DMA_FLAG_ERROR = (1UL << 2U),
    GD32_HAL_DMA_FLAG_FIFO_UNSUPPORTED = (1UL << 3U),
    GD32_HAL_DMA_FLAG_DIRECT_UNSUPPORTED = (1UL << 4U)
} GD32_HAL_DMAFlag;

typedef enum
{
    GD32_HAL_DMA_INTERRUPT_FULL = (1UL << 0U),
    GD32_HAL_DMA_INTERRUPT_HALF = (1UL << 1U),
    GD32_HAL_DMA_INTERRUPT_ERROR = (1UL << 2U),
    GD32_HAL_DMA_INTERRUPT_FIFO_UNSUPPORTED = (1UL << 3U),
    GD32_HAL_DMA_INTERRUPT_DIRECT_UNSUPPORTED = (1UL << 4U)
} GD32_HAL_DMAInterrupt;

typedef struct
{
    GD32_HAL_DMADirection direction;
    uint8_t periph_increment;
    uint8_t memory_increment;
    uint8_t periph_width;
    uint8_t memory_width;
    uint8_t circular;
    uint8_t priority;
} GD32_HAL_DMAConfig;

/* 默认钩子记录最近错误；产品工程可覆盖并接入断言、日志或安全状态。 */
extern __IO GD32_HAL_PortError gd32HalLastPortError;
extern __IO uint32_t gd32HalLastPortErrorDetail;
void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail);

int GD32_HAL_PortInit(void);
int GD32_HAL_PortDeInit(void);
uint32_t GD32_HAL_ReadUIDWord(uint8_t index);
const GD32_HAL_Resource *GD32_HAL_ResolveInstance(uintptr_t stm32_instance,
                                                  GD32_HAL_ResourceKind kind);
const GD32_HAL_Resource *GD32_HAL_ResolveDMAChannelAddress(
    uint32_t channel_address);
const GD32_HAL_Resource *GD32_HAL_ResolveDMAStream(uintptr_t stream_instance,
                                                   uint32_t stm32_channel,
                                                   uint32_t stm32_direction,
                                                   uint32_t *gd32_request);
int GD32_HAL_DMAStreamHasTimerCandidate(uintptr_t stream_instance,
                                        uint32_t stm32_channel,
                                        uint32_t stm32_direction);
int GD32_HAL_DMAStreamMatchesTimer(
    uintptr_t stream_instance,
    uint32_t stm32_channel,
    uint32_t stm32_direction,
    uint32_t timer_address,
    GD32_HAL_TIMERDMARequest request);

int GD32_HAL_GPIO_IsInstance(uint32_t gpio_address);
int GD32_HAL_GPIO_ArePinsAvailable(uint32_t gpio_address, uint32_t pins);
int GD32_HAL_GPIO_GetPortSource(uint32_t gpio_address, uint8_t *port_source);
void GD32_HAL_GPIO_InitPins(uint32_t gpio_address,
                            GD32_HAL_GPIOMode mode,
                            GD32_HAL_GPIOSpeed speed,
                            uint32_t pins);
void GD32_HAL_GPIO_DeInitPins(uint32_t gpio_address, uint32_t pins);
uint32_t GD32_HAL_GPIO_ReadPins(uint32_t gpio_address, uint32_t pins);
void GD32_HAL_GPIO_WritePins(uint32_t gpio_address, uint32_t pins, int set);
void GD32_HAL_GPIO_TogglePins(uint32_t gpio_address, uint32_t pins);
int GD32_HAL_GPIO_LockPins(uint32_t gpio_address, uint32_t pins);
void GD32_HAL_GPIO_ConfigEXTI(uint32_t gpio_address,
                              uint32_t pins,
                              GD32_HAL_EXTIMode mode,
                              GD32_HAL_EXTITrigger trigger);
uint32_t GD32_HAL_GPIO_EXTIPending(uint32_t pins);
void GD32_HAL_GPIO_EXTIClear(uint32_t pins);
void GD32_HAL_GPIO_EXTIGenerate(uint32_t pins);

int GD32_HAL_UART_IsInstance(uint32_t uart_address);
int GD32_HAL_UART_IsFlowControlCapable(uint32_t uart_address);
int GD32_HAL_UART_IsDMACapable(uint32_t uart_address);
int GD32_HAL_UART_IsDMARequestActive(uint32_t uart_address, int transmit);
int GD32_HAL_UART_IsDMAChannelValid(uint32_t uart_address,
                                    int transmit,
                                    uint32_t channel_address,
                                    uint32_t request);
uint32_t GD32_HAL_UART_GetDataAddress(uint32_t uart_address);
int GD32_HAL_UART_Configure(uint32_t uart_address, const GD32_HAL_UARTConfig *config);
void GD32_HAL_UART_DeInit(uint32_t uart_address);
void GD32_HAL_UART_Enable(uint32_t uart_address);
void GD32_HAL_UART_Disable(uint32_t uart_address);
uint32_t GD32_HAL_UART_GetFlag(uint32_t uart_address, GD32_HAL_UARTFlag flag);
void GD32_HAL_UART_ClearFlag(uint32_t uart_address, GD32_HAL_UARTFlag flag);
uint32_t GD32_HAL_UART_GetErrorFlags(uint32_t uart_address);
void GD32_HAL_UART_ClearErrors(uint32_t uart_address);
void GD32_HAL_UART_WriteData(uint32_t uart_address, uint16_t data);
uint16_t GD32_HAL_UART_ReadData(uint32_t uart_address);
void GD32_HAL_UART_SetInterrupt(uint32_t uart_address,
                               GD32_HAL_UARTInterrupt interrupt,
                               int enable);
int GD32_HAL_UART_IsInterruptEnabled(uint32_t uart_address,
                                    GD32_HAL_UARTInterrupt interrupt);
void GD32_HAL_UART_EnableDMARequest(uint32_t uart_address, int transmit);
void GD32_HAL_UART_DisableDMARequest(uint32_t uart_address, int transmit);
int GD32_HAL_DMA_IsInstance(uint32_t channel_address);
int GD32_HAL_DMA_IsRequestValid(uint32_t channel_address,
                                uint32_t request,
                                GD32_HAL_DMADirection direction);
uint32_t GD32_HAL_DMA_GetControllerAddress(uint32_t channel_address);
uint32_t GD32_HAL_DMA_GetChannelIndex(uint32_t channel_address);
int GD32_HAL_DMA_Claim(uint32_t channel_address, const void *owner);
void GD32_HAL_DMA_Release(uint32_t channel_address, const void *owner);
void GD32_HAL_DMA_ResetOwnership(void);
int GD32_HAL_DMA_Configure(uint32_t channel_address,
                           const GD32_HAL_DMAConfig *config);
void GD32_HAL_DMA_DeInit(uint32_t channel_address);
void GD32_HAL_DMA_SetTransfer(uint32_t channel_address,
                              GD32_HAL_DMADirection direction,
                              uint32_t source,
                              uint32_t destination,
                              uint32_t count);
void GD32_HAL_DMA_Enable(uint32_t channel_address);
void GD32_HAL_DMA_Disable(uint32_t channel_address);
int GD32_HAL_DMA_IsEnabled(uint32_t channel_address);
uint32_t GD32_HAL_DMA_GetFlag(uint32_t channel_address, uint32_t flags);
void GD32_HAL_DMA_ClearFlag(uint32_t channel_address, uint32_t flags);
void GD32_HAL_DMA_SetInterrupt(uint32_t channel_address,
                               uint32_t interrupts,
                               int enable);
uint32_t GD32_HAL_DMA_GetInterrupts(uint32_t channel_address);
void GD32_HAL_DMA_SetRemaining(uint32_t channel_address, uint32_t count);
uint32_t GD32_HAL_DMA_GetRemaining(uint32_t channel_address);
int GD32_HAL_TIMER_IsInstance(uint32_t timer_address);
int GD32_HAL_TIMER_IsChannelValid(uint32_t timer_address, uint8_t channel);
int GD32_HAL_TIMER_IsOnePulseCapable(uint32_t timer_address);
int GD32_HAL_TIMER_IsMasterSlaveCapable(uint32_t timer_address);
int GD32_HAL_TIMER_ConfigureBase(uint32_t timer_address,
                                 const GD32_HAL_TIMERBaseConfig *config);
void GD32_HAL_TIMER_DeInit(uint32_t timer_address);
void GD32_HAL_TIMER_Enable(uint32_t timer_address);
void GD32_HAL_TIMER_Disable(uint32_t timer_address);
void GD32_HAL_TIMER_DisableIfIdle(uint32_t timer_address);
int GD32_HAL_TIMER_IsEnabled(uint32_t timer_address);
int GD32_HAL_TIMER_IsTriggerSlaveMode(uint32_t timer_address);
void GD32_HAL_TIMER_SetInternalClock(uint32_t timer_address);
int GD32_HAL_TIMER_ConfigureOutput(uint32_t timer_address,
                                   uint8_t channel,
                                   const GD32_HAL_TIMEROCConfig *config);
int GD32_HAL_TIMER_ConfigureInput(uint32_t timer_address,
                                  uint8_t channel,
                                  const GD32_HAL_TIMERICConfig *config);
void GD32_HAL_TIMER_SetChannel(uint32_t timer_address,
                               uint8_t channel,
                               int enable);
int GD32_HAL_TIMER_IsChannelInput(uint32_t timer_address, uint8_t channel);
void GD32_HAL_TIMER_SetOnePulse(uint32_t timer_address, int single);
int GD32_HAL_TIMER_ConfigureMaster(uint32_t timer_address,
                                   uint8_t trigger_output,
                                   int master_slave_enable);
int GD32_HAL_TIMER_ConfigureSlave(uint32_t timer_address,
                                  const GD32_HAL_TIMERSlaveConfig *config);
int STM32_TIMER_TriggerToGD32ITI(uintptr_t stm32_timer_instance,
                                 uint8_t stm_itr,
                                 uint8_t *gd_itr);
void GD32_HAL_TIMER_SetInterrupt(uint32_t timer_address,
                                 uint32_t interrupts,
                                 int enable);
uint32_t GD32_HAL_TIMER_GetInterrupts(uint32_t timer_address);
uint32_t GD32_HAL_TIMER_GetFlag(uint32_t timer_address, uint32_t flags);
void GD32_HAL_TIMER_ClearFlag(uint32_t timer_address, uint32_t flags);
void GD32_HAL_TIMER_SetDMARequest(uint32_t timer_address,
                                  GD32_HAL_TIMERDMARequest request,
                                  int enable);
int GD32_HAL_TIMER_IsDMAChannelValid(uint32_t timer_address,
                                     GD32_HAL_TIMERDMARequest request,
                                     uint32_t channel_address,
                                     uint32_t request_token);
int GD32_HAL_TIMER_GetDMAMapping(uint32_t timer_address,
                                 GD32_HAL_TIMERDMARequest request,
                                 uint32_t *channel_address,
                                 uint32_t *request_token);
uint32_t GD32_HAL_TIMER_GetDMADataAddress(uint32_t timer_address,
                                          GD32_HAL_TIMERDMARequest request);
void GD32_HAL_TIMER_SetCounter(uint32_t timer_address, uint32_t value);
uint32_t GD32_HAL_TIMER_GetCounter(uint32_t timer_address);
void GD32_HAL_TIMER_SetAutoReload(uint32_t timer_address, uint32_t value);
uint32_t GD32_HAL_TIMER_GetAutoReload(uint32_t timer_address);
void GD32_HAL_TIMER_SetPrescaler(uint32_t timer_address, uint32_t value);
uint32_t GD32_HAL_TIMER_GetPrescaler(uint32_t timer_address);
void GD32_HAL_TIMER_SetCompare(uint32_t timer_address,
                               uint8_t channel,
                               uint32_t value);
uint32_t GD32_HAL_TIMER_GetCompare(uint32_t timer_address, uint8_t channel);
int GD32_HAL_ADC_IsInstance(uint32_t adc_address);
int GD32_HAL_ADC_MapSTM32Trigger(uint32_t stm32_source,
                                 uint32_t stm32_edge,
                                 GD32_HAL_ADCTrigger *trigger);
int GD32_HAL_ADC_MapSTM32SampleTime(uint32_t stm32_sample_time,
                                    uint8_t *gd32_sample_index);
int GD32_HAL_ADC_Configure(uint32_t adc_address,
                           const GD32_HAL_ADCConfig *config);
void GD32_HAL_ADC_DeInit(uint32_t adc_address);
int GD32_HAL_ADC_ConfigChannel(uint32_t adc_address,
                               uint8_t rank,
                               uint8_t channel,
                               uint8_t sample_time);
int GD32_HAL_ADC_EnableAndCalibrate(uint32_t adc_address);
void GD32_HAL_ADC_Disable(uint32_t adc_address);
int GD32_HAL_ADC_IsEnabled(uint32_t adc_address);
int GD32_HAL_ADC_IsCalibrationValid(uint32_t adc_address);
void GD32_HAL_ADC_SetCalibrationValid(uint32_t adc_address);
void GD32_HAL_ADC_InvalidateCalibration(uint32_t adc_address);
void GD32_HAL_ADC_StartSoftware(uint32_t adc_address);
uint32_t GD32_HAL_ADC_GetFlag(uint32_t adc_address, uint32_t flags);
void GD32_HAL_ADC_ClearFlag(uint32_t adc_address, uint32_t flags);
void GD32_HAL_ADC_SetInterrupt(uint32_t adc_address,
                               uint32_t interrupts,
                               int enable);
uint32_t GD32_HAL_ADC_GetInterrupts(uint32_t adc_address);
uint32_t GD32_HAL_ADC_ReadData(uint32_t adc_address);
uint32_t GD32_HAL_ADC_GetDataAddress(uint32_t adc_address);
void GD32_HAL_ADC_SetDMARequest(uint32_t adc_address, int enable);
int GD32_HAL_ADC_IsDMAChannelValid(uint32_t adc_address,
                                   uint32_t channel_address,
                                   uint32_t request_token);
int GD32_HAL_I2C_IsInstance(uint32_t i2c_address);
int GD32_HAL_I2C_Configure(uint32_t i2c_address,
                           const GD32_HAL_I2CConfig *config);
void GD32_HAL_I2C_DeInit(uint32_t i2c_address);
void GD32_HAL_I2C_Enable(uint32_t i2c_address);
void GD32_HAL_I2C_Disable(uint32_t i2c_address);
void GD32_HAL_I2C_Start(uint32_t i2c_address);
void GD32_HAL_I2C_Stop(uint32_t i2c_address);
void GD32_HAL_I2C_SetAck(uint32_t i2c_address, int enable);
void GD32_HAL_I2C_SetAckNext(uint32_t i2c_address, int next);
void GD32_HAL_I2C_SendAddress(uint32_t i2c_address,
                              uint16_t address,
                              int receive);
void GD32_HAL_I2C_WriteData(uint32_t i2c_address, uint8_t data);
uint8_t GD32_HAL_I2C_ReadData(uint32_t i2c_address);
uint32_t GD32_HAL_I2C_GetFlags(uint32_t i2c_address);
void GD32_HAL_I2C_ClearAddress(uint32_t i2c_address);
uint32_t GD32_HAL_I2C_GetErrors(uint32_t i2c_address);
void GD32_HAL_I2C_ClearErrors(uint32_t i2c_address, uint32_t errors);
void GD32_HAL_I2C_SetInterrupts(uint32_t i2c_address,
                                uint32_t interrupts,
                                int enable);
uint32_t GD32_HAL_I2C_GetInterrupts(uint32_t i2c_address);
void GD32_HAL_I2C_SoftwareReset(uint32_t i2c_address);
int GD32_HAL_SPI_IsInstance(uint32_t spi_address);
int GD32_HAL_SPI_Configure(uint32_t spi_address,
                           const GD32_HAL_SPIConfig *config);
void GD32_HAL_SPI_DeInit(uint32_t spi_address);
void GD32_HAL_SPI_Enable(uint32_t spi_address);
void GD32_HAL_SPI_Disable(uint32_t spi_address);
void GD32_HAL_SPI_SetDirection(uint32_t spi_address, uint8_t direction);
void GD32_HAL_SPI_WriteData(uint32_t spi_address, uint16_t data);
uint16_t GD32_HAL_SPI_ReadData(uint32_t spi_address);
uint32_t GD32_HAL_SPI_GetFlags(uint32_t spi_address);
uint32_t GD32_HAL_SPI_GetErrors(uint32_t spi_address);
void GD32_HAL_SPI_ClearErrors(uint32_t spi_address, uint32_t errors);
void GD32_HAL_SPI_SetInterrupts(uint32_t spi_address,
                                uint32_t interrupts,
                                int enable);
uint32_t GD32_HAL_SPI_GetInterrupts(uint32_t spi_address);
void GD32_HAL_SPI_SetDMARequest(uint32_t spi_address,
                                int transmit,
                                int enable);
uint32_t GD32_HAL_SPI_GetDataAddress(uint32_t spi_address);
int GD32_HAL_SPI_IsDMAChannelValid(uint32_t spi_address,
                                   int transmit,
                                   uint32_t channel_address,
                                   uint32_t request_token);
void GD32_HAL_RCC_DeInit(void);
int GD32_HAL_RCC_SetInstanceClock(uintptr_t stm32_instance,
                                  GD32_HAL_ResourceKind kind,
                                  int enable);
int GD32_HAL_RCC_SetCommonClock(GD32_HAL_RCCCommonClock clock, int enable);
int GD32_HAL_RCC_SetIRC8M(int enable, uint32_t calibration);
int GD32_HAL_RCC_SetHXTAL(uint32_t state);
int GD32_HAL_RCC_SetPLL(int enable);
uint32_t GD32_HAL_RCC_GetOscillatorState(void);
uint32_t GD32_HAL_RCC_GetIRC8MCalibration(void);
uint32_t GD32_HAL_RCC_GetSystemSource(void);
uint32_t GD32_HAL_RCC_GetSourceFrequency(uint32_t source);
uint32_t GD32_HAL_RCC_GetClockFrequency(GD32_HAL_RCCClock clock);
uint32_t GD32_HAL_RCC_GetAHBDivider(void);
uint32_t GD32_HAL_RCC_GetAPB1Divider(void);
uint32_t GD32_HAL_RCC_GetAPB2Divider(void);
int GD32_HAL_RCC_ApplyClockConfig(uint32_t source,
                                  uint32_t ahb_divider,
                                  uint32_t apb1_divider,
                                  uint32_t apb2_divider,
                                  int increasing);
void GD32_HAL_RCC_SetFlashWaitState(uint32_t wait_state);
uint32_t GD32_HAL_RCC_GetFlashWaitState(void);
void GD32_HAL_RCC_EnableClockMonitor(int enable);
uint32_t GD32_HAL_RCC_GetClockMonitorInterrupt(void);
void GD32_HAL_RCC_ClearClockMonitorInterrupt(void);
int GD32_HAL_EXTI_Configure(uint8_t line,
                            uint32_t mode,
                            uint32_t trigger,
                            uint32_t gpio_source);
void GD32_HAL_EXTI_GetConfig(uint8_t line,
                             uint32_t *mode,
                             uint32_t *trigger,
                             uint32_t *gpio_source);
void GD32_HAL_EXTI_ClearConfig(uint8_t line);
uint32_t GD32_HAL_EXTI_GetPending(uint8_t line);
void GD32_HAL_EXTI_ClearPending(uint8_t line);
void GD32_HAL_EXTI_GenerateSWI(uint8_t line);
void GD32_HAL_FLASH_Unlock(void);
void GD32_HAL_FLASH_Lock(void);
GD32_HAL_FLASHStatus GD32_HAL_FLASH_ProgramHalfWord(uint32_t address,
                                                    uint16_t data);
GD32_HAL_FLASHStatus GD32_HAL_FLASH_ProgramWord(uint32_t address,
                                                uint32_t data);
GD32_HAL_FLASHStatus GD32_HAL_FLASH_ErasePage(uint32_t page_address);
GD32_HAL_FLASHStatus GD32_HAL_FLASH_StartProgramIT(uint32_t address,
                                                   uint32_t data,
                                                   uint32_t width);
GD32_HAL_FLASHStatus GD32_HAL_FLASH_StartErasePageIT(uint32_t page_address);
uint32_t GD32_HAL_FLASH_GetInterrupts(void);
void GD32_HAL_FLASH_ClearInterrupts(uint32_t interrupts);
void GD32_HAL_FLASH_EndITOperation(void);
void GD32_HAL_SysTickIRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* STM32_HAL_PORT_API_H */
