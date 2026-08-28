#ifndef STM32F4XX_HAL_EXTI_H
#define STM32F4XX_HAL_EXTI_H

#include "stm32f4xx_hal_def.h"

typedef enum
{
    HAL_EXTI_COMMON_CB_ID = 0x00U
} EXTI_CallbackIDTypeDef;

typedef struct
{
    uint32_t Line;
    void (*PendingCallback)(void);
} EXTI_HandleTypeDef;

typedef struct
{
    uint32_t Line;
    uint32_t Mode;
    uint32_t Trigger;
    uint32_t GPIOSel;
} EXTI_ConfigTypeDef;

#define EXTI_PROPERTY_SHIFT 24U
#define EXTI_CONFIG (0x02UL << EXTI_PROPERTY_SHIFT)
#define EXTI_GPIO ((0x04UL << EXTI_PROPERTY_SHIFT) | EXTI_CONFIG)
#define EXTI_PIN_MASK 0x0000001FUL

#define EXTI_LINE_0  (EXTI_GPIO | 0x00U)
#define EXTI_LINE_1  (EXTI_GPIO | 0x01U)
#define EXTI_LINE_2  (EXTI_GPIO | 0x02U)
#define EXTI_LINE_3  (EXTI_GPIO | 0x03U)
#define EXTI_LINE_4  (EXTI_GPIO | 0x04U)
#define EXTI_LINE_5  (EXTI_GPIO | 0x05U)
#define EXTI_LINE_6  (EXTI_GPIO | 0x06U)
#define EXTI_LINE_7  (EXTI_GPIO | 0x07U)
#define EXTI_LINE_8  (EXTI_GPIO | 0x08U)
#define EXTI_LINE_9  (EXTI_GPIO | 0x09U)
#define EXTI_LINE_10 (EXTI_GPIO | 0x0AU)
#define EXTI_LINE_11 (EXTI_GPIO | 0x0BU)
#define EXTI_LINE_12 (EXTI_GPIO | 0x0CU)
#define EXTI_LINE_13 (EXTI_GPIO | 0x0DU)
#define EXTI_LINE_14 (EXTI_GPIO | 0x0EU)
#define EXTI_LINE_15 (EXTI_GPIO | 0x0FU)
#define EXTI_LINE_16 (EXTI_CONFIG | 0x10U)
#define EXTI_LINE_17 (EXTI_CONFIG | 0x11U)
#define EXTI_LINE_18 (EXTI_CONFIG | 0x12U)

#define EXTI_MODE_NONE      0x00000000U
#define EXTI_MODE_INTERRUPT 0x00000001U
#define EXTI_MODE_EVENT     0x00000002U
#define EXTI_TRIGGER_NONE            0x00000000U
#define EXTI_TRIGGER_RISING          0x00000001U
#define EXTI_TRIGGER_FALLING         0x00000002U
#define EXTI_TRIGGER_RISING_FALLING  0x00000003U

#define EXTI_GPIOA 0x00000000U
#define EXTI_GPIOB 0x00000001U
#define EXTI_GPIOC 0x00000002U
#define EXTI_GPIOD 0x00000003U

HAL_StatusTypeDef HAL_EXTI_SetConfigLine(EXTI_HandleTypeDef *hexti,
                                         EXTI_ConfigTypeDef *pExtiConfig);
HAL_StatusTypeDef HAL_EXTI_GetConfigLine(EXTI_HandleTypeDef *hexti,
                                         EXTI_ConfigTypeDef *pExtiConfig);
HAL_StatusTypeDef HAL_EXTI_ClearConfigLine(EXTI_HandleTypeDef *hexti);
HAL_StatusTypeDef HAL_EXTI_RegisterCallback(EXTI_HandleTypeDef *hexti,
                                            EXTI_CallbackIDTypeDef CallbackID,
                                            void (*pPendingCbfn)(void));
HAL_StatusTypeDef HAL_EXTI_GetHandle(EXTI_HandleTypeDef *hexti,
                                     uint32_t ExtiLine);
void HAL_EXTI_IRQHandler(EXTI_HandleTypeDef *hexti);
uint32_t HAL_EXTI_GetPending(EXTI_HandleTypeDef *hexti, uint32_t Edge);
void HAL_EXTI_ClearPending(EXTI_HandleTypeDef *hexti, uint32_t Edge);
void HAL_EXTI_GenerateSWI(EXTI_HandleTypeDef *hexti);

#endif /* STM32F4XX_HAL_EXTI_H */
