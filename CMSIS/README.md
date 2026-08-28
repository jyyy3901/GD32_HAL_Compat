# CMSIS 集成边界

本目录不复制或修改厂商 CMSIS。产品工程应从 GD32F403 官方固件包加入：

- `CMSIS/GD/GD32F403/Include/gd32f403.h`
- `CMSIS/GD/GD32F403/Source/system_gd32f403.c`
- 对应工具链的 GD32F403 startup 文件
- ARM CMSIS Core 头文件

兼容层公共头文件只允许经 `Include/stm32_hal_cmsis_bridge.h` 引入目标 device CMSIS。外设 SPL 头文件和寄存器定义只允许由 `Port/` 使用。

本仓库验证使用相邻只读目录 `../GD32F403RET6/CMSIS/`，不会改写官方文件。
