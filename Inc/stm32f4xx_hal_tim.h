#ifndef STM32F4XX_HAL_TIM_H
#define STM32F4XX_HAL_TIM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_dma.h"

#if (USE_HAL_TIM_REGISTER_CALLBACKS != 0U)
#error "Runtime TIM callback registration is not implemented; use weak callbacks"
#endif

/* Instance 故意保持不透明；GD32F403 的所有 TIMER 均为 16 位。 */
typedef struct GD32_HAL_TIM_TypeDef TIM_TypeDef;

#define TIM1  ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER0_ADDRESS)
#define TIM2  ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER1_ADDRESS)
#define TIM3  ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER2_ADDRESS)
#define TIM4  ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER3_ADDRESS)
#define TIM5  ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER4_ADDRESS)
#define TIM6  ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER5_ADDRESS)
#define TIM7  ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER6_ADDRESS)
#define TIM8  ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER7_ADDRESS)
#define TIM9  ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER8_ADDRESS)
#define TIM10 ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER9_ADDRESS)
#define TIM11 ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER10_ADDRESS)
#define TIM12 ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER11_ADDRESS)
#define TIM13 ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER12_ADDRESS)
#define TIM14 ((TIM_TypeDef *)(uintptr_t)GD32_HAL_TIMER13_ADDRESS)

#define TIM1_BRK_TIM9_IRQn     ((IRQn_Type)GD32_HAL_TIMER0_BRK_TIMER8_IRQn_VALUE)
#define TIM1_UP_TIM10_IRQn     ((IRQn_Type)GD32_HAL_TIMER0_UP_TIMER9_IRQn_VALUE)
#define TIM1_TRG_COM_TIM11_IRQn ((IRQn_Type)GD32_HAL_TIMER0_TRG_CMT_TIMER10_IRQn_VALUE)
#define TIM1_CC_IRQn           ((IRQn_Type)GD32_HAL_TIMER0_CHANNEL_IRQn_VALUE)
#define TIM2_IRQn              ((IRQn_Type)GD32_HAL_TIMER1_IRQn_VALUE)
#define TIM3_IRQn              ((IRQn_Type)GD32_HAL_TIMER2_IRQn_VALUE)
#define TIM4_IRQn              ((IRQn_Type)GD32_HAL_TIMER3_IRQn_VALUE)
#define TIM5_IRQn              ((IRQn_Type)GD32_HAL_TIMER4_IRQn_VALUE)
#define TIM6_DAC_IRQn          ((IRQn_Type)GD32_HAL_TIMER5_IRQn_VALUE)
#define TIM7_IRQn              ((IRQn_Type)GD32_HAL_TIMER6_IRQn_VALUE)
#define TIM8_BRK_TIM12_IRQn    ((IRQn_Type)GD32_HAL_TIMER7_BRK_TIMER11_IRQn_VALUE)
#define TIM8_UP_TIM13_IRQn     ((IRQn_Type)GD32_HAL_TIMER7_UP_TIMER12_IRQn_VALUE)
#define TIM8_TRG_COM_TIM14_IRQn ((IRQn_Type)GD32_HAL_TIMER7_TRG_CMT_TIMER13_IRQn_VALUE)
#define TIM8_CC_IRQn           ((IRQn_Type)GD32_HAL_TIMER7_CHANNEL_IRQn_VALUE)

typedef struct
{
    uint32_t Prescaler;
    uint32_t CounterMode;
    uint32_t Period;
    uint32_t ClockDivision;
    uint32_t RepetitionCounter;
    uint32_t AutoReloadPreload;
} TIM_Base_InitTypeDef;

typedef struct
{
    uint32_t OCMode;
    uint32_t Pulse;
    uint32_t OCPolarity;
    uint32_t OCNPolarity;
    uint32_t OCFastMode;
    uint32_t OCIdleState;
    uint32_t OCNIdleState;
} TIM_OC_InitTypeDef;

typedef struct
{
    uint32_t OCMode;
    uint32_t Pulse;
    uint32_t OCPolarity;
    uint32_t OCNPolarity;
    uint32_t OCIdleState;
    uint32_t OCNIdleState;
    uint32_t ICPolarity;
    uint32_t ICSelection;
    uint32_t ICFilter;
} TIM_OnePulse_InitTypeDef;

typedef struct
{
    uint32_t ICPolarity;
    uint32_t ICSelection;
    uint32_t ICPrescaler;
    uint32_t ICFilter;
} TIM_IC_InitTypeDef;

typedef struct
{
    uint32_t ClockSource;
    uint32_t ClockPolarity;
    uint32_t ClockPrescaler;
    uint32_t ClockFilter;
} TIM_ClockConfigTypeDef;

typedef struct
{
    uint32_t MasterOutputTrigger;
    uint32_t MasterSlaveMode;
} TIM_MasterConfigTypeDef;

typedef struct
{
    uint32_t SlaveMode;
    uint32_t InputTrigger;
    uint32_t TriggerPolarity;
    uint32_t TriggerPrescaler;
    uint32_t TriggerFilter;
} TIM_SlaveConfigTypeDef;

typedef enum
{
    HAL_TIM_STATE_RESET = 0x00U,
    HAL_TIM_STATE_READY = 0x01U,
    HAL_TIM_STATE_BUSY = 0x02U,
    HAL_TIM_STATE_TIMEOUT = 0x03U,
    HAL_TIM_STATE_ERROR = 0x04U
} HAL_TIM_StateTypeDef;

typedef enum
{
    HAL_TIM_CHANNEL_STATE_RESET = 0x00U,
    HAL_TIM_CHANNEL_STATE_READY = 0x01U,
    HAL_TIM_CHANNEL_STATE_BUSY = 0x02U
} HAL_TIM_ChannelStateTypeDef;

typedef enum
{
    HAL_DMA_BURST_STATE_RESET = 0x00U,
    HAL_DMA_BURST_STATE_READY = 0x01U,
    HAL_DMA_BURST_STATE_BUSY = 0x02U
} HAL_TIM_DMABurstStateTypeDef;

typedef enum
{
    HAL_TIM_ACTIVE_CHANNEL_CLEARED = 0x00U,
    HAL_TIM_ACTIVE_CHANNEL_1 = 0x01U,
    HAL_TIM_ACTIVE_CHANNEL_2 = 0x02U,
    HAL_TIM_ACTIVE_CHANNEL_3 = 0x04U,
    HAL_TIM_ACTIVE_CHANNEL_4 = 0x08U
} HAL_TIM_ActiveChannel;

typedef struct __TIM_HandleTypeDef
{
    TIM_TypeDef *Instance;
    TIM_Base_InitTypeDef Init;
    HAL_TIM_ActiveChannel Channel;
    DMA_HandleTypeDef *hdma[7];
    HAL_LockTypeDef Lock;
    __IO HAL_TIM_StateTypeDef State;
    __IO HAL_TIM_ChannelStateTypeDef ChannelState[4];
    __IO HAL_TIM_ChannelStateTypeDef ChannelNState[4];
    __IO HAL_TIM_DMABurstStateTypeDef DMABurstState;
} TIM_HandleTypeDef;

#define TIM_COUNTERMODE_UP             0x00000000U
#define TIM_COUNTERMODE_DOWN           0x00000010U
#define TIM_COUNTERMODE_CENTERALIGNED1 0x00000020U
#define TIM_COUNTERMODE_CENTERALIGNED2 0x00000040U
#define TIM_COUNTERMODE_CENTERALIGNED3 0x00000060U

#define TIM_CLOCKDIVISION_DIV1 0x00000000U
#define TIM_CLOCKDIVISION_DIV2 0x00000100U
#define TIM_CLOCKDIVISION_DIV4 0x00000200U
#define TIM_AUTORELOAD_PRELOAD_DISABLE 0x00000000U
#define TIM_AUTORELOAD_PRELOAD_ENABLE  0x00000080U

#define TIM_OCMODE_TIMING          0x00000000U
#define TIM_OCMODE_ACTIVE          0x00000010U
#define TIM_OCMODE_INACTIVE        0x00000020U
#define TIM_OCMODE_TOGGLE          0x00000030U
#define TIM_OCMODE_FORCED_INACTIVE 0x00000040U
#define TIM_OCMODE_FORCED_ACTIVE   0x00000050U
#define TIM_OCMODE_PWM1            0x00000060U
#define TIM_OCMODE_PWM2            0x00000070U
#define TIM_OCFAST_DISABLE         0x00000000U
#define TIM_OCFAST_ENABLE          0x00000004U
#define TIM_OCPOLARITY_HIGH        0x00000000U
#define TIM_OCPOLARITY_LOW         0x00000002U
#define TIM_OCNPOLARITY_HIGH       0x00000000U
#define TIM_OCNPOLARITY_LOW        0x00000008U
#define TIM_OCIDLESTATE_RESET      0x00000000U
#define TIM_OCIDLESTATE_SET        0x00000100U
#define TIM_OCNIDLESTATE_RESET     0x00000000U
#define TIM_OCNIDLESTATE_SET       0x00000200U

#define TIM_INPUTCHANNELPOLARITY_RISING   0x00000000U
#define TIM_INPUTCHANNELPOLARITY_FALLING  0x00000002U
#define TIM_INPUTCHANNELPOLARITY_BOTHEDGE 0x0000000AU
#define TIM_ICPOLARITY_RISING   TIM_INPUTCHANNELPOLARITY_RISING
#define TIM_ICPOLARITY_FALLING  TIM_INPUTCHANNELPOLARITY_FALLING
#define TIM_ICPOLARITY_BOTHEDGE TIM_INPUTCHANNELPOLARITY_BOTHEDGE
#define TIM_ICSELECTION_DIRECTTI   0x00000001U
#define TIM_ICSELECTION_INDIRECTTI 0x00000002U
#define TIM_ICSELECTION_TRC        0x00000003U
#define TIM_ICPSC_DIV1 0x00000000U
#define TIM_ICPSC_DIV2 0x00000004U
#define TIM_ICPSC_DIV4 0x00000008U
#define TIM_ICPSC_DIV8 0x0000000CU

#define TIM_OPMODE_REPETITIVE 0x00000000U
#define TIM_OPMODE_SINGLE     0x00000008U

#define TIM_IT_UPDATE  GD32_HAL_TIMER_INTERRUPT_UPDATE
#define TIM_IT_CC1     GD32_HAL_TIMER_INTERRUPT_CC1
#define TIM_IT_CC2     GD32_HAL_TIMER_INTERRUPT_CC2
#define TIM_IT_CC3     GD32_HAL_TIMER_INTERRUPT_CC3
#define TIM_IT_CC4     GD32_HAL_TIMER_INTERRUPT_CC4
#define TIM_IT_COM     GD32_HAL_TIMER_INTERRUPT_COM
#define TIM_IT_TRIGGER GD32_HAL_TIMER_INTERRUPT_TRIGGER
#define TIM_IT_BREAK   GD32_HAL_TIMER_INTERRUPT_BREAK

#define TIM_DMA_UPDATE  ((uint16_t)(1U << 0U))
#define TIM_DMA_CC1     ((uint16_t)(1U << 1U))
#define TIM_DMA_CC2     ((uint16_t)(1U << 2U))
#define TIM_DMA_CC3     ((uint16_t)(1U << 3U))
#define TIM_DMA_CC4     ((uint16_t)(1U << 4U))
#define TIM_DMA_COM     ((uint16_t)(1U << 5U))
#define TIM_DMA_TRIGGER ((uint16_t)(1U << 6U))

#define TIM_FLAG_UPDATE  GD32_HAL_TIMER_FLAG_UPDATE
#define TIM_FLAG_CC1     GD32_HAL_TIMER_FLAG_CC1
#define TIM_FLAG_CC2     GD32_HAL_TIMER_FLAG_CC2
#define TIM_FLAG_CC3     GD32_HAL_TIMER_FLAG_CC3
#define TIM_FLAG_CC4     GD32_HAL_TIMER_FLAG_CC4
#define TIM_FLAG_COM     GD32_HAL_TIMER_FLAG_COM
#define TIM_FLAG_TRIGGER GD32_HAL_TIMER_FLAG_TRIGGER
#define TIM_FLAG_BREAK   GD32_HAL_TIMER_FLAG_BREAK
#define TIM_FLAG_CC1OF   GD32_HAL_TIMER_FLAG_CC1_OVER
#define TIM_FLAG_CC2OF   GD32_HAL_TIMER_FLAG_CC2_OVER
#define TIM_FLAG_CC3OF   GD32_HAL_TIMER_FLAG_CC3_OVER
#define TIM_FLAG_CC4OF   GD32_HAL_TIMER_FLAG_CC4_OVER

#define TIM_CHANNEL_1   0x00000000U
#define TIM_CHANNEL_2   0x00000004U
#define TIM_CHANNEL_3   0x00000008U
#define TIM_CHANNEL_4   0x0000000CU
#define TIM_CHANNEL_ALL 0x0000003CU

#define TIM_TRGO_RESET  0x00000000U
#define TIM_TRGO_ENABLE 0x00000010U
#define TIM_TRGO_UPDATE 0x00000020U
#define TIM_TRGO_OC1    0x00000030U
#define TIM_TRGO_OC1REF 0x00000040U
#define TIM_TRGO_OC2REF 0x00000050U
#define TIM_TRGO_OC3REF 0x00000060U
#define TIM_TRGO_OC4REF 0x00000070U
#define TIM_MASTERSLAVEMODE_DISABLE 0x00000000U
#define TIM_MASTERSLAVEMODE_ENABLE  0x00000080U

#define TIM_SLAVEMODE_DISABLE   0x00000000U
#define TIM_SLAVEMODE_RESET     0x00000004U
#define TIM_SLAVEMODE_GATED     0x00000005U
#define TIM_SLAVEMODE_TRIGGER   0x00000006U
#define TIM_SLAVEMODE_EXTERNAL1 0x00000007U
#define TIM_TS_ITR0    0x00000000U
#define TIM_TS_ITR1    0x00000010U
#define TIM_TS_ITR2    0x00000020U
#define TIM_TS_ITR3    0x00000030U
#define TIM_TS_TI1F_ED 0x00000040U
#define TIM_TS_TI1FP1  0x00000050U
#define TIM_TS_TI2FP2  0x00000060U
#define TIM_TS_ETRF    0x00000070U
#define TIM_TS_NONE    0x0000FFFFU

#define TIM_TRIGGERPOLARITY_NONINVERTED 0x00000000U
#define TIM_TRIGGERPOLARITY_INVERTED    0x00008000U
#define TIM_TRIGGERPOLARITY_RISING      TIM_INPUTCHANNELPOLARITY_RISING
#define TIM_TRIGGERPOLARITY_FALLING     TIM_INPUTCHANNELPOLARITY_FALLING
#define TIM_TRIGGERPOLARITY_BOTHEDGE    TIM_INPUTCHANNELPOLARITY_BOTHEDGE
#define TIM_TRIGGERPRESCALER_DIV1 0x00000000U
#define TIM_TRIGGERPRESCALER_DIV2 0x00001000U
#define TIM_TRIGGERPRESCALER_DIV4 0x00002000U
#define TIM_TRIGGERPRESCALER_DIV8 0x00003000U

#define TIM_CLOCKSOURCE_INTERNAL 0x00000000U

#define TIM_DMA_ID_UPDATE      0U
#define TIM_DMA_ID_CC1         1U
#define TIM_DMA_ID_CC2         2U
#define TIM_DMA_ID_CC3         3U
#define TIM_DMA_ID_CC4         4U
#define TIM_DMA_ID_COMMUTATION 5U
#define TIM_DMA_ID_TRIGGER     6U

#define __HAL_TIM_RESET_HANDLE_STATE(__HANDLE__) \
    ((__HANDLE__)->State = HAL_TIM_STATE_RESET)
#define __HAL_TIM_ENABLE(__HANDLE__) \
    GD32_HAL_TIMER_Enable((uint32_t)(uintptr_t)((__HANDLE__)->Instance))
#define __HAL_TIM_DISABLE(__HANDLE__) \
    GD32_HAL_TIMER_DisableIfIdle((uint32_t)(uintptr_t)((__HANDLE__)->Instance))
#define __HAL_TIM_ENABLE_IT(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_TIMER_SetInterrupt((uint32_t)(uintptr_t)((__HANDLE__)->Instance), (__INTERRUPT__), 1)
#define __HAL_TIM_DISABLE_IT(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_TIMER_SetInterrupt((uint32_t)(uintptr_t)((__HANDLE__)->Instance), (__INTERRUPT__), 0)
#define __HAL_TIM_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) \
    (GD32_HAL_TIMER_GetInterrupts((uint32_t)(uintptr_t)((__HANDLE__)->Instance)) & (__INTERRUPT__))
#define __HAL_TIM_GET_FLAG(__HANDLE__, __FLAG__) \
    GD32_HAL_TIMER_GetFlag((uint32_t)(uintptr_t)((__HANDLE__)->Instance), (__FLAG__))
#define __HAL_TIM_CLEAR_FLAG(__HANDLE__, __FLAG__) \
    GD32_HAL_TIMER_ClearFlag((uint32_t)(uintptr_t)((__HANDLE__)->Instance), (__FLAG__))
#define __HAL_TIM_CLEAR_IT(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_TIMER_ClearFlag((uint32_t)(uintptr_t)((__HANDLE__)->Instance), (__INTERRUPT__))
#define __HAL_TIM_ENABLE_DMA(__HANDLE__, __DMA__) \
    GD32_HAL_TIMER_SetDMARequest((uint32_t)(uintptr_t)((__HANDLE__)->Instance), \
                                 (GD32_HAL_TIMERDMARequest)TIM_DMARequestToID(__DMA__), 1)
#define __HAL_TIM_DISABLE_DMA(__HANDLE__, __DMA__) \
    GD32_HAL_TIMER_SetDMARequest((uint32_t)(uintptr_t)((__HANDLE__)->Instance), \
                                 (GD32_HAL_TIMERDMARequest)TIM_DMARequestToID(__DMA__), 0)
#define __HAL_TIM_SET_COUNTER(__HANDLE__, __COUNTER__) \
    GD32_HAL_TIMER_SetCounter((uint32_t)(uintptr_t)((__HANDLE__)->Instance), (__COUNTER__))
#define __HAL_TIM_GET_COUNTER(__HANDLE__) \
    GD32_HAL_TIMER_GetCounter((uint32_t)(uintptr_t)((__HANDLE__)->Instance))
#define __HAL_TIM_SET_AUTORELOAD(__HANDLE__, __AUTORELOAD__) \
    do { \
        GD32_HAL_TIMER_SetAutoReload((uint32_t)(uintptr_t)((__HANDLE__)->Instance), (__AUTORELOAD__)); \
        if ((__AUTORELOAD__) <= 0xFFFFU) { (__HANDLE__)->Init.Period = (__AUTORELOAD__); } \
    } while (0U)
#define __HAL_TIM_GET_AUTORELOAD(__HANDLE__) \
    GD32_HAL_TIMER_GetAutoReload((uint32_t)(uintptr_t)((__HANDLE__)->Instance))
#define __HAL_TIM_SET_PRESCALER(__HANDLE__, __PRESC__) \
    GD32_HAL_TIMER_SetPrescaler((uint32_t)(uintptr_t)((__HANDLE__)->Instance), (__PRESC__))
#define __HAL_TIM_GET_PRESCALER(__HANDLE__) \
    GD32_HAL_TIMER_GetPrescaler((uint32_t)(uintptr_t)((__HANDLE__)->Instance))
#define __HAL_TIM_SET_COMPARE(__HANDLE__, __CHANNEL__, __COMPARE__) \
    GD32_HAL_TIMER_SetCompare((uint32_t)(uintptr_t)((__HANDLE__)->Instance), \
                              (uint8_t)((__CHANNEL__) >> 2U), (__COMPARE__))
#define __HAL_TIM_GET_COMPARE(__HANDLE__, __CHANNEL__) \
    GD32_HAL_TIMER_GetCompare((uint32_t)(uintptr_t)((__HANDLE__)->Instance), \
                              (uint8_t)((__CHANNEL__) >> 2U))

#define IS_TIM_INSTANCE(INSTANCE) \
    GD32_HAL_TIMER_IsInstance((uint32_t)(uintptr_t)(INSTANCE))
#define IS_TIM_CHANNELS(CHANNEL) \
    (((CHANNEL) == TIM_CHANNEL_1) || ((CHANNEL) == TIM_CHANNEL_2) || \
     ((CHANNEL) == TIM_CHANNEL_3) || ((CHANNEL) == TIM_CHANNEL_4))

static inline uint32_t TIM_DMARequestToID(uint32_t request)
{
    uint32_t id = 0U;
    while ((request > 1U) && (id < TIM_DMA_ID_TRIGGER))
    {
        request >>= 1U;
        ++id;
    }
    return id;
}

HAL_StatusTypeDef HAL_TIM_Base_Init(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_Base_DeInit(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_Base_Start(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_Base_Stop(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_Base_Start_IT(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_Base_Stop_IT(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_Base_Start_DMA(TIM_HandleTypeDef *htim,
                                         const uint32_t *pData,
                                         uint16_t Length);
HAL_StatusTypeDef HAL_TIM_Base_Stop_DMA(TIM_HandleTypeDef *htim);

HAL_StatusTypeDef HAL_TIM_OC_Init(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_OC_DeInit(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_OC_ConfigChannel(TIM_HandleTypeDef *htim,
                                           const TIM_OC_InitTypeDef *sConfig,
                                           uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_OC_Start(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_OC_Stop(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_OC_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_OC_Stop_IT(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_OC_Start_DMA(TIM_HandleTypeDef *htim,
                                       uint32_t Channel,
                                       const uint32_t *pData,
                                       uint16_t Length);
HAL_StatusTypeDef HAL_TIM_OC_Stop_DMA(TIM_HandleTypeDef *htim, uint32_t Channel);

HAL_StatusTypeDef HAL_TIM_PWM_Init(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_PWM_DeInit(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_PWM_ConfigChannel(TIM_HandleTypeDef *htim,
                                            const TIM_OC_InitTypeDef *sConfig,
                                            uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_PWM_Stop(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_PWM_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_PWM_Stop_IT(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_PWM_Start_DMA(TIM_HandleTypeDef *htim,
                                        uint32_t Channel,
                                        const uint32_t *pData,
                                        uint16_t Length);
HAL_StatusTypeDef HAL_TIM_PWM_Stop_DMA(TIM_HandleTypeDef *htim, uint32_t Channel);

HAL_StatusTypeDef HAL_TIM_IC_Init(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_IC_DeInit(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_IC_ConfigChannel(TIM_HandleTypeDef *htim,
                                           const TIM_IC_InitTypeDef *sConfig,
                                           uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_IC_Start(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_IC_Stop(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_IC_Stop_IT(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef HAL_TIM_IC_Start_DMA(TIM_HandleTypeDef *htim,
                                       uint32_t Channel,
                                       uint32_t *pData,
                                       uint16_t Length);
HAL_StatusTypeDef HAL_TIM_IC_Stop_DMA(TIM_HandleTypeDef *htim, uint32_t Channel);

HAL_StatusTypeDef HAL_TIM_OnePulse_Init(TIM_HandleTypeDef *htim, uint32_t OnePulseMode);
HAL_StatusTypeDef HAL_TIM_OnePulse_DeInit(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef HAL_TIM_OnePulse_ConfigChannel(TIM_HandleTypeDef *htim,
                                                 TIM_OnePulse_InitTypeDef *sConfig,
                                                 uint32_t OutputChannel,
                                                 uint32_t InputChannel);
HAL_StatusTypeDef HAL_TIM_OnePulse_Start(TIM_HandleTypeDef *htim, uint32_t OutputChannel);
HAL_StatusTypeDef HAL_TIM_OnePulse_Stop(TIM_HandleTypeDef *htim, uint32_t OutputChannel);
HAL_StatusTypeDef HAL_TIM_OnePulse_Start_IT(TIM_HandleTypeDef *htim, uint32_t OutputChannel);
HAL_StatusTypeDef HAL_TIM_OnePulse_Stop_IT(TIM_HandleTypeDef *htim, uint32_t OutputChannel);

HAL_StatusTypeDef HAL_TIM_ConfigClockSource(TIM_HandleTypeDef *htim,
                                             const TIM_ClockConfigTypeDef *sClockSourceConfig);
HAL_StatusTypeDef HAL_TIM_SlaveConfigSynchro(TIM_HandleTypeDef *htim,
                                              const TIM_SlaveConfigTypeDef *sSlaveConfig);
HAL_StatusTypeDef HAL_TIM_SlaveConfigSynchro_IT(TIM_HandleTypeDef *htim,
                                                 const TIM_SlaveConfigTypeDef *sSlaveConfig);
void HAL_TIM_IRQHandler(TIM_HandleTypeDef *htim);
uint32_t HAL_TIM_ReadCapturedValue(const TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_TIM_StateTypeDef HAL_TIM_Base_GetState(const TIM_HandleTypeDef *htim);
HAL_TIM_StateTypeDef HAL_TIM_OC_GetState(const TIM_HandleTypeDef *htim);
HAL_TIM_StateTypeDef HAL_TIM_PWM_GetState(const TIM_HandleTypeDef *htim);
HAL_TIM_StateTypeDef HAL_TIM_IC_GetState(const TIM_HandleTypeDef *htim);
HAL_TIM_StateTypeDef HAL_TIM_OnePulse_GetState(const TIM_HandleTypeDef *htim);
HAL_TIM_ActiveChannel HAL_TIM_GetActiveChannel(const TIM_HandleTypeDef *htim);
HAL_TIM_ChannelStateTypeDef HAL_TIM_GetChannelState(const TIM_HandleTypeDef *htim,
                                                     uint32_t Channel);

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim);
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim);
void HAL_TIM_OC_MspInit(TIM_HandleTypeDef *htim);
void HAL_TIM_OC_MspDeInit(TIM_HandleTypeDef *htim);
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim);
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef *htim);
void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *htim);
void HAL_TIM_IC_MspDeInit(TIM_HandleTypeDef *htim);
void HAL_TIM_OnePulse_MspInit(TIM_HandleTypeDef *htim);
void HAL_TIM_OnePulse_MspDeInit(TIM_HandleTypeDef *htim);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_PeriodElapsedHalfCpltCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_IC_CaptureHalfCpltCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_PWM_PulseFinishedHalfCpltCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_TriggerCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_TriggerHalfCpltCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_ErrorCallback(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif

#endif /* STM32F4XX_HAL_TIM_H */
