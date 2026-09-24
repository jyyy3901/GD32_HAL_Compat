#ifndef STM32_HAL_CMSIS_BRIDGE_H
#define STM32_HAL_CMSIS_BRIDGE_H

/*
 * 唯一允许 HAL compatible 公共层引入目标 device CMSIS 的位置。
 * SPL 外设头文件仍只允许在 Port 层使用。
 *
 * GD32F403 SPL V3.0.3 的 device 头会自行定义
 * USE_STDPERIPH_DRIVER，然后包含项目级 gd32f403_libopt.h。
 * 公共 HAL 头只需要 device/CMSIS 定义，所以在此局部屏蔽
 * libopt 的 include guard，并在 device 头返回后恢复调用方原有宏状态。
 */
#ifdef USE_STDPERIPH_DRIVER
#define STM32_HAL_CMSIS_BRIDGE_KEEP_USE_STDPERIPH_DRIVER
#endif

#ifndef GD32F403_LIBOPT_H
#define GD32F403_LIBOPT_H
#define STM32_HAL_CMSIS_BRIDGE_REMOVE_LIBOPT_GUARD
#endif

#include "gd32f403.h"

#ifdef STM32_HAL_CMSIS_BRIDGE_REMOVE_LIBOPT_GUARD
#undef STM32_HAL_CMSIS_BRIDGE_REMOVE_LIBOPT_GUARD
#undef GD32F403_LIBOPT_H
#endif

#ifdef STM32_HAL_CMSIS_BRIDGE_KEEP_USE_STDPERIPH_DRIVER
#undef STM32_HAL_CMSIS_BRIDGE_KEEP_USE_STDPERIPH_DRIVER
#else
#undef USE_STDPERIPH_DRIVER
#endif

#endif /* STM32_HAL_CMSIS_BRIDGE_H */
