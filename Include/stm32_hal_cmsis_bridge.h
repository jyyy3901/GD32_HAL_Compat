#ifndef STM32_HAL_CMSIS_BRIDGE_H
#define STM32_HAL_CMSIS_BRIDGE_H

/*
 * 唯一允许 HAL compatible 公共层引入目标 device CMSIS 的位置。
 * SPL 外设头文件仍只允许在 Port 层使用。
 */
#include "gd32f403.h"

#endif /* STM32_HAL_CMSIS_BRIDGE_H */
