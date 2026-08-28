# 未支持与有限支持功能

> 0.9.0 的权威限制见 `LIMITATION.md`；本文件保留详细研发记录。

## 1. 规则

- 不允许未实现函数返回 HAL_OK。
- 有状态返回值的功能应返回 HAL_ERROR/HAL_TIMEOUT/HAL_BUSY。
- 无返回值且只能运行期发现的硬件差异，调用 `GD32_HAL_ErrorHook()`。
- 默认错误钩子记录最近错误及 detail；相关配置不会继续执行。
- 无目标硬件实例或无法安全表达的宏，保持未定义或映射到明确的未支持符号，使编译失败。

## 2. 当前未实现模块

以下功能尚无可链接实现或安全等价：

- UART ReceiveToIdle、HalfDuplex、LIN、MultiProcessor
- USART 同步模式 HAL
- ADC injected group / analog watchdog / multimode / oversampling HAL 扩展
- I2C Slave/Listen/Sequential/DMA
- SPI CRC、master 2-lines RX-only、Abort API
- RCC PLL 参数生成、LSE/LSI、RTC/备份域
- FLASH option bytes、mass erase、byte/double-word programming
- EXTI19..22

RCC、FLASH 和独立 EXTI 模块已开启，但只暴露已实现的安全子集；不支持项返回 `HAL_ERROR`、触发错误钩子或保持无声明。

## 3. GPIO 有限支持

| 功能 | 当前行为 | 原因/处理 |
|---|---|---|
| GPIOA/B/C | 支持 0..15 | GD32F403RET6 LQFP64 已封装 |
| GPIOD | 默认仅 PD2；条件开放 PD0/PD1 | 其他 D 引脚未封装；PD0/1 与晶振复用，需停用 HXTAL、remap 并显式开启配置宏 |
| GPIOE/F/G | 不暴露 Instance 宏 | 当前具体型号/封装不可用 |
| Alternate 数字字段 | 非零值触发错误钩子并拒绝本次配置 | STM32 AFR 编号不等于 GD32 AFIO remap |
| 输出/AF Pull-up/down | 触发错误钩子并拒绝本次配置 | GD32 F1 风格 GPIO 缺少该硬件维度 |
| HAL_GPIO_DeInit | 恢复浮空输入、清输出和 EXTI | 不能自动恢复每个外设 remap 位 |

## 4. RCC

- `HAL_RCC_OscConfig()` 支持 HSI/IRC8M 与 HSE/HXTAL 的 ON/OFF/BYPASS；`RCC_PLL_ON`、LSE、LSI 返回 `HAL_ERROR`，因为 STM32 PLLM/N/P/Q 与 GD32 PREDV/整数倍频不可直接换算。
- `HAL_RCC_ClockConfig()` 只切换 HSI、HSE 或已经由 GD32 原生代码配置且运行的 PLL，并校验目标频率、Flash wait state 和切换状态；不能代替产品原生 PLL/USB 时钟初始化。
- CSS、当前配置读取和频率查询已实现；RTC/备份域时钟未实现。
- `__HAL_RCC_I2C3_CLK_ENABLE()` 引用故意不存在的类型，使使用点编译失败，因为 GD32F403RET6 只有 I2C0/I2C1。
- 常用外设 ENABLE/DISABLE 已实现；未提供的 reset/low-power 门控宏不得假定可用。

## 5. TIMER 有限支持

| 功能 | 当前行为 | 原因/处理 |
|---|---|---|
| Base/PWM/OC/IC/OnePulse | 已实现 16 位配置、状态、IRQ 和常用 Callback | 待真实波形与中断板测 |
| TIM2/TIM5 32 位范围 | 初始化/DMA 输出大于 `0xFFFF` 返回 `HAL_ERROR`；void 写宏触发错误钩子并拒绝写入 | GD32F403 所有 TIMER 均为 16 位 |
| ITR0..3 | 按 STM 源定时器语义和 GD ITI 矩阵解析 | 不直接复用数值；保留项返回 `HAL_ERROR` |
| TIM9 ITR2/3 | 返回 `HAL_ERROR` | STM32 是 TIM10_OC/TIM11_OC，GD 对应连接是 TRGO，语义不等价 |
| TIMER DMA | update/CC normal/circular 已接入固定请求校验 | 必须 word/word、正确 Parent/方向/物理 Channel/令牌 |
| CH4 双边沿 IC | 返回 `HAL_ERROR` | GD 通道 3 无等价的互补极性位 |
| 互补输出、死区/刹车 | 不提供相关 API | 高级定时器安全语义尚未实现 |
| Encoder、Hall、DMA Burst | 不提供相关 API | 尚未实现，禁止返回假成功 |
| CMT/BRK IRQ Callback | 当前不处理 | 本阶段 IRQ 覆盖 Update、CC1..4、Trigger |

## 6. DMA 有限支持

| 功能 | 当前行为 | 原因/处理 |
|---|---|---|
| normal/circular | 已实现；circular 保持 BUSY 并重复 Callback | circular 需 Abort/DeInit 显式停止 |
| M2M/P2M/M2P | 已实现 | M2M 只接受 `GD32_DMA_REQUEST_MEMORY` 且拒绝 circular |
| 8/16/32 位和地址递增 | 已实现，并检查起止地址对齐 | 对齐错误返回 HAL_ERROR |
| STM32 `DMA1/2_Streamx` | 故意不定义，负向编译必须失败 | GD32 是固定 Channel 请求模型，不能机械映射 |
| `Init.Channel` | 必须使用 `GD32_DMA_REQUEST_*` | 用表 10-3/10-4 校验物理 Channel 归属 |
| 同一 Channel 多请求 | Start 时独占；冲突返回 HAL_BUSY | 硬件会把多个外设请求直接 OR |
| FIFO/burst/PFCTRL | HAL_DMA_Init 返回 HAL_ERROR/NOT_SUPPORTED | GD32 无硬件等价 |
| double-buffer/M1 Callback | 注册返回 HAL_ERROR/NOT_SUPPORTED | GD32 无 M1 地址寄存器和 DBM |
| circular polling | 返回 HAL_ERROR/NOT_SUPPORTED，传输继续 | polling completion 无法表达持续循环生命周期 |
| Abort_IT | 完成停机后同步调用 Abort Callback | GD32 清 CHEN 不保证再产生可用于异步完成的 IRQ |

DMA HAL 只管理 DMA Channel 自身。UART、TIMER、ADC 与 SPI 已分别配对管理父外设请求使能位；I2C DMA 未实现并明确返回 `HAL_ERROR`。

## 7. UART 有限支持

| 功能 | 当前行为 | 原因/处理 |
|---|---|---|
| polling TX/RX | 已实现 8 位及 9 位无校验路径、统一 Tick timeout | 待真实串口回环验证 |
| IT TX/RX | 已实现独立 `gState/RxState`、计数、IRQ 和 weak Callback | 待真实 IRQ/错误注入验证 |
| PERR/FERR/NERR/ORERR | 按 enable 条件设置 ErrorCode；ORE 终止接收 | 按 STAT0 后 DATA 的顺序清除 |
| `UART_OVERSAMPLING_8` | 初始化返回 HAL_ERROR | GD32F403 固定 16 倍过采样 |
| GD32 UART3/4 | 不暴露成 STM32F401 Instance | 作为 GD32 原生 SPL 扩展 |
| UART DMA normal/circular | 已实现 TX/RX、half/full、Pause/Resume/Stop | normal TX 在 DMA full 后继续等待 USART TC；circular 保持 BUSY |
| UART/DMA Handle 绑定 | 必须 `__HAL_LINKDMA()`，并校验方向、8/16 位宽度、固定 Channel/请求令牌 | 不匹配返回 HAL_ERROR，禁止静默接错请求 |
| DMA/UART error 与 Abort | 关闭 USART request，停止并释放相关 DMA Channel，再恢复 UART 状态/回调 | `HAL_DMA_Abort_IT()` 当前为硬件停机后的同步回调语义 |
| GD32 UART4 DMA | 兼容层不提供假别名 | User Manual 明确 UART4 不支持 DMA |
| 同步/Smartcard/可配置数据极性 | 当前不提供 | USART0/1/2 留待独立 USART HAL；UART3/4 能力更少 |

## 8. ADC 有限支持

| 功能 | 当前行为 | 原因/处理 |
|---|---|---|
| ADC1 | 映射 GD ADC0，polling/IT/DMA 已实现 | STM32F401 只有 ADC1 |
| ADC2/ADC3 | 不暴露 | 源器件不存在，避免把 GD 原生 ADC1/2 泄漏成 STM Instance |
| 校准 | 每次从 disable 启动时自动执行 | GD 校准因子在 ADC 掉电后失效 |
| 多 rank EOC | 只支持 sequence EOC | GD routine scan 仅在序列结束置 EOC |
| falling/both trigger edge | 初始化返回 `HAL_ERROR` | GD ADC 无独立触发极性字段 |
| 不可映射 Timer trigger | 初始化返回 `HAL_ERROR` | TIMER event 集合不等价 |
| STM 480-cycle sample | ConfigChannel 返回 `HAL_ERROR` | GD 最大 239.5 cycles，不能静默缩短 |
| VBAT channel 18 | ConfigChannel 返回 `HAL_ERROR` | GD ADC0 只有 channel 0..17，无等价 VBAT mux |
| Overrun | 不置 `HAL_ADC_ERROR_OVR` | GD ADC_STAT 无 OVR 位或 OVR IRQ；DMA error 仍进入 ErrorCallback |
| ADC clock | 默认限制 0.1..40 MHz | VREFP 2.4..2.6 V 时产品必须把 `GD32_HAL_ADC_MAX_CLOCK_HZ` 收紧为 30 MHz |
| DMA | 仅 DMA0 CH0 / ADC0，P2M word/word | 固定请求硬件与 STM `uint32_t *` buffer 布局 |
| injected/watchdog/multimode/oversampling | 不提供 HAL API | 尚未建立完整状态、IRQ 和跨 ADC 语义 |

## 9. I2C / SPI 有限支持

| 功能 | 当前行为 | 原因/处理 |
|---|---|---|
| I2C master polling/IT | 7/10 位已实现；1/2/N 字节接收分支独立 | 待逻辑分析仪验证 ACK/NACK/STOP 边界 |
| I2C Mem/IsDeviceReady | 仅 7 位；Mem 支持 8/16 位地址与 repeated START | 与 STM32 HAL 对应接口语义一致 |
| I2C bus recovery | timeout/BERR/ARLO 后软件复位并重配外设 | 不会 GPIO bit-bang，无法释放外部拉低的 SDA/SCL |
| I2C DMA/Slave | DMA API 返回 `HAL_ERROR`；Slave API 不提供 | 尚未闭合 DMA last/STOP 与 Slave 状态机 |
| SPI polling/IT | 8/16 位 TX/RX/TxRx；full-duplex 主机 RX 发 dummy frame | TX-only 收尾等 TBE/TRANS 并清 OVR |
| SPI DMA | TX/RX/TxRx normal/circular、half/full/error | 校验 Parent、固定 Channel/request、方向、宽度与模式 |
| SPI CRC enable | Init 返回 `HAL_ERROR` | 尚未实现 CRC next frame、收尾和长度语义 |
| SPI master 2-lines RX-only | Init 返回 `HAL_ERROR` | 目标硬件连续出 SCK，末两帧禁用时序尚未板测 |

## 10. FLASH

| 功能 | 当前行为 | 原因/处理 |
|---|---|---|
| 默认写入 | 全部拒绝 | 产品必须先用 `GD32_HAL_FLASH_SetWritableRegion()` 授权 2 KB 对齐的数据分区 |
| half-word/word polling/IT | 已实现 | 地址、对齐、Flash 范围、授权区和执行页均检查 |
| byte/double-word | 返回 `HAL_ERROR` | 不伪造 STM32 编程粒度语义 |
| STM32 sector 0..7 erase | 展开为对应地址范围内的 GD32 2 KB page | Callback 仍按原 STM32 sector 报告一次 |
| GD32 page erase | `GD32_HAL_FLASHEx_ErasePages*()` | 显式页地址和页数，仍受授权区保护 |
| mass erase / option bytes | 返回 `HAL_ERROR`/不提供 API | 当前安全模型不允许全片破坏性操作 |

## 11. EXTI 有限支持

- 只定义 EXTI0..18；目标没有可安全映射的 STM32 EXTI19..22，负向编译会失败。
- GPIOA/B/C 可路由 0..15；GPIOD 默认仅 PD2，PD0/PD1 需停用 HXTAL 并显式允许。
- GD32 对每条线只有单一 pending 位；双边沿中断不能只凭 pending 位确定上升沿或下降沿。

## 12. 验证边界

当前已完成 ARM 目标静态编译和 Host Mock 行为测试；尚未在真实 GD32F403RET6 板上验证：

- HXTAL/PLL/SystemCoreClock。
- SysTick 实际 1 ms 周期。
- GPIO 电气速度、PD0/PD1 remap。
- EXTI/NVIC 的真实中断响应。
- UART 实际波特率、五个实例的 polling/IT、错误标志和回调顺序。
- USART0/1/2、UART3 的 UART DMA normal/circular、half/full/error IRQ、TC 收尾、Pause/Resume/Stop/Abort 和总线压力。
- DMA 的 12 个 Channel、normal/circular 实际搬运、half/full/error IRQ、Abort 和共享请求冲突。
- TIMER Base/PWM/OC/IC/OnePulse 波形、Update/CC/Trigger IRQ、固定 DMA 请求、主从级联、APB 倍频和高级 TIMER 主输出。
- ADC 单次/连续/scan、Timer/EXTI trigger、校准、known-voltage 精度、温度/Vref、DMA normal/circular 和数据丢失监控。
- I2C 100/400 kHz、7/10 位、repeated START、1/2/N 字节 ACK/NACK/STOP、NACK/ARLO/BERR/timeout、外部拉低总线恢复和 IRQ 延迟。
- SPI 三种实例、四种 CPOL/CPHA、8/16 位、主从、full-duplex dummy RX、IT、normal/circular DMA、OVR/MODF/FRE、最高实际 SCK 与片选时序。
- RCC HSI/HSE/既有 PLL 切换、CSS、Flash wait state、SystemCoreClock 和 Tick 的真实频率。
- 独立 EXTI GPIO/内部线的路由、边沿、事件/中断、共享 IRQ 与软件触发。
- 专用数据分区的 Flash half-word/word、sector-to-page 擦除、IT、错误路径、寿命和掉电恢复。

因此当前成果是可继续开发的 Phase 8 基础设施，不是量产签核版本。
