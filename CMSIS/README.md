# CMSIS 集成边界

本目录不复制或修改厂商 CMSIS。产品工程应从 GD32F403 官方固件包加入：

- `CMSIS/GD/GD32F403/Include/gd32f403.h`
- `CMSIS/GD/GD32F403/Source/system_gd32f403.c`
- 对应工具链的 GD32F403 startup 文件
- ARM CMSIS Core 头文件
- 产品工程自己的 `gd32f403_libopt.h`（按实际链接的 SPL 模块选择）

兼容层公共头文件只允许经 `Include/stm32_hal_cmsis_bridge.h` 引入目标 device CMSIS。外设 SPL 头文件和寄存器定义只允许由 `Port/` 使用。

官方 Firmware Library 的 `gd32f403.h` 在启用 SPL 时会包含 `gd32f403_libopt.h`，但库根目录不提供通用副本；该文件属于应用工程配置，不是 CMSIS/SPL 本体。本仓库只在 `Tests/ArmShims/` 保留一份测试模块选择头。

默认验证使用相邻只读目录 `../GD32F403RET6/CMSIS/`。`Tests/run_checks.ps1` 和 `Tests/run_iar_checks.ps1` 也可用 `-VendorRoot` 指向官方固件库根目录；脚本只读取厂商文件，不会复制或改写。

`GD32F403_Demo_Suites_V3.0.3` 随带的 CMSIS 是 GD32F403 系列级依赖，可用于 RET6 目标编译；其 `GD32403Z-EVAL` startup/project 配置面向 ZET6/LQFP144 开发板，不能作为 RET6 板级引脚或链接配置直接照搬。
套件 Demo 工程标注的 IAR 版本为 EWARM 7.40.2，不能替代本项目对 IAR 9.30 的独立编译验证。
