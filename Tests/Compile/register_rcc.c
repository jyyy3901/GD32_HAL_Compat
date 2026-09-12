#include "stm32f4xx_hal.h"

/* RCC 寄存器模型不做假 overlay；验证稳定的 HAL clock 宏仍可编译。 */
void RegisterRCCBoundaryPositive(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();
}
