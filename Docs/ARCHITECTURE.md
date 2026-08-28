# 0.9.0 架构

## 目标

0.9.0 是面向“STM32F401VEH6 HAL 应用语义 -> GD32F403RET6 SPL”的 API 兼容层，不是 STM32 HAL 的复制品，也不复用 STM32 寄存器布局。

```text
Application
    |
STM32 HAL Compatible API       Include/stm32f4xx_hal_*.h
    |
Compatibility State Machines  Source/stm32f4xx_hal_*.c
    |
GD32F403 Port Contract         Include/stm32_hal_port_api.h
    |
GD32F403 Port                  Port/gd32_*_port.c
    |
Official GD32 SPL / CMSIS
    |
GD32F403RET6
```

## 目录职责

| 目录 | 职责 | 禁止内容 |
|---|---|---|
| `Include/` | STM32 HAL 风格类型、宏、API；不透明 Instance；Port contract | GD32 寄存器地址、直接 SPL 调用、STM32 寄存器结构 |
| `Source/` | HAL 状态机、超时、锁、IRQ 处理、Callback 调度 | 直接读写 GD32/STM32 外设寄存器、直接调用 SPL |
| `Port/` | Instance/clock/IRQ/capability 解析与 GD32 SPL 转换 | 伪造目标不存在的 FIFO、Burst、32 位 TIMER 等能力 |
| `CMSIS/` | 官方 CMSIS 的外部集成说明 | 厂商源码副本或修改版 |
| `GD32_SPL/` | 官方 SPL 的外部集成说明 | 厂商源码副本或修改版 |
| `Tests/` | ARM 严格编译、官方 SPL 链接、Analyzer、Host 状态机测试 | 把 Mock 结果当作板级签核 |

## Instance 与资源

`USART1`、`TIM2`、`ADC1` 等公共 Instance 是 `0xF401xxxx` 形式的不可解引用语义令牌。`Port/gd32_instance_map.c` 在 Init 时把令牌解析成 `GD32_HAL_Resource`：

- `gd32_instance`：目标外设地址；
- `gd32_clock`：目标 RCU 资源；
- `gd32_irq`：目标向量号；
- `capabilities`：DMA、主从、TRGO、通道数、流控等能力；
- DMA controller/channel 等目标索引。

Handle 保留 STM32 常用字段，并缓存 `GD32_INSTANCE`、`GD32_IRQ_NUMBER`、`GD32_RESOURCE`。DMA 另外保存 `gd32_dma_periph` 和 `gd32_dma_channel`。应用不得解引用 `Instance` 或把它与 GD32 地址比较。

## 调用与错误路径

典型 UART 路径：

```text
HAL_UART_Transmit()
  -> UART Handle/state/timeout validation
  -> GD32_HAL_UART_WriteData()
  -> gd32_uart_port.c
  -> usart_data_transmit()
```

Port 操作失败时，返回值 API 必须返回 `HAL_ERROR/HAL_TIMEOUT/HAL_BUSY`；void 宏或 void API 通过 `GD32_HAL_ErrorHook()` 报告。禁止“未执行操作但返回 `HAL_OK`”。默认错误钩子记录最近错误，产品可用强定义覆盖。

## IRQ 与 Callback

产品中断文件使用 GD32 startup 的原生入口，并调用对应 HAL handler：

```c
void USART0_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}
```

状态和标志只由 `HAL_*_IRQHandler()` 收敛，随后调用 weak Callback。Port 不得越过 HAL handler 直接调用业务 Callback。GD32 共享 TIMER IRQ 需要产品工程根据已绑定的 Handle 逐个分发。

## GD32 原生能力

GD32F403 多出的 UART、TIMER、ADC 等能力不伪装成 STM32F401 名称。产品可以在 MSP 或独立原生模块中直接使用官方 SPL；兼容层 `HAL_DeInit()` 只复位兼容资源，避免无故破坏这些原生外设。

## 验证边界

`Tests/run_checks.ps1` 验证代码级 ABI、状态机和官方 SPL 链接。真实波形、引脚复用、时钟树、IRQ 延迟、DMA 总线竞争、ADC 精度/采样时间和长期稳定性仍必须在 GD32F403RET6 板上验证。
