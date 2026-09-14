# GD32 SPL 集成边界

本目录不内置 GD32 官方 SPL，避免兼容层维护厂商源码分叉。产品工程应使用 GD32F403 Standard Peripheral Library V3.0.3 或经项目验证的兼容版本，并加入实际使用模块的源文件。

当前兼容范围需要 RCU、GPIO、USART、DMA、TIMER、ADC；I2C、SPI、EXTI、FMC 功能还分别需要对应 SPL 源文件。

规则：

- 不修改官方 SPL；
- 不在 `Source/` 直接调用 SPL；
- 所有 SPL 函数、寄存器和 `RCU_*` 枚举只在 `Port/` 出现；
- GD32 原生扩展可由产品工程直接使用 SPL，但不能伪装成不存在的 STM32F401 外设。

默认验证使用相邻只读目录 `../GD32F403RET6/GD32F403_standard_peripheral/`。`Tests/run_checks.ps1` 和 `Tests/run_iar_checks.ps1` 可用 `-VendorRoot` 指向另一份官方固件库根目录；脚本不会修改或将厂商库纳入本仓库。

`GD32F403_Demo_Suites_V3.0.3/GD32F403_Firmware_Library` 中的 SPL 已经目标编译/链接检查。套件内的 `GD32403Z-EVAL` 示例只能作为外设调用参考，其 GPIOE/F/G、AFIO remap、DMA channel 和晶振配置都必须按实际 RET6/LQFP64 板重新核对。
