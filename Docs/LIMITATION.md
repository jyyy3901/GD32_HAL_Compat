# 限制与兼容等级

## API 状态

| 模块 | 当前范围 | 等级 |
|---|---|---|
| HAL Core/Tick | Init/DeInit、Tick、Delay、UID、错误钩子 | 接口兼容 |
| Cortex/NVIC/MPU | 常用 NVIC、SysTick、MPU 子集 | 接口兼容 |
| GPIO | Init/DeInit、Read/Write/Toggle、Lock、EXTI handler | 常用数字 I/O 完全兼容；AF/封装为接口兼容 |
| UART | Init/DeInit、polling、IT、DMA、Abort、IRQ、weak Callback | 接口兼容 |
| DMA | Init/DeInit、Start/Start_IT、Poll、Abort、IRQ、Callback | 接口兼容，内部完全采用 GD32 Channel 模型 |
| TIMER | Base、PWM、OC、IC、OnePulse、IRQ、DMA、Master/Slave、TRGO | 接口兼容 |
| ADC | Init、Channel、Start/Stop、Poll/IT/DMA、IRQ、校准 | 接口兼容 |
| I2C/SPI/EXTI/RCC/FLASH | 已实现的安全子集 | 接口兼容，未实现项明确失败 |

## 明确不兼容

- 只允许解引用 `REGISTER_COMPATIBILITY_MATRIX.md` 明确列出的 A/B 寄存器成员；Class C 成员不提供或不保证。
- STM32 DMA Stream 可作为 `HAL_DMA_Init()` 的 opaque token；任何 Stream 寄存器解引用仍在编译期失败。
- DMA FIFO、Burst、PFCTRL、double buffer 没有等价实现，非默认配置返回 `HAL_ERROR` 或编译失败。
- GD32F403 TIMER 全部为 16 位；TIM2/TIM5 超出 `0xFFFF` 的 Period/Counter/CCR 不截断，明确失败。
- TIM2/TIM5 直接 CNT/ARR/CCR 写无法被 HAL 拦截，属于 conditional/unsafe register compatibility。
- ADC 在 HAL 进入时检测到 `ADON=0` 会使 calibration-valid 失效并重新校准；direct `ADON+SWSTART` 在 strict mode 不支持。
- 应用若在两次 compatibility-layer 调用之间直接清除又重新置位 `ADON`，最终寄存器状态与“从未掉电”相同，纯 C 寄存器 overlay 无法观测这段历史；该序列不能透明保证重新校准，必须改走 `HAL_ADC_Stop()/HAL_ADC_Start()` 或在掉电期间调用兼容层。此限制不做假兼容声明。
- GPIO MODER/OTYPER/OSPEEDR/PUPDR/AFR、RCC、I2C、DMA Stream、FLASH/FMC 的整块 STM32 register overlay 不提供。
- TIMER Encoder、Hall、dead-time/break、complementary output、DMA burst 不在当前兼容范围。
- ADC falling/both external trigger edge、480-cycle sampling、不可映射 trigger、per-conversion EOC 序列语义不伪造。
- `USE_HAL_*_REGISTER_CALLBACKS` 运行时回调注册未实现；使用 weak Callback。
- GPIO STM32 AF 编号不自动转换成 GD32 AFIO remap；引脚与 remap 必须按目标板重配。
- UART 只支持 GD32 硬件的 16 倍过采样；`UART_OVERSAMPLING_8` 返回 `HAL_ERROR`。
- RCC 只提供常用门控与有限时钟查询/配置，不复制 STM32 PLL/backup-domain 模型。

## 器件与封装

STM32F401VEH6 与 GD32F403RET6 不是引脚兼容替换。源器件存在而目标封装或目标芯片不可用的 GPIOE/H、I2C3、SPI4 等资源需要业务工程重新分配。兼容层不会为这些资源返回成功。

`GD32F403_Demo_Suites_V3.0.3` 中的配套板是 GD32403Z-EVAL（GD32F403ZET6/LQFP144），不是 GD32F403RET6/LQFP64。其 CMSIS/SPL 可作为同系列软件依赖，但示例的引脚、端口、时钟、外设连线和板级初始化不属于 RET6 透明兼容范围。

## 板级待验证

必须验证时钟、NVIC 优先级、共享 IRQ、UART 波特率/错误注入、DMA 请求冲突与 normal/circular 状态、TIMER PWM/捕获/级联/TRGO、ADC 输入范围/采样时间/触发/DMA/校准，以及所有目标引脚和 AFIO remap。代码检查结果不等于硬件或量产签核。
