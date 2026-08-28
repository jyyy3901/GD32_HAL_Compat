# GD32 HAL Compatibility Layer 0.9.0

面向 `STM32F401VEH6 HAL 应用 -> GD32F403RET6 SPL` 的 API 兼容层。0.9.0 在 0.8.0 状态机基础上完成架构分层，不复制 STM32 HAL，不假设两颗 MCU 寄存器一致，也不为不存在的 GD32 能力返回假成功。

## 架构

```text
Application
  -> Include/  STM32 HAL Compatible API
  -> Source/   HAL state/IRQ/callback semantics
  -> Port/     GD32F403 instance/resource translation
  -> official GD32 SPL/CMSIS
  -> GD32F403RET6
```

公共 `USART1/TIM2/ADC1` 等 Instance 是不可解引用的 STM32 语义令牌。Init 时由 `Port/gd32_instance_map.c` 解析成目标地址、clock、IRQ 和 capability，并缓存到 Handle 的 `GD32_*` 字段。

## 0.9.0 范围

- Phase 1：HAL Core、GPIO、Tick、NVIC/MPU 子集；
- Phase 2：UART polling/IT/DMA/Abort/IRQ/weak Callback；
- Phase 3：GD32 controller/channel 模型的 DMA 基础、IT、Poll、Abort；
- Phase 4：TIMER Base/PWM/OC/IC/OnePulse、DMA、TRGO、Master/Slave 和 ITR->ITI；
- Phase 5：ADC regular polling/IT/DMA、序列、触发、采样时间和校准；
- I2C、SPI、EXTI、RCC、FLASH 保留 0.8.0 已实现安全子集并做回归保护，不在本次继续扩张。

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

0.9.0 核心源文件：

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
- DMA 不接受 `DMA1_Streamx/DMA2_Streamx` 地址；选择 `GD32_DMAx_CHANNELy` 和对应 `GD32_DMA_REQUEST_*`。
- FIFO、Burst、PFCTRL、double buffer 无目标等价，明确失败。
- GD32F403 TIMER 全部 16 位；TIM2/TIM5 的 32 位范围必须重构。
- ITR 不是按数字直译；`stm32_timer_trigger_map.c` 按 slave Timer 查源，再查 GD ITI 矩阵。
- ADC 外部触发只接受目标可保持语义的源和 rising edge；采样时间采用不短于请求值的保守映射，480 cycles 明确失败。
- UART 只支持 16 倍过采样。

## 自检

```powershell
pwsh.exe -File .\Tests\run_checks.ps1
```

检查包含 ARM Cortex-M4 `-Wall -Wextra -Werror` 编译、与官方 GD32 SPL 的目标 ELF 链接、unsupported compile guards、Clang Analyzer、Instance/RCC 映射及各模块 Host 状态机测试。

## 文档

- [0.8.0 审查](Docs/V0.8.0_REVIEW.md)
- [0.9.0 架构](Docs/ARCHITECTURE.md)
- [STM32/GD32 映射](Docs/STM32_GD32_MAPPING.md)
- [限制](Docs/LIMITATION.md)
- [TIMER trigger 映射](Docs/TIMER_TRIGGER_MAPPING.md)
- [DMA 差异](Docs/DMA_DIFFERENCE.md)
- [移植指南](Docs/PORTING_GUIDE.md)

所有自动检查仍属于代码级证据。真实 GD32F403RET6 板上的引脚、时钟、IRQ、DMA、TIMER 波形和 ADC 性能尚需产品工程验证，不能据此宣称量产签核。
