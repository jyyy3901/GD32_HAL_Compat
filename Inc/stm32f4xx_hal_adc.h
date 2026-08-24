#ifndef STM32F4XX_HAL_ADC_H
#define STM32F4XX_HAL_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_dma.h"

#if (USE_HAL_ADC_REGISTER_CALLBACKS != 0U)
#error "Runtime ADC callback registration is not implemented; use weak callbacks"
#endif

typedef struct GD32_HAL_ADC_TypeDef ADC_TypeDef;

/* STM32F401 只有 ADC1；兼容层把它语义映射到 GD32 ADC0。 */
#ifdef ADC0
#undef ADC0
#endif
#ifdef ADC1
#undef ADC1
#endif
#ifdef ADC2
#undef ADC2
#endif
#define ADC1 ((ADC_TypeDef *)(uintptr_t)GD32_HAL_ADC0_ADDRESS)
#define ADC_IRQn ((IRQn_Type)GD32_HAL_ADC0_1_IRQn_VALUE)

typedef struct
{
    uint32_t ClockPrescaler;
    uint32_t Resolution;
    uint32_t DataAlign;
    FunctionalState ScanConvMode;
    uint32_t EOCSelection;
    FunctionalState ContinuousConvMode;
    uint32_t NbrOfConversion;
    FunctionalState DiscontinuousConvMode;
    uint32_t NbrOfDiscConversion;
    uint32_t ExternalTrigConv;
    uint32_t ExternalTrigConvEdge;
    FunctionalState DMAContinuousRequests;
} ADC_InitTypeDef;

typedef struct
{
    uint32_t Channel;
    uint32_t Rank;
    uint32_t SamplingTime;
    uint32_t Offset;
} ADC_ChannelConfTypeDef;

#define HAL_ADC_STATE_RESET          0x00000000U
#define HAL_ADC_STATE_READY          0x00000001U
#define HAL_ADC_STATE_BUSY_INTERNAL  0x00000002U
#define HAL_ADC_STATE_TIMEOUT        0x00000004U
#define HAL_ADC_STATE_ERROR_INTERNAL 0x00000010U
#define HAL_ADC_STATE_ERROR_CONFIG   0x00000020U
#define HAL_ADC_STATE_ERROR_DMA      0x00000040U
#define HAL_ADC_STATE_REG_BUSY       0x00000100U
#define HAL_ADC_STATE_REG_EOC        0x00000200U
#define HAL_ADC_STATE_REG_OVR        0x00000400U

#define HAL_ADC_ERROR_NONE     0x00000000U
#define HAL_ADC_ERROR_INTERNAL 0x00000001U
#define HAL_ADC_ERROR_OVR      0x00000002U
#define HAL_ADC_ERROR_DMA      0x00000004U

typedef struct
{
    ADC_TypeDef *Instance;
    ADC_InitTypeDef Init;
    __IO uint32_t NbrOfCurrentConversionRank;
    DMA_HandleTypeDef *DMA_Handle;
    HAL_LockTypeDef Lock;
    __IO uint32_t State;
    __IO uint32_t ErrorCode;
} ADC_HandleTypeDef;

#define ADC_CLOCK_SYNC_PCLK_DIV2 0x00000000U
#define ADC_CLOCK_SYNC_PCLK_DIV4 0x00000001U
#define ADC_CLOCK_SYNC_PCLK_DIV6 0x00000002U
#define ADC_CLOCK_SYNC_PCLK_DIV8 0x00000003U

#ifdef ADC_RESOLUTION_12B
#undef ADC_RESOLUTION_12B
#undef ADC_RESOLUTION_10B
#undef ADC_RESOLUTION_8B
#undef ADC_RESOLUTION_6B
#endif
#define ADC_RESOLUTION_12B 0x00000000U
#define ADC_RESOLUTION_10B 0x00000001U
#define ADC_RESOLUTION_8B  0x00000002U
#define ADC_RESOLUTION_6B  0x00000003U

#define ADC_EXTERNALTRIGCONVEDGE_NONE          0x00000000U
#define ADC_EXTERNALTRIGCONVEDGE_RISING        0x00000001U
#define ADC_EXTERNALTRIGCONVEDGE_FALLING       0x00000002U
#define ADC_EXTERNALTRIGCONVEDGE_RISINGFALLING 0x00000003U

#define ADC_EXTERNALTRIGCONV_T1_CC1   0x00000000U
#define ADC_EXTERNALTRIGCONV_T1_CC2   0x00000001U
#define ADC_EXTERNALTRIGCONV_T1_CC3   0x00000002U
#define ADC_EXTERNALTRIGCONV_T2_CC2   0x00000003U
#define ADC_EXTERNALTRIGCONV_T2_CC3   0x00000004U
#define ADC_EXTERNALTRIGCONV_T2_CC4   0x00000005U
#define ADC_EXTERNALTRIGCONV_T2_TRGO  0x00000006U
#define ADC_EXTERNALTRIGCONV_T3_CC1   0x00000007U
#define ADC_EXTERNALTRIGCONV_T3_TRGO  0x00000008U
#define ADC_EXTERNALTRIGCONV_T4_CC4   0x00000009U
#define ADC_EXTERNALTRIGCONV_T5_CC1   0x0000000AU
#define ADC_EXTERNALTRIGCONV_T5_CC2   0x0000000BU
#define ADC_EXTERNALTRIGCONV_T5_CC3   0x0000000CU
#define ADC_EXTERNALTRIGCONV_T8_CC1   0x0000000DU
#define ADC_EXTERNALTRIGCONV_T8_TRGO  0x0000000EU
#define ADC_EXTERNALTRIGCONV_Ext_IT11 0x0000000FU
#define ADC_SOFTWARE_START            0x00000010U

#ifdef ADC_DATAALIGN_RIGHT
#undef ADC_DATAALIGN_RIGHT
#undef ADC_DATAALIGN_LEFT
#endif
#define ADC_DATAALIGN_RIGHT 0x00000000U
#define ADC_DATAALIGN_LEFT  0x00000001U

#ifdef ADC_CHANNEL_0
#undef ADC_CHANNEL_0
#undef ADC_CHANNEL_1
#undef ADC_CHANNEL_2
#undef ADC_CHANNEL_3
#undef ADC_CHANNEL_4
#undef ADC_CHANNEL_5
#undef ADC_CHANNEL_6
#undef ADC_CHANNEL_7
#undef ADC_CHANNEL_8
#undef ADC_CHANNEL_9
#undef ADC_CHANNEL_10
#undef ADC_CHANNEL_11
#undef ADC_CHANNEL_12
#undef ADC_CHANNEL_13
#undef ADC_CHANNEL_14
#undef ADC_CHANNEL_15
#undef ADC_CHANNEL_16
#undef ADC_CHANNEL_17
#endif
#define ADC_CHANNEL_0  0U
#define ADC_CHANNEL_1  1U
#define ADC_CHANNEL_2  2U
#define ADC_CHANNEL_3  3U
#define ADC_CHANNEL_4  4U
#define ADC_CHANNEL_5  5U
#define ADC_CHANNEL_6  6U
#define ADC_CHANNEL_7  7U
#define ADC_CHANNEL_8  8U
#define ADC_CHANNEL_9  9U
#define ADC_CHANNEL_10 10U
#define ADC_CHANNEL_11 11U
#define ADC_CHANNEL_12 12U
#define ADC_CHANNEL_13 13U
#define ADC_CHANNEL_14 14U
#define ADC_CHANNEL_15 15U
#define ADC_CHANNEL_16 16U
#define ADC_CHANNEL_17 17U
#define ADC_CHANNEL_18 18U
#define ADC_CHANNEL_VREFINT ADC_CHANNEL_17
#define ADC_CHANNEL_VBAT    ADC_CHANNEL_18

#define ADC_SAMPLETIME_3CYCLES   0U
#define ADC_SAMPLETIME_15CYCLES  1U
#define ADC_SAMPLETIME_28CYCLES  2U
#define ADC_SAMPLETIME_56CYCLES  3U
#define ADC_SAMPLETIME_84CYCLES  4U
#define ADC_SAMPLETIME_112CYCLES 5U
#define ADC_SAMPLETIME_144CYCLES 6U
#define ADC_SAMPLETIME_480CYCLES 7U

#define ADC_REGULAR_RANK_1  1U
#define ADC_REGULAR_RANK_2  2U
#define ADC_REGULAR_RANK_3  3U
#define ADC_REGULAR_RANK_4  4U
#define ADC_REGULAR_RANK_5  5U
#define ADC_REGULAR_RANK_6  6U
#define ADC_REGULAR_RANK_7  7U
#define ADC_REGULAR_RANK_8  8U
#define ADC_REGULAR_RANK_9  9U
#define ADC_REGULAR_RANK_10 10U
#define ADC_REGULAR_RANK_11 11U
#define ADC_REGULAR_RANK_12 12U
#define ADC_REGULAR_RANK_13 13U
#define ADC_REGULAR_RANK_14 14U
#define ADC_REGULAR_RANK_15 15U
#define ADC_REGULAR_RANK_16 16U

#define ADC_EOC_SEQ_CONV        0x00000000U
#define ADC_EOC_SINGLE_CONV     0x00000001U
#define ADC_EOC_SINGLE_SEQ_CONV 0x00000002U

#ifdef ADC_FLAG_EOC
#undef ADC_FLAG_EOC
#endif
#define ADC_IT_EOC GD32_HAL_ADC_INTERRUPT_EOC
#define ADC_IT_OVR GD32_HAL_ADC_INTERRUPT_OVERRUN_UNAVAILABLE
#define ADC_FLAG_EOC GD32_HAL_ADC_FLAG_EOC
#define ADC_FLAG_OVR GD32_HAL_ADC_FLAG_OVERRUN_UNAVAILABLE

#define __HAL_ADC_RESET_HANDLE_STATE(__HANDLE__) \
    ((__HANDLE__)->State = HAL_ADC_STATE_RESET)
#define __HAL_ADC_ENABLE(__HANDLE__) \
    ((void)GD32_HAL_ADC_EnableAndCalibrate((uint32_t)(uintptr_t)((__HANDLE__)->Instance)))
#define __HAL_ADC_DISABLE(__HANDLE__) \
    GD32_HAL_ADC_Disable((uint32_t)(uintptr_t)((__HANDLE__)->Instance))
#define __HAL_ADC_GET_FLAG(__HANDLE__, __FLAG__) \
    GD32_HAL_ADC_GetFlag((uint32_t)(uintptr_t)((__HANDLE__)->Instance), (__FLAG__))
#define __HAL_ADC_CLEAR_FLAG(__HANDLE__, __FLAG__) \
    GD32_HAL_ADC_ClearFlag((uint32_t)(uintptr_t)((__HANDLE__)->Instance), (__FLAG__))
#define __HAL_ADC_ENABLE_IT(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_ADC_SetInterrupt((uint32_t)(uintptr_t)((__HANDLE__)->Instance), (__INTERRUPT__), 1)
#define __HAL_ADC_DISABLE_IT(__HANDLE__, __INTERRUPT__) \
    GD32_HAL_ADC_SetInterrupt((uint32_t)(uintptr_t)((__HANDLE__)->Instance), (__INTERRUPT__), 0)
#define __HAL_ADC_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) \
    ((GD32_HAL_ADC_GetInterrupts((uint32_t)(uintptr_t)((__HANDLE__)->Instance)) & \
      (__INTERRUPT__)) != 0U)

HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_DeInit(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef *hadc,
                                        ADC_ChannelConfTypeDef *sConfig);
HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef *hadc,
                                            uint32_t Timeout);
uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_Start_IT(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_Stop_IT(ADC_HandleTypeDef *hadc);
HAL_StatusTypeDef HAL_ADC_Start_DMA(ADC_HandleTypeDef *hadc,
                                    uint32_t *pData,
                                    uint32_t Length);
HAL_StatusTypeDef HAL_ADC_Stop_DMA(ADC_HandleTypeDef *hadc);
void HAL_ADC_IRQHandler(ADC_HandleTypeDef *hadc);
uint32_t HAL_ADC_GetState(const ADC_HandleTypeDef *hadc);
uint32_t HAL_ADC_GetError(const ADC_HandleTypeDef *hadc);

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc);
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc);
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc);

#ifdef __cplusplus
}
#endif

#endif /* STM32F4XX_HAL_ADC_H */
