# STM32F401 DMA 与 GD32F403 DMA 差异

## 硬件模型

| 项目 | STM32F401 | GD32F403 | 0.10.1 策略 |
|---|---|---|---|
| 组织 | DMA1/2，各 8 Stream | DMA0 7 Channel、DMA1 5 Channel | 内部只使用 controller/channel |
| 请求选择 | Stream + Channel selector | 外设请求固定接到物理 Channel | 使用 `GD32_DMA_REQUEST_*` 令牌校验 |
| 多请求 | Channel selector 选择 | 多个请求在 Channel 前 OR | 同一物理 Channel 只能有一个活跃 owner |
| FIFO | 有 | 无 | 非 disable 配置返回 `HAL_ERROR` |
| Burst | 有 | 无 | 非 single 配置返回 `HAL_ERROR` |
| PFCTRL | 部分支持 | 无 STM32 等价 | 返回 `HAL_ERROR` |
| Double buffer | 有 | 无当前安全等价 | 编译或运行时明确不支持 |

## Handle

`DMA_HandleTypeDef.Instance` 可继续填写 CubeMX 风格的 `DMA1/2_Stream0..7` opaque token，也可填写兼容层原有的 GD32 Channel 语义令牌。两者都不是可解引用的 STM32 Stream 寄存器地址。Init 后缓存：

- `GD32_INSTANCE`：Channel register block；
- `gd32_dma_periph`：DMA0/DMA1；
- `gd32_dma_channel`：0..6 或 0..4；
- `GD32_REQUEST`：已转换的 GD32 固定外设请求；
- `GD32_RESOLVED_FROM`：原始 Stream/Channel token；
- `GD32_IRQ_NUMBER` 与 `GD32_RESOURCE`。

保留 `DMA_InitTypeDef.Channel/FIFOMode/FIFOThreshold/MemBurst/PeriphBurst` 是为了常见 STM32 初始化代码可编译，不代表目标硬件拥有这些能力。

## 初始化要求

0.10.0 可保留矩阵中能够唯一判定的 CubeMX 初始化语法。例如 USART1 TX：

```c
hdma.Instance = DMA2_Stream7;
hdma.Init.Channel = DMA_CHANNEL_4;
hdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
```

`HAL_DMA_Init()` 根据 Stream、Channel selector、方向以及后续绑定的外设能力，将它转换为 GD32 的物理 controller/channel/request。USART1/2/6、ADC1、SPI1/2/3 和 I2C1/2 的唯一映射立即解析。

TIMER request 不能只靠 Stream/Channel/Direction 唯一判定。0.10.1 对 RM0368 已确认的 TIM1..5 request 保存 deferred 状态；`HAL_TIM_Base/OC/PWM/IC_Start_DMA()` 使用 TIM Instance 与 `TIM_DMA_ID_UPDATE/CCx` 完成最终映射。STM32 request 与目标 GD32 event 任一不匹配即 `HAL_ERROR + ErrorHook`，不会把 Stream 数当作 GD32 Channel。显式目标 token 路径仍继续支持。

ADC 与 TIMER DMA 仅接受成对的 `HALFWORD/HALFWORD` 或 `WORD/WORD`。HALFWORD buffer 按 16 位 item 递增，WORD buffer 按 32 位 item 递增；`Length` 对两种模式都表示 item 数。目标 TIMER CAR/CHxCV 支持 16/32 位访问但有效字段只有 16 位，因此 WORD 输出中的任何 `> 0xFFFF` 值都会在启动前失败。

显式目标映射语法继续支持：

```c
DMA_HandleTypeDef hdma = {0};

hdma.Instance = GD32_DMA0_CHANNEL3;
hdma.Init.Channel = GD32_DMA_REQUEST_USART0_TX;
hdma.Init.Direction = DMA_MEMORY_TO_PERIPH;
hdma.Init.PeriphInc = DMA_PINC_DISABLE;
hdma.Init.MemInc = DMA_MINC_ENABLE;
hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
hdma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
hdma.Init.Mode = DMA_NORMAL;
hdma.Init.Priority = DMA_PRIORITY_HIGH;
hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
hdma.Init.MemBurst = DMA_MBURST_SINGLE;
hdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
```

外设 DMA API 还会检查 Parent、方向、数据宽度、固定 Channel/request 组合和外设 DMA capability。任何不匹配均返回 `HAL_ERROR`，不会启动“看似成功”的搬运。

## 寄存器边界

`DMA_Stream_TypeDef` 保持 incomplete type。`DMA2_Stream0->CR`、`DMA2_Stream0->NDTR` 等访问必须在编译期失败；不会用 GD32 Channel 寄存器伪造 STM32 Stream/FIFO 布局。literal STM32 Stream 地址也不属于兼容范围。

## 状态与 IRQ

- `HAL_DMA_Start()` 配置并实际使能 GD32 Channel 后才返回 `HAL_OK`。
- `HAL_DMA_Start_IT()` 配置 half/full/error 中断并使能 Channel。
- normal full 完成恢复 READY、释放 owner 并回调；circular full/half 保持 BUSY 和 owner。
- error/abort 会关闭 Channel、中断与父外设请求，并成对释放 owner。
- GD32 IRQ 入口必须调用 `HAL_DMA_IRQHandler(&hdma)`。

详细固定请求表仍可参考 `STM32_GD32_DIFFERENCES.md`，但产品必须以所用 GD32F403 User Manual 版本再次核对。
