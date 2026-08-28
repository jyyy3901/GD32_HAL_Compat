#include "stm32f4xx_hal.h"

/*
 * 在产品工程的 SysTick_Handler 中调用本函数。
 * 不直接占用启动文件中的弱中断名，避免与现有工程重复定义。
 */
void GD32_HAL_SysTickIRQHandler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}
