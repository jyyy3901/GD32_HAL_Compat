# 官方资料与源码清单

> 0.9.0 已将兼容层目录改为 `Include/Source/Port`；本文中的调研记录仍保留原始官方目录名称。

## 1. 扫描范围

- `GD32F403RET6/`
- `STM32F401VEH6/`
- `STM32F401VEH6/HAL库/stm32f4xx-hal-driver/`

本清单记录本轮实际读取和检索到的迁移输入。两个官方目录均保持只读。

## 2. GD32F403RET6 资料与器件层

| 类别 | 本地文件 |
|---|---|
| User Manual | `GD32F403RET6/GD32F403_User_Manual_Rev3.1.pdf` |
| Datasheet | `GD32F403RET6/GD32F403xx_Datasheet_Rev3.1.pdf` |
| Device Header | `GD32F403RET6/CMSIS/GD/GD32F403/Include/gd32f403.h` |
| System Header | `GD32F403RET6/CMSIS/GD/GD32F403/Include/system_gd32f403.h` |
| System Source | `GD32F403RET6/CMSIS/GD/GD32F403/Source/system_gd32f403.c` |
| GCC Startup | `GD32F403RET6/CMSIS/GD/GD32F403/Source/GCC/startup_gd32f403.S` |
| ARM Startup | `GD32F403RET6/CMSIS/GD/GD32F403/Source/ARM/startup_gd32f403.s` |
| IAR Startup | `GD32F403RET6/CMSIS/GD/GD32F403/Source/IAR/startup_gd32f403.s` |
| SPL 聚合配置 | `GD32F403RET6/gd32f403_libopt.h` |
| 示例 IRQ 文件 | `GD32F403RET6/gd32f403_it.c`、`gd32f403_it.h` |

CMSIS Core 位于 `GD32F403RET6/CMSIS/`；目标工程必须只选择一种编译器对应的 startup 文件。

## 3. GD32F403 SPL 外设驱动

`GD32F403RET6/GD32F403_standard_peripheral/Include/` 与 `Source/` 中已确认存在本项目涉及的成对模块：

| 迁移模块 | GD32 SPL 文件前缀 |
|---|---|
| Clock | `gd32f403_rcu` |
| GPIO/AFIO | `gd32f403_gpio` |
| EXTI | `gd32f403_exti` |
| Cortex 辅助 | `gd32f403_misc` |
| UART/USART | `gd32f403_usart` |
| DMA | `gd32f403_dma` |
| TIMER | `gd32f403_timer` |
| ADC | `gd32f403_adc` |
| I2C | `gd32f403_i2c` |
| SPI | `gd32f403_spi` |
| Flash | `gd32f403_fmc` |

当前兼容层直接使用 RCU、GPIO、EXTI、FMC、DMA、USART、TIMER、ADC、I2C 和 SPI SPL；NVIC、SysTick、MPU 使用 GD32 CMSIS Core 定义。

## 4. STM32 参考输入

| 类别 | 本地文件/目录 |
|---|---|
| STM32F401 Reference Manual | `STM32F401VEH6/rm0368-stm32f401xbc-and-stm32f401xde-advanced-armbased-32bit-mcus-stmicroelectronics.pdf` |
| Cortex-M4 Programming Manual | `STM32F401VEH6/pm0214-stm32-cortexm4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf` |
| STM32F4 HAL V1.8.5 | `STM32F401VEH6/HAL库/stm32f4xx-hal-driver/` |

本地 STM32 目录未提供具体应用工程、STM32 CMSIS Device、startup、system 或链接脚本；因此本轮只能建立驱动兼容基础设施，不能完成原产品应用的逐文件编译迁移。

## 5. 已审计的 STM32 HAL 模块

下列模块均检查了 `Include/` 声明和 `Source/` 实现；带 Ex 的模块也纳入 API Mapping：

- Core：`stm32f4xx_hal.[ch]`、`stm32f4xx_hal_def.h`、`stm32f4xx_hal_conf_template.h`。
- GPIO/Cortex/RCC：`stm32f4xx_hal_gpio*`、`stm32f4xx_hal_cortex*`、`stm32f4xx_hal_rcc*`。
- 串口：`stm32f4xx_hal_uart*`、`stm32f4xx_hal_usart*`。
- 数据搬运与定时：`stm32f4xx_hal_dma*`、`stm32f4xx_hal_tim*`。
- 模数转换：`stm32f4xx_hal_adc*`。
- 总线：`stm32f4xx_hal_i2c*`、`stm32f4xx_hal_spi*`。
- 系统外设：`stm32f4xx_hal_exti*`、`stm32f4xx_hal_flash*`。

Phase 8 额外逐页审计了 STM32 RM0368 的 Flash、RCC、EXTI 章节，以及 GD32F403 User Manual Rev3.1 的 FMC、RCU、EXTI 章节；实现以寄存器语义、SPL 头/源和具体 512 KB 型号边界交叉核对。

## 6. 版本与边界

- GD32 SPL：V3.0.3。
- STM32F4 HAL：V1.8.5。
- 目标只针对 `GD32F403RET6`（512 KB Flash、LQFP64）。
- 当前交付不修改上述任何官方文件，也不把未提供的 STM32 应用源代码视为已迁移。
