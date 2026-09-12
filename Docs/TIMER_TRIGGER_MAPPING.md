# TIMER ITR 到 GD32 ITI 映射

## 原则

STM32 `TIM_TS_ITR0..3` 的数字只是“目标 TIM 的第几个内部输入”，不是全局源编号。GD32 `TIMER_SMCFG_TRGSEL_ITI0..3` 也按目标 TIMER 具有不同来源。因此 `Port/stm32_timer_trigger_map.c` 先解析 STM32F401 ITR 的源定时器语义，再在目标 GD32 TIMER 的 ITI 矩阵中寻找同一源；找不到即返回错误。

## 0.10.0 可保持语义的映射

| STM32 slave | ITR0 | ITR1 | ITR2 | ITR3 |
|---|---|---|---|---|
| TIM1 -> TIMER0 | TIM5 -> ITI0 | TIM2 -> ITI1 | TIM3 -> ITI2 | TIM4 -> ITI3 |
| TIM2 -> TIMER1 | TIM1 -> ITI0 | 保留，拒绝 | TIM3 -> ITI2 | TIM4 -> ITI3 |
| TIM3 -> TIMER2 | TIM1 -> ITI0 | TIM2 -> ITI1 | TIM5 -> ITI2 | TIM4 -> ITI3 |
| TIM4 -> TIMER3 | TIM1 -> ITI0 | TIM2 -> ITI1 | TIM3 -> ITI2 | 保留，拒绝 |
| TIM5 -> TIMER4 | TIM2 -> ITI0 | TIM3 -> ITI1 | TIM4 -> ITI2 | 保留，拒绝 |
| TIM9 -> TIMER8 | TIM2 -> ITI0 | TIM3 -> ITI1 | TIM10_OC，拒绝 | TIM11_OC，拒绝 |

TIM10/TIM11 没有 STM32F401 slave controller，不接受 ITR 配置。TIM9 的 ITR2/3 是 OC 专用连接，而 GD32 TIMER8 ITI2/3 接收 TIMER9/TIMER10 的 TRGO，语义不同，不能只因为编号相同就映射。

`TIM_TS_ITR0..3` 保持 STM32 语义值，不能直接定义成 `TIMER_SMCFG_TRGSEL_ITIx`。即使应用同时使用 `TIMn->SMCR` 兼容寄存器，也必须通过 `HAL_TIM_SlaveConfigSynchro()` 完成 ITR 到 ITI 的 source/destination 二维转换；直接把 ITR mask 写入 `SMCR` 不属于安全兼容范围。

## GD32F403 官方 ITI 矩阵

| GD32 slave | ITI0 | ITI1 | ITI2 | ITI3 |
|---|---|---|---|---|
| TIMER0 | TIMER4_TRGO | TIMER1_TRGO | TIMER2_TRGO | TIMER3_TRGO |
| TIMER7 | TIMER0_TRGO | TIMER1_TRGO | TIMER3_TRGO | TIMER4_TRGO |
| TIMER1 | TIMER0_TRGO | TIMER7_TRGO | TIMER2_TRGO | TIMER3_TRGO |
| TIMER2 | TIMER0_TRGO | TIMER1_TRGO | TIMER4_TRGO | TIMER3_TRGO |
| TIMER3 | TIMER0_TRGO | TIMER1_TRGO | TIMER2_TRGO | TIMER7_TRGO |
| TIMER4 | TIMER1_TRGO | TIMER2_TRGO | TIMER3_TRGO | TIMER7_TRGO |
| TIMER8 | TIMER1_TRGO | TIMER2_TRGO | TIMER9_TRGO | TIMER10_TRGO |
| TIMER11 | TIMER3_TRGO | TIMER4_TRGO | TIMER12_TRGO | TIMER13_TRGO |

表中未被 STM32F401 兼容 Instance 使用的目标 TIMER 仍可由产品通过 GD32 SPL 原生配置。

## TRGO 与 ADC

`HAL_TIMEx_MasterConfigSynchronization()` 只配置目标 TIMER 实际支持的 TRGO。ADC trigger 是另一张固定矩阵，由 `gd32_adc_port.c` 独立转换；不得把 TIMER ITR/ITI 表当作 ADC trigger 表使用。
