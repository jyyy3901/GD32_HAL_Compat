# 0.9.0 限制与兼容等级

## API 状态

| 模块 | 0.9.0 范围 | 等级 |
|---|---|---|
| HAL Core/Tick | Init/DeInit、Tick、Delay、UID、错误钩子 | 接口兼容 |
| Cortex/NVIC/MPU | 常用 NVIC、SysTick、MPU 子集 | 接口兼容 |
| GPIO | Init/DeInit、Read/Write/Toggle、Lock、EXTI handler | 常用数字 I/O 完全兼容；AF/封装为接口兼容 |
| UART | Init/DeInit、polling、IT、DMA、Abort、IRQ、weak Callback | 接口兼容 |
| DMA | Init/DeInit、Start/Start_IT、Poll、Abort、IRQ、Callback | 接口兼容，内部完全采用 GD32 Channel 模型 |
| TIMER | Base、PWM、OC、IC、OnePulse、IRQ、DMA、Master/Slave、TRGO | 接口兼容 |
| ADC | Init、Channel、Start/Stop、Poll/IT/DMA、IRQ、校准 | 接口兼容 |
| I2C/SPI/EXTI/RCC/FLASH | 保留 0.8.0 已实现安全子集 | 回归保护，不是本次扩展目标 |

## 明确不兼容

- 不允许应用解引用 `Instance`，也不支持 STM32 寄存器字段访问。
- 不接受 STM32 DMA Stream 地址；必须选择 GD32 物理 Channel 和固定请求令牌。
- DMA FIFO、Burst、PFCTRL、double buffer 没有等价实现，非默认配置返回 `HAL_ERROR` 或编译失败。
- GD32F403 TIMER 全部为 16 位；TIM2/TIM5 超出 `0xFFFF` 的 Period/Counter/CCR 不截断，明确失败。
- TIMER Encoder、Hall、dead-time/break、complementary output、DMA burst 不在 0.9.0 范围。
- ADC falling/both external trigger edge、480-cycle sampling、不可映射 trigger、per-conversion EOC 序列语义不伪造。
- `USE_HAL_*_REGISTER_CALLBACKS` 运行时回调注册未实现；使用 weak Callback。
- GPIO STM32 AF 编号不自动转换成 GD32 AFIO remap；引脚与 remap 必须按目标板重配。
- UART 只支持 GD32 硬件的 16 倍过采样；`UART_OVERSAMPLING_8` 返回 `HAL_ERROR`。
- RCC 只提供常用门控与有限时钟查询/配置，不复制 STM32 PLL/backup-domain 模型。

## 器件与封装

STM32F401VEH6 与 GD32F403RET6 不是引脚兼容替换。源器件存在而目标封装或目标芯片不可用的 GPIOE/H、I2C3、SPI4 等资源需要业务工程重新分配。兼容层不会为这些资源返回成功。

## 板级待验证

必须验证时钟、NVIC 优先级、共享 IRQ、UART 波特率/错误注入、DMA 请求冲突与 normal/circular 状态、TIMER PWM/捕获/级联/TRGO、ADC 输入范围/采样时间/触发/DMA/校准，以及所有目标引脚和 AFIO remap。0.9.0 的代码检查结果不等于量产签核。
