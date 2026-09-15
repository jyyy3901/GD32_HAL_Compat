# GD32 HAL Compatibility Layer

当前版本：`0.10.3`。

面向 `STM32F401VEH6 HAL 应用 -> GD32F403RET6 SPL` 的源码兼容层。项目保留 HAL/Port 分层与既有状态机，并提供受严格矩阵约束的 CMSIS/Register compatibility；不复制 STM32 HAL/CMSIS，不修改官方 GD32 文件，也不为不存在的能力返回假成功。

当前兼容基线为 GD32F403 SPL `V3.0.3` 与 STM32F4 HAL reference `V1.8.5`。已用 `GD32F403_Demo_Suites_V3.0.3/GD32F403_Firmware_Library` 中的官方 CMSIS/SPL 重跑目标编译和链接；使用其他厂商库版本时必须重新运行完整检查并核对目标手册。

## 当前能力

- 提供 HAL Core、GPIO、UART、DMA、TIMER、ADC、I2C、SPI、EXTI、RCC、FLASH 的已声明安全子集；
- 提供 `stm32f4xx.h`、`stm32f401xe.h`、register type/bit/common macro compatibility；
- GPIO、USART、TIMER、ADC、SPI、EXTI 只开放矩阵确认安全的寄存器/bit，默认启用 strict mode；
- ADC HAL polling/IT/DMA Start 统一执行目标所需的 enable、稳定等待与 calibration gate，DMA 支持严格配对的 HALFWORD/HALFWORD 和 WORD/WORD；
- 常见 CubeMX `DMAx_Streamy + DMA_CHANNEL_n` 初始化语法可映射到 GD32 固定 Channel/request；TIMER Stream 在每次 Start 时按 Instance/event 重新验证，shared Handle 使用 active event 状态；
- DMA Stream 寄存器、RCC/GPIO 等不等价 register overlay、32 位 TIM2/TIM5 范围及其他无安全等价功能明确失败。

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
<GD32F403_VendorRoot>
<GD32F403_VendorRoot>/CMSIS
<GD32F403_VendorRoot>/CMSIS/GD/GD32F403/Include
<GD32F403_VendorRoot>/GD32F403_standard_peripheral/Include
<ProductProjectConfig>  # 包含项目自己的 gd32f403_libopt.h
```

兼容层公共头文件：

```text
Include/stm32f4xx.h
Include/stm32f401xe.h
Include/stm32f401_register_compat.h
Include/stm32f401_register_bits.h
```

主要源文件：

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
Port/gd32_timer_dma_port.c
Port/stm32_timer_trigger_map.c
Port/gd32_adc_port.c
Port/gd32_i2c_port.c
Port/gd32_i2c_flag_port.c
Port/gd32_spi_port.c
Port/gd32_instance_map.c
Port/gd32_tick_port.c
Port/gd32_irq_port.c
Port/gd32_rcc_port.c
```

同时加入官方 CMSIS system/startup、所用模块的 GD32 SPL 源文件，以及产品工程自己的 `gd32f403_libopt.h`。官方 Firmware Library 根目录不带该选择头，Demo 将它放在各应用工程内；不要从无关 Demo 盲目复制全模块配置。不要复制或修改官方库到本仓库。

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
- TIMER DMA semantic request 由 Port 明确映射到官方 `TIMER_DMA_UPD/CHxD/CMTD/TRGD`；不能把 enum 序号当作 `TIMER_DMAINTEN` bit。
- ADC 外部触发只接受目标可保持语义的源和 rising edge；采样时间采用不短于请求值的保守映射，480 cycles 明确失败。
- UART 只支持 16 倍过采样。
- ADC `ADON` 直接 enable 后再走 HAL Start 仍会校准；严格模式不定义直接 `SWSTART`。
- `TIM2/TIM5` 直接 CNT/ARR/CCR 访问仍只有 16 位，不能绕过 HAL 的范围保护。
- `GD32_HAL_I2C_GetFlags()` 在 ADDSEND pending 时不会读 STAT1；只有明确的 address-clear 路径执行 STAT0→STAT1 状态转换。

## 自检

```powershell
pwsh.exe -File .\Tests\run_checks.ps1
pwsh.exe -File .\Tests\run_iar_checks.ps1
```

`run_checks.ps1` 包含 ARM Cortex-M4 `-Wall -Wextra -Werror` 编译、与官方 GD32 SPL 的目标 ELF 链接、unsupported compile guards、Clang Analyzer、Instance/RCC 映射及各模块 Host 状态机测试。两个脚本均可用 `-VendorRoot <path>` 指向只读的官方固件库根目录；默认为相邻的 `../GD32F403RET6`。例如：

```powershell
pwsh.exe -File .\Tests\run_checks.ps1 `
  -VendorRoot '..\GD32F403_Demo_Suites_V3.0.3\GD32F403_Firmware_Library'
```

IAR 检查独立运行；未安装 IAR 时脚本明确输出 `SKIPPED`，不会伪报通过。

## 文档

- [当前架构](Docs/ARCHITECTURE.md)
- [STM32/GD32 映射](Docs/STM32_GD32_MAPPING.md)
- [限制](Docs/LIMITATION.md)
- [TIMER trigger 映射](Docs/TIMER_TRIGGER_MAPPING.md)
- [DMA 差异](Docs/DMA_DIFFERENCE.md)
- [移植指南](Docs/PORTING_GUIDE.md)
- [Register compatibility matrix](Docs/REGISTER_COMPATIBILITY_MATRIX.md)

所有自动检查仍属于代码级证据。真实 GD32F403RET6 板上的引脚、时钟、IRQ、DMA、TIMER 波形和 ADC 性能尚需产品工程验证，不能据此宣称量产签核。
