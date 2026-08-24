#ifndef STM32F4XX_HAL_I2C_H
#define STM32F4XX_HAL_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_dma.h"

#if (USE_HAL_I2C_REGISTER_CALLBACKS != 0U)
#error "Runtime I2C callback registration is not implemented; use weak callbacks"
#endif

typedef struct GD32_HAL_I2C_TypeDef I2C_TypeDef;

#ifdef I2C0
#undef I2C0
#endif
#ifdef I2C1
#undef I2C1
#endif
#define I2C1 ((I2C_TypeDef *)(uintptr_t)GD32_HAL_I2C0_ADDRESS)
#define I2C2 ((I2C_TypeDef *)(uintptr_t)GD32_HAL_I2C1_ADDRESS)

#define I2C1_EV_IRQn ((IRQn_Type)GD32_HAL_I2C0_EV_IRQn_VALUE)
#define I2C1_ER_IRQn ((IRQn_Type)GD32_HAL_I2C0_ER_IRQn_VALUE)
#define I2C2_EV_IRQn ((IRQn_Type)GD32_HAL_I2C1_EV_IRQn_VALUE)
#define I2C2_ER_IRQn ((IRQn_Type)GD32_HAL_I2C1_ER_IRQn_VALUE)

typedef struct
{
    uint32_t ClockSpeed;
    uint32_t DutyCycle;
    uint32_t OwnAddress1;
    uint32_t AddressingMode;
    uint32_t DualAddressMode;
    uint32_t OwnAddress2;
    uint32_t GeneralCallMode;
    uint32_t NoStretchMode;
} I2C_InitTypeDef;

typedef enum
{
    HAL_I2C_STATE_RESET = 0x00U,
    HAL_I2C_STATE_READY = 0x20U,
    HAL_I2C_STATE_BUSY = 0x24U,
    HAL_I2C_STATE_BUSY_TX = 0x21U,
    HAL_I2C_STATE_BUSY_RX = 0x22U,
    HAL_I2C_STATE_LISTEN = 0x28U,
    HAL_I2C_STATE_BUSY_TX_LISTEN = 0x29U,
    HAL_I2C_STATE_BUSY_RX_LISTEN = 0x2AU,
    HAL_I2C_STATE_ABORT = 0x60U,
    HAL_I2C_STATE_TIMEOUT = 0xA0U,
    HAL_I2C_STATE_ERROR = 0xE0U
} HAL_I2C_StateTypeDef;

typedef enum
{
    HAL_I2C_MODE_NONE = 0x00U,
    HAL_I2C_MODE_MASTER = 0x10U,
    HAL_I2C_MODE_SLAVE = 0x20U,
    HAL_I2C_MODE_MEM = 0x40U
} HAL_I2C_ModeTypeDef;

#define HAL_I2C_ERROR_NONE        0x00000000U
#define HAL_I2C_ERROR_BERR        0x00000001U
#define HAL_I2C_ERROR_ARLO        0x00000002U
#define HAL_I2C_ERROR_AF          0x00000004U
#define HAL_I2C_ERROR_OVR         0x00000008U
#define HAL_I2C_ERROR_DMA         0x00000010U
#define HAL_I2C_ERROR_TIMEOUT     0x00000020U
#define HAL_I2C_ERROR_SIZE        0x00000040U
#define HAL_I2C_ERROR_DMA_PARAM   0x00000080U
#define HAL_I2C_WRONG_START       0x00000200U

typedef struct __I2C_HandleTypeDef
{
    I2C_TypeDef *Instance;
    I2C_InitTypeDef Init;
    uint8_t *pBuffPtr;
    uint16_t XferSize;
    __IO uint16_t XferCount;
    __IO uint32_t XferOptions;
    __IO uint32_t PreviousState;
    DMA_HandleTypeDef *hdmatx;
    DMA_HandleTypeDef *hdmarx;
    HAL_LockTypeDef Lock;
    __IO HAL_I2C_StateTypeDef State;
    __IO HAL_I2C_ModeTypeDef Mode;
    __IO uint32_t ErrorCode;
    __IO uint32_t Devaddress;
    __IO uint32_t Memaddress;
    __IO uint32_t MemaddSize;
    __IO uint32_t EventCount;
} I2C_HandleTypeDef;

#define I2C_DUTYCYCLE_2    0x00000000U
#define I2C_DUTYCYCLE_16_9 0x00000001U
#define I2C_ADDRESSINGMODE_7BIT  0x00000000U
#define I2C_ADDRESSINGMODE_10BIT 0x00000001U
#define I2C_DUALADDRESS_DISABLE 0x00000000U
#define I2C_DUALADDRESS_ENABLE  0x00000001U
#define I2C_GENERALCALL_DISABLE 0x00000000U
#define I2C_GENERALCALL_ENABLE  0x00000001U
#define I2C_NOSTRETCH_DISABLE 0x00000000U
#define I2C_NOSTRETCH_ENABLE  0x00000001U
#define I2C_MEMADD_SIZE_8BIT  0x00000001U
#define I2C_MEMADD_SIZE_16BIT 0x00000002U
#define I2C_NO_OPTION_FRAME 0xFFFF0000U

#define I2C_IT_BUF GD32_HAL_I2C_INTERRUPT_BUFFER
#define I2C_IT_EVT GD32_HAL_I2C_INTERRUPT_EVENT
#define I2C_IT_ERR GD32_HAL_I2C_INTERRUPT_ERROR

#define I2C_FLAG_SB   GD32_HAL_I2C_FLAG_START
#define I2C_FLAG_ADDR GD32_HAL_I2C_FLAG_ADDRESS
#define I2C_FLAG_BTF  GD32_HAL_I2C_FLAG_BYTE_TRANSFER
#define I2C_FLAG_ADD10 GD32_HAL_I2C_FLAG_ADDRESS10
#define I2C_FLAG_RXNE GD32_HAL_I2C_FLAG_RX_NOT_EMPTY
#define I2C_FLAG_TXE  GD32_HAL_I2C_FLAG_TX_EMPTY
#define I2C_FLAG_BUSY GD32_HAL_I2C_FLAG_BUSY

#define __HAL_I2C_RESET_HANDLE_STATE(__HANDLE__) \
    do { (__HANDLE__)->State = HAL_I2C_STATE_RESET; \
         (__HANDLE__)->Mode = HAL_I2C_MODE_NONE; } while (0)
#define __HAL_I2C_ENABLE(__HANDLE__) \
    GD32_HAL_I2C_Enable((uint32_t)(uintptr_t)((__HANDLE__)->Instance))
#define __HAL_I2C_DISABLE(__HANDLE__) \
    GD32_HAL_I2C_Disable((uint32_t)(uintptr_t)((__HANDLE__)->Instance))
#define __HAL_I2C_ENABLE_IT(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_I2C_SetInterrupts((uint32_t)(uintptr_t)((__HANDLE__)->Instance), \
                               (__INTERRUPT__), 1)
#define __HAL_I2C_DISABLE_IT(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_I2C_SetInterrupts((uint32_t)(uintptr_t)((__HANDLE__)->Instance), \
                               (__INTERRUPT__), 0)
#define __HAL_I2C_GET_FLAG(__HANDLE__, __FLAG__) \
    ((GD32_HAL_I2C_GetFlags((uint32_t)(uintptr_t)((__HANDLE__)->Instance)) & \
      (__FLAG__)) != 0U)
#define __HAL_I2C_CLEAR_ADDRFLAG(__HANDLE__) \
    GD32_HAL_I2C_ClearAddress((uint32_t)(uintptr_t)((__HANDLE__)->Instance))

HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c,
                                          uint16_t DevAddress,
                                          uint8_t *pData,
                                          uint16_t Size,
                                          uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c,
                                         uint16_t DevAddress,
                                         uint8_t *pData,
                                         uint16_t Size,
                                         uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Master_Transmit_IT(I2C_HandleTypeDef *hi2c,
                                             uint16_t DevAddress,
                                             uint8_t *pData,
                                             uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Master_Receive_IT(I2C_HandleTypeDef *hi2c,
                                            uint16_t DevAddress,
                                            uint8_t *pData,
                                            uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c,
                                    uint16_t DevAddress,
                                    uint16_t MemAddress,
                                    uint16_t MemAddSize,
                                    uint8_t *pData,
                                    uint16_t Size,
                                    uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c,
                                   uint16_t DevAddress,
                                   uint16_t MemAddress,
                                   uint16_t MemAddSize,
                                   uint8_t *pData,
                                   uint16_t Size,
                                   uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c,
                                        uint16_t DevAddress,
                                        uint32_t Trials,
                                        uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Master_Transmit_DMA(I2C_HandleTypeDef *hi2c,
                                              uint16_t DevAddress,
                                              uint8_t *pData,
                                              uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Master_Receive_DMA(I2C_HandleTypeDef *hi2c,
                                             uint16_t DevAddress,
                                             uint8_t *pData,
                                             uint16_t Size);
void HAL_I2C_EV_IRQHandler(I2C_HandleTypeDef *hi2c);
void HAL_I2C_IRQHandler(I2C_HandleTypeDef *hi2c);
void HAL_I2C_ER_IRQHandler(I2C_HandleTypeDef *hi2c);
HAL_I2C_StateTypeDef HAL_I2C_GetState(const I2C_HandleTypeDef *hi2c);
HAL_I2C_ModeTypeDef HAL_I2C_GetMode(const I2C_HandleTypeDef *hi2c);
uint32_t HAL_I2C_GetError(const I2C_HandleTypeDef *hi2c);

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);

#ifdef __cplusplus
}
#endif

#endif /* STM32F4XX_HAL_I2C_H */
