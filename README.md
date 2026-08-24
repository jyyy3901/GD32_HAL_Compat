# GD32F403RET6 STM32F401 HAL Compatibility Layer

## 项目状态

这是一个面向 GD32F403RET6 + GD32F403 SPL V3.0.3 的 STM32F401 HAL 兼容层基础设施。当前版本 `0.8.0` 已完成 HAL Core、GPIO、SysTick、Cortex/NVIC/MPU、UART polling/IT/DMA/Abort、DMA 基础/IT/Abort、TIMER Base/PWM/OC/IC/OnePulse/IRQ/DMA/主从同步、ADC regular polling/IT/DMA、I2C master polling/IT/Mem/Ready、SPI polling/IT/DMA，以及有限 RCC 时钟 API、独立 EXTI Handle 和受保护 FLASH/FMC HAL。

USART 同步模式、I2C DMA/Slave、SPI CRC、STM32 PLL 参数复现、RTC/备份域时钟、Flash option bytes 与 mass erase 尚未实现。全部模块仍需真实目标板验证，不能据此认定量产签核。

## 目录

```text
GD32_HAL_Compat/
├─ Inc/       STM32 HAL 风格公共头文件
├─ Src/       HAL 语义实现，不直接依赖 GD32 寄存器布局
├─ Port/      GD32 SPL、Instance、IRQ、Tick 和硬件差异转换
├─ Docs/      差异、API Mapping、计划、限制、移植指南
└─ Tests/     Host Mock 与编译检查
```

## 第三方只读目录

- `../GD32F403RET6/`
- `../STM32F401VEH6/HAL库/stm32f4xx-hal-driver/`

兼容层不修改任何官方库文件。

## 加入 GD32 工程

源文件：

```text
GD32_HAL_Compat/Src/stm32f4xx_hal.c
GD32_HAL_Compat/Src/stm32f4xx_hal_gpio.c
GD32_HAL_Compat/Src/stm32f4xx_hal_cortex.c
GD32_HAL_Compat/Src/stm32f4xx_hal_dma.c
GD32_HAL_Compat/Src/stm32f4xx_hal_uart.c
GD32_HAL_Compat/Src/stm32f4xx_hal_uart_callbacks.c
GD32_HAL_Compat/Src/stm32f4xx_hal_tim.c
GD32_HAL_Compat/Src/stm32f4xx_hal_tim_ex.c
GD32_HAL_Compat/Src/stm32f4xx_hal_tim_callbacks.c
GD32_HAL_Compat/Src/stm32f4xx_hal_adc.c
GD32_HAL_Compat/Src/stm32f4xx_hal_adc_callbacks.c
GD32_HAL_Compat/Src/stm32f4xx_hal_i2c.c
GD32_HAL_Compat/Src/stm32f4xx_hal_i2c_callbacks.c
GD32_HAL_Compat/Src/stm32f4xx_hal_spi.c
GD32_HAL_Compat/Src/stm32f4xx_hal_spi_callbacks.c
GD32_HAL_Compat/Src/stm32f4xx_hal_rcc.c
GD32_HAL_Compat/Src/stm32f4xx_hal_rcc_callbacks.c
GD32_HAL_Compat/Src/stm32f4xx_hal_exti.c
GD32_HAL_Compat/Src/stm32f4xx_hal_flash.c
GD32_HAL_Compat/Src/stm32f4xx_hal_flash_ex.c
GD32_HAL_Compat/Src/stm32f4xx_hal_flash_callbacks.c
GD32_HAL_Compat/Port/gd32_hal_port.c
GD32_HAL_Compat/Port/gd32_hal_dma_port.c
GD32_HAL_Compat/Port/gd32_hal_uart_port.c
GD32_HAL_Compat/Port/gd32_hal_timer_port.c
GD32_HAL_Compat/Port/gd32_hal_timer_itr.c
GD32_HAL_Compat/Port/gd32_hal_adc_port.c
GD32_HAL_Compat/Port/gd32_hal_i2c_port.c
GD32_HAL_Compat/Port/gd32_hal_spi_port.c
GD32_HAL_Compat/Port/gd32_hal_rcc_port.c
GD32_HAL_Compat/Port/gd32_hal_exti_port.c
GD32_HAL_Compat/Port/gd32_hal_flash_port.c
GD32_HAL_Compat/Port/gd32_hal_tick.c
GD32_HAL_Compat/Port/gd32_hal_irq.c
```

Include path：

```text
GD32_HAL_Compat/Inc
GD32_HAL_Compat/Port
GD32F403RET6
GD32F403RET6/CMSIS
GD32F403RET6/CMSIS/GD/GD32F403/Include
GD32F403RET6/GD32F403_standard_peripheral/Include
```

同时加入 GD32 官方 CMSIS system/startup 与用到的 SPL 源文件。当前阶段至少需要 RCU、GPIO、EXTI、FMC、DMA、USART、TIMER、ADC、I2C、SPI 对应 SPL 源文件；只有产品代码调用 SPL misc API 时才需加入 MISC 源文件。

## SysTick IRQ

在产品的 GD32 中断文件中保留唯一的 `SysTick_Handler`，调用兼容层 helper：

```c
void SysTick_Handler(void)
{
    GD32_HAL_SysTickIRQHandler();
}
```

应用也可以直接调用 `HAL_IncTick()` 和 `HAL_SYSTICK_IRQHandler()`。兼容层不抢占启动文件弱中断名。

## GPIO 示例

```c
GPIO_InitTypeDef init = {0};

__HAL_RCC_GPIOA_CLK_ENABLE();
init.Pin = GPIO_PIN_5;
init.Mode = GPIO_MODE_OUTPUT_PP;
init.Pull = GPIO_NOPULL;
init.Speed = GPIO_SPEED_FREQ_HIGH;
HAL_GPIO_Init(GPIOA, &init);

HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
```

## GPIO 重要限制

- 当前目标是 LQFP64，只暴露 GPIOA/B/C/D；GPIOD 的硬件封装范围只有 PD0/PD1/PD2。
- PD0/PD1 与 HXTAL 引脚复用，默认禁止。只有停用 HXTAL、完成 GD32 原生 remap 并核对板级连接后，才可将 `GD32_HAL_ALLOW_PD01_GPIO` 配置为 `1U`。
- `GPIO_InitTypeDef.Alternate` 的 STM32 AF 编号不会自动转换为 GD32 AFIO remap。非默认映射必须在 `MX_GPIO_Init()`/MSP 中使用 GD32 SPL 重写。
- GD32 输出/AF 模式没有 STM32F4 式独立 Pull 配置。此类请求会调用 `GD32_HAL_ErrorHook()`。

## UART 示例与 IRQ

UART Handle 和业务调用保持 STM32 HAL 风格，GPIO/AFIO/RCU 仍在 MSP 中按 GD32 原生方式配置：

```c
UART_HandleTypeDef huart1 = {0};

huart1.Instance = USART1;       /* 兼容层映射到 GD32 USART0 */
huart1.Init.BaudRate = 115200U;
huart1.Init.WordLength = UART_WORDLENGTH_8B;
huart1.Init.StopBits = UART_STOPBITS_1;
huart1.Init.Parity = UART_PARITY_NONE;
huart1.Init.Mode = UART_MODE_TX_RX;
huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
huart1.Init.OverSampling = UART_OVERSAMPLING_16;
(void)HAL_UART_Init(&huart1);
```

目标中断名使用 GD32 startup 名称：

```c
void USART0_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}
```

限制：GD32 只支持 16 倍过采样；目标 UART3/4 不支持 CTS/RTS；GD32 UART4（兼容层 `UART5`）不支持 DMA。错误标志按 STAT0/DATA 顺序清除。

## DMA 示例与 IRQ

DMA 初始化必须显式选择 GD32 物理 Channel 和该 Channel 实际支持的请求令牌，不能继续使用 STM32 `DMA1_Streamx/DMA2_Streamx` 地址：

```c
DMA_HandleTypeDef hdma_usart0_tx = {0};

hdma_usart0_tx.Instance = GD32_DMA0_CHANNEL3;
hdma_usart0_tx.Init.Channel = GD32_DMA_REQUEST_USART0_TX;
hdma_usart0_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
hdma_usart0_tx.Init.PeriphInc = DMA_PINC_DISABLE;
hdma_usart0_tx.Init.MemInc = DMA_MINC_ENABLE;
hdma_usart0_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
hdma_usart0_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
hdma_usart0_tx.Init.Mode = DMA_NORMAL;
hdma_usart0_tx.Init.Priority = DMA_PRIORITY_HIGH;
hdma_usart0_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
hdma_usart0_tx.Init.MemBurst = DMA_MBURST_SINGLE;
hdma_usart0_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;
(void)HAL_DMA_Init(&hdma_usart0_tx);
__HAL_LINKDMA(&huart1, hdmatx, hdma_usart0_tx);
```

完成 UART 和 DMA Handle 初始化及 `__HAL_LINKDMA()` 后，可调用 `HAL_UART_Transmit_DMA()`、`HAL_UART_Receive_DMA()`、`HAL_UART_DMAPause()`、`HAL_UART_DMAResume()` 和 `HAL_UART_DMAStop()`。普通 TX 的 DMA 完成只表示数据已搬入 USART DATA；兼容层会继续等待 USART TC，再调用 `HAL_UART_TxCpltCallback()`。Circular TX/RX 在每次 DMA full/half 事件直接回调并保持 BUSY，必须显式 Stop 或 Abort。

目标中断文件使用 GD32 startup 名称：

```c
void DMA0_Channel3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_usart0_tx);
}
```

基础 DMA HAL 已支持 normal/circular、M2M/P2M/M2P、8/16/32 位、polling/IT、half/full/error Callback 和 Abort。FIFO、burst、double-buffer、PFCTRL 无硬件等价并会明确失败；`HAL_DMA_PollForTransfer()` 不接受 circular；`HAL_DMA_Abort_IT()` 在 GD32 上完成硬件停机后同步调用回调。UART DMA 会校验 Parent、方向、宽度以及固定物理 Channel/请求令牌，任何不匹配均返回 `HAL_ERROR`。

## TIMER 示例与限制

```c
TIM_HandleTypeDef htim2 = {0};
TIM_OC_InitTypeDef pwm = {0};

__HAL_RCC_TIM2_CLK_ENABLE();       /* 映射到 GD32 TIMER1 */
htim2.Instance = TIM2;
htim2.Init.Prescaler = 83U;
htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
htim2.Init.Period = 999U;
htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
(void)HAL_TIM_PWM_Init(&htim2);

pwm.OCMode = TIM_OCMODE_PWM1;
pwm.Pulse = 500U;
pwm.OCPolarity = TIM_OCPOLARITY_HIGH;
pwm.OCFastMode = TIM_OCFAST_DISABLE;
(void)HAL_TIM_PWM_ConfigChannel(&htim2, &pwm, TIM_CHANNEL_1);
(void)HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
```

`TIMn` 按名称映射到 `TIMER(n-1)`，但所有目标计数器、ARR 和 CCR 都限制为 `0..0xFFFF`。兼容层对超范围初始化返回 `HAL_ERROR`，对 void 写宏调用错误钩子并拒绝截断。Master/Slave 的 ITR 会先解析 STM32F401 的源定时器，再查询 GD32 ITI 矩阵；STM32 保留连接以及 TIM9 的 `TIM10_OC/TIM11_OC` 专用连接返回 `HAL_ERROR`。

TIMER DMA 必须使用 `GD32_DMA_REQUEST_TIMERx_*` 固定请求令牌、正确物理 Channel、`DMA_PDATAALIGN_WORD`/`DMA_MDATAALIGN_WORD`，并通过 `htim.hdma[TIM_DMA_ID_*]` 与 `Parent` 绑定。编码器、互补输出、死区/刹车、Hall 和 DMA Burst 当前不提供假兼容。

## I2C 示例与限制

`I2C1/I2C2` 分别映射到 GD32 `I2C0/I2C1`。`DevAddress` 保持 STM32 HAL 约定：7 位地址调用前左移一位。

```c
I2C_HandleTypeDef hi2c1 = {0};
uint8_t data[8];

hi2c1.Instance = I2C1;
hi2c1.Init.ClockSpeed = 400000U;
hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
(void)HAL_I2C_Init(&hi2c1);
(void)HAL_I2C_Mem_Read(&hi2c1, 0xA0U, 0x10U,
                       I2C_MEMADD_SIZE_8BIT, data, sizeof(data), 100U);
```

GD32 IRQ 入口分别调用 `HAL_I2C_EV_IRQHandler()` 与 `HAL_I2C_ER_IRQHandler()`；兼容别名 `HAL_I2C_IRQHandler()` 转发到 EV handler。当前支持 7/10 位 master polling/IT、7 位 Mem Read/Write 与 IsDeviceReady；接收末尾按 1/2/N 字节分别配置 ACK/POS/STOP。超时、BERR、ARLO 会执行 I2C 外设软件复位并重配，但这不能释放被外部器件硬拉低的 SDA/SCL，物理总线恢复仍须在产品 MSP/板级代码中以 GPIO 脉冲实现。I2C DMA 与 Slave API 当前不支持。

## SPI 示例、IRQ 与 DMA

`SPI1/2/3` 分别映射到 GD32 `SPI0/1/2`。主机全双工 `HAL_SPI_Receive*()` 通过发送 `0xFF/0xFFFF` dummy frame 产生 SCK。

```c
SPI_HandleTypeDef hspi1 = {0};

hspi1.Instance = SPI1;
hspi1.Init.Mode = SPI_MODE_MASTER;
hspi1.Init.Direction = SPI_DIRECTION_2LINES;
hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
hspi1.Init.NSS = SPI_NSS_SOFT;
hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
hspi1.Init.CRCPolynomial = 7U;
(void)HAL_SPI_Init(&hspi1);
```

SPI DMA 必须绑定正确 Parent、方向、BYTE/HALFWORD 宽度和固定请求：SPI1 为 DMA0 CH1 RX/CH2 TX，SPI2 为 DMA0 CH3 RX/CH4 TX，SPI3 为 DMA1 CH0 RX/CH1 TX。全双工 normal DMA 由 RX 完成事件统一收尾；circular 保持 BUSY 并重复 half/full Callback。TX-only 完成会等待 TBE=1、TRANS=0 并清 OVR。当前明确拒绝 master `SPI_DIRECTION_2LINES_RXONLY` 和 CRC enable；需要这两项时使用经板测的 GD32 原生驱动，不能依赖本兼容层。

## RCC 时钟 API 边界

产品启动时仍应使用经审计的 GD32 原生 `SystemClock_Config()`/`system_gd32f403.c` 建立 PLL 时钟树，不能复制 STM32 的 `PLLM/PLLN/PLLP/PLLQ` 数值。兼容层支持 HSI/IRC8M、HSE/HXTAL 的开关/旁路、当前时钟源与 AHB/APB 分频切换、Flash wait-state 顺序、频率查询、CSS/NMI，以及切换到已经由原生代码配置并运行的 PLL。`HAL_RCC_OscConfig()` 请求 `RCC_PLL_ON`、LSE 或 LSI 会明确返回 `HAL_ERROR`。

`HAL_RCC_ClockConfig()` 会检查目标上限：SYSCLK/AHB/APB2 不超过 168 MHz，APB1 不超过 84 MHz；切换成功后更新 `SystemCoreClock` 并重新初始化 HAL Tick。

## 独立 EXTI Handle

`HAL_EXTI_GetHandle()`、配置、pending、软件触发、IRQ 和 Callback 已实现。只提供 GD32F403 实际存在的 EXTI0..18；GPIO 路由使用 AFIO，EXTI16/17/18 分别对应 LVD、RTC Alarm、USBFS Wakeup。GPIOA/B/C 可路由 0..15，GPIOD 默认仅允许 PD2，PD0/PD1 仍受晶振复用保护。目标 IRQ 文件使用 GD32 startup 中的 `EXTI0_IRQHandler`、`EXTI5_9_IRQHandler` 等名称，并在入口调用对应 Handle 的 `HAL_EXTI_IRQHandler()`。

## FLASH/FMC 安全用法

FLASH 写入默认全部拒绝。产品必须先在链接脚本中保留不含代码、向量表和只读常量的数据分区，再用 2 KB 对齐的地址和长度显式授权：

```c
(void)GD32_HAL_FLASH_SetWritableRegion(0x08060000U, 0x00020000U);
(void)HAL_FLASH_Unlock();
(void)HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x08060000U, 0x12345678U);
(void)HAL_FLASH_Lock();
```

支持 half-word/word polling 与 IT 编程、STM32 sector 0..7 擦除请求和 GD32 原生页擦除扩展。STM32 sector 会展开成覆盖同一地址范围的多个 GD32 2 KB page；mass erase、byte/double-word 和 option bytes 明确不支持。向量表首个 2 KB page、当前执行代码所在 page、授权区外地址均被拒绝；若需完整执行镜像保护，授权区必须与链接脚本的数据分区严格一致。

## 错误钩子

兼容层提供弱函数：

```c
void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail);
```

产品工程可覆盖它并接入断言、故障记录或安全状态。默认实现记录错误，但不阻塞启动。

默认实现会把最近一次错误写入 `gd32HalLastPortError` 和 `gd32HalLastPortErrorDetail`，因此未覆盖钩子时仍可由调试器或故障采集代码观察；对不可安全表达的 GPIO 配置，兼容层会拒绝执行。

## 编译检查

项目的 `Tests/run_checks.ps1` 执行 ARM Cortex-M4 严格对象编译、Clang Analyzer 静态分析、unsupported/不可用 EXTI line 负向编译、弱 Callback 符号检查、TIMER/ADC/I2C/SPI/RCC/EXTI/FLASH 与官方 GD32 SPL 源码的目标 ELF 链接，以及 GPIO/EXTI、Core/SysTick/NVIC/MPU、UART、DMA、TIMER/ITR、ADC、I2C、SPI、RCC、独立 EXTI、FLASH Host Mock 测试。Host Mock 只能检查 API/状态与调用行为，不能代替目标板时钟、引脚、电气、协议波形、Flash 供电中断安全、DMA 总线传输和 IRQ 验证。

## 文档入口

- `Docs/STM32_GD32_DIFFERENCES.md`
- `Docs/SOURCE_INVENTORY.md`
- `Docs/HAL_API_MAPPING.md`
- `Docs/IMPLEMENTATION_PLAN.md`
- `Docs/PORTING_GUIDE.md`
- `Docs/UNSUPPORTED_FEATURES.md`
