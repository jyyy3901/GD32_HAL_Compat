#include "stm32f4xx_hal.h"

void TargetSmoke(void)
{
    RCC_ClkInitTypeDef clock = {0};
    EXTI_HandleTypeDef exti = {0};
    EXTI_ConfigTypeDef exti_config = {0};
    GD32_FLASH_PageEraseInitTypeDef pages = {0};
    uint32_t page_error;

    clock.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
                      RCC_CLOCKTYPE_PCLK2;
    clock.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clock.APB1CLKDivider = RCC_HCLK_DIV2;
    clock.APB2CLKDivider = RCC_HCLK_DIV1;
    (void)HAL_RCC_ClockConfig(&clock, FLASH_LATENCY_1);
    (void)__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST);
    __HAL_RCC_CLEAR_RESET_FLAGS();
    (void)HAL_EXTI_GetHandle(&exti, EXTI_LINE_2);
    exti_config.Line = EXTI_LINE_2;
    exti_config.Mode = EXTI_MODE_INTERRUPT;
    exti_config.Trigger = EXTI_TRIGGER_RISING;
    exti_config.GPIOSel = EXTI_GPIOD;
    (void)HAL_EXTI_SetConfigLine(&exti, &exti_config);
    (void)GD32_HAL_FLASH_SetWritableRegion(0x08060000U, 0x00020000U);
    (void)HAL_FLASH_Unlock();
    (void)HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                            0x08060000U,
                            0x12345678U);
    pages.PageAddress = 0x08060000U;
    pages.NbPages = 1U;
    (void)GD32_HAL_FLASHEx_ErasePages(&pages, &page_error);
    (void)HAL_FLASH_Lock();
}
