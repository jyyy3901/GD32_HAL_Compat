# STM32F401VEH6 与 GD32F403RET6 差异基线

## 1. 结论与适用范围

本文只针对以下组合：

- 源平台：STM32F401VEH6 + STM32F4 HAL V1.8.5。
- 目标平台：GD32F403RET6 + GD32F403 SPL V3.0.3。
- 目标封装：`R` = LQFP64，Flash `E` = 512 KB。

GD32F403 与 STM32F401 都采用 Cortex-M4，但不能视为寄存器兼容器件。兼容策略必须以 HAL 语义为边界，在 Port 层转换 Instance、状态、标志、IRQ、DMA 请求和触发关系。

## 2. 官方依据

| 资料 | 本地路径 | 本文使用位置 |
|---|---|---|
| GD32F403 User Manual Rev3.1 | `GD32F403RET6/GD32F403_User_Manual_Rev3.1.pdf` | RCU §5、GPIO/AFIO §8、DMA §10、ADC §12、TIMER §16、USART §17、I2C §18、SPI §19、FMC §2 |
| GD32F403xx Datasheet Rev3.1 | `GD32F403RET6/GD32F403xx_Datasheet_Rev3.1.pdf` | 器件概述 §1、型号资源表 §2.1、LQFP64 引脚表 §2.6.4 |
| GD32F403 SPL V3.0.3 | `GD32F403RET6/GD32F403_standard_peripheral/` | 所有目标 API、常量和实现核对 |
| STM32F401 Reference Manual RM0368 | `STM32F401VEH6/rm0368-stm32f401xbc-and-stm32f401xde-advanced-armbased-32bit-mcus-stmicroelectronics.pdf` | RCC、GPIO、EXTI、FLASH、DMA、ADC、TIM、USART、I2C、SPI 的源平台语义 |
| STM32 Cortex-M4 Programming Manual PM0214 | `STM32F401VEH6/pm0214-stm32-cortexm4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf` | NVIC、SysTick、MPU |
| STM32 HAL V1.8.5 | `STM32F401VEH6/HAL库/stm32f4xx-hal-driver/` | Handle、状态机、锁、超时、IRQ、Callback、DMA 绑定 |

## 3. 器件级资源

| 项目 | STM32F401VEH6 | GD32F403RET6 | 迁移影响 |
|---|---|---|---|
| CPU | Cortex-M4F，最高 84 MHz | Cortex-M4F，最高 168 MHz | CMSIS/NVIC/MPU 语义接近；时钟参数必须重算 |
| Flash | 512 KB | 512 KB | 容量相同，擦除组织不同 |
| SRAM | 96 KB | 96 KB | 容量相同，链接脚本仍须使用 GD32 官方内存布局 |
| 封装 | `V` = LQFP100 | `R` = LQFP64 | 不是引脚兼容替换，必须重新做 Pin/PCB 审核 |
| GPIO | LQFP100 可用端口/引脚较多 | 数据手册列出 51 个 GPIO | `GPIOE` 等原工程用法不能默认保留 |
| ADC | ADC1，12 位 | ADC0/1/2，LQFP64 每单元最多 16 外部通道集合 | Instance 与触发映射均需转换 |
| 串口 | USART1/2/6、UART4/5 等按具体型号 | USART0/1/2 + UART3/4 | 编号整体偏移，UART3/4 功能是精简子集 |
| DMA | DMA1/DMA2，各 8 Stream，带 Channel 选择/FIFO | DMA0 7 Channel，DMA1 5 Channel，固定请求 OR 映射 | Handle 不能直接保存 STM32 Stream 指针 |
| TIMER | TIM1、TIM2-5、TIM9-11；TIM2/TIM5 为 32 位 | TIMER0-13 均为 16 位，分高级/通用 L0/L1/L2/基本型 | 32 位周期与级联必须重构 |

GD32 型号资源来自 Datasheet Rev3.1 表 2-2；F403RET6 对应 `RE` 列：512 KB Flash、96 KB SRAM、LQFP64、51 GPIO、3 ADC、3 SPI、2 I2C、3 USART、2 UART。

## 4. 外设差异矩阵

| STM32F401 | GD32F403RET6 | 难度 | 核心差异与策略 |
|---|---|---:|---|
| HAL Core | 兼容层软件实现 | 中 | 保留 Tick、Timeout、Lock、weak callback；不用 SPL 是否提供来决定 |
| GPIO | GPIO + AFIO | 中 | GD32 使用 CTL0/CTL1、OCTL、AFIO remap；不是 MODER/AFRL/AFRH |
| RCC | RCU | 高 | 门控、HSI/HSE、当前/既有时钟切换和查询已转换；PLL 参数仍须原生配置 |
| NVIC/SysTick/MPU | CMSIS Cortex-M4 | 低 | 使用 GD32 CMSIS 的 IRQn_Type 和 Core API |
| UART/USART | USART0-2/UART3-4 | 中/高 | 轮询可组合；IT/DMA 必须重建 Handle 状态机和错误清除顺序 |
| TIM | TIMER | 高 | 编号偏移、计数宽度、ITIx 关系、TRGO 和 DMA 请求映射需查表转换 |
| ADC | ADC | 高 | GD32 有校准/硬件过采样；触发源、序列、EOC/DMA 行为不同 |
| DMA | DMA | 高 | Stream+Channel+FIFO 模型变为固定 Channel；无 1:1 Instance |
| I2C | I2C0/1 | 高 | 需要完整 START/repeated START/ACK/STOP 状态机；无 STM32 I2C3 |
| SPI | SPI0-2 | 中 | 轮询可组合；双工、CRC、DMA、错误标志仍需状态机 |
| EXTI | EXTI + AFIO | 低/中 | GPIO 路由在 AFIO，不是 SYSCFG；pending 清除仍为写 1 清零 |
| FLASH | FMC | 高 | STM32 按 sector 擦除；GD32F403RET6 按 2 KB page 擦除 |

## 5. GPIO

### 5.1 寄存器和配置模型

STM32F401 GPIO 使用独立的 MODER、OTYPER、OSPEEDR、PUPDR 和 AFRL/AFRH。GD32F403 GPIO 使用 CTL0/CTL1 中每引脚 4 位模式字段；输入上下拉通过 OCTL 位选择；复用功能通过 AFIO 默认映射/重映射选择。依据 GD32 User Manual §8 与 `gd32f403_gpio.h`。

因此：

- `GPIO_InitTypeDef.Pin/Mode/Pull/Speed` 可以语义转换。
- `GPIO_InitTypeDef.Alternate` 的 STM32 AF 数值不能机械映射到 GD32。
- 默认引脚功能可用 `GPIO_MODE_AF_PP/OD` 配置电气模式；非默认 remap 必须由 GD32 原生初始化显式调用 `gpio_pin_remap_config()`。
- GD32 输出/AF 模式没有 STM32F4 式独立 PUPDR；输出模式请求 Pull-up/Pull-down 时兼容层会调用错误钩子并拒绝本次配置。

### 5.2 LQFP64 引脚边界

当前头文件只暴露 `GPIOA`、`GPIOB`、`GPIOC`、`GPIOD`。A/B/C 允许 0..15；D 的封装范围只有 PD0/PD1/PD2。PD0/PD1 与 OSCIN/OSCOUT 复用，兼容层默认只允许 PD2；只有停用 HXTAL、完成重映射并将 `GD32_HAL_ALLOW_PD01_GPIO` 设为 `1U` 后才允许 PD0/PD1。`GPIOE/F/G` 不对 GD32F403RET6 暴露，避免把其他封装资源误当成当前型号资源。

## 6. Clock / RCC / RCU

- STM32F401 最高 84 MHz；GD32F403 最高 168 MHz。
- 当前 GD32 `system_gd32f403.c` 选择 `__SYSTEM_CLOCK_168M_PLL_HXTAL`，并把 AHB 配为 168 MHz、APB2 配为 168 MHz、APB1 配为 84 MHz；外部晶振宏 `HXTAL_VALUE` 为 25 MHz。
- GD32 还具有 IRC8M 和 IRC48M；不能复制 STM32 HSI16/PLL 参数。
- TIMER 输入时钟仍可能受 APB 分频倍频规则影响；每个 TIMER 初始化必须从 `rcu_clock_freq_get()` 和当前分频计算，禁止写死 84/168 MHz。
- 常用 `__HAL_RCC_xxx_CLK_ENABLE/DISABLE()`、频率/配置查询、CSS、HSI/HSE 控制和当前/既有时钟切换已实现。
- STM32 的 PLLM/N/P/Q 不能等价转换为 GD32 PREDV/整数倍频；`HAL_RCC_OscConfig(RCC_PLL_ON)` 明确失败，产品仍用 GD32 原生 SystemClock 配置 PLL。
- `HAL_RCC_ClockConfig()` 校验 SYSCLK/AHB/APB2 168 MHz、APB1 84 MHz 上限，并按频率变化顺序调整 Flash wait state、更新 `SystemCoreClock` 和 Tick。

## 7. UART / USART

建议的 Instance 编号映射：

| STM32 名称 | GD32 名称 | 备注 |
|---|---|---|
| USART1 | USART0 | APB2 |
| USART2 | USART1 | APB1 |
| USART3 | USART2 | APB1 |
| UART4 | UART3 | APB1，精简功能 |
| UART5 | UART4 | APB1，精简功能 |

GD32 User Manual §17 明确 USART0/1/2 为完整实现；UART3/4 不支持同步、Smartcard、CTS/RTS 等若干功能。轮询收发可由 `usart_flag_get()`、`usart_data_transmit()`、`usart_data_receive()` 和 `HAL_GetTick()` 组合。IT/DMA 必须维护 `pTxBuffPtr/TxXferCount/pRxBuffPtr/RxXferCount/gState/RxState/ErrorCode`，并严格处理 PERR/FERR/NERR/ORERR/RBNE/TBE/TC。

第二阶段 UART 设计还采用以下硬件约束：

- GD32F403 USART 异步接收固定为 16 倍过采样；STM32 的 `UART_OVERSAMPLING_8` 无等价配置，初始化时必须返回 `HAL_ERROR`。
- GD32 UART3/4 不支持 CTS/RTS；对应 STM32 UART4/5 Handle 请求硬件流控时必须拒绝。
- GD32 User Manual 指出 UART4 不支持 DMA；对应 STM32 `UART5` 的 DMA API 已实现为明确失败。
- PERR、FERR、NERR、ORERR 和 IDLEF 不能用普通写零函数随意清除，必须先读 `USART_STAT0`，再读 `USART_DATA`。
- USART0 使用 APB2，USART1/2、UART3/4 使用 APB1；SPL 波特率发生器固定按 16 倍过采样计算，Port 层必须检查分频值是否落入 BAUD 寄存器范围。

## 8. DMA

STM32F401 DMA 是 Stream + Channel 选择模型，并支持 FIFO、burst、双缓冲等扩展。GD32F403 DMA 是固定请求映射的 Channel 模型：

- DMA0：Channel0..6。
- DMA1：Channel0..4。
- 每 Channel 有传输完成、半完成和错误中断。
- 支持 normal、circular、memory-to-memory、宽度、增量和优先级。
- 同一 GD32 Channel 上的多个外设请求在硬件上 OR；软件必须保证同一时间只使能一个请求。

请求映射依据 GD32 User Manual 图 10-4/10-5、表 10-3/10-4。兼容层需要把 STM32 `DMA_Stream_TypeDef *Instance + Init.Channel` 转换为 `{controller, channel}`，不能把 Stream 基地址直接当作 GD32 Channel。

Phase 3 使用不透明的 `GD32_DMA0_CHANNEL0..6`、`GD32_DMA1_CHANNEL0..4` 作为 Handle Instance，并用请求令牌校验固定映射：

| 物理 Channel | 支持的 `GD32_DMA_REQUEST_*` |
|---|---|
| DMA0 CH0 | ADC0、TIMER1_CH2、TIMER3_CH0 |
| DMA0 CH1 | SPI0_RX、USART2_TX、TIMER0_CH0、TIMER1_UP、TIMER2_CH2 |
| DMA0 CH2 | SPI0_TX、USART2_RX、TIMER0_CH1、TIMER2_CH3、TIMER2_UP |
| DMA0 CH3 | SPI1/I2S1_RX、USART0_TX、I2C1_TX、TIMER0_CH3/TRG/COM、TIMER3_CH1 |
| DMA0 CH4 | SPI1/I2S1_TX、USART0_RX、I2C1_RX、TIMER0_UP、TIMER1_CH0、TIMER3_CH2 |
| DMA0 CH5 | USART1_RX、I2C0_TX、TIMER0_CH2、TIMER2_CH0/TRG |
| DMA0 CH6 | USART1_TX、I2C0_RX、TIMER1_CH1/CH3、TIMER3_UP |
| DMA1 CH0 | SPI2/I2S2_RX、TIMER4_CH3/TRG、TIMER7_CH2/UP |
| DMA1 CH1 | SPI2/I2S2_TX、TIMER4_CH2/UP、TIMER7_CH3/TRG/COM |
| DMA1 CH2 | UART3_RX、TIMER5_UP、DAC_CH0、TIMER7_CH0 |
| DMA1 CH3 | SDIO、TIMER4_CH1、TIMER6_UP、DAC_CH1 |
| DMA1 CH4 | ADC2、UART3_TX、TIMER4_CH0、TIMER7_CH1 |

同一 Channel 的多个请求在硬件中先 OR 再进入 DMA。兼容层的 Handle 独占可阻止两个 DMA Handle 同时启动；UART Phase 4 与 TIMER Phase 5 均增加 Parent、方向、位宽、固定 Channel/请求令牌校验，并在错误和 Stop 路径配对关闭父外设 DMA 请求。TIMER normal 完成与 STM HAL 一致，仅恢复状态并回调，继续运行最后一个 ARR/CCR，应用随后调用 `HAL_TIM_*_Stop_DMA()` 停止请求/通道。ADC 仍须在对应阶段实现相同协同。UART4 没有 DMA 请求。

## 9. TIMER 和硬件级联

### 9.1 Instance

基础编号转换为 `TIMn -> TIMER(n-1)`，例如 TIM1 -> TIMER0、TIM5 -> TIMER4、TIM11 -> TIMER10。该规则只表示实例名，不保证功能相等。

关键限制：GD32F403 Datasheet 将这些 TIMER 均列为 16 位；STM32F401 的 TIM2/TIM5 为 32 位。任何 ARR/CNT/CCR 超过 0xFFFF 的原代码必须改为软件扩展、DMA 分段或经验证的硬件级联。

### 9.2 GD32 ITI 连接矩阵

以下矩阵来自 GD32 User Manual Rev3.1 §16 首页脚注，不能使用 STM32 ITR 表替代：

| Slave TIMER | ITI0 | ITI1 | ITI2 | ITI3 |
|---|---|---|---|---|
| TIMER0 | TIMER4_TRGO | TIMER1_TRGO | TIMER2_TRGO | TIMER3_TRGO |
| TIMER7 | TIMER0_TRGO | TIMER1_TRGO | TIMER3_TRGO | TIMER4_TRGO |
| TIMER1 | TIMER0_TRGO | TIMER7_TRGO | TIMER2_TRGO | TIMER3_TRGO |
| TIMER2 | TIMER0_TRGO | TIMER1_TRGO | TIMER4_TRGO | TIMER3_TRGO |
| TIMER3 | TIMER0_TRGO | TIMER1_TRGO | TIMER2_TRGO | TIMER7_TRGO |
| TIMER4 | TIMER1_TRGO | TIMER2_TRGO | TIMER3_TRGO | TIMER7_TRGO |
| TIMER8 | TIMER1_TRGO | TIMER2_TRGO | TIMER9_TRGO | TIMER10_TRGO |
| TIMER11 | TIMER3_TRGO | TIMER4_TRGO | TIMER12_TRGO | TIMER13_TRGO |

`HAL_TIMEx_MasterConfigSynchronization()` 可映射到 `timer_master_output_trigger_source_select()` 和 `timer_master_slave_mode_config()`；`HAL_TIM_SlaveConfigSynchro()` 可映射到 `timer_input_trigger_source_select()` 与 `timer_slave_mode_select()`。Port 层必须按目标 TIMER 查表转换，而不是只转换 `TIM_TS_ITR0..3` 的数字。

Phase 5 的实际转换先查 RM0368 中当前 F401 Slave TIM 的 ITR 源，再用上表查目标 GD TIMER 的 ITI 下标。可保持语义的连接是：TIM1 ITR0..3、TIM2 ITR0/2/3、TIM3 ITR0..3、TIM4 ITR0/1/2、TIM5 ITR0/1/2、TIM9 ITR0/1。RM0368 标记 Reserved 的连接直接失败；TIM9 ITR2/3 在 STM32 上连接 `TIM10_OC/TIM11_OC`，而 GD 表给出 `TIMER9_TRGO/TIMER10_TRGO`，也必须失败，不能把 OC 与 TRGO 当成同一信号。

所有 GD32F403 TIMER 的 PSC/CNT/CAR/CHxCV 均按 16 位边界处理。Base/PWM/OC/IC/OnePulse、Update/CC/Trigger IRQ、TRGO/Slave 和 update/CC DMA 已实现；Encoder、Hall、互补输出、死区/刹车、CMT/BRK Callback 和 DMA Burst 仍未实现。TIMER DMA 为保持 STM HAL `uint32_t *` buffer 步进，要求 DMA WORD/WORD，同时在输出启动前逐项检查数据不超过 `0xFFFF`。

## 10. ADC

| 维度 | STM32F401 | GD32F403RET6 | 策略 |
|---|---|---|---|
| Instance | F401 只有 ADC1 | ADC0/1/2 | 公共 `ADC1` 只映射 GD `ADC0`；不泄漏 ADC0/1/2 原生宏 |
| 分辨率/对齐 | 12/10/8/6 位，right/left | 同样支持四档分辨率和两种对齐 | 显式枚举转换 |
| ADC 时钟 | 最大 36 MHz | VDDA/VREFP ≥ 2.6 V 时 0.1..40 MHz | PCLK2 /2/4/6/8 转换后运行期检查；低 VREFP 用配置宏收紧到 30 MHz |
| 校准 | F401 HAL 无前台校准流程 | 掉电会丢失校准因子 | `HAL_ADC_Init()` 只配置；每次从 disable 启动时等待至少 14 CK_ADC 并自动校准 |
| regular 序列 | 可选择 rank EOC 或 sequence EOC | routine scan 只在序列结束置 EOC | 多 rank 只接受 `ADC_EOC_SEQ_CONV`；单 rank 时两种 EOC 语义等价 |
| 触发边沿 | rising/falling/both | ADC 本体没有独立极性字段 | 软件触发只接受 NONE；外部触发只接受 RISING；falling/both 明确失败 |
| DMA | STM32 DMA2 Stream/Channel | ADC0 固定 DMA0 CH0 | 必须 `GD32_DMA_REQUEST_ADC0`、P2M、word/word、MINC；normal/circular 与 DDS 语义绑定 |
| Overrun | ADC_SR.OVR、OVR IRQ | ADC_STAT 无 OVR 位/IRQ | `HAL_ADC_ERROR_OVR` 无法硬件观测；不伪造事件，DMA error 仍可观测 |
| 过采样 | F401 无硬件过采样 | GD32 支持硬件 oversampling | 作为 GD32 扩展，不伪装成基础 HAL 字段 |

保持语义的常规触发转换如下；其余 STM32 枚举返回 `HAL_ERROR`：

| STM32 HAL 触发 | GD32F403 ADC0 触发 |
|---|---|
| TIM1_CH1/CH2/CH3 | TIMER0_CH0/CH1/CH2 |
| TIM2_CH2 | TIMER1_CH1 |
| TIM3_TRGO | TIMER2_TRGO |
| TIM4_CH4 | TIMER3_CH3 |
| TIM8_TRGO | TIMER7_TRGO，并置 ADC0 routine remap |
| EXTI11 | EXTI11，并清 ADC0 routine remap |

采样周期不能直接复制。兼容层采用不短于 STM32 请求的保守档位：3→7.5、15/28→28.5、56→71.5、84/112/144→239.5 cycles；GD 最大 239.5 cycles 小于 STM32 480 cycles，因此 480 明确失败。内部温度/Vref 通道还要按目标 ADC 时钟核对数据手册建议的绝对采样时间。ADC Timer Trigger 必须作为 TIMER + ADC + DMA 的联合板测项。

## 11. I2C

GD32F403RET6 只有 I2C0/I2C1，对应 STM32 命名的 I2C1/I2C2；STM32 I2C3 没有目标实例。GD32 SPL 提供底层 START、STOP、ACK、addressing、data 和 flag API，没有等价的 HAL 事务状态机。兼容层已独立实现：

- BUSY 超时与总线恢复。
- 7/10 位地址处理。
- repeated START。
- 1/2/N 字节接收时 ACK/NACK 时序。
- event/error IRQ 分离。
- polling/IT 的 master 事务与 weak callback。

当前限制：Mem/IsDeviceReady 只支持 7 位；I2C DMA、Slave/Listen/Sequential 尚未实现。软件恢复只复位并重配 I2C 外设，不能代替 GPIO SCL 脉冲释放外部拉低的总线。

## 12. SPI

SPI1/2/3 分别映射到 SPI0/1/2。兼容层已实现 8/16 位 polling/IT/DMA TX/RX/TxRx；主机 full-duplex RX 发送 dummy frame，TX-only 与 DMA normal 收尾等待 TBE/TRANS 并清 OVR，全双工 DMA 由 RX 完成统一关闭两侧请求。DMA 固定校验如下：SPI0 RX/TX 为 DMA0 CH1/CH2，SPI1 RX/TX 为 DMA0 CH3/CH4，SPI2 RX/TX 为 DMA1 CH0/CH1。

目标 master receive-only 模式启用后会连续产生 SCK，末两帧禁用时序与普通 full-duplex 不等价，因此当前拒绝 master `SPI_DIRECTION_2LINES_RXONLY`。CRC next-frame/长度/收尾语义尚未闭合，`CRCCalculation=ENABLE` 也明确失败。

## 13. EXTI

两者均有 GPIO EXTI0..15，但路由单元不同：STM32F401 使用 SYSCFG_EXTICR，GD32F403 使用 AFIO_EXTISS。GPIO HAL 和独立 `HAL_EXTI_*` Handle API 均已通过 `gpio_exti_source_select()`、`exti_init()` 和 pending 写 1 清除实现中断/事件、上升/下降/双边沿与 Callback。目标只开放 EXTI0..18；GPIO 封装/晶振复用限制继续执行，EXTI19..22 不定义。

## 14. FLASH / FMC

STM32F401 按 sector 擦除；GD32F403RET6（512 KB）按 2 KB page 擦除。兼容层把 STM32 sector 0..7 的地址范围展开为 GD32 page，并另提供显式页擦除 API；只接受 half-word/word programming。默认没有可写范围，产品必须用 2 KB 对齐的链接分区和 `GD32_HAL_FLASH_SetWritableRegion()` 显式授权。向量表首个 page、当前执行页、越界和授权区外操作均被拒绝；mass erase、byte/double-word 和 option bytes 不支持。

## 15. NVIC / SysTick / MPU

二者均为 Cortex-M4、4 个 NVIC 优先级位并具有 MPU。首批实现直接使用 GD32 CMSIS：`NVIC_SetPriorityGrouping`、`NVIC_EncodePriority`、`NVIC_EnableIRQ`、`SysTick_Config` 和 MPU 寄存器。IRQ 枚举名称仍是 GD32 名称，例如 USART0_IRQn；旧工程的 STM32 IRQn 名称需在后续 IRQ alias 层中逐项映射。

GD32 默认 168 MHz 时，10 Hz Tick 的 HCLK 重装值为 16,800,000，超过 SysTick 24 位上限 16,777,216。`HAL_InitTick()` 在 HCLK 重装值超限时自动改用 HCLK/8；100 Hz 和 1 kHz 仍优先使用 HCLK。

## 16. 架构决策

1. HAL 头文件保持 STM32 常用名字，Port 层持有全部 GD32 SPL 依赖。
2. GPIO Instance 使用不透明指针，禁止应用直接访问不存在的 STM32 寄存器字段。
3. 初始化代码允许按 GD32 原生方式重写；尤其是 AFIO remap、PLL/Clock、DMA 请求和 TIMER/ADC 触发。RCC 兼容 API 只管理安全可转换的子集。
4. 无法安全等价的功能不返回假成功；有返回值的 API 返回 HAL_ERROR，void API 使用 `GD32_HAL_ErrorHook()` 并在未支持文档中登记。
5. FLASH 采用默认拒绝的显式写区模型；链接脚本分区是安全边界的一部分，HAL 地址检查不能替代产品镜像布局审计。
6. 每个模块完成代码质量门后仍保留板级质量门；Host Mock 和目标链接不能替代真实时钟、IRQ、总线波形或掉电验证。
