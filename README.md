# GD32 HAL Compatibility Layer 0.10.1

面向 `STM32F401VEH6 HAL 应用 -> GD32F403RET6 SPL` 的源码兼容层。0.10.0 在 0.9.0 状态机与 Port 分层上增加受严格矩阵约束的 CMSIS/Register compatibility；不复制 STM32 HAL/CMSIS，不修改官方 GD32 文件，也不为不存在的能力返回假成功。

## 0.10.1 修复范围

- ADC 只在同一连续上电周期复用 calibration-valid；检测到 `ADCON=0` 后先失效状态，再使能、稳定、reset calibration 和 calibration；
- ADC DMA 支持严格匹配的 `HALFWORD/HALFWORD` 与 `WORD/WORD`，校验 buffer 对齐且 `Length` 始终表示 transfer item 数；
- STM32 CubeMX TIM DMA Stream 初始化增加事件感知的延迟解析：`HAL_DMA_Init()` 保留 Stream/Channel/Direction 语义，`HAL_TIM_*_Start_DMA()` 结合 TIM Instance 和 `TIM_DMA_ID_xxx` 后才绑定 GD32 固定 Channel/request；
- TIM DMA 的 HALFWORD buffer 按 `uint16_t` element 解释，WORD buffer 按 `uint32_t` element 做 16 位目标范围检查；normal/circular ownership 与回调状态保持既有语义。

## 架构

```text
Application
  -> Include/  STM32 HAL Compatible API
  -> Source/   HAL state/IRQ/callback semantics
  -> Port/     GD32F403 instance/resource translation
  -> official GD32 SPL/CMSIS
  -> GD32F403RET6
```

公共 `GPIOA/USART1/TIM1/ADC1/SPI1` 等 Instance 现在是其 GD32 映射目标的真实物理地址，并只通过 `stm32f401_register_compat.h` 暴露已确认成员。资源表同时保留 STM32 semantic ID、public address、GD32 address、clock、IRQ 与 capability，既有 `GD32_HAL_ResolveInstance()`/Handle 缓存路径不变。DMA Stream 仍是不可解引用的初始化 token。

## 0.10.0 增量范围

- 保留 0.9.0 HAL Core、GPIO、UART、DMA、TIMER、ADC、I2C、SPI、EXTI、RCC、FLASH 状态机；
- 新增 `stm32f4xx.h`、`stm32f401xe.h`、Register type/bit/common macro 层；
- GPIO 安全子集、USART、TIMER、ADC 配置子集、SPI、EXTI 的直接符号访问；
- 默认 strict mode，危险成员/bit 不暴露；
- ADC calibration 独立于 enable 状态，所有 HAL polling/IT/DMA Start 统一校准；
- 常见 CubeMX `DMAx_Streamy + DMA_CHANNEL_n` 初始化语法映射到 GD32 固定 Channel/request；Stream 寄存器继续编译失败。

兼容层只暴露 STM32F401VE 实际存在且可安全映射的串口 `USART1/2/6`、定时器 `TIM1..5/TIM9..11` 和 `ADC1`。GD32 多出的 UART/TIMER/ADC 能力通过官方 SPL 原生使用，不借用不存在的 STM32F401 名称。

## 目录

```text
GD32_HAL_Compat/
├─ Include/    STM32 HAL compatible headers 与 Port contract
├─ Source/     HAL API、状态机、IRQ、Callback
├─ Port/       GD32 GPIO/UART/DMA/TIMER/ADC/RCC 等实现与资源表
├─ CMSIS/      官方 CMSIS 外部集成说明
├─ GD32_SPL/   官方 SPL 外部集成说明
├─ Docs/       架构、映射、限制、差异和移植文档
└─ Tests/      ARM 编译/链接、Analyzer、Host Mock
```

## 工程集成

Include path：

```text
GD32_HAL_Compat/Include
GD32_HAL_Compat/Port
GD32F403RET6
GD32F403RET6/CMSIS
GD32F403RET6/CMSIS/GD/GD32F403/Include
GD32F403RET6/GD32F403_standard_peripheral/Include
```

0.10.0 新增公共头文件：

```text
Include/stm32f4xx.h
Include/stm32f401xe.h
Include/stm32f401_register_compat.h
Include/stm32f401_register_bits.h
```

核心源文件沿用 0.9.0：

```text
Source/stm32f4xx_hal.c
Source/stm32f4xx_hal_cortex.c
Source/stm32f4xx_hal_gpio.c
Source/stm32f4xx_hal_uart.c
Source/stm32f4xx_hal_uart_callbacks.c
Source/stm32f4xx_hal_dma.c
Source/stm32f4xx_hal_tim.c
Source/stm32f4xx_hal_tim_ex.c
Source/stm32f4xx_hal_tim_callbacks.c
Source/stm32f4xx_hal_adc.c
Source/stm32f4xx_hal_adc_callbacks.c
Port/gd32_core_port.c
Port/gd32_gpio_port.c
Port/gd32_uart_port.c
Port/gd32_dma_port.c
Port/gd32_timer_port.c
Port/stm32_timer_trigger_map.c
Port/gd32_adc_port.c
Port/gd32_instance_map.c
Port/gd32_tick_port.c
Port/gd32_irq_port.c
Port/gd32_rcc_port.c
```

同时加入官方 CMSIS system/startup，以及所用模块的 GD32 SPL 源文件。不要复制或修改官方库到本仓库。

## IRQ 示例

兼容层不抢占产品 startup 的弱中断名。目标中断文件必须使用 GD32 入口并进入 HAL handler：

```c
void SysTick_Handler(void)
{
    GD32_HAL_SysTickIRQHandler();
}

void USART0_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

void DMA0_Channel3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart0_tx);
}
```

Callback 只由 `HAL_*_IRQHandler()`/DMA 收尾路径调用，Port 不绕过 HAL 状态机。

## 关键迁移点

- GPIO AF 电气模式可用，但 STM32 `Alternate` 编号不会自动变成 GD32 AFIO remap；在 MSP 中按目标引脚配置。
- DMA 接受矩阵中可唯一转换的 `DMA1/2_Streamx + DMA_CHANNEL_n + Direction` 初始化语法，也继续接受显式 `GD32_DMAx_CHANNELy + GD32_DMA_REQUEST_*`；任何 Stream 寄存器解引用都失败。
- FIFO、Burst、PFCTRL、double buffer 无目标等价，明确失败。
- GD32F403 TIMER 全部 16 位；TIM2/TIM5 的 32 位范围必须重构。
- ITR 不是按数字直译；`stm32_timer_trigger_map.c` 按 slave Timer 查源，再查 GD ITI 矩阵。
- ADC 外部触发只接受目标可保持语义的源和 rising edge；采样时间采用不短于请求值的保守映射，480 cycles 明确失败。
- UART 只支持 16 倍过采样。
- ADC `ADON` 直接 enable 后再走 HAL Start 仍会校准；严格模式不定义直接 `SWSTART`。
- `TIM2/TIM5` 直接 CNT/ARR/CCR 访问仍只有 16 位，不能绕过 HAL 的范围保护。

## 自检

```powershell
pwsh.exe -File .\Tests\run_checks.ps1
```

检查包含 ARM Cortex-M4 `-Wall -Wextra -Werror` 编译、与官方 GD32 SPL 的目标 ELF 链接、unsupported compile guards、Clang Analyzer、Instance/RCC 映射及各模块 Host 状态机测试。

## 文档

- [0.8.0 审查](Docs/V0.8.0_REVIEW.md)
- [0.10.0 架构](Docs/ARCHITECTURE.md)
- [STM32/GD32 映射](Docs/STM32_GD32_MAPPING.md)
- [限制](Docs/LIMITATION.md)
- [TIMER trigger 映射](Docs/TIMER_TRIGGER_MAPPING.md)
- [DMA 差异](Docs/DMA_DIFFERENCE.md)
- [移植指南](Docs/PORTING_GUIDE.md)
- [Register compatibility matrix](Docs/REGISTER_COMPATIBILITY_MATRIX.md)

所有自动检查仍属于代码级证据。真实 GD32F403RET6 板上的引脚、时钟、IRQ、DMA、TIMER 波形和 ADC 性能尚需产品工程验证，不能据此宣称量产签核。
