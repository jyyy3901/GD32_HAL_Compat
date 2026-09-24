#ifndef STM32F4XX_HAL_H
#define STM32F4XX_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal_def.h"

typedef enum
{
    HAL_TICK_FREQ_10HZ = 100U,
    HAL_TICK_FREQ_100HZ = 10U,
    HAL_TICK_FREQ_1KHZ = 1U,
    HAL_TICK_FREQ_DEFAULT = HAL_TICK_FREQ_1KHZ
} HAL_TickFreqTypeDef;

#define GD32_HAL_COMPAT_VERSION_MAIN 0U
#define GD32_HAL_COMPAT_VERSION_MINOR 10U
#define GD32_HAL_COMPAT_VERSION_PATCH 4U
#define GD32_HAL_COMPAT_VERSION ((GD32_HAL_COMPAT_VERSION_MAIN << 24U) | \
                                 (GD32_HAL_COMPAT_VERSION_MINOR << 16U) | \
                                 (GD32_HAL_COMPAT_VERSION_PATCH << 8U))

extern __IO uint32_t uwTick;
extern uint32_t uwTickPrio;
extern HAL_TickFreqTypeDef uwTickFreq;

HAL_StatusTypeDef HAL_Init(void);
HAL_StatusTypeDef HAL_DeInit(void);
void HAL_MspInit(void);
void HAL_MspDeInit(void);
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority);
void HAL_IncTick(void);
uint32_t HAL_GetTick(void);
uint32_t HAL_GetTickPrio(void);
HAL_StatusTypeDef HAL_SetTickFreq(HAL_TickFreqTypeDef Freq);
HAL_TickFreqTypeDef HAL_GetTickFreq(void);
void HAL_Delay(uint32_t Delay);
void HAL_SuspendTick(void);
void HAL_ResumeTick(void);
uint32_t HAL_GetHalVersion(void);
uint32_t HAL_GetUIDw0(void);
uint32_t HAL_GetUIDw1(void);
uint32_t HAL_GetUIDw2(void);

#ifdef HAL_RCC_MODULE_ENABLED
#include "stm32f4xx_hal_rcc.h"
#endif
#ifdef HAL_FLASH_MODULE_ENABLED
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#endif
#ifdef HAL_EXTI_MODULE_ENABLED
#include "stm32f4xx_hal_exti.h"
#endif
#ifdef HAL_GPIO_MODULE_ENABLED
#include "stm32f4xx_hal_gpio.h"
#endif
#ifdef HAL_CORTEX_MODULE_ENABLED
#include "stm32f4xx_hal_cortex.h"
#endif
#ifdef HAL_DMA_MODULE_ENABLED
#include "stm32f4xx_hal_dma.h"
#endif
#ifdef HAL_UART_MODULE_ENABLED
#include "stm32f4xx_hal_uart.h"
#endif
#ifdef HAL_TIM_MODULE_ENABLED
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_tim_ex.h"
#endif
#ifdef HAL_ADC_MODULE_ENABLED
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_adc_ex.h"
#endif
#ifdef HAL_I2C_MODULE_ENABLED
#include "stm32f4xx_hal_i2c.h"
#endif
#ifdef HAL_SPI_MODULE_ENABLED
#include "stm32f4xx_hal_spi.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* STM32F4XX_HAL_H */
