#ifndef STM32F4XX_HAL_DMA_H
#define STM32F4XX_HAL_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal_def.h"

/* Instance 保持不透明，禁止上层按 STM32 Stream 寄存器布局访问。 */
typedef struct GD32_HAL_DMA_Stream_TypeDef DMA_Stream_TypeDef;

#define DMA1_Stream0 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA1_STREAM0)
#define DMA1_Stream1 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA1_STREAM1)
#define DMA1_Stream2 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA1_STREAM2)
#define DMA1_Stream3 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA1_STREAM3)
#define DMA1_Stream4 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA1_STREAM4)
#define DMA1_Stream5 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA1_STREAM5)
#define DMA1_Stream6 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA1_STREAM6)
#define DMA1_Stream7 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA1_STREAM7)
#define DMA2_Stream0 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA2_STREAM0)
#define DMA2_Stream1 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA2_STREAM1)
#define DMA2_Stream2 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA2_STREAM2)
#define DMA2_Stream3 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA2_STREAM3)
#define DMA2_Stream4 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA2_STREAM4)
#define DMA2_Stream5 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA2_STREAM5)
#define DMA2_Stream6 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA2_STREAM6)
#define DMA2_Stream7 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA2_STREAM7)

#define GD32_DMA0_CHANNEL0 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA0_CHANNEL0)
#define GD32_DMA0_CHANNEL1 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA0_CHANNEL1)
#define GD32_DMA0_CHANNEL2 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA0_CHANNEL2)
#define GD32_DMA0_CHANNEL3 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA0_CHANNEL3)
#define GD32_DMA0_CHANNEL4 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA0_CHANNEL4)
#define GD32_DMA0_CHANNEL5 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA0_CHANNEL5)
#define GD32_DMA0_CHANNEL6 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA0_CHANNEL6)
#define GD32_DMA1_CHANNEL0 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA1_CHANNEL0)
#define GD32_DMA1_CHANNEL1 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA1_CHANNEL1)
#define GD32_DMA1_CHANNEL2 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA1_CHANNEL2)
#define GD32_DMA1_CHANNEL3 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA1_CHANNEL3)
#define GD32_DMA1_CHANNEL4 ((DMA_Stream_TypeDef *)(uintptr_t)STM32_DMA1_CHANNEL4)

/* M2M 可使用任一物理 Channel；外设请求必须使用下列显式令牌。 */
#define GD32_DMA_REQUEST_MEMORY GD32_HAL_DMA_REQUEST_MEMORY

#define GD32_DMA_REQUEST_ADC0           GD32_HAL_DMA_REQUEST_ENCODE(0U, 0U, 0U)
#define GD32_DMA_REQUEST_TIMER1_CH2      GD32_HAL_DMA_REQUEST_ENCODE(0U, 0U, 1U)
#define GD32_DMA_REQUEST_TIMER3_CH0      GD32_HAL_DMA_REQUEST_ENCODE(0U, 0U, 2U)
#define GD32_DMA_REQUEST_SPI0_RX         GD32_HAL_DMA_REQUEST_ENCODE(0U, 1U, 0U)
#define GD32_DMA_REQUEST_USART2_TX       GD32_HAL_DMA_REQUEST_ENCODE(0U, 1U, 1U)
#define GD32_DMA_REQUEST_TIMER0_CH0      GD32_HAL_DMA_REQUEST_ENCODE(0U, 1U, 2U)
#define GD32_DMA_REQUEST_TIMER1_UP       GD32_HAL_DMA_REQUEST_ENCODE(0U, 1U, 3U)
#define GD32_DMA_REQUEST_TIMER2_CH2      GD32_HAL_DMA_REQUEST_ENCODE(0U, 1U, 4U)
#define GD32_DMA_REQUEST_SPI0_TX         GD32_HAL_DMA_REQUEST_ENCODE(0U, 2U, 0U)
#define GD32_DMA_REQUEST_USART2_RX       GD32_HAL_DMA_REQUEST_ENCODE(0U, 2U, 1U)
#define GD32_DMA_REQUEST_TIMER0_CH1      GD32_HAL_DMA_REQUEST_ENCODE(0U, 2U, 2U)
#define GD32_DMA_REQUEST_TIMER2_CH3      GD32_HAL_DMA_REQUEST_ENCODE(0U, 2U, 3U)
#define GD32_DMA_REQUEST_TIMER2_UP       GD32_HAL_DMA_REQUEST_ENCODE(0U, 2U, 4U)
#define GD32_DMA_REQUEST_SPI1_I2S1_RX    GD32_HAL_DMA_REQUEST_ENCODE(0U, 3U, 0U)
#define GD32_DMA_REQUEST_USART0_TX       GD32_HAL_DMA_REQUEST_ENCODE(0U, 3U, 1U)
#define GD32_DMA_REQUEST_I2C1_TX         GD32_HAL_DMA_REQUEST_ENCODE(0U, 3U, 2U)
#define GD32_DMA_REQUEST_TIMER0_CH3      GD32_HAL_DMA_REQUEST_ENCODE(0U, 3U, 3U)
#define GD32_DMA_REQUEST_TIMER0_TRG      GD32_HAL_DMA_REQUEST_ENCODE(0U, 3U, 4U)
#define GD32_DMA_REQUEST_TIMER0_COM      GD32_HAL_DMA_REQUEST_ENCODE(0U, 3U, 5U)
#define GD32_DMA_REQUEST_TIMER3_CH1      GD32_HAL_DMA_REQUEST_ENCODE(0U, 3U, 6U)
#define GD32_DMA_REQUEST_SPI1_I2S1_TX    GD32_HAL_DMA_REQUEST_ENCODE(0U, 4U, 0U)
#define GD32_DMA_REQUEST_USART0_RX       GD32_HAL_DMA_REQUEST_ENCODE(0U, 4U, 1U)
#define GD32_DMA_REQUEST_I2C1_RX         GD32_HAL_DMA_REQUEST_ENCODE(0U, 4U, 2U)
#define GD32_DMA_REQUEST_TIMER0_UP       GD32_HAL_DMA_REQUEST_ENCODE(0U, 4U, 3U)
#define GD32_DMA_REQUEST_TIMER1_CH0      GD32_HAL_DMA_REQUEST_ENCODE(0U, 4U, 4U)
#define GD32_DMA_REQUEST_TIMER3_CH2      GD32_HAL_DMA_REQUEST_ENCODE(0U, 4U, 5U)
#define GD32_DMA_REQUEST_USART1_RX       GD32_HAL_DMA_REQUEST_ENCODE(0U, 5U, 0U)
#define GD32_DMA_REQUEST_I2C0_TX         GD32_HAL_DMA_REQUEST_ENCODE(0U, 5U, 1U)
#define GD32_DMA_REQUEST_TIMER0_CH2      GD32_HAL_DMA_REQUEST_ENCODE(0U, 5U, 2U)
#define GD32_DMA_REQUEST_TIMER2_CH0      GD32_HAL_DMA_REQUEST_ENCODE(0U, 5U, 3U)
#define GD32_DMA_REQUEST_TIMER2_TRG      GD32_HAL_DMA_REQUEST_ENCODE(0U, 5U, 4U)
#define GD32_DMA_REQUEST_USART1_TX       GD32_HAL_DMA_REQUEST_ENCODE(0U, 6U, 0U)
#define GD32_DMA_REQUEST_I2C0_RX         GD32_HAL_DMA_REQUEST_ENCODE(0U, 6U, 1U)
#define GD32_DMA_REQUEST_TIMER1_CH1      GD32_HAL_DMA_REQUEST_ENCODE(0U, 6U, 2U)
#define GD32_DMA_REQUEST_TIMER1_CH3      GD32_HAL_DMA_REQUEST_ENCODE(0U, 6U, 3U)
#define GD32_DMA_REQUEST_TIMER3_UP       GD32_HAL_DMA_REQUEST_ENCODE(0U, 6U, 4U)

#define GD32_DMA_REQUEST_SPI2_I2S2_RX    GD32_HAL_DMA_REQUEST_ENCODE(1U, 0U, 0U)
#define GD32_DMA_REQUEST_TIMER4_CH3      GD32_HAL_DMA_REQUEST_ENCODE(1U, 0U, 1U)
#define GD32_DMA_REQUEST_TIMER4_TRG      GD32_HAL_DMA_REQUEST_ENCODE(1U, 0U, 2U)
#define GD32_DMA_REQUEST_TIMER7_CH2      GD32_HAL_DMA_REQUEST_ENCODE(1U, 0U, 3U)
#define GD32_DMA_REQUEST_TIMER7_UP       GD32_HAL_DMA_REQUEST_ENCODE(1U, 0U, 4U)
#define GD32_DMA_REQUEST_SPI2_I2S2_TX    GD32_HAL_DMA_REQUEST_ENCODE(1U, 1U, 0U)
#define GD32_DMA_REQUEST_TIMER4_CH2      GD32_HAL_DMA_REQUEST_ENCODE(1U, 1U, 1U)
#define GD32_DMA_REQUEST_TIMER4_UP       GD32_HAL_DMA_REQUEST_ENCODE(1U, 1U, 2U)
#define GD32_DMA_REQUEST_TIMER7_CH3      GD32_HAL_DMA_REQUEST_ENCODE(1U, 1U, 3U)
#define GD32_DMA_REQUEST_TIMER7_TRG      GD32_HAL_DMA_REQUEST_ENCODE(1U, 1U, 4U)
#define GD32_DMA_REQUEST_TIMER7_COM      GD32_HAL_DMA_REQUEST_ENCODE(1U, 1U, 5U)
#define GD32_DMA_REQUEST_UART3_RX        GD32_HAL_DMA_REQUEST_ENCODE(1U, 2U, 0U)
#define GD32_DMA_REQUEST_TIMER5_UP       GD32_HAL_DMA_REQUEST_ENCODE(1U, 2U, 1U)
#define GD32_DMA_REQUEST_DAC_CH0         GD32_HAL_DMA_REQUEST_ENCODE(1U, 2U, 2U)
#define GD32_DMA_REQUEST_TIMER7_CH0      GD32_HAL_DMA_REQUEST_ENCODE(1U, 2U, 3U)
#define GD32_DMA_REQUEST_SDIO            GD32_HAL_DMA_REQUEST_ENCODE(1U, 3U, 0U)
#define GD32_DMA_REQUEST_TIMER4_CH1      GD32_HAL_DMA_REQUEST_ENCODE(1U, 3U, 1U)
#define GD32_DMA_REQUEST_TIMER6_UP       GD32_HAL_DMA_REQUEST_ENCODE(1U, 3U, 2U)
#define GD32_DMA_REQUEST_DAC_CH1         GD32_HAL_DMA_REQUEST_ENCODE(1U, 3U, 3U)
#define GD32_DMA_REQUEST_ADC2            GD32_HAL_DMA_REQUEST_ENCODE(1U, 4U, 0U)
#define GD32_DMA_REQUEST_UART3_TX        GD32_HAL_DMA_REQUEST_ENCODE(1U, 4U, 1U)
#define GD32_DMA_REQUEST_TIMER4_CH0      GD32_HAL_DMA_REQUEST_ENCODE(1U, 4U, 2U)
#define GD32_DMA_REQUEST_TIMER7_CH1      GD32_HAL_DMA_REQUEST_ENCODE(1U, 4U, 3U)

typedef struct
{
    uint32_t Channel;
    uint32_t Direction;
    uint32_t PeriphInc;
    uint32_t MemInc;
    uint32_t PeriphDataAlignment;
    uint32_t MemDataAlignment;
    uint32_t Mode;
    uint32_t Priority;
    uint32_t FIFOMode;
    uint32_t FIFOThreshold;
    uint32_t MemBurst;
    uint32_t PeriphBurst;
} DMA_InitTypeDef;

typedef enum
{
    HAL_DMA_STATE_RESET = 0x00U,
    HAL_DMA_STATE_READY = 0x01U,
    HAL_DMA_STATE_BUSY = 0x02U,
    HAL_DMA_STATE_TIMEOUT = 0x03U,
    HAL_DMA_STATE_ERROR = 0x04U,
    HAL_DMA_STATE_ABORT = 0x05U
} HAL_DMA_StateTypeDef;

typedef enum
{
    HAL_DMA_FULL_TRANSFER = 0x00U,
    HAL_DMA_HALF_TRANSFER = 0x01U
} HAL_DMA_LevelCompleteTypeDef;

typedef enum
{
    HAL_DMA_XFER_CPLT_CB_ID = 0x00U,
    HAL_DMA_XFER_HALFCPLT_CB_ID = 0x01U,
    HAL_DMA_XFER_M1CPLT_CB_ID = 0x02U,
    HAL_DMA_XFER_M1HALFCPLT_CB_ID = 0x03U,
    HAL_DMA_XFER_ERROR_CB_ID = 0x04U,
    HAL_DMA_XFER_ABORT_CB_ID = 0x05U,
    HAL_DMA_XFER_ALL_CB_ID = 0x06U
} HAL_DMA_CallbackIDTypeDef;

typedef enum
{
    GD32_HAL_DMA_MAPPING_UNINITIALIZED = 0x00U,
    GD32_HAL_DMA_MAPPING_NATIVE = 0x01U,
    GD32_HAL_DMA_MAPPING_STM32_UNIQUE = 0x02U,
    GD32_HAL_DMA_MAPPING_STM32_TIMER = 0x03U
} GD32_HAL_DMAMappingOrigin;

#define GD32_HAL_DMA_ACTIVE_TIM_NONE 0xFFU

#ifndef GD32_HAL_DMA_HANDLE_TYPEDEF
#define GD32_HAL_DMA_HANDLE_TYPEDEF
typedef struct __DMA_HandleTypeDef DMA_HandleTypeDef;
#endif

struct __DMA_HandleTypeDef
{
    DMA_Stream_TypeDef *Instance;
    DMA_InitTypeDef Init;
    HAL_LockTypeDef Lock;
    __IO HAL_DMA_StateTypeDef State;
    void *Parent;
    void (*XferCpltCallback)(DMA_HandleTypeDef *hdma);
    void (*XferHalfCpltCallback)(DMA_HandleTypeDef *hdma);
    void (*XferM1CpltCallback)(DMA_HandleTypeDef *hdma);
    void (*XferM1HalfCpltCallback)(DMA_HandleTypeDef *hdma);
    void (*XferErrorCallback)(DMA_HandleTypeDef *hdma);
    void (*XferAbortCallback)(DMA_HandleTypeDef *hdma);
    __IO uint32_t ErrorCode;
    uint32_t StreamBaseAddress;
    uint32_t StreamIndex;
    uint32_t GD32_INSTANCE;
    uint32_t gd32_dma_periph;
    uint32_t gd32_dma_channel;
    uint32_t GD32_REQUEST;
    uintptr_t GD32_RESOLVED_FROM;
    int32_t GD32_IRQ_NUMBER;
    const GD32_HAL_Resource *GD32_RESOURCE;
    uint8_t GD32_DEFERRED;
    GD32_HAL_DMAMappingOrigin GD32_MAPPING_ORIGIN;
    uint8_t GD32_ACTIVE_TIM_DMA_ID;
};

/*
 * 外设 HAL 只消费映射结果，不解释 GD32 DMA 寄存器布局。
 * 已初始化 Handle 走缓存；静态初始化 Handle 仍可由语义令牌安全解析。
 */
static inline uint32_t GD32_HAL_DMA_MappedInstance(const DMA_HandleTypeDef *hdma)
{
    const GD32_HAL_Resource *resource;

    if (hdma == NULL)
    {
        return 0U;
    }
    if ((hdma->GD32_RESOURCE != NULL) &&
        (hdma->GD32_RESOLVED_FROM == (uintptr_t)hdma->Instance))
    {
        return hdma->GD32_INSTANCE;
    }
    resource = GD32_HAL_ResolveInstance((uintptr_t)hdma->Instance,
                                        GD32_HAL_RESOURCE_DMA);
    return (resource != NULL) ? resource->gd32_instance : 0U;
}

static inline uint32_t GD32_HAL_DMA_MappedRequest(const DMA_HandleTypeDef *hdma)
{
    if (hdma == NULL)
    {
        return GD32_HAL_DMA_REQUEST_MEMORY;
    }
    if ((hdma->GD32_RESOURCE != NULL) &&
        (hdma->GD32_RESOLVED_FROM == (uintptr_t)hdma->Instance))
    {
        return hdma->GD32_REQUEST;
    }
    return hdma->Init.Channel;
}

#define HAL_DMA_ERROR_NONE             0x00000000U
#define HAL_DMA_ERROR_TE               0x00000001U
#define HAL_DMA_ERROR_FE               0x00000002U
#define HAL_DMA_ERROR_DME              0x00000004U
#define HAL_DMA_ERROR_TIMEOUT          0x00000020U
#define HAL_DMA_ERROR_PARAM            0x00000040U
#define HAL_DMA_ERROR_NO_XFER          0x00000080U
#define HAL_DMA_ERROR_NOT_SUPPORTED    0x00000100U
#define HAL_DMA_ERROR_CHANNEL_CONFLICT 0x00000200U
#define HAL_DMA_ERROR_REQUEST          0x00000400U

#define DMA_CHANNEL_0 0x00000000U
#define DMA_CHANNEL_1 0x02000000U
#define DMA_CHANNEL_2 0x04000000U
#define DMA_CHANNEL_3 0x06000000U
#define DMA_CHANNEL_4 0x08000000U
#define DMA_CHANNEL_5 0x0A000000U
#define DMA_CHANNEL_6 0x0C000000U
#define DMA_CHANNEL_7 0x0E000000U

#define DMA_PERIPH_TO_MEMORY 0x00000000U
#define DMA_MEMORY_TO_PERIPH 0x00000040U
#define DMA_MEMORY_TO_MEMORY 0x00000080U
#define DMA_PINC_DISABLE     0x00000000U
#define DMA_PINC_ENABLE      0x00000200U
#define DMA_MINC_DISABLE     0x00000000U
#define DMA_MINC_ENABLE      0x00000400U
#define DMA_PDATAALIGN_BYTE     0x00000000U
#define DMA_PDATAALIGN_HALFWORD 0x00000800U
#define DMA_PDATAALIGN_WORD     0x00001000U
#define DMA_MDATAALIGN_BYTE     0x00000000U
#define DMA_MDATAALIGN_HALFWORD 0x00002000U
#define DMA_MDATAALIGN_WORD     0x00004000U
#define DMA_NORMAL   0x00000000U
#define DMA_CIRCULAR 0x00000100U
#define DMA_PFCTRL   0x00000020U

/* 复用 GD32 SPL 的同名优先级值，Host Mock 下补齐相同编码。 */
#ifndef DMA_PRIORITY_LOW
#define DMA_PRIORITY_LOW       0x00000000U
#define DMA_PRIORITY_MEDIUM    0x00001000U
#define DMA_PRIORITY_HIGH      0x00002000U
#define DMA_PRIORITY_ULTRA_HIGH 0x00003000U
#endif
#define DMA_PRIORITY_VERY_HIGH DMA_PRIORITY_ULTRA_HIGH

#define DMA_FIFOMODE_DISABLE 0x00000000U
#define DMA_FIFOMODE_ENABLE  0x00000004U
#define DMA_FIFO_THRESHOLD_1QUARTERFULL  0x00000000U
#define DMA_FIFO_THRESHOLD_HALFFULL      0x00000001U
#define DMA_FIFO_THRESHOLD_3QUARTERSFULL 0x00000002U
#define DMA_FIFO_THRESHOLD_FULL          0x00000003U
#define DMA_MBURST_SINGLE 0x00000000U
#define DMA_MBURST_INC4   0x00800000U
#define DMA_MBURST_INC8   0x01000000U
#define DMA_MBURST_INC16  0x01800000U
#define DMA_PBURST_SINGLE 0x00000000U
#define DMA_PBURST_INC4   0x00200000U
#define DMA_PBURST_INC8   0x00400000U
#define DMA_PBURST_INC16  0x00600000U

#define DMA_IT_TC  GD32_HAL_DMA_INTERRUPT_FULL
#define DMA_IT_HT  GD32_HAL_DMA_INTERRUPT_HALF
#define DMA_IT_TE  GD32_HAL_DMA_INTERRUPT_ERROR
#define DMA_IT_FE  GD32_HAL_DMA_INTERRUPT_FIFO_UNSUPPORTED
#define DMA_IT_DME GD32_HAL_DMA_INTERRUPT_DIRECT_UNSUPPORTED

#define DMA_FLAG_TCIF0_4 GD32_HAL_DMA_FLAG_FULL
#define DMA_FLAG_TCIF1_5 GD32_HAL_DMA_FLAG_FULL
#define DMA_FLAG_TCIF2_6 GD32_HAL_DMA_FLAG_FULL
#define DMA_FLAG_TCIF3_7 GD32_HAL_DMA_FLAG_FULL
#define DMA_FLAG_HTIF0_4 GD32_HAL_DMA_FLAG_HALF
#define DMA_FLAG_HTIF1_5 GD32_HAL_DMA_FLAG_HALF
#define DMA_FLAG_HTIF2_6 GD32_HAL_DMA_FLAG_HALF
#define DMA_FLAG_HTIF3_7 GD32_HAL_DMA_FLAG_HALF
#define DMA_FLAG_TEIF0_4 GD32_HAL_DMA_FLAG_ERROR
#define DMA_FLAG_TEIF1_5 GD32_HAL_DMA_FLAG_ERROR
#define DMA_FLAG_TEIF2_6 GD32_HAL_DMA_FLAG_ERROR
#define DMA_FLAG_TEIF3_7 GD32_HAL_DMA_FLAG_ERROR
#define DMA_FLAG_FEIF0_4 GD32_HAL_DMA_FLAG_FIFO_UNSUPPORTED
#define DMA_FLAG_FEIF1_5 GD32_HAL_DMA_FLAG_FIFO_UNSUPPORTED
#define DMA_FLAG_FEIF2_6 GD32_HAL_DMA_FLAG_FIFO_UNSUPPORTED
#define DMA_FLAG_FEIF3_7 GD32_HAL_DMA_FLAG_FIFO_UNSUPPORTED
#define DMA_FLAG_DMEIF0_4 GD32_HAL_DMA_FLAG_DIRECT_UNSUPPORTED
#define DMA_FLAG_DMEIF1_5 GD32_HAL_DMA_FLAG_DIRECT_UNSUPPORTED
#define DMA_FLAG_DMEIF2_6 GD32_HAL_DMA_FLAG_DIRECT_UNSUPPORTED
#define DMA_FLAG_DMEIF3_7 GD32_HAL_DMA_FLAG_DIRECT_UNSUPPORTED

#define __HAL_DMA_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_DMA_STATE_RESET)
#define __HAL_DMA_ENABLE(__HANDLE__) \
    GD32_HAL_DMA_Enable((__HANDLE__)->GD32_INSTANCE)
#define __HAL_DMA_DISABLE(__HANDLE__) \
    GD32_HAL_DMA_Disable((__HANDLE__)->GD32_INSTANCE)
#define __HAL_DMA_GET_COUNTER(__HANDLE__) \
    GD32_HAL_DMA_GetRemaining((__HANDLE__)->GD32_INSTANCE)
#define __HAL_DMA_SET_COUNTER(__HANDLE__, __COUNTER__) \
    GD32_HAL_DMA_SetRemaining((__HANDLE__)->GD32_INSTANCE, (__COUNTER__))
#define __HAL_DMA_GET_FLAG(__HANDLE__, __FLAG__) \
    GD32_HAL_DMA_GetFlag((__HANDLE__)->GD32_INSTANCE, (__FLAG__))
#define __HAL_DMA_CLEAR_FLAG(__HANDLE__, __FLAG__) \
    GD32_HAL_DMA_ClearFlag((__HANDLE__)->GD32_INSTANCE, (__FLAG__))
#define __HAL_DMA_ENABLE_IT(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_DMA_SetInterrupt((__HANDLE__)->GD32_INSTANCE, (__INTERRUPT__), 1)
#define __HAL_DMA_DISABLE_IT(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_DMA_SetInterrupt((__HANDLE__)->GD32_INSTANCE, (__INTERRUPT__), 0)
#define __HAL_DMA_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) \
    (GD32_HAL_DMA_GetInterrupts((__HANDLE__)->GD32_INSTANCE) & (__INTERRUPT__))
#define __HAL_DMA_GET_TC_FLAG_INDEX(__HANDLE__) (UNUSED(__HANDLE__), DMA_FLAG_TCIF0_4)
#define __HAL_DMA_GET_HT_FLAG_INDEX(__HANDLE__) (UNUSED(__HANDLE__), DMA_FLAG_HTIF0_4)
#define __HAL_DMA_GET_TE_FLAG_INDEX(__HANDLE__) (UNUSED(__HANDLE__), DMA_FLAG_TEIF0_4)
#define __HAL_DMA_GET_FE_FLAG_INDEX(__HANDLE__) (UNUSED(__HANDLE__), DMA_FLAG_FEIF0_4)
#define __HAL_DMA_GET_DME_FLAG_INDEX(__HANDLE__) (UNUSED(__HANDLE__), DMA_FLAG_DMEIF0_4)

#define IS_DMA_STREAM_ALL_INSTANCE(INSTANCE) \
    (GD32_HAL_ResolveInstance((uintptr_t)(INSTANCE), GD32_HAL_RESOURCE_DMA) != NULL)
#define IS_DMA_BUFFER_SIZE(SIZE) (((SIZE) >= 1U) && ((SIZE) <= 65535U))

HAL_StatusTypeDef HAL_DMA_Init(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef HAL_DMA_DeInit(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef HAL_DMA_Start(DMA_HandleTypeDef *hdma,
                                uint32_t SrcAddress,
                                uint32_t DstAddress,
                                uint32_t DataLength);
HAL_StatusTypeDef HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma,
                                   uint32_t SrcAddress,
                                   uint32_t DstAddress,
                                   uint32_t DataLength);
HAL_StatusTypeDef HAL_DMA_Abort(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef HAL_DMA_Abort_IT(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef HAL_DMA_PollForTransfer(DMA_HandleTypeDef *hdma,
                                          HAL_DMA_LevelCompleteTypeDef CompleteLevel,
                                          uint32_t Timeout);
void HAL_DMA_IRQHandler(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef HAL_DMA_CleanCallbacks(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef HAL_DMA_RegisterCallback(DMA_HandleTypeDef *hdma,
                                           HAL_DMA_CallbackIDTypeDef CallbackID,
                                           void (*pCallback)(DMA_HandleTypeDef *hdma));
HAL_StatusTypeDef HAL_DMA_UnRegisterCallback(DMA_HandleTypeDef *hdma,
                                             HAL_DMA_CallbackIDTypeDef CallbackID);
HAL_DMA_StateTypeDef HAL_DMA_GetState(const DMA_HandleTypeDef *hdma);
uint32_t HAL_DMA_GetError(const DMA_HandleTypeDef *hdma);

/* Compatibility-private: finalizes an ambiguous CubeMX TIM Stream mapping. */
HAL_StatusTypeDef GD32_HAL_DMA_ResolveForTimer(
    DMA_HandleTypeDef *hdma,
    uint32_t timer_address,
    GD32_HAL_TIMERDMARequest request);

#ifdef __cplusplus
}
#endif

#endif /* STM32F4XX_HAL_DMA_H */
