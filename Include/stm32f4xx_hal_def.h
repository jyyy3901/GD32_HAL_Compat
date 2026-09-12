#ifndef STM32F4XX_HAL_DEF_H
#define STM32F4XX_HAL_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f401xe.h"
#include "stm32_hal_port_api.h"
#include <stddef.h>
#include <stdint.h>

typedef enum
{
    HAL_OK = 0x00U,
    HAL_ERROR = 0x01U,
    HAL_BUSY = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;

typedef enum
{
    HAL_UNLOCKED = 0x00U,
    HAL_LOCKED = 0x01U
} HAL_LockTypeDef;

/* GD32 CMSIS 使用 ControlStatus；保留 STM32 HAL 的公开类型名称。 */
typedef ControlStatus FunctionalState;

#ifndef UNUSED
#define UNUSED(X) ((void)(X))
#endif

#define HAL_MAX_DELAY 0xFFFFFFFFU
#define HAL_IS_BIT_SET(REG, BIT) ((((REG) & (BIT)) == (BIT)))
#define HAL_IS_BIT_CLR(REG, BIT) ((((REG) & (BIT)) == 0U))

#define __HAL_LINKDMA(__HANDLE__, __PPP_DMA_FIELD__, __DMA_HANDLE__)       \
    do                                                                    \
    {                                                                     \
        (__HANDLE__)->__PPP_DMA_FIELD__ = &(__DMA_HANDLE__);              \
        (__DMA_HANDLE__).Parent = (__HANDLE__);                            \
    } while (0U)

#define __HAL_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = 0U)

#if (USE_RTOS == 1U)
#error "USE_RTOS must remain 0 in GD32 HAL Compatibility Layer"
#else
#define __HAL_LOCK(__HANDLE__)                 \
    do                                         \
    {                                          \
        if ((__HANDLE__)->Lock == HAL_LOCKED)  \
        {                                      \
            return HAL_BUSY;                   \
        }                                      \
        (__HANDLE__)->Lock = HAL_LOCKED;       \
    } while (0U)

#define __HAL_UNLOCK(__HANDLE__)               \
    do                                         \
    {                                          \
        (__HANDLE__)->Lock = HAL_UNLOCKED;     \
    } while (0U)
#endif

#ifndef __weak
#if defined(__ICCARM__)
#define __weak __weak
#elif defined(__GNUC__) || defined(__clang__) || defined(__ARMCC_VERSION)
#define __weak __attribute__((weak))
#else
#define __weak
#endif
#endif

#ifndef __packed
#if defined(__ICCARM__)
#define __packed __packed
#elif defined(__GNUC__) || defined(__clang__) || defined(__ARMCC_VERSION)
#define __packed __attribute__((packed))
#else
#define __packed
#endif
#endif

#if (USE_FULL_ASSERT == 1U)
void assert_failed(uint8_t *file, uint32_t line);
#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
#else
#define assert_param(expr) ((void)0U)
#endif

#ifdef __cplusplus
}
#endif

#endif /* STM32F4XX_HAL_DEF_H */
