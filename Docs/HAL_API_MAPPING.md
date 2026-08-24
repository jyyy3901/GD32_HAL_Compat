# STM32F401 HAL 到 GD32F403 SPL API Mapping

## 1. 状态定义

- `已实现`：已有代码，且已通过 ARM Cortex-M4 目标对象严格编译。
- `已实现/有限制`：主语义存在，但硬件差异必须按备注处理。
- `计划`：已有明确实现基础，尚未提交代码。
- `原生初始化`：API 可保留，但硬件初始化应重写为 GD32 SPL/Port 配置。
- `不支持`：当前不能安全等价；不得返回假成功。

### 1.1 固定字段总表

本表统一给出要求的九个字段；后续分模块表用于展开具体 API、标志和回调。`组合`表示必须组合 SPL、软件状态或寄存器语义，不能机械改名。

| STM32 HAL API/族 | STM32 HAL 行为 | GD32 SPL/实现基础 | 直接映射 | 状态机 | IRQ | DMA | 难度 | 当前状态 |
|---|---|---|:---:|:---:|:---:|:---:|---:|---|
| HAL Core/Tick | 初始化、毫秒时基、超时、锁和弱回调 | CMSIS SysTick + 软件状态 + RCU | 组合 | 是 | 是 | 否 | 中 | 已实现 |
| HAL GPIO 基础 I/O | 配置、读写、翻转、锁定 | `gpio_init`、GPIO ISTAT/OCTL/BOP/LOCK | 组合 | 否 | 否 | 否 | 中 | 已实现/有限制 |
| HAL GPIO EXTI | GPIO 路由、pending、IRQ 和回调 | AFIO + EXTI | 组合 | 否 | 是 | 否 | 中 | 已实现 |
| HAL Cortex/NVIC/MPU | 优先级、IRQ 控制、SysTick、MPU | GD32 CMSIS Core | 是 | 否 | 是 | 否 | 低 | 已实现 |
| RCC 门控宏 | 外设时钟 enable/disable | `rcu_periph_clock_enable/disable` | 组合 | 否 | 否 | 否 | 低 | 已实现/有限制 |
| RCC 时钟树 API | 振荡器、PLL、总线频率与 SystemCoreClock | RCU oscillator/prescaler API + 原生 PLL | 否 | 是 | 否 | 否 | 高 | 已实现/PLL 原生初始化 |
| UART polling | 阻塞收发、Tick timeout、Handle 状态 | USART flags/data + HAL Tick | 否 | 是 | 否 | 否 | 中 | 已实现 |
| UART IT/IRQ/Abort | 异步计数、错误、Abort 和回调 | USART interrupt/flags + 软件 Handle | 否 | 是 | 是 | 否 | 高 | 已实现/有限制 |
| UART DMA | 外设 Handle 与 DMA Handle 协同收发 | USART DMA request + DMA HAL | 否 | 是 | 是 | 是 | 高 | 已实现/有限制 |
| USART 同步模式 | 同步时钟收发与单一 State | USART0/1/2 同步功能 | 否 | 是 | 可选 | 可选 | 高 | 计划 |
| DMA 基础/IRQ | 配置、启动、轮询、Abort、half/full/error 回调 | DMA0/1 固定 Channel | 否 | 是 | 可选 | 自身 | 高 | 已实现/有限制 |
| DMA FIFO/Burst/DoubleBuffer | STM32 Stream 扩展能力 | 无等价目标硬件 | 否 | - | - | - | 高 | 不支持 |
| TIMER Base/PWM/OC/IC | 计数、通道输出/输入和回调 | TIMER init/channel/interrupt API | 否 | 是 | 可选 | 可选 | 高 | 已实现/有限制 |
| TIMER Master/Slave/TRGO | 触发输出、内部触发和级联 | master output + slave mode + ITI 源语义查表 | 否 | 是 | 可选 | 可选 | 高 | 已实现/有限制 |
| TIM2/TIM5 32 位范围 | 32 位 ARR/CNT/CCR | GD32 对应 TIMER 仅 16 位 | 否 | - | - | - | 高 | 不支持 |
| ADC polling/IT | 常规序列、触发、校准、转换和错误 | ADC routine + calibration + EOC | 否 | 是 | 是 | 否 | 高 | 已实现/有限制 |
| ADC DMA | 扫描/连续转换的数据搬运和回调 | ADC0 request + DMA HAL | 否 | 是 | 是 | 是 | 高 | 已实现/固定 Channel |
| I2C polling/IT | START、地址、ACK、repeated START、STOP、错误恢复 | I2C flags/data/interrupt API | 否 | 是 | 可选 | 否 | 高 | 已实现/待板测 |
| I2C DMA | 事务尾部 ACK/STOP 与 DMA 协同 | I2C DMA last-transfer + DMA HAL | 否 | 是 | 是 | 是 | 高 | 不支持/明确失败 |
| SPI polling/IT | 双工收发、OVR 与 timeout | SPI flags/data/interrupt API | 否 | 是 | 可选 | 否 | 高 | 已实现/CRC 除外 |
| SPI DMA | TX/RX DMA 完成顺序与错误回调 | SPI DMA request + DMA HAL | 否 | 是 | 是 | 是 | 高 | 已实现/待板测 |
| FLASH/FMC | 解锁、编程、擦除、错误和回调 | FMC page/program/flag API + 显式写区 | 否 | 是 | 可选 | 否 | 高 | 已实现/有限制 |
| 独立 HAL EXTI | Line Handle、配置、IRQ 和 Callback | AFIO + EXTI + 软件 Handle | 否 | 是 | 是 | 否 | 中 | 已实现/0..18 |

## 2. HAL Core

| STM32 HAL API | STM32 行为 | GD32 实现基础 | 直接映射 | 状态机/IRQ/DMA | 难度 | 状态 |
|---|---|---|:---:|---|---:|---|
| HAL_Init | 初始化优先级组、Tick、MSP | CMSIS + RCU_AF + HAL_InitTick | 否 | Tick IRQ | 中 | 已实现 |
| HAL_DeInit | 停 Tick、复位外设、MSP DeInit | SysTick + RCU reset + DMA deinit | 否 | 无 | 中 | 已实现 |
| HAL_MspInit/DeInit | 用户弱回调 | `__weak` | 是 | Callback | 低 | 已实现 |
| HAL_InitTick | 配置周期 Tick | `SystemCoreClockUpdate` + `SysTick_Config` | 组合 | SysTick IRQ | 低 | 已实现 |
| HAL_IncTick | 按 TickFreq 累加 | 软件变量 | 是 | IRQ 调用 | 低 | 已实现 |
| HAL_GetTick | 返回毫秒基准 | 软件变量 | 是 | 无 | 低 | 已实现 |
| HAL_Delay | 基于 Tick 阻塞并处理回绕 | HAL_GetTick | 否 | 依赖 Tick IRQ | 低 | 已实现 |
| HAL_GetTickPrio | 返回 Tick IRQ 优先级 | 软件变量 | 是 | 无 | 低 | 已实现 |
| HAL_SetTickFreq/GetTickFreq | 10/100/1000 Hz 重配 | SysTick reload | 否 | Tick IRQ | 低 | 已实现 |
| HAL_SuspendTick/ResumeTick | 关闭/开启 Tick 中断 | SysTick CTRL.TICKINT | 是 | IRQ | 低 | 已实现 |
| HAL_GetHalVersion | 返回兼容层版本 | 软件常量 | 是 | 无 | 低 | 已实现 |
| HAL_GetUIDw0/1/2 | 读 96 位 UID | 0x1FFFF7E8/EC/F0 | 是 | 无 | 低 | 已实现 |
| __HAL_LOCK/__HAL_UNLOCK | Handle 非 RTOS 锁 | 软件字段 | 是 | 状态 | 低 | 已实现 |
| __HAL_LINKDMA | 外设与 DMA Handle 双向绑定 | 软件指针 | 是 | DMA Parent | 低 | 已实现 |
| GD32_HAL_ErrorHook | void API 的不可等价配置诊断 | 默认记录最近错误，可由产品覆盖 | 组合 | 错误钩子 | 低 | 已实现 |

## 3. GPIO / EXTI

| STM32 HAL API/宏 | GD32 SPL/寄存器基础 | 策略 | IRQ | 难度 | 状态 |
|---|---|---|:---:|---:|---|
| HAL_GPIO_Init | `gpio_init`、`gpio_exti_source_select`、`exti_init` | Mode/Pull/Speed 转换；AF remap 原生配置 | 可选 | 中 | 已实现/有限制 |
| HAL_GPIO_DeInit | `gpio_init(IN_FLOATING)` + EXTI disable/clear | 仅选定引脚 | 否 | 中 | 已实现 |
| HAL_GPIO_ReadPin | `gpio_input_port_get` | 位掩码非零返回 SET | 否 | 低 | 已实现 |
| HAL_GPIO_WritePin | `gpio_bit_set/reset` | 原子置位/复位 | 否 | 低 | 已实现 |
| HAL_GPIO_TogglePin | GPIO_OCTL + GPIO_BOP | 一次 BOP 写完成 set/reset | 否 | 低 | 已实现 |
| HAL_GPIO_LockPin | `gpio_pin_lock` + LKK 验证 | 成功返回 HAL_OK | 否 | 低 | 已实现 |
| HAL_GPIO_EXTI_IRQHandler | EXTI_PD | 检查、清除、Callback | 是 | 低 | 已实现 |
| HAL_GPIO_EXTI_Callback | weak 回调 | 用户覆盖 | 是 | 低 | 已实现 |
| __HAL_GPIO_EXTI_GET/CLEAR_* | EXTI_PD | Port 函数封装 | 是 | 低 | 已实现 |
| __HAL_GPIO_EXTI_GENERATE_SWIT | `exti_software_interrupt_enable` | 直接封装 | 是 | 低 | 已实现 |
| GPIO_Init.Alternate | AFIO 默认映射/remap | STM32 AF 编号无 1:1；非零触发错误钩子 | 否 | 高 | 原生初始化 |
| 输出模式 Pull | GD32 无独立 PUPDR | 触发错误钩子，不伪造寄存器行为 | 否 | 高 | 不支持 |

## 4. Cortex / NVIC / SysTick / MPU

| STM32 HAL API | GD32 实现基础 | 兼容策略 | 状态 |
|---|---|---|---|
| HAL_NVIC_SetPriorityGrouping | CMSIS NVIC_SetPriorityGrouping | 直接封装 | 已实现 |
| HAL_NVIC_SetPriority | NVIC_EncodePriority + NVIC_SetPriority | 保留 preempt/sub priority 语义 | 已实现 |
| HAL_NVIC_EnableIRQ/DisableIRQ | CMSIS | 直接封装 | 已实现 |
| HAL_NVIC_SystemReset | NVIC_SystemReset | 直接封装 | 已实现 |
| HAL_NVIC_GetPriorityGrouping/GetPriority | CMSIS | 直接封装 | 已实现 |
| HAL_NVIC_Get/Set/ClearPendingIRQ | CMSIS | 直接封装 | 已实现 |
| HAL_NVIC_GetActive | CMSIS | 直接封装 | 已实现 |
| HAL_SYSTICK_Config | SysTick_Config | 直接封装 | 已实现 |
| HAL_SYSTICK_CLKSourceConfig | SysTick CTRL.CLKSOURCE | HCLK/HCLK_DIV8 | 已实现 |
| HAL_SYSTICK_IRQHandler/Callback | weak Callback | 应用 SysTick_Handler 调 Port helper | 已实现 |
| HAL_MPU_Enable/Disable | CMSIS MPU/SCB | 语义封装 | 已实现 |
| HAL_MPU_ConfigRegion | MPU RNR/RBAR/RASR | 字段编码与 Cortex-M4 相同 | 已实现 |
| HAL_CORTEX_ClearEvent | SEV + WFE | 直接封装 | 已实现 |

## 5. RCC / RCU

| STM32 HAL API/宏 | GD32 实现基础 | 策略 | 状态 |
|---|---|---|---|
| __HAL_RCC_GPIOA-D_CLK_ENABLE/DISABLE | `rcu_periph_clock_enable/disable` | 直接宏 | 已实现 |
| __HAL_RCC_DMA1/2_CLK_ENABLE/DISABLE | RCU_DMA0/1 | 编号转换 | 已实现 |
| __HAL_RCC_USART1/2/3、UART4/5_CLK_ENABLE | RCU_USART0/1/2、UART3/4 | 编号转换 | 已实现 |
| __HAL_RCC_TIM1..14_CLK_ENABLE/DISABLE | RCU_TIMER0..13 | 编号转换，不表示功能等价 | 已实现 |
| __HAL_RCC_ADC1/2/3_CLK_ENABLE | RCU_ADC0/1/2 | 编号转换 | 已实现 |
| __HAL_RCC_I2C1/2_CLK_ENABLE | RCU_I2C0/1 | 编号转换 | 已实现 |
| __HAL_RCC_I2C3_CLK_ENABLE | 无目标实例 | 使用点触发明确的未定义类型编译错误 | 不支持 |
| __HAL_RCC_SPI1/2/3_CLK_ENABLE | RCU_SPI0/1/2 | 编号转换 | 已实现 |
| HAL_RCC_OscConfig | rcu_osci_* | HSI/HSE ON/OFF/BYPASS；PLL ON、LSE/LSI 失败 | 已实现/有限制 |
| HAL_RCC_ClockConfig | rcu_system/ahb/apb_clock_* | 当前/既有源切换、频率上限、Flash wait、SystemCoreClock、Tick | 已实现/有限制 |
| HAL_RCC_GetSysClockFreq/GetHCLKFreq/GetPCLKxFreq | `rcu_clock_freq_get` | 组合返回 | 已实现 |
| HAL_RCC_GetOscConfig/GetClockConfig、CSS/NMI | RCU config/interrupt | 读取当前配置、时钟监视与 weak Callback | 已实现 |
| SystemClock_Config | `system_gd32f403.c` 或产品自定义 | GD32 原生实现 | 原生初始化 |

## 6. UART

| STM32 HAL API | GD32 SPL 基础 | 直接映射 | 状态机 | IRQ | DMA | 难度 | 状态 |
|---|---|:---:|:---:|:---:|:---:|---:|---|
| HAL_UART_Init/DeInit | `usart_deinit`、baud、word length、stop、parity、enable | 组合 | 是 | 否 | 否 | 中 | 已实现/有限制 |
| HAL_UART_MspInit/DeInit | weak | 是 | 否 | 否 | 否 | 低 | 已实现 |
| HAL_UART_Transmit | TBE/TC + data_transmit + Tick | 否 | gState | 否 | 否 | 中 | 已实现 |
| HAL_UART_Receive | RBNE + data_receive + Tick | 否 | RxState | 否 | 否 | 中 | 已实现 |
| HAL_UART_Transmit_IT | TBE/TC interrupt | 否 | TX count | 是 | 否 | 高 | 已实现 |
| HAL_UART_Receive_IT | RBNE/error interrupt | 否 | RX count/error | 是 | 否 | 高 | 已实现 |
| HAL_UART_Transmit_DMA | `usart_dma_transmit_config` + HAL_DMA；normal 末尾等待 TC | 否 | TX + DMA | 是 | 是 | 高 | 已实现/有限制 |
| HAL_UART_Receive_DMA | `usart_dma_receive_config` + HAL_DMA | 否 | RX + DMA | 是 | 是 | 高 | 已实现/有限制 |
| HAL_UART_DMAPause/Resume/Stop | USART DMA enable + DMA channel | 否 | 是 | 可选 | 是 | 高 | 已实现 |
| HAL_UART_Abort/AbortTransmit/AbortReceive | disable IT/request 并停止已绑定 DMA Channel | 否 | 是 | 是 | 是 | 高 | 已实现/有限制 |
| HAL_UART_*Abort*_IT | DMA 硬件停机后完成回调 | 否 | 是 | 是 | 是 | 高 | 已实现/同步完成 |
| HAL_UART_IRQHandler | STAT/INT enable + Handle；DMA RX 错误协同 Abort | 否 | 是 | 是 | 是 | 高 | 已实现 |
| HAL_UART_GetState/GetError | Handle 字段 | 是 | 是 | 否 | 否 | 低 | 已实现 |
| HAL_UART_Tx/RxCpltCallback | weak | 是 | 是 | 是 | 否 | 中 | 已实现 |
| HAL_UART_ErrorCallback | weak | 是 | 是 | 是 | 否 | 中 | 已实现 |
| HAL_UART_Abort*CpltCallback | weak | 是 | 是 | 是 | 否 | 中 | 已实现 |
| HAL_UARTEx_ReceiveToIdle* | IDLEF + count/DMA counter | 否 | 是 | 是 | 可选 | 高 | 计划 |
| HalfDuplex/LIN/MultiProcessor | GD32 USART mode API | 部分可组合 | 是 | 是 | 可选 | 高 | 计划 |
| UART3/4 CTS/RTS/同步/Smartcard | 硬件不支持 | 否 | - | - | - | - | 不支持 |

第四阶段固定策略：STM32 `USART1/2/3、UART4/5` 分别映射到 GD32 `USART0/1/2、UART3/4`；`UART_OVERSAMPLING_8` 和 UART3/4 硬件流控在初始化时返回 `HAL_ERROR`。UART DMA 只接受 User Manual 固定映射及通过 `__HAL_LINKDMA()` 绑定的 Handle；普通 TX 必须在 DMA full 后继续等待 TC，circular 模式保持 BUSY。GD32 UART4 无 DMA，因此兼容层 `UART5` 的 DMA 启动明确失败。

## 7. USART 同步模式

`HAL_USART_*` 使用与 UART 不同的单一 `State` 和同步时钟语义。USART0/1/2 可由 GD32 USART 同步功能实现；UART3/4 不可用于此模块。Polling/IT/DMA/Abort/IRQ/Callback 全部计划独立实现，不与 UART Handle 强行共用状态机。

## 8. DMA

| STM32 HAL API | GD32 SPL 基础 | 直接映射 | 状态机 | IRQ | 难度 | 状态 |
|---|---|:---:|:---:|:---:|---:|---|
| HAL_DMA_Init | `dma_deinit` + `dma_init` + circular/m2m config | 组合 | 是 | 否 | 高 | 已实现/有限制 |
| HAL_DMA_DeInit | `dma_deinit` | 是 | 是 | 否 | 中 | 已实现 |
| HAL_DMA_Start | address/count/direction + enable | 组合 | 是 | 否 | 高 | 已实现 |
| HAL_DMA_Start_IT | Start + ERR/HTF/FTF interrupt | 组合 | 是 | 是 | 高 | 已实现 |
| HAL_DMA_Abort | interrupt disable + channel disable | 组合 | 是 | 否 | 中 | 已实现 |
| HAL_DMA_Abort_IT | 停止 Channel 后同步完成 Callback | 组合 | 是 | Callback | 高 | 已实现/语义记录 |
| HAL_DMA_PollForTransfer | flag + HAL_GetTick | 组合 | 是 | 否 | 中 | 已实现/normal only |
| HAL_DMA_IRQHandler | ERR/HTF/FTF flag + callbacks | 组合 | 是 | 是 | 高 | 已实现 |
| HAL_DMA_Register/UnRegisterCallback | Handle 函数指针 | 是 | 是 | 否 | 中 | 已实现；M1 拒绝 |
| HAL_DMA_GetState/GetError | Handle 字段 | 是 | 是 | 否 | 低 | 已实现 |
| __HAL_DMA_SET/GET_COUNTER | DMA_CHxCNT Port inline | 组合 | 是 | 否 | 中 | 已实现 |
| FIFO/Burst/DoubleBuffer | 无等价硬件 | 否 | - | - | 高 | 不支持 |

第三阶段固定策略：`Instance` 使用 `GD32_DMA0_CHANNEL0..6` 或 `GD32_DMA1_CHANNEL0..4`；`Init.Channel` 不再表示 STM32 Stream 的可选 CHSEL，而必须填写表 10-3/10-4 对应的 `GD32_DMA_REQUEST_*`。M2M 使用 `GD32_DMA_REQUEST_MEMORY`。同一物理 Channel 的多个 Handle 可初始化，但同一时刻只有一个 Handle 能 Start；Start 获得独占后会重放当前 Handle 配置，防止后 Init 的 Handle 污染宽度/方向。冲突返回 `HAL_BUSY` 并设置 `HAL_DMA_ERROR_CHANNEL_CONFLICT`。

## 9. TIMER

| STM32 HAL API | GD32 SPL 基础 | 状态机 | IRQ | DMA | 难度 | 状态 |
|---|---|:---:|:---:|:---:|---:|---|
| HAL_TIM_Base_Init/DeInit | `timer_init/deinit` | 是 | 否 | 否 | 中 | 已实现 |
| HAL_TIM_Base_Start/Stop | `timer_enable/disable` | 是 | 否 | 否 | 低 | 已实现 |
| HAL_TIM_Base_Start_IT/Stop_IT | update interrupt | 是 | 是 | 否 | 中 | 已实现 |
| HAL_TIM_Base_Start_DMA/Stop_DMA | update DMA request | 是 | 是 | 是 | 高 | 已实现/固定 Channel |
| HAL_TIM_PWM_Init/DeInit | timer init + output config | 是 | 否 | 否 | 中 | 已实现 |
| HAL_TIM_PWM_ConfigChannel | output mode/pulse/polarity/shadow | 是 | 否 | 否 | 高 | 已实现 |
| HAL_TIM_PWM_Start/Stop | channel state + primary output | 是 | 否 | 否 | 中 | 已实现 |
| HAL_TIM_PWM_Start_IT/DMA | channel interrupt/DMA | 是 | 是 | 可选 | 高 | 已实现 |
| HAL_TIM_OC_* | output compare API 组合 | 是 | 可选 | 可选 | 高 | 已实现 |
| HAL_TIM_IC_* | input capture API 组合 | 是 | 可选 | 可选 | 高 | 已实现/CH4 双边沿除外 |
| HAL_TIM_OnePulse_* | single pulse + OC/IC | 是 | 可选 | 否 | 高 | 已实现/CH1-CH2 配对 |
| HAL_TIM_IRQHandler | Update/CHx/TRG flag | 是 | 是 | 可选 | 高 | 已实现 |
| HAL_TIMEx_MasterConfigSynchronization | master output + MSM | 是 | 否 | 否 | 高 | 已实现/能力检查 |
| HAL_TIM_SlaveConfigSynchro | input trigger + slave mode + ITR 语义转换 | 是 | 可选 | 否 | 高 | 已实现/能力检查 |
| __HAL_TIM_SET/GET_COUNTER | TIMER_CNT | 否，Port 封装 | 否 | 否 | 中 | 已实现/16 位检查 |
| __HAL_TIM_SET/GET_COMPARE | TIMER_CHxCV | 否，Channel 转换 | 否 | 否 | 中 | 已实现/16 位检查 |
| __HAL_TIM_SET/GET_AUTORELOAD/PRESCALER | CAR/PSC | 否，Port 封装 | 否 | 否 | 中 | 已实现/16 位检查 |
| TIM2/TIM5 32 位计数 | GD32 TIMER1/4 仅 16 位 | 无等价 | - | - | 高 | 不支持 |

第五阶段固定策略：公共 `TIM1..TIM14` 仅是不透明 Instance，应用不能访问 STM32 寄存器字段。所有目标 TIMER 均按 16 位检查；DMA 使用 32 位内存/外设传输宽度保存 `uint32_t` HAL buffer 布局，同时逐项拒绝大于 `0xFFFF` 的输出数据。F401 ITR 先解析源定时器再查 GD ITI：TIM1 全部、TIM2 ITR0/2/3、TIM3 全部、TIM4 ITR0/1/2、TIM5 ITR0/1/2、TIM9 ITR0/1 可保持语义；保留项及 TIM9 ITR2/3 的 OC 专用连接返回 `HAL_ERROR`。

## 10. ADC

| STM32 HAL API | GD32 SPL 基础 | 状态机 | IRQ | DMA | 难度 | 状态 |
|---|---|:---:|:---:|:---:|---:|---|
| HAL_ADC_Init/DeInit | ADC config/deinit；校准延迟到首次 enable | 是 | 否 | 否 | 高 | 已实现 |
| HAL_ADC_ConfigChannel | routine rank + 保守 sampling 转换 | 是 | 否 | 否 | 中 | 已实现/480 cycles 拒绝 |
| HAL_ADC_Start/Stop | enable + 自动校准 + software/external trigger | 是 | 否 | 否 | 中 | 已实现 |
| HAL_ADC_PollForConversion | sequence EOC + Tick | 是 | 否 | 否 | 中 | 已实现 |
| HAL_ADC_GetValue | routine data register | 否 | 否 | 否 | 低 | 已实现 |
| HAL_ADC_Start_IT/Stop_IT | sequence EOC interrupt | 是 | 是 | 否 | 高 | 已实现/无 OVR IRQ |
| HAL_ADC_Start_DMA/Stop_DMA | ADC request + HAL_DMA | 是 | 是 | 是 | 高 | 已实现/ADC0 固定 DMA0 CH0 |
| HAL_ADC_IRQHandler | EOC | 是 | 是 | 否 | 高 | 已实现/无 OVR/watchdog |
| HAL_ADC_ConvCplt/HalfCplt/ErrorCallback | weak + DMA callback | 是 | 是 | 是 | 高 | 已实现 |
| injected group / analog watchdog | inserted/watchdog SPL | - | - | - | 高 | 不支持 |
| STM32 trigger enum | GD32 trigger enum + ADC0 AFIO remap | 查表转换 | 否 | 否 | 高 | 已实现/不可映射项失败 |

第六阶段固定策略：只暴露 F401 的 `ADC1`，映射目标 ADC0。扫描 EOC 是序列结束而不是每 rank；目标没有 OVR 标志，故不生成虚假的 `HAL_ADC_ERROR_OVR`。DMA 只接受 `GD32_DMA0_CHANNEL0 + GD32_DMA_REQUEST_ADC0`，并要求 Parent、P2M、PINC disable、MINC enable、word/word；`DMAContinuousRequests=ENABLE` 必须 circular，DISABLE 必须 normal。

## 11. I2C

| STM32 HAL API | GD32 SPL 基础 | 状态机 | IRQ | DMA | 难度 | 状态 |
|---|---|:---:|:---:|:---:|---:|---|
| HAL_I2C_Init/DeInit | clock/mode/address/ack/enable | 是 | 否 | 否 | 高 | 已实现/≤400 kHz |
| HAL_I2C_Master_Transmit/Receive | START/address/data/ACK/STOP flags | 是 | 否 | 否 | 高 | 已实现/7、10 位 |
| HAL_I2C_Master_Transmit_IT/Receive_IT | event/error interrupt | 是 | 是 | 否 | 高 | 已实现/7、10 位 |
| HAL_I2C_Master_Transmit_DMA/Receive_DMA | DMA + last transfer | 是 | 是 | 是 | 高 | 不支持/返回 HAL_ERROR |
| HAL_I2C_Mem_Write/Read | repeated START + address bytes | 是 | 可选 | 可选 | 高 | 已实现/7 位、polling |
| HAL_I2C_IsDeviceReady | 多次 START/address/NACK | 是 | 否 | 否 | 高 | 已实现/7 位 |
| HAL_I2C_EV_IRQHandler/HAL_I2C_IRQHandler | SBSEND/ADDSEND/TBE/RBNE/BTC | 是 | 是 | 否 | 高 | 已实现 |
| HAL_I2C_ER_IRQHandler | AERR/BERR/LOSTARB/OUERR/PECERR | 是 | 是 | 否 | 高 | 已实现 |
| HAL_I2C3_* | 无 I2C2 目标实例 | - | - | - | - | 不支持 |

## 12. SPI

| STM32 HAL API | GD32 SPL 基础 | 状态机 | IRQ | DMA | 难度 | 状态 |
|---|---|:---:|:---:|:---:|---:|---|
| HAL_SPI_Init/DeInit | `spi_init/spi_i2s_deinit` | 是 | 否 | 否 | 中 | 已实现/CRC 与 master RXONLY 除外 |
| HAL_SPI_Transmit | TBE/TRANS + transmit + Tick | 是 | 否 | 否 | 中 | 已实现/8、16 位 |
| HAL_SPI_Receive | RBNE + receive；主机收需时钟 | 是 | 否 | 否 | 高 | 已实现/full-duplex dummy clock |
| HAL_SPI_TransmitReceive | TBE/RBNE 协调 | 是 | 否 | 否 | 高 | 已实现 |
| HAL_SPI_*_IT | SPI interrupt flags | 是 | 是 | 否 | 高 | 已实现 |
| HAL_SPI_*_DMA | SPI DMA enable + HAL_DMA | 是 | 是 | 是 | 高 | 已实现/normal、circular |
| HAL_SPI_IRQHandler | TBE/RBNE/MODF/OVR/FRE | 是 | 是 | 可选 | 高 | 已实现 |
| HAL_SPI_GetState/GetError | Handle 字段 | 是 | 否 | 否 | 低 | 已实现 |

第七阶段固定策略：I2C 事务层不直接读取 GD 寄存器；1/2/N 字节接收严格区分 ACK/POS/STOP，Mem Read 在内存地址阶段后发 repeated START。超时/BERR/ARLO 执行外设软件复位和重配，不声称能恢复外部拉低总线。SPI DMA 同时校验 Parent、固定物理 Channel/request、方向、PINC/MINC、BYTE/HALFWORD 宽度和 normal/circular 配对；全双工由 RX DMA 完成统一收尾。

## 13. FLASH / EXTI 独立模块

| STM32 HAL API | GD32 实现基础 | 策略 | 状态 |
|---|---|---|---|
| HAL_FLASH_Unlock/Lock | `fmc_unlock/lock` | 直接封装 | 已实现 |
| HAL_FLASH_Program/Program_IT | word/halfword programming | 默认拒绝；授权区、对齐、执行页检查 | 已实现/有限制 |
| HAL_FLASHEx_Erase/Erase_IT | `fmc_page_erase` | STM32 sector 0..7 展开为 2 KB page | 已实现/有限制 |
| GD32_HAL_FLASHEx_ErasePages* | `fmc_page_erase` | GD32 原生页地址/页数扩展 | 已实现 |
| HAL_FLASH_IRQHandler/Callback | FMC flag/interrupt | 状态机 + weak callback | 已实现 |
| HAL_EXTI_GetHandle | 软件 EXTI Handle | 只接受 EXTI0..18 | 已实现/有限制 |
| HAL_EXTI_Set/Get/ClearConfigLine | AFIO/EXTI | GPIO/内部线、interrupt/event、edge 组合 | 已实现 |
| HAL_EXTI_IRQHandler/Callback | EXTI pending | 清 pending 后调用 Handle Callback | 已实现 |

## 14. Instance 和宏策略

- GPIO 已采用不透明 `GPIO_TypeDef *`，宏值为 GD32 基地址的类型安全指针。
- UART/TIMER/ADC/SPI/I2C 已采用相同不透明 Instance 类型；Port 层转换为 `uint32_t` GD32 基地址。
- DMA 不能仅用基地址，需要显式 `{controller, channel}` 描述符。
- 高频宏不能直接访问 STM32 字段，例如 `Instance->CR1/ARR/CCR`；必须改为 `__STATIC_INLINE` Port 函数。
- 无法安全实现的宏不定义或映射到明确的未支持符号，使使用点在编译阶段暴露。
