#include "gd32f403_gpio.h"
#include "gd32f403_usart.h"
#include "gd32f403_timer.h"
#include "gd32f403_adc.h"
#include "gd32f403_spi.h"
#include "gd32f403_exti.h"

/* 在 public facade 重定义 Instance 名称前保存官方 SPL 的目标地址。 */
enum
{
    GD32_VENDOR_GPIOA_BASE = GPIOA,
    GD32_VENDOR_USART0_BASE = USART0,
    GD32_VENDOR_TIMER0_BASE = TIMER0,
    GD32_VENDOR_ADC0_BASE = ADC0,
    GD32_VENDOR_SPI0_BASE = SPI0,
    GD32_VENDOR_EXTI_BASE = EXTI,
    GD32_VENDOR_GPIO_PIN_0 = GPIO_PIN_0,
    GD32_VENDOR_GPIO_PIN_1 = GPIO_PIN_1,
    GD32_VENDOR_GPIO_PIN_2 = GPIO_PIN_2,
    GD32_VENDOR_GPIO_PIN_3 = GPIO_PIN_3,
    GD32_VENDOR_GPIO_PIN_4 = GPIO_PIN_4,
    GD32_VENDOR_GPIO_PIN_5 = GPIO_PIN_5,
    GD32_VENDOR_GPIO_PIN_6 = GPIO_PIN_6,
    GD32_VENDOR_GPIO_PIN_7 = GPIO_PIN_7,
    GD32_VENDOR_GPIO_PIN_8 = GPIO_PIN_8,
    GD32_VENDOR_GPIO_PIN_9 = GPIO_PIN_9,
    GD32_VENDOR_GPIO_PIN_10 = GPIO_PIN_10,
    GD32_VENDOR_GPIO_PIN_11 = GPIO_PIN_11,
    GD32_VENDOR_GPIO_PIN_12 = GPIO_PIN_12,
    GD32_VENDOR_GPIO_PIN_13 = GPIO_PIN_13,
    GD32_VENDOR_GPIO_PIN_14 = GPIO_PIN_14,
    GD32_VENDOR_GPIO_PIN_15 = GPIO_PIN_15
};

#include "stm32f4xx_hal.h"

STM32_COMPAT_STATIC_ASSERT(GPIOA_BASE == GD32_VENDOR_GPIOA_BASE,
                           gpioa_vendor_base);
STM32_COMPAT_STATIC_ASSERT(USART1_BASE == GD32_VENDOR_USART0_BASE,
                           usart1_vendor_base);
STM32_COMPAT_STATIC_ASSERT(TIM1_BASE == GD32_VENDOR_TIMER0_BASE,
                           tim1_vendor_base);
STM32_COMPAT_STATIC_ASSERT(ADC1_BASE == GD32_VENDOR_ADC0_BASE,
                           adc1_vendor_base);
STM32_COMPAT_STATIC_ASSERT(SPI1_BASE == GD32_VENDOR_SPI0_BASE,
                           spi1_vendor_base);
STM32_COMPAT_STATIC_ASSERT(EXTI_BASE == GD32_VENDOR_EXTI_BASE,
                           exti_vendor_base);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_0 == GD32_VENDOR_GPIO_PIN_0, gpio_pin0_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_1 == GD32_VENDOR_GPIO_PIN_1, gpio_pin1_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_2 == GD32_VENDOR_GPIO_PIN_2, gpio_pin2_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_3 == GD32_VENDOR_GPIO_PIN_3, gpio_pin3_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_4 == GD32_VENDOR_GPIO_PIN_4, gpio_pin4_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_5 == GD32_VENDOR_GPIO_PIN_5, gpio_pin5_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_6 == GD32_VENDOR_GPIO_PIN_6, gpio_pin6_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_7 == GD32_VENDOR_GPIO_PIN_7, gpio_pin7_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_8 == GD32_VENDOR_GPIO_PIN_8, gpio_pin8_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_9 == GD32_VENDOR_GPIO_PIN_9, gpio_pin9_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_10 == GD32_VENDOR_GPIO_PIN_10, gpio_pin10_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_11 == GD32_VENDOR_GPIO_PIN_11, gpio_pin11_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_12 == GD32_VENDOR_GPIO_PIN_12, gpio_pin12_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_13 == GD32_VENDOR_GPIO_PIN_13, gpio_pin13_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_14 == GD32_VENDOR_GPIO_PIN_14, gpio_pin14_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_PIN_15 == GD32_VENDOR_GPIO_PIN_15, gpio_pin15_vendor_bit);

STM32_COMPAT_STATIC_ASSERT(GPIO_ODR_OD5 == GPIO_OCTL_OCTL5,
                           gpio_odr_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_BSRR_BS5 == GPIO_BOP_BOP5,
                           gpio_bsrr_set_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_BSRR_BR5 == GPIO_BOP_CR5,
                           gpio_bsrr_reset_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(USART_SR_RXNE == USART_STAT0_RBNE,
                           usart_rxne_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(USART_SR_TC == USART_STAT0_TC,
                           usart_tc_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(USART_CR1_RE == USART_CTL0_REN,
                           usart_re_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(USART_CR1_TE == USART_CTL0_TEN,
                           usart_te_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(USART_CR1_RXNEIE == USART_CTL0_RBNEIE,
                           usart_rxneie_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(TIM_CR1_CEN == TIMER_CTL0_CEN,
                           tim_cen_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(TIM_DIER_UIE == TIMER_DMAINTEN_UPIE,
                           tim_uie_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(TIM_EGR_UG == TIMER_SWEVG_UPG,
                           tim_ug_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(TIMER_DMA_UPD == TIMER_DMAINTEN_UPDEN,
                           tim_dma_update_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(TIMER_DMA_CH0D == TIMER_DMAINTEN_CH0DEN,
                           tim_dma_ch0_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(TIMER_DMA_CH1D == TIMER_DMAINTEN_CH1DEN,
                           tim_dma_ch1_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(TIMER_DMA_CH2D == TIMER_DMAINTEN_CH2DEN,
                           tim_dma_ch2_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(TIMER_DMA_CH3D == TIMER_DMAINTEN_CH3DEN,
                           tim_dma_ch3_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(TIMER_DMA_CMTD == TIMER_DMAINTEN_CMTDEN,
                           tim_dma_commutation_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(TIMER_DMA_TRGD == TIMER_DMAINTEN_TRGDEN,
                           tim_dma_trigger_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(
    ((TIMER_DMA_UPD | TIMER_DMA_CH0D | TIMER_DMA_CH1D |
      TIMER_DMA_CH2D | TIMER_DMA_CH3D | TIMER_DMA_CMTD |
      TIMER_DMA_TRGD) & 0x00FFU) == 0U,
    tim_dma_sources_do_not_overlap_interrupt_enable_bits);
STM32_COMPAT_STATIC_ASSERT(ADC_SR_EOC == ADC_STAT_EOC,
                           adc_eoc_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(ADC_CR1_SCAN == ADC_CTL0_SM,
                           adc_scan_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(ADC_CR2_ADON == ADC_CTL1_ADCON,
                           adc_adon_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(ADC_SQR1_L == ADC_RSQ0_RL,
                           adc_length_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(SPI_CR1_SPE == SPI_CTL0_SPIEN,
                           spi_enable_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(SPI_SR_RXNE == SPI_STAT_RBNE,
                           spi_rxne_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(EXTI_IMR_MR5 == EXTI_INTEN_INTEN5,
                           exti_imr_vendor_bit);
STM32_COMPAT_STATIC_ASSERT(EXTI_PR_PR5 == EXTI_PD_PD5,
                           exti_pending_vendor_bit);
