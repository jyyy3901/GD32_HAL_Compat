#ifndef GD32F403_RCU_H
#define GD32F403_RCU_H
#include "gd32f403.h"
extern volatile uint32_t mock_rcu_cfg0;
#define RCU_CFG0 mock_rcu_cfg0
#define RCU_CFG0_ADCPSC (3UL << 14U)
#define CK_APB2 0U
#define RCU_CKADC_CKAPB2_DIV2 0U
#define RCU_CKADC_CKAPB2_DIV4 1U
#define RCU_CKADC_CKAPB2_DIV6 2U
#define RCU_CKADC_CKAPB2_DIV8 3U
#define RCU_AF 0U
uint32_t rcu_clock_freq_get(uint32_t clock);
void rcu_adc_clock_config(uint32_t divider);
void rcu_periph_clock_enable(uint32_t periph);
#endif
