# STM32F401VEH6 到 GD32F403RET6 移植指南

> 0.9.0 的架构、映射和限制以 `ARCHITECTURE.md`、`STM32_GD32_MAPPING.md`、`LIMITATION.md` 为准。

## 1. 先做硬件迁移审计

源器件为 LQFP100，目标为 LQFP64。开始代码移植前先建立 Pin/net 表，确认每个 GPIO、ADC、TIMER channel、USART、SPI、I2C、SWD、HXTAL、BOOT 和电源引脚。GPIOE 等源端口不能靠兼容层解决。

## 2. 保留的应用文件

优先保留：

- 与寄存器无关的业务逻辑。
- 只调用已支持 HAL API 的模块。
- HAL weak Callback 的应用实现。
- 与 MCU 无关的协议、算法和数据结构。

需要逐项审计：

- 直接访问 `Instance->CRx/SR/DR/ARR/CCR` 的代码。
- 依赖 STM32 IRQn、DMA Stream/Channel、GPIO AF number 的代码。
- `HAL_RCC_*` 时钟树配置；门控与查询可保留，STM32 PLL 参数必须重写。
- TIM2/TIM5 32 位范围。

## 3. 删除或替换的底层文件

不要把以下 STM32 文件加入 GD32 链接：

- STM32 startup、system_stm32f4xx、链接脚本。
- STM32 CMSIS device 源/头文件。
- STM32 HAL Driver 的 `.c` 文件。
- CubeMX 生成的 STM32 IRQ、MSP 和 SystemClock 实现。

替换为：

- GD32F403 官方 startup。
- GD32 `system_gd32f403.c` 或经过审计的产品时钟文件。
- GD32F403 链接脚本。
- GD32 SPL 源文件。
- 本 Compatibility Layer 源文件。

## 4. Include 顺序

应用继续包含：

```c
#include "stm32f4xx_hal.h"
```

工程 Include path 应让 `GD32_HAL_Compat/Inc` 位于 STM32 HAL Inc 之前，并且完全移除 STM32 CMSIS device include path，避免同名 `GPIOA`、IRQn_Type 和寄存器类型混入。

## 5. SystemClock_Config

重写为 GD32 原生实现，不复制 STM32 PLL 参数。当前官方 `system_gd32f403.c` 使用 25 MHz HXTAL 配置 168 MHz；产品必须核对实际晶振、Flash wait state、USB 48 MHz、APB1/APB2、ADC 和 TIMER 时钟。兼容层的 `HAL_RCC_OscConfig()` 可控制 HSI/HSE，但请求 `RCC_PLL_ON` 会返回 `HAL_ERROR`；PLL 必须先由 GD32 原生代码建立。

时钟改变后调用：

```c
SystemCoreClockUpdate();
(void)HAL_InitTick(HAL_GetTickPrio());
```

## 6. MX_xxx_Init

函数名可以保留，内部按 GD32 硬件重写：

```c
static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /* 默认 AF 与 remap 在这里用 GD32 SPL 明确配置。 */
}
```

当前可使用 GPIO、UART polling/IT/DMA/Abort、基础 DMA、TIMER Base/PWM/OC/IC/OnePulse/IRQ/DMA/主从 HAL、ADC regular polling/IT/DMA、I2C master polling/IT/Mem/Ready、SPI polling/IT/DMA、有限 RCC 时钟 API、独立 EXTI Handle，以及受写区保护的 FLASH polling/IT API。ADC injected/watchdog、I2C DMA/Slave、SPI CRC、RCC PLL 参数配置和 Flash option bytes 等未实现 API 不要调用。

## 7. MSP

保留 `HAL_MspInit()` 和后续 `HAL_UART_MspInit()` 等函数名，内部执行：

- RCU clock enable。
- GPIO mode + AFIO remap。
- DMA controller/channel 选择。
- NVIC priority/enable。

不要在业务层散布 `#ifdef GD32`；厂商差异集中在 MSP、MX 和 Port 层。

## 8. IRQ 文件

使用 GD32 startup 定义的中断名。例如 USART1 的 STM32 业务 Handle 可能对应 GD32 `USART0_IRQHandler`：

```c
void USART0_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}
```

当前 UART polling/IT/DMA 均使用此 UART IRQ 入口处理错误和 TX TC 收尾。当前 SysTick 使用：

```c
void SysTick_Handler(void)
{
    GD32_HAL_SysTickIRQHandler();
}
```

GPIO EXTI IRQ 可调用 `HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_x)`；使用独立 Handle 时调用 `HAL_EXTI_IRQHandler(&hexti)`。Flash IT 操作由 `FMC_IRQHandler()` 调用 `HAL_FLASH_IRQHandler()`。

DMA IRQ 使用 GD32 startup 的 `DMA0_Channelx_IRQHandler`/`DMA1_Channelx_IRQHandler` 名称，并调用 `HAL_DMA_IRQHandler(&hdma_x)`。不要保留 STM32 `DMA1_Streamx_IRQHandler` 名称。

TIMER IRQ 也使用 GD32 startup 名称。例如兼容层 `TIM2` 对应 `TIMER1_IRQHandler`；共享向量 `TIM1_UP_TIM10` 对应 `TIMER0_UP_TIMER9_IRQHandler`。每个入口调用相应 Handle 的 `HAL_TIM_IRQHandler()`。

## 9. DMA Instance 与请求迁移

STM32 的 `DMA1/2_Streamx + Init.Channel` 不是 GD32 的可选请求路由。迁移每个 DMA Handle 时：

1. 从 User Manual 表 10-3/10-4 找到目标外设请求固定 Channel。
2. 把 `Instance` 改为 `GD32_DMA0_CHANNELx` 或 `GD32_DMA1_CHANNELx`。
3. 把 `Init.Channel` 改为对应 `GD32_DMA_REQUEST_*`。
4. FIFO、burst、PFCTRL、double-buffer 配置必须移除或重构。
5. UART、ADC、SPI 使用 `__HAL_LINKDMA()` 绑定；TIMER 使用 `htim.hdma[TIM_DMA_ID_*]` 与 `Parent` 绑定。UART/TIMER/ADC/SPI 均管理自身 DMA 请求使能位；I2C DMA 当前明确不支持。

ADC1 固定映射 GD ADC0，DMA 固定为 `GD32_DMA0_CHANNEL0 + GD32_DMA_REQUEST_ADC0`，方向 P2M，PINC disable，MINC enable，外设/内存均 word。`DMAContinuousRequests=ENABLE` 必须搭配 circular；DISABLE 必须搭配 normal。`HAL_ADC_Init()` 后尚未校准，首次 Start/Start_IT/Start_DMA 会在 enable 后等待并校准；每次 Stop 造成 ADC 掉电，下一次 Start 会重新校准。

I2C1/I2C2 映射 GD I2C0/I2C1。7 位 `DevAddress` 保持 STM HAL 左移一位约定；Mem/IsDeviceReady 仅支持 7 位。中断文件分别在 `I2C0_EV_IRQHandler`/`I2C0_ER_IRQHandler` 中调用 `HAL_I2C_EV_IRQHandler(&hi2c1)`/`HAL_I2C_ER_IRQHandler(&hi2c1)`，I2C1 对应同理。软件复位恢复不包含 GPIO SCL 脉冲；外部器件拉低 SDA/SCL 的产品必须实现独立板级恢复流程。

SPI1/2/3 映射 GD SPI0/1/2。SPI DMA 固定映射如下，8 位帧使用 BYTE/BYTE，16 位帧使用 HALFWORD/HALFWORD，PINC disable、MINC enable：

| 兼容层 SPI | TX DMA | RX DMA |
|---|---|---|
| SPI1 → GD SPI0 | DMA0 CH2 / `GD32_DMA_REQUEST_SPI0_TX` | DMA0 CH1 / `GD32_DMA_REQUEST_SPI0_RX` |
| SPI2 → GD SPI1 | DMA0 CH4 / `GD32_DMA_REQUEST_SPI1_I2S1_TX` | DMA0 CH3 / `GD32_DMA_REQUEST_SPI1_I2S1_RX` |
| SPI3 → GD SPI2 | DMA1 CH1 / `GD32_DMA_REQUEST_SPI2_I2S2_TX` | DMA1 CH0 / `GD32_DMA_REQUEST_SPI2_I2S2_RX` |

主机接收使用 `SPI_DIRECTION_2LINES` 和 dummy-clock；`SPI_DIRECTION_2LINES_RXONLY` 主机初始化、CRC enable 当前返回 `HAL_ERROR`。

UART 固定映射如下；必须同时匹配物理 Channel 和请求令牌：

| 兼容层 UART | TX DMA | RX DMA |
|---|---|---|
| USART1 → GD USART0 | DMA0 CH3 / `GD32_DMA_REQUEST_USART0_TX` | DMA0 CH4 / `GD32_DMA_REQUEST_USART0_RX` |
| USART2 → GD USART1 | DMA0 CH6 / `GD32_DMA_REQUEST_USART1_TX` | DMA0 CH5 / `GD32_DMA_REQUEST_USART1_RX` |
| USART6 → GD USART2 | DMA0 CH1 / `GD32_DMA_REQUEST_USART2_TX` | DMA0 CH2 / `GD32_DMA_REQUEST_USART2_RX` |

8 位或带校验的有效 8 位数据使用 BYTE 对齐；9 位无校验数据使用 HALFWORD 对齐。外设地址不递增、内存地址递增。初始化 UART/DMA 后执行 `__HAL_LINKDMA(&huart, hdmatx, hdma_tx)` 或 RX 对应绑定，再调用 UART DMA API。

## 10. TIMER 迁移

1. `TIMn` 名称可保留，但先确认目标 `TIMER(n-1)` 的通道数、计数模式、OnePulse、主从和 DMA 能力。
2. 把 TIM2/TIM5 的 ARR、CNT、CCR、DMA buffer 全部审计为 `<= 0xFFFF`；需要更大范围时重构，不接受截断。
3. PWM/OC/IC GPIO 和 remap 仍在 MSP 中按 GD32 Datasheet/AFIO 原生配置。
4. 不修改 `TIM_TS_ITR0..3` 常量；兼容层会按 F401 源连接与 GD ITI 矩阵解析。不等价连接会返回 `HAL_ERROR`，此时必须重新选择级联拓扑。
5. TIMER DMA buffer 使用 `uint32_t`，DMA 配置使用 WORD/WORD、外设不递增、内存递增，并选用对应 `GD32_DMA_REQUEST_TIMERx_*`。
6. 编码器、互补输出、死区/刹车、Hall、DMA Burst 必须保留为待重构项，不能调用未提供 API。

## 11. GPIO AF / remap

STM32 的 `GPIO_AF7_USART1` 等数值不能用于选择 GD32 外设。迁移步骤：

1. 从 GD32F403RET6 Datasheet LQFP64 引脚表确认目标引脚功能。
2. 判断是默认功能还是 AFIO remap。
3. 使用 `gpio_pin_remap_config()` 明确配置 remap。
4. 再使用 HAL_GPIO_Init 配置 AF_PP/AF_OD 电气模式，或全部使用 GD32 SPL 原生初始化。

PD0/PD1 还必须先停用 HXTAL，并完成 `GPIO_PD01_REMAP` 与板级审计，然后把 `GD32_HAL_ALLOW_PD01_GPIO` 设为 `1U`；默认配置会拒绝这两个引脚。

## 12. RCC 迁移

1. 用 GD32 原生代码建立 PLL、USB 48 MHz 和初始总线时钟；不要迁移 CubeMX 的 `PLLM/PLLN/PLLP/PLLQ`。
2. 可保留常用时钟门控、`HAL_RCC_Get*Freq()`、HSI/HSE 控制和当前/已配置时钟源切换。
3. `HAL_RCC_ClockConfig()` 会校验 168/168/84/168 MHz 的 SYSCLK/AHB/APB1/APB2 上限，按升频前、降频后的顺序调整 Flash wait state，并更新 Tick。
4. LSE/LSI、RTC/备份域与新建 PLL 请求当前返回 `HAL_ERROR`，应留在原生系统时钟代码中。

## 13. EXTI 迁移

只使用 `EXTI_LINE_0..18`。GPIO0..15 通过 `EXTI_ConfigTypeDef.GPIOSel` 映射到 AFIO；GPIOD 仍遵守 PD0/PD1 晶振保护和仅 PD0/1/2 封装边界。目标芯片没有兼容层可安全暴露的 EXTI19..22，旧代码必须改写。GD32 只有一个 pending 位，双边沿触发后 IRQ 无法仅靠该位判断实际边沿方向。

## 14. FLASH 分区与擦写

FLASH 默认无可写范围。先在 GD32 链接脚本中建立独立、2 KB 对齐且不承载代码/向量/常量的数据分区，再在启动时调用 `GD32_HAL_FLASH_SetWritableRegion(start, length)`。STM32 sector 0..7 的擦除请求会扩展为地址范围内的多个 GD32 2 KB page；也可用 `GD32_HAL_FLASHEx_ErasePages()` 明确按页擦除。

只支持 half-word/word 编程；byte、double-word、mass erase 和 option bytes 返回 `HAL_ERROR`。擦除或编程期间掉电仍可能破坏目标页，产品必须设计冗余、校验、版本和掉电恢复策略。不要把当前 PC 所在页或向量表首个 2 KB page 纳入授权区。

## 15. 编译器

- GCC/Clang：`__weak` 和 `__packed` 使用 attribute。
- ARMClang：兼容 attribute。
- IAR：使用 IAR 关键字/CMSIS 定义。

发布前要分别建立 IAR、GCC ARM Embedded、ARMCC/ARMClang 工程；当前自动检查使用 Clang ARM target。

## 16. 最小上板顺序

1. 启动文件、链接脚本和 SystemCoreClock。
2. SWD、HardFault、SysTick 1 ms。
3. 单个 GPIO SET/RESET/Toggle。
4. GPIO input 与 EXTI。
5. 再进入 UART polling 和 IT，然后单独验证 DMA M2M、DMA 外设请求，最后验证 UART DMA normal/circular 与 TC 收尾。
6. TIMER 按 Base 周期、PWM、Update/CC IRQ、OC、IC、OnePulse、DMA、主从级联的顺序逐项测波形。
7. 测量 HSI/HSE/PLL、SYSCLK/HCLK/PCLK 与 Tick，再验证 EXTI0..18 中实际使用的线路。
8. 只在专用测试分区验证 Flash half-word/word、sector-to-page 展开、IT Callback、越界拒绝和掉电恢复。

每一步保留示波器/逻辑分析仪或串口回环证据。编译通过不是硬件验收。
