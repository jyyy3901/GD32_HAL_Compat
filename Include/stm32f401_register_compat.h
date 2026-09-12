#ifndef STM32F401_REGISTER_COMPAT_H
#define STM32F401_REGISTER_COMPAT_H

#include "stm32_hal_instance_map.h"
#include <stddef.h>
#include <stdint.h>

#ifndef __I
#define __I volatile const
#endif
#ifndef __O
#define __O volatile
#endif
#ifndef __IO
#define __IO volatile
#endif
#ifndef __STATIC_INLINE
#define __STATIC_INLINE static inline
#endif

#ifndef GD32_HAL_STRICT_STM32_COMPAT
#define GD32_HAL_STRICT_STM32_COMPAT 1
#endif

#ifndef GD32_HAL_ENABLE_UNSAFE_REGISTER_COMPAT
#define GD32_HAL_ENABLE_UNSAFE_REGISTER_COMPAT 0
#endif

/* IAR、GCC、ARMClang 都支持的文件域静态断言形式。 */
#define STM32_COMPAT_JOIN_INNER(A, B) A##B
#define STM32_COMPAT_JOIN(A, B) STM32_COMPAT_JOIN_INNER(A, B)
#define STM32_COMPAT_STATIC_ASSERT(EXPR, MESSAGE) \
    typedef char STM32_COMPAT_JOIN(stm32_compat_assert_, __LINE__)[(EXPR) ? 1 : -1]

/*
 * GD32F403 GPIO 不是 STM32F4 MODER/AFR 模型。严格模式只暴露已证明
 * offset 与访问副作用兼容的运行时数据、位操作和锁寄存器。
 */
typedef struct
{
    uint32_t RESERVED0[2];
    __I  uint32_t IDR;
    __IO uint32_t ODR;
    __O  uint32_t BSRR;
    uint32_t RESERVED1;
    __IO uint32_t LCKR;
} GPIO_TypeDef;

typedef struct
{
    __IO uint32_t SR;
    __IO uint32_t DR;
    __IO uint32_t BRR;
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t CR3;
    __IO uint32_t GTPR;
} USART_TypeDef;

typedef struct
{
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SMCR;
    __IO uint32_t DIER;
    __IO uint32_t SR;
    __IO uint32_t EGR;
    __IO uint32_t CCMR1;
    __IO uint32_t CCMR2;
    __IO uint32_t CCER;
    __IO uint32_t CNT;
    __IO uint32_t PSC;
    __IO uint32_t ARR;
    __IO uint32_t RCR;
    __IO uint32_t CCR1;
    __IO uint32_t CCR2;
    __IO uint32_t CCR3;
    __IO uint32_t CCR4;
    __IO uint32_t BDTR;
    /* DMA burst 模型未证明安全，严格模式不暴露 DCR/DMAR 成员。 */
    uint32_t RESERVED1[2];
} TIM_TypeDef;

typedef struct
{
    __IO uint32_t SR;
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SMPR1;
    __IO uint32_t SMPR2;
    uint32_t RESERVED0[6];
    __IO uint32_t SQR1;
    __IO uint32_t SQR2;
    __IO uint32_t SQR3;
    uint32_t RESERVED1[5];
    __IO uint32_t DR;
} ADC_TypeDef;

typedef struct
{
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SR;
    __IO uint32_t DR;
    __IO uint32_t CRCPR;
    __IO uint32_t RXCRCR;
    __IO uint32_t TXCRCR;
    __IO uint32_t I2SCFGR;
    __IO uint32_t I2SPR;
} SPI_TypeDef;

typedef struct
{
    __IO uint32_t IMR;
    __IO uint32_t EMR;
    __IO uint32_t RTSR;
    __IO uint32_t FTSR;
    __IO uint32_t SWIER;
    __IO uint32_t PR;
} EXTI_TypeDef;

typedef struct { uint8_t pad; TIM_TypeDef value; } STM32_COMPAT_TIM_AlignProbe;
typedef struct { uint8_t pad; USART_TypeDef value; } STM32_COMPAT_USART_AlignProbe;

#define GPIOA_BASE STM32_GPIO_INSTANCE_A
#define GPIOB_BASE STM32_GPIO_INSTANCE_B
#define GPIOC_BASE STM32_GPIO_INSTANCE_C
#define GPIOD_BASE STM32_GPIO_INSTANCE_D
#define USART1_BASE STM32_UART_INSTANCE_1
#define USART2_BASE STM32_UART_INSTANCE_2
#define USART6_BASE STM32_UART_INSTANCE_6
#define TIM1_BASE STM32_TIM_INSTANCE_1
#define TIM2_BASE STM32_TIM_INSTANCE_2
#define TIM3_BASE STM32_TIM_INSTANCE_3
#define TIM4_BASE STM32_TIM_INSTANCE_4
#define TIM5_BASE STM32_TIM_INSTANCE_5
#define TIM9_BASE STM32_TIM_INSTANCE_9
#define TIM10_BASE STM32_TIM_INSTANCE_10
#define TIM11_BASE STM32_TIM_INSTANCE_11
#define ADC1_BASE STM32_ADC_INSTANCE_1
#define SPI1_BASE STM32_SPI_INSTANCE_1
#define SPI2_BASE STM32_SPI_INSTANCE_2
#define SPI3_BASE STM32_SPI_INSTANCE_3

#ifdef GPIOA
#undef GPIOA
#endif
#ifdef GPIOB
#undef GPIOB
#endif
#ifdef GPIOC
#undef GPIOC
#endif
#ifdef GPIOD
#undef GPIOD
#endif
#ifdef GPIOE
#undef GPIOE
#endif
#ifdef GPIOF
#undef GPIOF
#endif
#ifdef GPIOG
#undef GPIOG
#endif
#ifdef USART1
#undef USART1
#endif
#ifdef USART2
#undef USART2
#endif
#ifdef USART6
#undef USART6
#endif
#ifdef TIM1
#undef TIM1
#endif
#ifdef TIM2
#undef TIM2
#endif
#ifdef TIM3
#undef TIM3
#endif
#ifdef TIM4
#undef TIM4
#endif
#ifdef TIM5
#undef TIM5
#endif
#ifdef TIM9
#undef TIM9
#endif
#ifdef TIM10
#undef TIM10
#endif
#ifdef TIM11
#undef TIM11
#endif
#ifdef ADC1
#undef ADC1
#endif
#ifdef ADC0
#undef ADC0
#endif
#ifdef ADC2
#undef ADC2
#endif
#ifdef SPI0
#undef SPI0
#endif
#ifdef SPI1
#undef SPI1
#endif
#ifdef SPI2
#undef SPI2
#endif
#ifdef SPI3
#undef SPI3
#endif
#ifdef EXTI
#undef EXTI
#endif

#define GPIOA ((GPIO_TypeDef *)(uintptr_t)GPIOA_BASE)
#define GPIOB ((GPIO_TypeDef *)(uintptr_t)GPIOB_BASE)
#define GPIOC ((GPIO_TypeDef *)(uintptr_t)GPIOC_BASE)
#define GPIOD ((GPIO_TypeDef *)(uintptr_t)GPIOD_BASE)
#define USART1 ((USART_TypeDef *)(uintptr_t)USART1_BASE)
#define USART2 ((USART_TypeDef *)(uintptr_t)USART2_BASE)
#define USART6 ((USART_TypeDef *)(uintptr_t)USART6_BASE)
#define TIM1 ((TIM_TypeDef *)(uintptr_t)TIM1_BASE)
#define TIM2 ((TIM_TypeDef *)(uintptr_t)TIM2_BASE)
#define TIM3 ((TIM_TypeDef *)(uintptr_t)TIM3_BASE)
#define TIM4 ((TIM_TypeDef *)(uintptr_t)TIM4_BASE)
#define TIM5 ((TIM_TypeDef *)(uintptr_t)TIM5_BASE)
#define TIM9 ((TIM_TypeDef *)(uintptr_t)TIM9_BASE)
#define TIM10 ((TIM_TypeDef *)(uintptr_t)TIM10_BASE)
#define TIM11 ((TIM_TypeDef *)(uintptr_t)TIM11_BASE)
#define ADC1 ((ADC_TypeDef *)(uintptr_t)ADC1_BASE)
#define SPI1 ((SPI_TypeDef *)(uintptr_t)SPI1_BASE)
#define SPI2 ((SPI_TypeDef *)(uintptr_t)SPI2_BASE)
#define SPI3 ((SPI_TypeDef *)(uintptr_t)SPI3_BASE)
#define EXTI ((EXTI_TypeDef *)(uintptr_t)EXTI_BASE)

STM32_COMPAT_STATIC_ASSERT(offsetof(GPIO_TypeDef, IDR) == 0x08U, gpio_idr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(GPIO_TypeDef, ODR) == 0x0CU, gpio_odr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(GPIO_TypeDef, BSRR) == 0x10U, gpio_bsrr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(GPIO_TypeDef, LCKR) == 0x18U, gpio_lckr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(USART_TypeDef, SR) == 0x00U, usart_sr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(USART_TypeDef, DR) == 0x04U, usart_dr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(USART_TypeDef, BRR) == 0x08U, usart_brr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(USART_TypeDef, CR1) == 0x0CU, usart_cr1_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(USART_TypeDef, CR2) == 0x10U, usart_cr2_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(USART_TypeDef, CR3) == 0x14U, usart_cr3_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(USART_TypeDef, GTPR) == 0x18U, usart_gtpr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, CR1) == 0x00U, tim_cr1_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, CR2) == 0x04U, tim_cr2_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, SMCR) == 0x08U, tim_smcr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, DIER) == 0x0CU, tim_dier_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, SR) == 0x10U, tim_sr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, EGR) == 0x14U, tim_egr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, CCMR1) == 0x18U, tim_ccmr1_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, CCMR2) == 0x1CU, tim_ccmr2_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, CCER) == 0x20U, tim_ccer_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, CNT) == 0x24U, tim_cnt_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, PSC) == 0x28U, tim_psc_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, ARR) == 0x2CU, tim_arr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, RCR) == 0x30U, tim_rcr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, CCR1) == 0x34U, tim_ccr1_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, CCR2) == 0x38U, tim_ccr2_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, CCR3) == 0x3CU, tim_ccr3_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, CCR4) == 0x40U, tim_ccr4_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(TIM_TypeDef, BDTR) == 0x44U, tim_bdtr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(ADC_TypeDef, SR) == 0x00U, adc_sr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(ADC_TypeDef, CR1) == 0x04U, adc_cr1_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(ADC_TypeDef, CR2) == 0x08U, adc_cr2_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(ADC_TypeDef, SMPR1) == 0x0CU, adc_smpr1_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(ADC_TypeDef, SMPR2) == 0x10U, adc_smpr2_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(ADC_TypeDef, SQR1) == 0x2CU, adc_sqr1_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(ADC_TypeDef, SQR2) == 0x30U, adc_sqr2_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(ADC_TypeDef, SQR3) == 0x34U, adc_sqr3_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(ADC_TypeDef, DR) == 0x4CU, adc_dr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(SPI_TypeDef, CR1) == 0x00U, spi_cr1_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(SPI_TypeDef, CR2) == 0x04U, spi_cr2_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(SPI_TypeDef, SR) == 0x08U, spi_sr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(SPI_TypeDef, DR) == 0x0CU, spi_dr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(SPI_TypeDef, CRCPR) == 0x10U, spi_crcpr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(SPI_TypeDef, RXCRCR) == 0x14U, spi_rxcrcr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(SPI_TypeDef, TXCRCR) == 0x18U, spi_txcrcr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(SPI_TypeDef, I2SCFGR) == 0x1CU, spi_i2scfgr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(SPI_TypeDef, I2SPR) == 0x20U, spi_i2spr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(EXTI_TypeDef, IMR) == 0x00U, exti_imr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(EXTI_TypeDef, EMR) == 0x04U, exti_emr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(EXTI_TypeDef, RTSR) == 0x08U, exti_rtsr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(EXTI_TypeDef, FTSR) == 0x0CU, exti_ftsr_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(EXTI_TypeDef, SWIER) == 0x10U, exti_swier_offset);
STM32_COMPAT_STATIC_ASSERT(offsetof(EXTI_TypeDef, PR) == 0x14U, exti_pr_offset);
STM32_COMPAT_STATIC_ASSERT(sizeof(TIM_TypeDef) == 0x50U, tim_register_size);
STM32_COMPAT_STATIC_ASSERT(sizeof(USART_TypeDef) == 0x1CU, usart_register_size);
STM32_COMPAT_STATIC_ASSERT(sizeof(GPIO_TypeDef) == 0x1CU, gpio_register_size);
STM32_COMPAT_STATIC_ASSERT(sizeof(ADC_TypeDef) == 0x50U, adc_register_size);
STM32_COMPAT_STATIC_ASSERT(sizeof(SPI_TypeDef) == 0x24U, spi_register_size);
STM32_COMPAT_STATIC_ASSERT(sizeof(EXTI_TypeDef) == 0x18U, exti_register_size);
STM32_COMPAT_STATIC_ASSERT(offsetof(STM32_COMPAT_TIM_AlignProbe, value) == 4U,
                           tim_register_alignment);
STM32_COMPAT_STATIC_ASSERT(offsetof(STM32_COMPAT_USART_AlignProbe, value) == 4U,
                           usart_register_alignment);
STM32_COMPAT_STATIC_ASSERT(GPIOA_BASE == 0x40010800UL, gpioa_target_base);
STM32_COMPAT_STATIC_ASSERT(GPIOB_BASE == 0x40010C00UL, gpiob_target_base);
STM32_COMPAT_STATIC_ASSERT(GPIOC_BASE == 0x40011000UL, gpioc_target_base);
STM32_COMPAT_STATIC_ASSERT(GPIOD_BASE == 0x40011400UL, gpiod_target_base);
STM32_COMPAT_STATIC_ASSERT(USART1_BASE == 0x40013800UL, usart1_target_base);
STM32_COMPAT_STATIC_ASSERT(USART2_BASE == 0x40004400UL, usart2_target_base);
STM32_COMPAT_STATIC_ASSERT(USART6_BASE == 0x40004800UL, usart6_target_base);
STM32_COMPAT_STATIC_ASSERT(TIM1_BASE == 0x40012C00UL, tim1_target_base);
STM32_COMPAT_STATIC_ASSERT(TIM2_BASE == 0x40000000UL, tim2_target_base);
STM32_COMPAT_STATIC_ASSERT(TIM3_BASE == 0x40000400UL, tim3_target_base);
STM32_COMPAT_STATIC_ASSERT(TIM4_BASE == 0x40000800UL, tim4_target_base);
STM32_COMPAT_STATIC_ASSERT(TIM5_BASE == 0x40000C00UL, tim5_target_base);
STM32_COMPAT_STATIC_ASSERT(TIM9_BASE == 0x40014C00UL, tim9_target_base);
STM32_COMPAT_STATIC_ASSERT(TIM10_BASE == 0x40015000UL, tim10_target_base);
STM32_COMPAT_STATIC_ASSERT(TIM11_BASE == 0x40015400UL, tim11_target_base);
STM32_COMPAT_STATIC_ASSERT(ADC1_BASE == 0x40012400UL, adc1_target_base);
STM32_COMPAT_STATIC_ASSERT(SPI1_BASE == 0x40013000UL, spi1_target_base);
STM32_COMPAT_STATIC_ASSERT(SPI2_BASE == 0x40003800UL, spi2_target_base);
STM32_COMPAT_STATIC_ASSERT(SPI3_BASE == 0x40003C00UL, spi3_target_base);

#endif /* STM32F401_REGISTER_COMPAT_H */
