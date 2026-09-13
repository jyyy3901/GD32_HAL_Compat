# STM32F401 Register Compatibility Matrix

本矩阵是 0.10.1 严格模式的公开契约。`A` 表示可安全直接 overlay；`B` 表示寄存器可访问，但必须使用本项目提供的 STM32 名称 bit mask 转换；`C` 表示不提供、编译失败或只能走 HAL/Port 明确错误路径。裸 STM32 绝对地址和 magic mask 永远不在兼容范围。

## Coverage summary

| Feature | Level |
|---|---|
| `HAL_GPIO_WritePin()` 等既有 API | HAL compatible |
| HAL function/type/config macro | source compatible in documented HAL scope |
| `GPIOA/USART1/TIM1/ADC1/SPI1` | real-target-address Instance compatible |
| `GPIOA->ODR` | register compatible, Class A |
| `TIM1->CR1`, `USART1->SR`, `SPI1->DR`, `EXTI->PR` | register compatible, Class A/B by bit |
| ADC HAL Start calibration | automatically compatible |
| ADC direct `ADON` then HAL Start | conditional: HAL calibrates when invalid/disabled was observed |
| ADC direct `ADON+SWSTART` | Class C in strict mode |
| `DMA2_Stream0` in `HAL_DMA_Init()` | init-token compatible for listed mappings |
| TIM CubeMX Stream + Channel | deferred init-token compatible; final event mapping at TIM Start DMA |
| `DMA2_Stream0->CR` | Class C; incomplete type compile failure |
| TIM2/TIM5 32-bit range | Class C; HAL rejects `>0xFFFF`, direct access unsafe |
| `RCC->...`, I2C direct registers, FLASH/FMC direct registers | Class C |

## GPIO

GD32F403 使用 F1 风格 CTL0/CTL1 与集中 AFIO remap；不能伪装成 STM32F4 MODER/AFR 模型。

| STM32 peripheral | STM32 reg | STM32 offset | GD32 peripheral | GD32 reg | GD32 offset | Semantic | Class |
|---|---:|---:|---|---:|---:|---|---:|
| GPIOA-D | MODER | 0x00 | GPIOA-D | CTL0/CTL1 | 0x00/0x04 | per-pin mode encoding differs | C |
| GPIOA-D | OTYPER | 0x04 | GPIOA-D | folded into CTL0/1 | n/a | no independent register | C |
| GPIOA-D | OSPEEDR | 0x08 | GPIOA-D | CTL0/1 + SPD | 0x00/0x04/0x3C | hardware model differs | C |
| GPIOA-D | PUPDR | 0x0C | GPIOA-D | CTL0/1 + OCTL | 0x00/0x04/0x0C | pull model differs | C |
| GPIOA-D | IDR | 0x10 | GPIOA-D | ISTAT | 0x08 | input pin state; compatibility view relocates member | A |
| GPIOA-D | ODR | 0x14 | GPIOA-D | OCTL | 0x0C | output latch | A |
| GPIOA-D | BSRR | 0x18 | GPIOA-D | BOP | 0x10 | low-half set/high-half reset | A |
| GPIOA-D | LCKR | 0x1C | GPIOA-D | LOCK | 0x18 | pin configuration lock sequence | A |
| GPIOA-D | AFR[0/1] | 0x20/0x24 | AFIO | PCF0/PCF1 | separate block | AF/remap topology differs | C |

## USART

STM32 `USART1/2/6` 分别指向 GD32 `USART0/1/2` 的真实地址。

| STM32 reg | offset | GD32 reg | offset | semantic | class |
|---|---:|---|---:|---|---:|
| SR | 0x00 | STAT0 | 0x00 | status/error flags; documented read sequences still required | A |
| DR | 0x04 | DATA | 0x04 | 9-bit data register | A |
| BRR | 0x08 | BAUD | 0x08 | integer/fraction baud divider | A |
| CR1 | 0x0C | CTL0 | 0x0C | enable, RX/TX, parity and interrupts | B |
| CR2 | 0x10 | CTL1 | 0x10 | stop/clock/LIN controls | B |
| CR3 | 0x14 | CTL2 | 0x14 | DMA, flow control and error interrupt | B |
| GTPR | 0x18 | GP | 0x18 | guard time/prescaler | A |

已公开常用 `USART_SR_*`、`USART_CR1_*`、`USART_CR2_*`、`USART_CR3_*` 均采用 GD32 实际 bit。不能用 STM32 literal address 或 magic mask 替代符号。

## TIMER

GD32 TIMER0..4/8..10 的主寄存器块与下表 offset 对齐；0.10.0 仍保留按 source/destination 转换 ITR→ITI 的 Port 逻辑。

| STM32 reg | offset | GD32 reg | offset | semantic | class |
|---|---:|---|---:|---|---:|
| CR1 | 0x00 | CTL0 | 0x00 | counter control | B |
| CR2 | 0x04 | CTL1 | 0x04 | master/output idle control | B |
| SMCR | 0x08 | SMCFG | 0x08 | slave/trigger control; ITR values not directly portable | B |
| DIER | 0x0C | DMAINTEN | 0x0C | DMA/interrupt enable | B |
| SR | 0x10 | INTF | 0x10 | flags; write-0 clear behavior | A |
| EGR | 0x14 | SWEVG | 0x14 | software event generation | A |
| CCMR1 | 0x18 | CHCTL0 | 0x18 | channel 1/2 mode | B |
| CCMR2 | 0x1C | CHCTL1 | 0x1C | channel 3/4 mode | B |
| CCER | 0x20 | CHCTL2 | 0x20 | channel enable/polarity | B |
| CNT | 0x24 | CNT | 0x24 | target counter is 16-bit | A, conditional |
| PSC | 0x28 | PSC | 0x28 | 16-bit prescaler | A |
| ARR | 0x2C | CAR | 0x2C | target auto-reload is 16-bit | A, conditional |
| RCR | 0x30 | CREP | 0x30 | advanced-timer repetition only | A on TIM1; C otherwise |
| CCR1..4 | 0x34..0x40 | CH0CV..CH3CV | 0x34..0x40 | target compare/capture is 16-bit | A, conditional by channel |
| BDTR | 0x44 | CCHP | 0x44 | advanced-timer protection/output | B on TIM1; C otherwise |
| DCR | 0x48 | DMACFG | 0x48 | DMA burst layout; member and HAL burst feature unsupported | C; member absent |
| DMAR | 0x4C | DMATB | 0x4C | DMA transfer buffer; member and HAL burst feature unsupported | C; member absent |

`TIM2/TIM5->CNT/ARR/CCRx` 能编译但只有低 16 位硬件有效，属于 conditional/unsafe register compatibility；产品代码不得用它替代现有 HAL `>0xFFFF` 拒绝门。

## ADC

| STM32 reg | offset | GD32 reg | offset | semantic | class |
|---|---:|---|---:|---|---:|
| SR | 0x00 | STAT | 0x00 | AWD/EOC/JEOC/start flags | B; OVR 不存在 |
| CR1 | 0x04 | CTL0 | 0x04 | scan/watchdog/resolution | B |
| CR2 | 0x08 | CTL1 | 0x08 | enable/DMA/alignment/trigger | B; direct start C |
| SMPR1 | 0x0C | SAMPT0 | 0x0C | channels 10..17 sample time | A/B |
| SMPR2 | 0x10 | SAMPT1 | 0x10 | channels 0..9 sample time | A/B |
| SQR1 | 0x2C | RSQ0 | 0x2C | regular length/ranks 13..16 | A/B |
| SQR2 | 0x30 | RSQ1 | 0x30 | regular ranks 7..12 | A/B |
| SQR3 | 0x34 | RSQ2 | 0x34 | regular ranks 1..6 | A/B |
| DR | 0x4C | RDATA | 0x4C | regular conversion data | A |

严格模式公开 `ADC_CR2_ADON`，因此“寄存器 enable 后调用 HAL Start”可用；HAL 仍检查独立 calibration state。`ADC_CR2_SWSTART` 默认不定义，因为直接启动无法插入稳定等待与校准。GD32 不存在等价 OVR flag，`ADC_SR_OVR` 也不定义。

## SPI

| STM32 reg | offset | GD32 reg | offset | semantic | class |
|---|---:|---|---:|---|---:|
| CR1 | 0x00 | CTL0 | 0x00 | core SPI control | B |
| CR2 | 0x04 | CTL1 | 0x04 | DMA/IRQ/NSS/TI | B |
| SR | 0x08 | STAT | 0x08 | status/error flags | A |
| DR | 0x0C | DATA | 0x0C | 16-bit data register | A |
| CRCPR | 0x10 | CRCPOLY | 0x10 | CRC polynomial | A; HAL CRC mode remains unsupported |
| RXCRCR | 0x14 | RCRC | 0x14 | RX CRC | A |
| TXCRCR | 0x18 | TCRC | 0x18 | TX CRC | A |
| I2SCFGR | 0x1C | I2SCTL | 0x1C | I2S mode control | B |
| I2SPR | 0x20 | I2SPSC | 0x20 | I2S prescaler | B |

## EXTI

| STM32 reg | offset | GD32 reg | offset | semantic | class |
|---|---:|---|---:|---|---:|
| IMR | 0x00 | INTEN | 0x00 | interrupt mask | A |
| EMR | 0x04 | EVEN | 0x04 | event mask | A |
| RTSR | 0x08 | RTEN | 0x08 | rising trigger | A |
| FTSR | 0x0C | FTEN | 0x0C | falling trigger | A |
| SWIER | 0x10 | SWIEV | 0x10 | software event | A |
| PR | 0x14 | PD | 0x14 | pending, write-one-to-clear | A |

只保证目标存在的 EXTI0..18；EXTI19..22 不暴露。

## DMA Stream

STM32 Stream 与 GD32 physical Channel 是不同硬件模型。以下分类适用于所有 `DMA1/2_Stream0..7`。

| STM32 reg | STM32 offset | GD32 candidate | GD32 offset | semantic | class |
|---|---:|---|---:|---|---:|
| CR | 0x00 | DMA_CHxCTL | channel-dependent | request select/FIFO/burst model differs | C |
| NDTR | 0x04 | DMA_CHxCNT | channel-dependent | count width/layout not exposed as Stream | C |
| PAR | 0x08 | DMA_CHxPADDR | channel-dependent | address register belongs to resolved physical channel | C |
| M0AR | 0x0C | DMA_CHxMADDR | channel-dependent | no STM32 Stream overlay | C |
| M1AR | 0x10 | none | n/a | double buffer unavailable | C |
| FCR | 0x14 | none | n/a | FIFO unavailable | C |

Stream 名称只用于 `HAL_DMA_Init()` 的初始化 token；`DMA_Stream_TypeDef` 保持 incomplete type。TIM1..5 的已确认 Stream/Channel request 在 `HAL_DMA_Init()` 后保持 deferred，直至 TIM Start DMA 提供 UPDATE/CCx event 上下文；未确认或目标无对应固定 Channel 的组合明确失败。

## RCC / RCU

| STM32 reg | STM32 offset | GD32 reg(s) | GD32 offset | semantic | class |
|---|---:|---|---:|---|---:|
| AHB1ENR | 0x30 | AHBEN + APB2EN | 0x14 + 0x18 | STM32 GPIO/DMA bits cannot share one target word | C |
| APB1ENR | 0x40 | APB1EN | 0x1C | some bits align, whole-register source semantics not proven | C |
| APB2ENR | 0x44 | APB2EN | 0x18 | register offset and several resource bits differ | C |

常用 `RCC_AHB1ENR_*`/`RCC_APB*ENR_*` register-bit 名称不暴露；对应功能通过 `__HAL_RCC_*_CLK_ENABLE/DISABLE()` 保留。

## I2C

offset 虽大部分一致，但严格模式在完成状态读取/清除副作用的板级验证前保持 opaque type。

| STM32 reg | offset | GD32 reg | offset | semantic | class |
|---|---:|---|---:|---|---:|
| CR1 | 0x00 | CTL0 | 0x00 | control/reset/ACK side effects not declared register-safe | C |
| CR2 | 0x04 | CTL1 | 0x04 | clock/interrupt/DMA control | C |
| OAR1 | 0x08 | SADDR0 | 0x08 | own address encoding | C |
| OAR2 | 0x0C | SADDR1 | 0x0C | second address encoding | C |
| DR | 0x10 | DATA | 0x10 | transfer data side effects | C |
| SR1 | 0x14 | STAT0 | 0x14 | flag clearing may depend on ordered reads/writes | C |
| SR2 | 0x18 | STAT1 | ordered read with STAT0 is sequence-sensitive | C |
| CCR | 0x1C | CKCFG | 0x1C | timing encoding requires target clock model | C |
| TRISE | 0x20 | RT | 0x20 | timing semantics require target verification | C |
| FLTR | 0x24 | no same-offset register | n/a | GD32 FMPCFG is at 0x90 and is not STM32 FLTR | C |

## FLASH / FMC

| STM32 reg | STM32 offset | GD32 reg | GD32 offset | semantic | class |
|---|---:|---|---:|---|---:|
| ACR | 0x00 | WS | 0x00 | latency/cache/prefetch field set differs | C |
| KEYR | 0x04 | KEY0 | 0x04 | unlock sequence is target-specific | C |
| OPTKEYR | 0x08 | OBKEY | 0x08 | option-byte programming not supported | C |
| SR | 0x0C | STAT0 | 0x0C | flag layout and clear semantics differ | C |
| CR | 0x10 | CTL0 | 0x10 | page/sector/program model differs | C |
| OPTCR | 0x14 | ADDR0 | 0x14 | target word is address, not option control | C |

## Class C blocks

| Block | Reason | Required path |
|---|---|---|
| DMA Stream registers | STM32 Stream/FIFO/channel-selector model vs GD32 physical Channel model | opaque Stream token + `HAL_DMA_Init()` mapping |
| RCC register overlay | STM32 AHB1ENR 同时容纳 GPIO/DMA，而 GD32 将其分到 APB2EN/AHBEN，单一成员无法安全 overlay | `__HAL_RCC_*` / HAL / Port |
| I2C direct registers | 状态读取/清除顺序与副作用尚未完成逐序列硬件验证 | existing HAL I2C API |
| FLASH/FMC direct registers | unlock/status/clear/program/erase 模型差异 | protected HAL/Port API |

## Evidence boundary

offset/bit 依据 GD32F403 User Manual 与本仓库验证使用的官方 SPL V3.0.3 头文件；ADC calibration 生命周期依据 User Manual §12.4.1：校准因子保持到下一次 ADC power-off。HAL 控制的 disable、DeInit、clock/reset 路径会保守地使软件状态失效；完全绕过 HAL 的 direct power-off/re-enable 无法被软件层观察。静态断言、ARM 编译、官方 SPL 链接和 Host Mock 只构成 code-level validation，不是板级或量产验证。
