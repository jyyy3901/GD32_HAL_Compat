# STM32F401VEH6 与 GD32F403RET6 映射

## 兼容等级

| 等级 | 含义 |
|---|---|
| 完全兼容 | 在已声明范围内，调用、状态与可观察行为一致 |
| 接口兼容 | API/Handle 习惯保留，但硬件参数、引脚、时序或能力需重新验证 |
| 不兼容 | 没有安全等价；编译失败或运行时明确返回错误 |
| GD32 原生扩展 | 只通过 GD32 SPL 使用，不借用 STM32F401 名称 |

## Instance

| STM32F401 语义 | GD32F403 目标 | 备注 |
|---|---|---|
| GPIOA/B/C/D | GPIOA/B/C/D | 目标 LQFP64 仅开放经封装表确认的引脚；PD0/PD1 默认保留给 HXTAL |
| USART1 | USART0 | APB2，目标 IRQ `USART0_IRQn` |
| USART2 | USART1 | APB1，目标 IRQ `USART1_IRQn` |
| USART6 | USART2 | APB1，目标 IRQ `USART2_IRQn` |
| TIM1 | TIMER0 | 高级 TIMER，16 位 |
| TIM2/3/4/5 | TIMER1/2/3/4 | 目标全部 16 位；TIM2/TIM5 的 32 位范围不可保留 |
| TIM9/10/11 | TIMER8/9/10 | 分别为 2/1/1 通道 |
| ADC1 | ADC0 | regular ADC，固定触发/序列/DMA 差异 |
| I2C1/2 | I2C0/1 | 既有模块，0.10.0 只回归不扩张 |
| SPI1/2/3 | SPI0/1/2 | 既有 HAL 模块；0.10.0 仅增加已确认 register view |

STM32F401 不存在的 `USART3/UART4/UART5`、`TIM6/7/8/12/13/14`、`ADC2/3` 不作为兼容 Instance 暴露。GD32 的 UART3/4、TIMER5/6/7/11/12/13、ADC1/2 等属于 GD32 原生扩展。

0.10.0 中上述 GPIO/UART/TIM/ADC/SPI Instance 使用目标真实地址；DMA Stream 保持 opaque token。逐寄存器与 bit 等级见 `REGISTER_COMPATIBILITY_MATRIX.md`。

## Handle

| Handle | 保留的 STM32 常用字段 | GD32 映射字段 |
|---|---|---|
| `UART_HandleTypeDef` | Instance、Init、TX/RX 指针与计数、DMA、gState/RxState/ErrorCode | GD32_INSTANCE、GD32_IRQ_NUMBER、GD32_RESOURCE |
| `DMA_HandleTypeDef` | Instance、Init、Parent、Callback、State/ErrorCode、Stream 兼容字段 | GD32_INSTANCE、GD32_REQUEST、GD32_RESOLVED_FROM、controller/channel、IRQ/resource |
| `TIM_HandleTypeDef` | Instance、Init、hdma[]、State、Channel | ErrorCode、GD32 instance/IRQ/resource |
| `ADC_HandleTypeDef` | Instance、Init、DMA、State/ErrorCode | GD32 instance/IRQ/resource |

## GPIO

`GPIO_MODE_INPUT/OUTPUT_PP/OUTPUT_OD/AF_PP/AF_OD/ANALOG`、输入 pull、Read/Write/Toggle 可用。GD32F403 SPL V3.0.3 实际提供 F1 风格 `gpio_init()`，并没有 `gpio_mode_set()`/`gpio_output_options_set()`；Port 使用官方真实 API，不能发明不存在的函数。

STM32 `Alternate` 数值仅为源码接口兼容。GD32 的缺省/重映射选择必须在 MSP 中用 AFIO SPL 配置；兼容层不会把 STM32 AF 编号误当 GD32 remap。输出/AF 模式没有 STM32F4 独立 PUPDR，非 `GPIO_NOPULL` 请求明确报错。

## RCC

常用 `__HAL_RCC_*_CLK_ENABLE/DISABLE()` 只把 STM32 语义资源交给 Port；`RCU_*` 与 `rcu_periph_clock_enable/disable()` 不再暴露在公共 HAL 宏中。系统时钟仍按 GD32 时钟模型配置，不复现 STM32 PLL 寄存器参数。

STM32 `RCC->AHB1ENR` 不能安全 overlay：同一 STM32 寄存器内的 GPIO 与 DMA clock bit 在 GD32 分属 APB2EN 与 AHBEN，因此 strict mode 不提供 RCC register view。
