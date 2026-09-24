#include "stm32f4xx_hal.h"

#ifdef GD32_HAL_TEST_EXPECT_USE_STDPERIPH_DRIVER
#ifndef USE_STDPERIPH_DRIVER
#error "CMSIS bridge did not preserve the project USE_STDPERIPH_DRIVER definition"
#endif
#else
#ifdef USE_STDPERIPH_DRIVER
#error "CMSIS bridge leaked the device header USE_STDPERIPH_DRIVER definition"
#endif
#endif

#ifdef GD32F403_LIBOPT_H
#error "public HAL header leaked the project gd32f403_libopt.h"
#endif
#ifdef GD32_HAL_TEST_LIBOPT_INCLUDED
#error "public HAL header included the project gd32f403_libopt.h"
#endif
#ifdef RCU_GPIOA
#error "public HAL header leaked the GD32 RCU peripheral API"
#endif
#ifdef GPIO_MODE_AIN
#error "public HAL header leaked the GD32 GPIO mode API"
#endif
#ifdef TIMER_CHCTL0
#error "public HAL header leaked the GD32 TIMER register API"
#endif

STM32_COMPAT_STATIC_ASSERT(GPIO_MODE_AF_PP == (MODE_AF | OUTPUT_PP),
                           gpio_mode_is_compat_semantics);
STM32_COMPAT_STATIC_ASSERT(ADC_FLAG_EOC == GD32_HAL_ADC_FLAG_EOC,
                           adc_flag_is_compat_semantics);
