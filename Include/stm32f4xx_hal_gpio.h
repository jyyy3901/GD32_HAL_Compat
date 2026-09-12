#ifndef STM32F4XX_HAL_GPIO_H
#define STM32F4XX_HAL_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal_def.h"

/* GPIO_TypeDef/Instance 由严格寄存器兼容层按安全成员统一定义。 */

typedef struct
{
    uint32_t Pin;
    uint32_t Mode;
    uint32_t Pull;
    uint32_t Speed;
    uint32_t Alternate;
} GPIO_InitTypeDef;

typedef enum
{
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET
} GPIO_PinState;

/* 引脚位定义与 GD32 SPL 数值相同；保留 STM32 的 GPIO_PIN_All 拼写。 */
#define GPIO_PIN_All ((uint16_t)0xFFFFU)
#define GPIO_PIN_MASK 0x0000FFFFU

#define GPIO_MODE_Pos 0U
#define GPIO_MODE (0x3UL << GPIO_MODE_Pos)
#define MODE_INPUT (0x0UL << GPIO_MODE_Pos)
#define MODE_OUTPUT (0x1UL << GPIO_MODE_Pos)
#define MODE_AF (0x2UL << GPIO_MODE_Pos)
#define MODE_ANALOG (0x3UL << GPIO_MODE_Pos)
#define OUTPUT_TYPE_Pos 4U
#define OUTPUT_TYPE (0x1UL << OUTPUT_TYPE_Pos)
#define OUTPUT_PP (0x0UL << OUTPUT_TYPE_Pos)
#define OUTPUT_OD (0x1UL << OUTPUT_TYPE_Pos)
#define EXTI_MODE_Pos 16U
#define EXTI_MODE (0x3UL << EXTI_MODE_Pos)
#define EXTI_IT (0x1UL << EXTI_MODE_Pos)
#define EXTI_EVT (0x2UL << EXTI_MODE_Pos)
#define TRIGGER_MODE_Pos 20U
#define TRIGGER_MODE (0x7UL << TRIGGER_MODE_Pos)
#define TRIGGER_RISING (0x1UL << TRIGGER_MODE_Pos)
#define TRIGGER_FALLING (0x2UL << TRIGGER_MODE_Pos)

/* gd32f403_gpio.h 中存在同名但不同编码的宏，必须先取消定义。 */
#ifdef GPIO_MODE_AF_PP
#undef GPIO_MODE_AF_PP
#endif
#ifdef GPIO_MODE_AF_OD
#undef GPIO_MODE_AF_OD
#endif

#define GPIO_MODE_INPUT MODE_INPUT
#define GPIO_MODE_OUTPUT_PP (MODE_OUTPUT | OUTPUT_PP)
#define GPIO_MODE_OUTPUT_OD (MODE_OUTPUT | OUTPUT_OD)
#define GPIO_MODE_AF_PP (MODE_AF | OUTPUT_PP)
#define GPIO_MODE_AF_OD (MODE_AF | OUTPUT_OD)
#define GPIO_MODE_ANALOG MODE_ANALOG
#define GPIO_MODE_IT_RISING (MODE_INPUT | EXTI_IT | TRIGGER_RISING)
#define GPIO_MODE_IT_FALLING (MODE_INPUT | EXTI_IT | TRIGGER_FALLING)
#define GPIO_MODE_IT_RISING_FALLING (MODE_INPUT | EXTI_IT | TRIGGER_RISING | TRIGGER_FALLING)
#define GPIO_MODE_EVT_RISING (MODE_INPUT | EXTI_EVT | TRIGGER_RISING)
#define GPIO_MODE_EVT_FALLING (MODE_INPUT | EXTI_EVT | TRIGGER_FALLING)
#define GPIO_MODE_EVT_RISING_FALLING (MODE_INPUT | EXTI_EVT | TRIGGER_RISING | TRIGGER_FALLING)

#define GPIO_SPEED_FREQ_LOW 0x00000000U
#define GPIO_SPEED_FREQ_MEDIUM 0x00000001U
#define GPIO_SPEED_FREQ_HIGH 0x00000002U
#define GPIO_SPEED_FREQ_VERY_HIGH 0x00000003U

#define GPIO_NOPULL 0x00000000U
#define GPIO_PULLUP 0x00000001U
#define GPIO_PULLDOWN 0x00000002U

/* STM32F401 常见 AF 编码仅用于源码兼容；GD32 AFIO remap 由 MSP 配置。 */
#define GPIO_AF1_TIM1   0x01U
#define GPIO_AF1_TIM2   0x01U
#define GPIO_AF2_TIM3   0x02U
#define GPIO_AF2_TIM4   0x02U
#define GPIO_AF2_TIM5   0x02U
#define GPIO_AF3_TIM9   0x03U
#define GPIO_AF3_TIM10  0x03U
#define GPIO_AF3_TIM11  0x03U
#define GPIO_AF4_I2C1   0x04U
#define GPIO_AF4_I2C2   0x04U
#define GPIO_AF4_I2C3   0x04U
#define GPIO_AF5_SPI1   0x05U
#define GPIO_AF5_SPI2   0x05U
#define GPIO_AF5_SPI3   0x05U
#define GPIO_AF5_SPI4   0x05U
#define GPIO_AF6_SPI3   0x06U
#define GPIO_AF7_USART1 0x07U
#define GPIO_AF7_USART2 0x07U
#define GPIO_AF8_USART6 0x08U

#define __HAL_GPIO_EXTI_GET_FLAG(__EXTI_LINE__) GD32_HAL_GPIO_EXTIPending((uint32_t)(__EXTI_LINE__))
#define __HAL_GPIO_EXTI_CLEAR_FLAG(__EXTI_LINE__) GD32_HAL_GPIO_EXTIClear((uint32_t)(__EXTI_LINE__))
#define __HAL_GPIO_EXTI_GET_IT(__EXTI_LINE__) GD32_HAL_GPIO_EXTIPending((uint32_t)(__EXTI_LINE__))
#define __HAL_GPIO_EXTI_CLEAR_IT(__EXTI_LINE__) GD32_HAL_GPIO_EXTIClear((uint32_t)(__EXTI_LINE__))
#define __HAL_GPIO_EXTI_GENERATE_SWIT(__EXTI_LINE__) GD32_HAL_GPIO_EXTIGenerate((uint32_t)(__EXTI_LINE__))

#define IS_GPIO_PIN_ACTION(ACTION) (((ACTION) == GPIO_PIN_RESET) || ((ACTION) == GPIO_PIN_SET))
#define IS_GPIO_PIN(PIN) (((((uint32_t)(PIN)) & GPIO_PIN_MASK) != 0U) && ((((uint32_t)(PIN)) & ~GPIO_PIN_MASK) == 0U))
#define IS_GPIO_MODE(MODE) (((MODE) == GPIO_MODE_INPUT) || ((MODE) == GPIO_MODE_OUTPUT_PP) || \
                            ((MODE) == GPIO_MODE_OUTPUT_OD) || ((MODE) == GPIO_MODE_AF_PP) || \
                            ((MODE) == GPIO_MODE_AF_OD) || ((MODE) == GPIO_MODE_IT_RISING) || \
                            ((MODE) == GPIO_MODE_IT_FALLING) || ((MODE) == GPIO_MODE_IT_RISING_FALLING) || \
                            ((MODE) == GPIO_MODE_EVT_RISING) || ((MODE) == GPIO_MODE_EVT_FALLING) || \
                            ((MODE) == GPIO_MODE_EVT_RISING_FALLING) || ((MODE) == GPIO_MODE_ANALOG))
#define IS_GPIO_SPEED(SPEED) (((SPEED) == GPIO_SPEED_FREQ_LOW) || ((SPEED) == GPIO_SPEED_FREQ_MEDIUM) || \
                              ((SPEED) == GPIO_SPEED_FREQ_HIGH) || ((SPEED) == GPIO_SPEED_FREQ_VERY_HIGH))
#define IS_GPIO_PULL(PULL) (((PULL) == GPIO_NOPULL) || ((PULL) == GPIO_PULLUP) || ((PULL) == GPIO_PULLDOWN))
#define IS_GPIO_AF(AF) ((AF) <= 15U)

void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);
void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
HAL_StatusTypeDef HAL_GPIO_LockPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void HAL_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif /* STM32F4XX_HAL_GPIO_H */
