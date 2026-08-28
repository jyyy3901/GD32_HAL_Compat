#ifndef STM32F4XX_HAL_UART_H
#define STM32F4XX_HAL_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal_def.h"

#if (USE_HAL_UART_REGISTER_CALLBACKS != 0U)
#error "Runtime UART callback registration is not implemented; use weak callbacks"
#endif

/* 公共 Instance 保持不透明，避免应用访问 STM32 CRx/SR/DR 寄存器布局。 */
typedef struct GD32_HAL_USART_TypeDef USART_TypeDef;

#ifndef GD32_HAL_DMA_HANDLE_TYPEDEF
#define GD32_HAL_DMA_HANDLE_TYPEDEF
typedef struct __DMA_HandleTypeDef DMA_HandleTypeDef;
#endif

#ifdef USART1
#undef USART1
#endif
#ifdef USART2
#undef USART2
#endif
#ifdef USART6
#undef USART6
#endif

#define USART1 ((USART_TypeDef *)(uintptr_t)STM32_UART_INSTANCE_1)
#define USART2 ((USART_TypeDef *)(uintptr_t)STM32_UART_INSTANCE_2)
#define USART6 ((USART_TypeDef *)(uintptr_t)STM32_UART_INSTANCE_6)

/* IRQ 值由集中映射头定义，避免 GD32 零起始命名被宏递归改写。 */
#define USART1_IRQn ((IRQn_Type)STM32_HAL_IRQ_USART1)
#define USART2_IRQn ((IRQn_Type)STM32_HAL_IRQ_USART2)
#define USART6_IRQn ((IRQn_Type)STM32_HAL_IRQ_USART6)

typedef struct
{
    uint32_t BaudRate;
    uint32_t WordLength;
    uint32_t StopBits;
    uint32_t Parity;
    uint32_t Mode;
    uint32_t HwFlowCtl;
    uint32_t OverSampling;
} UART_InitTypeDef;

typedef enum
{
    HAL_UART_STATE_RESET = 0x00U,
    HAL_UART_STATE_READY = 0x20U,
    HAL_UART_STATE_BUSY = 0x24U,
    HAL_UART_STATE_BUSY_TX = 0x21U,
    HAL_UART_STATE_BUSY_RX = 0x22U,
    HAL_UART_STATE_BUSY_TX_RX = 0x23U,
    HAL_UART_STATE_TIMEOUT = 0xA0U,
    HAL_UART_STATE_ERROR = 0xE0U
} HAL_UART_StateTypeDef;

typedef uint32_t HAL_UART_RxTypeTypeDef;
typedef uint32_t HAL_UART_RxEventTypeTypeDef;

typedef struct __UART_HandleTypeDef
{
    USART_TypeDef *Instance;
    UART_InitTypeDef Init;
    const uint8_t *pTxBuffPtr;
    uint16_t TxXferSize;
    __IO uint16_t TxXferCount;
    uint8_t *pRxBuffPtr;
    uint16_t RxXferSize;
    __IO uint16_t RxXferCount;
    __IO HAL_UART_RxTypeTypeDef ReceptionType;
    __IO HAL_UART_RxEventTypeTypeDef RxEventType;
    DMA_HandleTypeDef *hdmatx;
    DMA_HandleTypeDef *hdmarx;
    HAL_LockTypeDef Lock;
    __IO HAL_UART_StateTypeDef gState;
    __IO HAL_UART_StateTypeDef RxState;
    __IO uint32_t ErrorCode;
    uint32_t GD32_INSTANCE;
    int32_t GD32_IRQ_NUMBER;
    const GD32_HAL_Resource *GD32_RESOURCE;
} UART_HandleTypeDef;

#define HAL_UART_ERROR_NONE 0x00000000U
#define HAL_UART_ERROR_PE   0x00000001U
#define HAL_UART_ERROR_NE   0x00000002U
#define HAL_UART_ERROR_FE   0x00000004U
#define HAL_UART_ERROR_ORE  0x00000008U
#define HAL_UART_ERROR_DMA  0x00000010U

#define UART_WORDLENGTH_8B 0x00000000U
#define UART_WORDLENGTH_9B 0x00001000U
#define UART_STOPBITS_1    0x00000000U
#define UART_STOPBITS_2    0x00002000U
#define UART_PARITY_NONE   0x00000000U
#define UART_PARITY_EVEN   0x00000400U
#define UART_PARITY_ODD    0x00000600U
#define UART_HWCONTROL_NONE    0x00000000U
#define UART_HWCONTROL_RTS     0x00000100U
#define UART_HWCONTROL_CTS     0x00000200U
#define UART_HWCONTROL_RTS_CTS 0x00000300U
#define UART_MODE_RX       0x00000004U
#define UART_MODE_TX       0x00000008U
#define UART_MODE_TX_RX    0x0000000CU
#define UART_OVERSAMPLING_16 0x00000000U
#define UART_OVERSAMPLING_8  0x00008000U
#define UART_STATE_DISABLE 0x00000000U
#define UART_STATE_ENABLE  0x00002000U

#define HAL_UART_RECEPTION_STANDARD 0x00000000U
#define HAL_UART_RECEPTION_TOIDLE   0x00000001U
#define HAL_UART_RXEVENT_TC   0x00000000U
#define HAL_UART_RXEVENT_HT   0x00000001U
#define HAL_UART_RXEVENT_IDLE 0x00000002U

#define UART_FLAG_CTS  GD32_HAL_UART_FLAG_CTS
#define UART_FLAG_LBD  GD32_HAL_UART_FLAG_LIN_BREAK
#define UART_FLAG_TXE  GD32_HAL_UART_FLAG_TX_EMPTY
#define UART_FLAG_TC   GD32_HAL_UART_FLAG_TX_COMPLETE
#define UART_FLAG_RXNE GD32_HAL_UART_FLAG_RX_NOT_EMPTY
#define UART_FLAG_IDLE GD32_HAL_UART_FLAG_IDLE
#define UART_FLAG_ORE  GD32_HAL_UART_FLAG_ORE
#define UART_FLAG_NE   GD32_HAL_UART_FLAG_NE
#define UART_FLAG_FE   GD32_HAL_UART_FLAG_FE
#define UART_FLAG_PE   GD32_HAL_UART_FLAG_PE

#define UART_IT_PE   GD32_HAL_UART_INTERRUPT_PE
#define UART_IT_TXE  GD32_HAL_UART_INTERRUPT_TX_EMPTY
#define UART_IT_TC   GD32_HAL_UART_INTERRUPT_TX_COMPLETE
#define UART_IT_RXNE GD32_HAL_UART_INTERRUPT_RX_NOT_EMPTY
#define UART_IT_IDLE GD32_HAL_UART_INTERRUPT_IDLE
#define UART_IT_LBD  GD32_HAL_UART_INTERRUPT_LIN_BREAK
#define UART_IT_CTS  GD32_HAL_UART_INTERRUPT_CTS
#define UART_IT_ERR  GD32_HAL_UART_INTERRUPT_ERROR

#define __HAL_UART_RESET_HANDLE_STATE(__HANDLE__)            \
    do                                                       \
    {                                                        \
        (__HANDLE__)->gState = HAL_UART_STATE_RESET;         \
        (__HANDLE__)->RxState = HAL_UART_STATE_RESET;        \
    } while (0U)

#define __HAL_UART_GET_FLAG(__HANDLE__, __FLAG__) \
    (GD32_HAL_UART_GetFlag((__HANDLE__)->GD32_INSTANCE, \
                           (GD32_HAL_UARTFlag)(__FLAG__)) != 0U)
#define __HAL_UART_CLEAR_FLAG(__HANDLE__, __FLAG__) \
    GD32_HAL_UART_ClearFlag((__HANDLE__)->GD32_INSTANCE, \
                            (GD32_HAL_UARTFlag)(__FLAG__))
#define __HAL_UART_CLEAR_PEFLAG(__HANDLE__) \
    GD32_HAL_UART_ClearErrors((__HANDLE__)->GD32_INSTANCE)
#define __HAL_UART_CLEAR_FEFLAG(__HANDLE__) __HAL_UART_CLEAR_PEFLAG(__HANDLE__)
#define __HAL_UART_CLEAR_NEFLAG(__HANDLE__) __HAL_UART_CLEAR_PEFLAG(__HANDLE__)
#define __HAL_UART_CLEAR_OREFLAG(__HANDLE__) __HAL_UART_CLEAR_PEFLAG(__HANDLE__)
#define __HAL_UART_CLEAR_IDLEFLAG(__HANDLE__) __HAL_UART_CLEAR_PEFLAG(__HANDLE__)
#define __HAL_UART_FLUSH_DRREGISTER(__HANDLE__) \
    ((void)GD32_HAL_UART_ReadData((__HANDLE__)->GD32_INSTANCE))
#define __HAL_UART_ENABLE_IT(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_UART_SetInterrupt((__HANDLE__)->GD32_INSTANCE, \
                               (GD32_HAL_UARTInterrupt)(__INTERRUPT__), 1)
#define __HAL_UART_DISABLE_IT(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_UART_SetInterrupt((__HANDLE__)->GD32_INSTANCE, \
                               (GD32_HAL_UARTInterrupt)(__INTERRUPT__), 0)
#define __HAL_UART_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_UART_IsInterruptEnabled((__HANDLE__)->GD32_INSTANCE, \
                                     (GD32_HAL_UARTInterrupt)(__INTERRUPT__))
#define __HAL_UART_ENABLE(__HANDLE__) \
    GD32_HAL_UART_Enable((__HANDLE__)->GD32_INSTANCE)
#define __HAL_UART_DISABLE(__HANDLE__) \
    GD32_HAL_UART_Disable((__HANDLE__)->GD32_INSTANCE)

#define IS_UART_WORD_LENGTH(VALUE) (((VALUE) == UART_WORDLENGTH_8B) || ((VALUE) == UART_WORDLENGTH_9B))
#define IS_UART_STOPBITS(VALUE) (((VALUE) == UART_STOPBITS_1) || ((VALUE) == UART_STOPBITS_2))
#define IS_UART_PARITY(VALUE) (((VALUE) == UART_PARITY_NONE) || ((VALUE) == UART_PARITY_EVEN) || \
                               ((VALUE) == UART_PARITY_ODD))
#define IS_UART_HARDWARE_FLOW_CONTROL(VALUE) (((VALUE) == UART_HWCONTROL_NONE) || \
                                               ((VALUE) == UART_HWCONTROL_RTS) || \
                                               ((VALUE) == UART_HWCONTROL_CTS) || \
                                               ((VALUE) == UART_HWCONTROL_RTS_CTS))
#define IS_UART_MODE(VALUE) (((VALUE) == UART_MODE_RX) || ((VALUE) == UART_MODE_TX) || \
                             ((VALUE) == UART_MODE_TX_RX))
#define IS_UART_OVERSAMPLING(VALUE) (((VALUE) == UART_OVERSAMPLING_16) || \
                                     ((VALUE) == UART_OVERSAMPLING_8))
#define IS_UART_BAUDRATE(VALUE) (((VALUE) > 0U) && ((VALUE) <= 10500000U))
#define IS_UART_INSTANCE(INSTANCE) \
    (GD32_HAL_ResolveInstance((uintptr_t)(INSTANCE), GD32_HAL_RESOURCE_UART) != NULL)
#define IS_UART_HWFLOW_INSTANCE(INSTANCE) \
    ((GD32_HAL_ResolveInstance((uintptr_t)(INSTANCE), GD32_HAL_RESOURCE_UART) != NULL) && \
     ((GD32_HAL_ResolveInstance((uintptr_t)(INSTANCE), GD32_HAL_RESOURCE_UART)->capabilities & \
       GD32_HAL_CAP_FLOW_CONTROL) != 0U))

HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart);
HAL_StatusTypeDef HAL_UART_DeInit(UART_HandleTypeDef *huart);
void HAL_UART_MspInit(UART_HandleTypeDef *huart);
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart);
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart,
                                    const uint8_t *pData,
                                    uint16_t Size,
                                    uint32_t Timeout);
HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *huart,
                                   uint8_t *pData,
                                   uint16_t Size,
                                   uint32_t Timeout);
HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *huart,
                                       const uint8_t *pData,
                                       uint16_t Size);
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *huart,
                                      uint8_t *pData,
                                      uint16_t Size);
HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart,
                                        const uint8_t *pData,
                                        uint16_t Size);
HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef *huart,
                                       uint8_t *pData,
                                       uint16_t Size);
HAL_StatusTypeDef HAL_UART_DMAPause(UART_HandleTypeDef *huart);
HAL_StatusTypeDef HAL_UART_DMAResume(UART_HandleTypeDef *huart);
HAL_StatusTypeDef HAL_UART_DMAStop(UART_HandleTypeDef *huart);
HAL_StatusTypeDef HAL_UART_Abort(UART_HandleTypeDef *huart);
HAL_StatusTypeDef HAL_UART_AbortTransmit(UART_HandleTypeDef *huart);
HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef *huart);
HAL_StatusTypeDef HAL_UART_Abort_IT(UART_HandleTypeDef *huart);
HAL_StatusTypeDef HAL_UART_AbortTransmit_IT(UART_HandleTypeDef *huart);
HAL_StatusTypeDef HAL_UART_AbortReceive_IT(UART_HandleTypeDef *huart);
void HAL_UART_IRQHandler(UART_HandleTypeDef *huart);
HAL_UART_StateTypeDef HAL_UART_GetState(const UART_HandleTypeDef *huart);
uint32_t HAL_UART_GetError(const UART_HandleTypeDef *huart);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_AbortTransmitCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* STM32F4XX_HAL_UART_H */
