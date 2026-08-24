#ifndef STM32F4XX_HAL_SPI_H
#define STM32F4XX_HAL_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_dma.h"

#if (USE_HAL_SPI_REGISTER_CALLBACKS != 0U)
#error "Runtime SPI callback registration is not implemented; use weak callbacks"
#endif

typedef struct GD32_HAL_SPI_TypeDef SPI_TypeDef;

#ifdef SPI0
#undef SPI0
#endif
#ifdef SPI1
#undef SPI1
#endif
#ifdef SPI2
#undef SPI2
#endif
#ifdef SPI_NSS_SOFT
#undef SPI_NSS_SOFT
#endif
#ifdef SPI_NSS_HARD
#undef SPI_NSS_HARD
#endif
#define SPI1 ((SPI_TypeDef *)(uintptr_t)GD32_HAL_SPI0_ADDRESS)
#define SPI2 ((SPI_TypeDef *)(uintptr_t)GD32_HAL_SPI1_ADDRESS)
#define SPI3 ((SPI_TypeDef *)(uintptr_t)GD32_HAL_SPI2_ADDRESS)

#define SPI1_IRQn ((IRQn_Type)GD32_HAL_SPI0_IRQn_VALUE)
#define SPI2_IRQn ((IRQn_Type)GD32_HAL_SPI1_IRQn_VALUE)
#define SPI3_IRQn ((IRQn_Type)GD32_HAL_SPI2_IRQn_VALUE)

typedef struct
{
    uint32_t Mode;
    uint32_t Direction;
    uint32_t DataSize;
    uint32_t CLKPolarity;
    uint32_t CLKPhase;
    uint32_t NSS;
    uint32_t BaudRatePrescaler;
    uint32_t FirstBit;
    uint32_t TIMode;
    uint32_t CRCCalculation;
    uint32_t CRCPolynomial;
} SPI_InitTypeDef;

typedef enum
{
    HAL_SPI_STATE_RESET = 0x00U,
    HAL_SPI_STATE_READY = 0x01U,
    HAL_SPI_STATE_BUSY = 0x02U,
    HAL_SPI_STATE_BUSY_TX = 0x03U,
    HAL_SPI_STATE_BUSY_RX = 0x04U,
    HAL_SPI_STATE_BUSY_TX_RX = 0x05U,
    HAL_SPI_STATE_ERROR = 0x06U,
    HAL_SPI_STATE_ABORT = 0x07U
} HAL_SPI_StateTypeDef;

#define HAL_SPI_ERROR_NONE  0x00000000U
#define HAL_SPI_ERROR_MODF  0x00000001U
#define HAL_SPI_ERROR_CRC   0x00000002U
#define HAL_SPI_ERROR_OVR   0x00000004U
#define HAL_SPI_ERROR_FRE   0x00000008U
#define HAL_SPI_ERROR_DMA   0x00000010U
#define HAL_SPI_ERROR_FLAG  0x00000020U
#define HAL_SPI_ERROR_ABORT 0x00000040U

typedef struct __SPI_HandleTypeDef
{
    SPI_TypeDef *Instance;
    SPI_InitTypeDef Init;
    const uint8_t *pTxBuffPtr;
    uint16_t TxXferSize;
    __IO uint16_t TxXferCount;
    uint8_t *pRxBuffPtr;
    uint16_t RxXferSize;
    __IO uint16_t RxXferCount;
    void (*RxISR)(struct __SPI_HandleTypeDef *hspi);
    void (*TxISR)(struct __SPI_HandleTypeDef *hspi);
    DMA_HandleTypeDef *hdmatx;
    DMA_HandleTypeDef *hdmarx;
    HAL_LockTypeDef Lock;
    __IO HAL_SPI_StateTypeDef State;
    __IO uint32_t ErrorCode;
    __IO uint8_t DmaActive;
    __IO uint8_t DmaCircular;
} SPI_HandleTypeDef;

#define SPI_MODE_SLAVE  0x00000000U
#define SPI_MODE_MASTER 0x00000001U
#define SPI_DIRECTION_2LINES         0x00000000U
#define SPI_DIRECTION_2LINES_RXONLY  0x00000001U
#define SPI_DIRECTION_1LINE          0x00000002U
#define SPI_DATASIZE_8BIT  0x00000000U
#define SPI_DATASIZE_16BIT 0x00000001U
#define SPI_POLARITY_LOW  0x00000000U
#define SPI_POLARITY_HIGH 0x00000001U
#define SPI_PHASE_1EDGE 0x00000000U
#define SPI_PHASE_2EDGE 0x00000001U
#define SPI_NSS_SOFT        0x00000000U
#define SPI_NSS_HARD_INPUT  0x00000001U
#define SPI_NSS_HARD_OUTPUT 0x00000002U
#define SPI_BAUDRATEPRESCALER_2   2U
#define SPI_BAUDRATEPRESCALER_4   4U
#define SPI_BAUDRATEPRESCALER_8   8U
#define SPI_BAUDRATEPRESCALER_16  16U
#define SPI_BAUDRATEPRESCALER_32  32U
#define SPI_BAUDRATEPRESCALER_64  64U
#define SPI_BAUDRATEPRESCALER_128 128U
#define SPI_BAUDRATEPRESCALER_256 256U
#define SPI_FIRSTBIT_MSB 0x00000000U
#define SPI_FIRSTBIT_LSB 0x00000001U
#define SPI_TIMODE_DISABLE 0x00000000U
#define SPI_TIMODE_ENABLE  0x00000001U
#define SPI_CRCCALCULATION_DISABLE 0x00000000U
#define SPI_CRCCALCULATION_ENABLE  0x00000001U

#define SPI_IT_TXE GD32_HAL_SPI_INTERRUPT_TX
#define SPI_IT_RXNE GD32_HAL_SPI_INTERRUPT_RX
#define SPI_IT_ERR GD32_HAL_SPI_INTERRUPT_ERROR
#define SPI_FLAG_RXNE GD32_HAL_SPI_FLAG_RX_NOT_EMPTY
#define SPI_FLAG_TXE GD32_HAL_SPI_FLAG_TX_EMPTY
#define SPI_FLAG_BSY GD32_HAL_SPI_FLAG_BUSY

#define __HAL_SPI_RESET_HANDLE_STATE(__HANDLE__) \
    ((__HANDLE__)->State = HAL_SPI_STATE_RESET)
#define __HAL_SPI_ENABLE(__HANDLE__) \
    GD32_HAL_SPI_Enable((uint32_t)(uintptr_t)((__HANDLE__)->Instance))
#define __HAL_SPI_DISABLE(__HANDLE__) \
    GD32_HAL_SPI_Disable((uint32_t)(uintptr_t)((__HANDLE__)->Instance))
#define __HAL_SPI_ENABLE_IT(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_SPI_SetInterrupts((uint32_t)(uintptr_t)((__HANDLE__)->Instance), \
                               (__INTERRUPT__), 1)
#define __HAL_SPI_DISABLE_IT(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_SPI_SetInterrupts((uint32_t)(uintptr_t)((__HANDLE__)->Instance), \
                               (__INTERRUPT__), 0)
#define __HAL_SPI_GET_FLAG(__HANDLE__, __FLAG__) \
    ((GD32_HAL_SPI_GetFlags((uint32_t)(uintptr_t)((__HANDLE__)->Instance)) & \
      (__FLAG__)) != 0U)

HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef HAL_SPI_DeInit(SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *hspi,
                                   const uint8_t *pData,
                                   uint16_t Size,
                                   uint32_t Timeout);
HAL_StatusTypeDef HAL_SPI_Receive(SPI_HandleTypeDef *hspi,
                                  uint8_t *pData,
                                  uint16_t Size,
                                  uint32_t Timeout);
HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi,
                                          const uint8_t *pTxData,
                                          uint8_t *pRxData,
                                          uint16_t Size,
                                          uint32_t Timeout);
HAL_StatusTypeDef HAL_SPI_Transmit_IT(SPI_HandleTypeDef *hspi,
                                      const uint8_t *pData,
                                      uint16_t Size);
HAL_StatusTypeDef HAL_SPI_Receive_IT(SPI_HandleTypeDef *hspi,
                                     uint8_t *pData,
                                     uint16_t Size);
HAL_StatusTypeDef HAL_SPI_TransmitReceive_IT(SPI_HandleTypeDef *hspi,
                                             const uint8_t *pTxData,
                                             uint8_t *pRxData,
                                             uint16_t Size);
HAL_StatusTypeDef HAL_SPI_Transmit_DMA(SPI_HandleTypeDef *hspi,
                                       const uint8_t *pData,
                                       uint16_t Size);
HAL_StatusTypeDef HAL_SPI_Receive_DMA(SPI_HandleTypeDef *hspi,
                                      uint8_t *pData,
                                      uint16_t Size);
HAL_StatusTypeDef HAL_SPI_TransmitReceive_DMA(SPI_HandleTypeDef *hspi,
                                              const uint8_t *pTxData,
                                              uint8_t *pRxData,
                                              uint16_t Size);
HAL_StatusTypeDef HAL_SPI_DMAPause(SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef HAL_SPI_DMAResume(SPI_HandleTypeDef *hspi);
HAL_StatusTypeDef HAL_SPI_DMAStop(SPI_HandleTypeDef *hspi);
void HAL_SPI_IRQHandler(SPI_HandleTypeDef *hspi);
HAL_SPI_StateTypeDef HAL_SPI_GetState(const SPI_HandleTypeDef *hspi);
uint32_t HAL_SPI_GetError(const SPI_HandleTypeDef *hspi);

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi);
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_RxHalfCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_TxRxHalfCpltCallback(SPI_HandleTypeDef *hspi);
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);

#ifdef __cplusplus
}
#endif

#endif /* STM32F4XX_HAL_SPI_H */
