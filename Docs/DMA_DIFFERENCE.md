# STM32F401 DMA 与 GD32F403 DMA 差异

## 硬件模型

| 项目 | STM32F401 | GD32F403 | 0.9.0 策略 |
|---|---|---|---|
| 组织 | DMA1/2，各 8 Stream | DMA0 7 Channel、DMA1 5 Channel | 内部只使用 controller/channel |
| 请求选择 | Stream + Channel selector | 外设请求固定接到物理 Channel | 使用 `GD32_DMA_REQUEST_*` 令牌校验 |
| 多请求 | Channel selector 选择 | 多个请求在 Channel 前 OR | 同一物理 Channel 只能有一个活跃 owner |
| FIFO | 有 | 无 | 非 disable 配置返回 `HAL_ERROR` |
| Burst | 有 | 无 | 非 single 配置返回 `HAL_ERROR` |
| PFCTRL | 部分支持 | 无 STM32 等价 | 返回 `HAL_ERROR` |
| Double buffer | 有 | 无当前安全等价 | 编译或运行时明确不支持 |

## Handle

`DMA_HandleTypeDef.Instance` 是 GD32 Channel 的语义令牌，不是 STM32 Stream 寄存器地址。Init 后缓存：

- `GD32_INSTANCE`：Channel register block；
- `gd32_dma_periph`：DMA0/DMA1；
- `gd32_dma_channel`：0..6 或 0..4；
- `GD32_IRQ_NUMBER` 与 `GD32_RESOURCE`。

保留 `DMA_InitTypeDef.Channel/FIFOMode/FIFOThreshold/MemBurst/PeriphBurst` 是为了常见 STM32 初始化代码可编译，不代表目标硬件拥有这些能力。

## 初始化要求

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

## 状态与 IRQ

- `HAL_DMA_Start()` 配置并实际使能 GD32 Channel 后才返回 `HAL_OK`。
- `HAL_DMA_Start_IT()` 配置 half/full/error 中断并使能 Channel。
- normal full 完成恢复 READY、释放 owner 并回调；circular full/half 保持 BUSY 和 owner。
- error/abort 会关闭 Channel、中断与父外设请求，并成对释放 owner。
- GD32 IRQ 入口必须调用 `HAL_DMA_IRQHandler(&hdma)`。

详细固定请求表仍可参考 `STM32_GD32_DIFFERENCES.md`，但产品必须以所用 GD32F403 User Manual 版本再次核对。
