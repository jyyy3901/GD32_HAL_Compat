#ifndef STM32F401_REGISTER_BITS_H
#define STM32F401_REGISTER_BITS_H

/* GPIO：只给严格模式实际开放的寄存器定义位名。 */
#define GPIO_IDR_ID0  (1UL << 0U)
#define GPIO_IDR_ID1  (1UL << 1U)
#define GPIO_IDR_ID2  (1UL << 2U)
#define GPIO_IDR_ID3  (1UL << 3U)
#define GPIO_IDR_ID4  (1UL << 4U)
#define GPIO_IDR_ID5  (1UL << 5U)
#define GPIO_IDR_ID6  (1UL << 6U)
#define GPIO_IDR_ID7  (1UL << 7U)
#define GPIO_IDR_ID8  (1UL << 8U)
#define GPIO_IDR_ID9  (1UL << 9U)
#define GPIO_IDR_ID10 (1UL << 10U)
#define GPIO_IDR_ID11 (1UL << 11U)
#define GPIO_IDR_ID12 (1UL << 12U)
#define GPIO_IDR_ID13 (1UL << 13U)
#define GPIO_IDR_ID14 (1UL << 14U)
#define GPIO_IDR_ID15 (1UL << 15U)
#define GPIO_ODR_OD0  GPIO_IDR_ID0
#define GPIO_ODR_OD1  GPIO_IDR_ID1
#define GPIO_ODR_OD2  GPIO_IDR_ID2
#define GPIO_ODR_OD3  GPIO_IDR_ID3
#define GPIO_ODR_OD4  GPIO_IDR_ID4
#define GPIO_ODR_OD5  GPIO_IDR_ID5
#define GPIO_ODR_OD6  GPIO_IDR_ID6
#define GPIO_ODR_OD7  GPIO_IDR_ID7
#define GPIO_ODR_OD8  GPIO_IDR_ID8
#define GPIO_ODR_OD9  GPIO_IDR_ID9
#define GPIO_ODR_OD10 GPIO_IDR_ID10
#define GPIO_ODR_OD11 GPIO_IDR_ID11
#define GPIO_ODR_OD12 GPIO_IDR_ID12
#define GPIO_ODR_OD13 GPIO_IDR_ID13
#define GPIO_ODR_OD14 GPIO_IDR_ID14
#define GPIO_ODR_OD15 GPIO_IDR_ID15
#define GPIO_BSRR_BS0  (1UL << 0U)
#define GPIO_BSRR_BS1  (1UL << 1U)
#define GPIO_BSRR_BS2  (1UL << 2U)
#define GPIO_BSRR_BS3  (1UL << 3U)
#define GPIO_BSRR_BS4  (1UL << 4U)
#define GPIO_BSRR_BS5  (1UL << 5U)
#define GPIO_BSRR_BS6  (1UL << 6U)
#define GPIO_BSRR_BS7  (1UL << 7U)
#define GPIO_BSRR_BS8  (1UL << 8U)
#define GPIO_BSRR_BS9  (1UL << 9U)
#define GPIO_BSRR_BS10 (1UL << 10U)
#define GPIO_BSRR_BS11 (1UL << 11U)
#define GPIO_BSRR_BS12 (1UL << 12U)
#define GPIO_BSRR_BS13 (1UL << 13U)
#define GPIO_BSRR_BS14 (1UL << 14U)
#define GPIO_BSRR_BS15 (1UL << 15U)
#define GPIO_BSRR_BR0  (1UL << 16U)
#define GPIO_BSRR_BR1  (1UL << 17U)
#define GPIO_BSRR_BR2  (1UL << 18U)
#define GPIO_BSRR_BR3  (1UL << 19U)
#define GPIO_BSRR_BR4  (1UL << 20U)
#define GPIO_BSRR_BR5  (1UL << 21U)
#define GPIO_BSRR_BR6  (1UL << 22U)
#define GPIO_BSRR_BR7  (1UL << 23U)
#define GPIO_BSRR_BR8  (1UL << 24U)
#define GPIO_BSRR_BR9  (1UL << 25U)
#define GPIO_BSRR_BR10 (1UL << 26U)
#define GPIO_BSRR_BR11 (1UL << 27U)
#define GPIO_BSRR_BR12 (1UL << 28U)
#define GPIO_BSRR_BR13 (1UL << 29U)
#define GPIO_BSRR_BR14 (1UL << 30U)
#define GPIO_BSRR_BR15 (1UL << 31U)

/* USART：GD32 STAT0/DATA/BAUD/CTL0..2/GP 与下列语义同位。 */
#define USART_SR_PE    (1UL << 0U)
#define USART_SR_FE    (1UL << 1U)
#define USART_SR_NE    (1UL << 2U)
#define USART_SR_ORE   (1UL << 3U)
#define USART_SR_IDLE  (1UL << 4U)
#define USART_SR_RXNE  (1UL << 5U)
#define USART_SR_TC    (1UL << 6U)
#define USART_SR_TXE   (1UL << 7U)
#define USART_SR_LBD   (1UL << 8U)
#define USART_SR_CTS   (1UL << 9U)
#define USART_CR1_SBK      (1UL << 0U)
#define USART_CR1_RWU      (1UL << 1U)
#define USART_CR1_RE       (1UL << 2U)
#define USART_CR1_TE       (1UL << 3U)
#define USART_CR1_IDLEIE   (1UL << 4U)
#define USART_CR1_RXNEIE   (1UL << 5U)
#define USART_CR1_TCIE     (1UL << 6U)
#define USART_CR1_TXEIE    (1UL << 7U)
#define USART_CR1_PEIE     (1UL << 8U)
#define USART_CR1_PS       (1UL << 9U)
#define USART_CR1_PCE      (1UL << 10U)
#define USART_CR1_WAKE     (1UL << 11U)
#define USART_CR1_M        (1UL << 12U)
#define USART_CR1_UE       (1UL << 13U)
#define USART_CR2_LBDIE    (1UL << 6U)
#define USART_CR2_CLKEN    (1UL << 11U)
#define USART_CR2_STOP     (3UL << 12U)
#define USART_CR2_LINEN    (1UL << 14U)
#define USART_CR3_EIE      (1UL << 0U)
#define USART_CR3_IREN     (1UL << 1U)
#define USART_CR3_IRLP     (1UL << 2U)
#define USART_CR3_HDSEL    (1UL << 3U)
#define USART_CR3_SCEN     (1UL << 5U)
#define USART_CR3_DMAR     (1UL << 6U)
#define USART_CR3_DMAT     (1UL << 7U)
#define USART_CR3_RTSE     (1UL << 8U)
#define USART_CR3_CTSE     (1UL << 9U)
#define USART_CR3_CTSIE    (1UL << 10U)

/* TIMER：布局和常用控制/状态位与 GD32 TIMER0..10 对应。 */
#define TIM_CR1_CEN  (1UL << 0U)
#define TIM_CR1_UDIS (1UL << 1U)
#define TIM_CR1_URS  (1UL << 2U)
#define TIM_CR1_OPM  (1UL << 3U)
#define TIM_CR1_DIR  (1UL << 4U)
#define TIM_CR1_CMS  (3UL << 5U)
#define TIM_CR1_ARPE (1UL << 7U)
#define TIM_CR1_CKD  (3UL << 8U)
#define TIM_CR2_CCPC (1UL << 0U)
#define TIM_CR2_CCUS (1UL << 2U)
#define TIM_CR2_CCDS (1UL << 3U)
#define TIM_CR2_MMS  (7UL << 4U)
#define TIM_CR2_TI1S (1UL << 7U)
#define TIM_SMCR_SMS (7UL << 0U)
#define TIM_SMCR_TS  (7UL << 4U)
#define TIM_SMCR_MSM (1UL << 7U)
#define TIM_DIER_UIE   (1UL << 0U)
#define TIM_DIER_CC1IE (1UL << 1U)
#define TIM_DIER_CC2IE (1UL << 2U)
#define TIM_DIER_CC3IE (1UL << 3U)
#define TIM_DIER_CC4IE (1UL << 4U)
#define TIM_DIER_TIE   (1UL << 6U)
#define TIM_DIER_UDE   (1UL << 8U)
#define TIM_SR_UIF   (1UL << 0U)
#define TIM_SR_CC1IF (1UL << 1U)
#define TIM_SR_CC2IF (1UL << 2U)
#define TIM_SR_CC3IF (1UL << 3U)
#define TIM_SR_CC4IF (1UL << 4U)
#define TIM_SR_TIF   (1UL << 6U)
#define TIM_EGR_UG   (1UL << 0U)
#define TIM_EGR_CC1G (1UL << 1U)
#define TIM_EGR_CC2G (1UL << 2U)
#define TIM_EGR_CC3G (1UL << 3U)
#define TIM_EGR_CC4G (1UL << 4U)
#define TIM_CCER_CC1E  (1UL << 0U)
#define TIM_CCER_CC1P  (1UL << 1U)
#define TIM_CCER_CC1NE (1UL << 2U)
#define TIM_CCER_CC2E  (1UL << 4U)
#define TIM_CCER_CC2P  (1UL << 5U)
#define TIM_CCER_CC3E  (1UL << 8U)
#define TIM_CCER_CC3P  (1UL << 9U)
#define TIM_CCER_CC4E  (1UL << 12U)
#define TIM_CCER_CC4P  (1UL << 13U)
#define TIM_BDTR_MOE (1UL << 15U)

/* ADC：不暴露 GD32 不存在的 OVR；软件直接启动默认禁止。 */
#define ADC_SR_AWD   (1UL << 0U)
#define ADC_SR_EOC   (1UL << 1U)
#define ADC_SR_JEOC  (1UL << 2U)
#define ADC_SR_JSTRT (1UL << 3U)
#define ADC_SR_STRT  (1UL << 4U)
#define ADC_CR1_AWDCH   (0x1FUL << 0U)
#define ADC_CR1_EOCIE   (1UL << 5U)
#define ADC_CR1_AWDIE   (1UL << 6U)
#define ADC_CR1_JEOCIE  (1UL << 7U)
#define ADC_CR1_SCAN    (1UL << 8U)
#define ADC_CR1_AWDSGL  (1UL << 9U)
#define ADC_CR1_JAUTO   (1UL << 10U)
#define ADC_CR1_DISCEN  (1UL << 11U)
#define ADC_CR1_JDISCEN (1UL << 12U)
#define ADC_CR1_DISCNUM (7UL << 13U)
#define ADC_CR1_JAWDEN  (1UL << 22U)
#define ADC_CR1_AWDEN   (1UL << 23U)
#define ADC_CR1_RES     (3UL << 24U)
#define ADC_CR2_ADON    (1UL << 0U)
#define ADC_CR2_CONT    (1UL << 1U)
#define ADC_CR2_DMA     (1UL << 8U)
#define ADC_CR2_ALIGN   (1UL << 11U)
#define ADC_CR2_EXTSEL  (7UL << 17U)
#define ADC_CR2_EXTEN_0 (1UL << 20U)
#define ADC_CR2_EXTEN   ADC_CR2_EXTEN_0
#if (GD32_HAL_ENABLE_UNSAFE_REGISTER_COMPAT != 0)
#define ADC_CR2_SWSTART (1UL << 22U)
#endif
#define ADC_SQR1_L (0xFUL << 20U)

/* SPI/I2S：目标寄存器逐 offset 对应，常用位同位。 */
#define SPI_CR1_CPHA     (1UL << 0U)
#define SPI_CR1_CPOL     (1UL << 1U)
#define SPI_CR1_MSTR     (1UL << 2U)
#define SPI_CR1_BR       (7UL << 3U)
#define SPI_CR1_SPE      (1UL << 6U)
#define SPI_CR1_LSBFIRST (1UL << 7U)
#define SPI_CR1_SSI      (1UL << 8U)
#define SPI_CR1_SSM      (1UL << 9U)
#define SPI_CR1_RXONLY   (1UL << 10U)
#define SPI_CR1_DFF      (1UL << 11U)
#define SPI_CR1_CRCNEXT  (1UL << 12U)
#define SPI_CR1_CRCEN    (1UL << 13U)
#define SPI_CR1_BIDIOE   (1UL << 14U)
#define SPI_CR1_BIDIMODE (1UL << 15U)
#define SPI_CR2_RXDMAEN (1UL << 0U)
#define SPI_CR2_TXDMAEN (1UL << 1U)
#define SPI_CR2_SSOE    (1UL << 2U)
#define SPI_CR2_FRF     (1UL << 4U)
#define SPI_CR2_ERRIE   (1UL << 5U)
#define SPI_CR2_RXNEIE  (1UL << 6U)
#define SPI_CR2_TXEIE   (1UL << 7U)
#define SPI_SR_RXNE   (1UL << 0U)
#define SPI_SR_TXE    (1UL << 1U)
#define SPI_SR_CHSIDE (1UL << 2U)
#define SPI_SR_UDR    (1UL << 3U)
#define SPI_SR_CRCERR (1UL << 4U)
#define SPI_SR_MODF   (1UL << 5U)
#define SPI_SR_OVR    (1UL << 6U)
#define SPI_SR_BSY    (1UL << 7U)
#define SPI_SR_FRE    (1UL << 8U)

/* EXTI 0..18 的寄存器布局和 W1C pending 语义对应。 */
#define EXTI_IMR_MR0 (1UL << 0U)
#define EXTI_IMR_MR1 (1UL << 1U)
#define EXTI_IMR_MR2 (1UL << 2U)
#define EXTI_IMR_MR3 (1UL << 3U)
#define EXTI_IMR_MR4 (1UL << 4U)
#define EXTI_IMR_MR5 (1UL << 5U)
#define EXTI_IMR_MR6 (1UL << 6U)
#define EXTI_IMR_MR7 (1UL << 7U)
#define EXTI_IMR_MR8 (1UL << 8U)
#define EXTI_IMR_MR9 (1UL << 9U)
#define EXTI_IMR_MR10 (1UL << 10U)
#define EXTI_IMR_MR11 (1UL << 11U)
#define EXTI_IMR_MR12 (1UL << 12U)
#define EXTI_IMR_MR13 (1UL << 13U)
#define EXTI_IMR_MR14 (1UL << 14U)
#define EXTI_IMR_MR15 (1UL << 15U)
#define EXTI_IMR_MR16 (1UL << 16U)
#define EXTI_IMR_MR17 (1UL << 17U)
#define EXTI_IMR_MR18 (1UL << 18U)
#define EXTI_PR_PR0 EXTI_IMR_MR0
#define EXTI_PR_PR1 EXTI_IMR_MR1
#define EXTI_PR_PR2 EXTI_IMR_MR2
#define EXTI_PR_PR3 EXTI_IMR_MR3
#define EXTI_PR_PR4 EXTI_IMR_MR4
#define EXTI_PR_PR5 EXTI_IMR_MR5
#define EXTI_PR_PR6 EXTI_IMR_MR6
#define EXTI_PR_PR7 EXTI_IMR_MR7
#define EXTI_PR_PR8 EXTI_IMR_MR8
#define EXTI_PR_PR9 EXTI_IMR_MR9
#define EXTI_PR_PR10 EXTI_IMR_MR10
#define EXTI_PR_PR11 EXTI_IMR_MR11
#define EXTI_PR_PR12 EXTI_IMR_MR12
#define EXTI_PR_PR13 EXTI_IMR_MR13
#define EXTI_PR_PR14 EXTI_IMR_MR14
#define EXTI_PR_PR15 EXTI_IMR_MR15
#define EXTI_PR_PR16 EXTI_IMR_MR16
#define EXTI_PR_PR17 EXTI_IMR_MR17
#define EXTI_PR_PR18 EXTI_IMR_MR18

STM32_COMPAT_STATIC_ASSERT(TIM_CR1_CEN == (1UL << 0U), tim_cen_bit);
STM32_COMPAT_STATIC_ASSERT(USART_SR_RXNE == (1UL << 5U), usart_rxne_bit);
STM32_COMPAT_STATIC_ASSERT(GPIO_ODR_OD5 == (1UL << 5U), gpio_od5_bit);
STM32_COMPAT_STATIC_ASSERT(ADC_CR2_ADON == (1UL << 0U), adc_adon_bit);

#endif /* STM32F401_REGISTER_BITS_H */
