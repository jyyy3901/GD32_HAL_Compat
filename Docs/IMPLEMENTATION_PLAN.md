# 实施计划与质量门

> 本文件保留 0.8.0 阶段记录。0.9.0 当前方案以 `V0.8.0_REVIEW.md` 和 `ARCHITECTURE.md` 为准。

## 1. 开发基线

- 第三方目录 `GD32F403RET6/` 与 `STM32F401VEH6/HAL库/` 只读。
- 所有新增代码位于 `GD32_HAL_Compat/`。
- C11 裸机、无 RTOS、无动态分配。
- 每阶段先更新差异/Mapping，再实现，再做 ARM 目标严格编译和行为测试。

## 2. Phase 0：调研和架构（已完成）

任务：

- 盘点两套官方 PDF、CMSIS、startup、system 和驱动文件。
- 确认 GD32F403RET6 型号资源、封装边界、SPL V3.0.3。
- 建立 `Include/`、`Source/`、`Port/`、`Docs/`、`Tests/` 分层。
- 建立差异、API Mapping、未支持机制和移植指南。

质量门：所有硬件结论可回溯到本地官方资料或 SPL，不引用未确认的寄存器位。

## 3. Phase 1：HAL Core / GPIO / SysTick / Cortex（已完成）

实现：

- HAL_StatusTypeDef、HAL_LockTypeDef、HAL_TickFreqTypeDef。
- `__weak`、`__packed`、`__HAL_LOCK`、`__HAL_UNLOCK`、`__HAL_LINKDMA`。
- HAL_Init/DeInit、Tick、Delay、Tick frequency。
- GPIO init/deinit/read/write/toggle/lock、EXTI IRQ/Callback。
- NVIC、SysTick 和 MPU 常用 API。
- 常用 RCC 门控宏。

质量门：

- ARM Cortex-M4 目标对象以 `-Wall -Wextra -Werror` 编译通过。
- Clang Analyzer 对首阶段全部 C 源文件无告警通过。
- GPIO Host Mock 覆盖 SET/RESET/Toggle/Input/EXTI/非法封装引脚。
- Core Host Mock 覆盖 Init/DeInit、Tick 频率、SysTick IRQ、NVIC 和 MPU。
- 在真实 GD32F403RET6 板上复核 SysTick 1 ms 和 GPIO 波形。

## 4. Phase 2：UART polling / IT（代码完成，待板测）

任务：

- 定义 UART_InitTypeDef、UART_HandleTypeDef、HAL_UART_StateTypeDef、ErrorCode。
- 建立 STM32F401 USART1/2/6 到 GD32 USART0/1/2 的 Instance 表；GD32 UART3/4 只作原生扩展。
- 实现 Init/DeInit、polling TX/RX 和 Timeout。
- 实现独立 TX/RX IT 状态机、错误路径、Abort、IRQ 和 weak Callback。
- 确认 UART3/4 精简功能并在初始化时拒绝不可用选项。
- 仅接受 GD32 硬件实际支持的 16 倍过采样；按 APB 时钟检查波特率分频范围。
- PERR/FERR/NERR/ORERR 按 User Manual 的 STAT0/DATA 读取顺序清除。

已通过的代码质量门：ARM 目标对象严格编译、Clang Analyzer、0/N 字节参数检查、8/9 位 polling、IT TX/RX、HAL_BUSY/timeout、FERR/ORERR、Abort、DMA 阶段守卫，以及 Callback 次数和顺序。

待板级质量门：实际波特率、polling/IT loopback、PERR/FERR/NERR/ORERR 注入、全部五个实例 IRQ、MSP 引脚/remap 和长时间连续收发。

## 5. Phase 3：DMA（代码完成，待板测）

任务：

- Handle Instance 采用 `{DMA0/1, Channel0..6}` 的类型安全描述，不使用 STM32 Stream 地址。
- 实现 Init/DeInit/Start/Start_IT/Poll/Abort/IRQ/State/Error。
- 实现 half/full/error Callback 和 `__HAL_LINKDMA` 父子绑定。
- 建立 GD32 User Manual 表 10-3/10-4 的请求占用表与冲突检查。

质量门：normal/circular、M2M/P2M/M2P、8/16/32 位、half/full/error；同一 Channel 多请求冲突必须被拒绝。

已通过的代码质量门：ARM 目标对象严格编译、Clang Analyzer、Instance/Request 配对、normal/circular、M2M/P2M/M2P、8/16/32 位配置、half/full/error、poll timeout、Abort/Abort_IT、Callback 注册、`__HAL_LINKDMA`、计数/标志宏，以及同一 Channel 多 Handle 冲突。

待板级质量门：12 个 Channel 的寄存器配置、normal/circular 实际搬运、half/full/error IRQ、不同数据宽度和地址递增、外设请求启停协同、总线压力、Abort 边界和所有请求共享冲突。

## 6. Phase 4：UART DMA（代码完成，待板测）

实现：UART Handle 与 DMA Handle 双向绑定；固定 Channel/请求令牌、方向、宽度和 Parent 校验；TX/RX normal/circular；DMA 完成后 USART TC；Pause/Resume/Stop；UART/DMA error；同步硬件 Abort 与 Callback 状态一致性。

已通过的代码质量门：TX/RX normal/circular、half/full Callback、TX DMA 完成后 TC 收尾、DMA error、UART error、Pause/Resume/Stop、普通/暂停态 Abort、全双工错误释放、重复启动 HAL_BUSY、错误 Parent/通道/请求拒绝、ARM 严格编译和 Clang Analyzer。

待板级质量门：USART0/1/2、UART3 的 TX/RX normal/circular 实际搬运；half/full/error IRQ；TC 时序；总线压力；Pause/Resume/Stop/Abort；错误注入；所有固定 Channel 请求映射。

## 7. Phase 5：TIMER（代码完成，待板测）

已实现：

- Instance、Channel、clock、counter width capability 表。
- Base/PWM/OC/IC/OnePulse、Update/CC/Trigger IRQ 与 weak Callback。
- 常用 `__HAL_TIM_*` 宏改为 Port inline，禁止 STM32 寄存器字段直写。
- Master/TRGO/Slave/ITIx 按“STM 源定时器 -> GD 目标 ITI”查询官方矩阵；保留/不等价连接失败。
- TIMER update/CC DMA request 与 DMA Channel/请求令牌/Parent/方向/32 位 buffer 宽度绑定。

已通过的代码质量门：Base、PWM、Update IRQ、OC、IC、OnePulse、TRGO、Slave、Trigger IRQ、PWM DMA、16 位越界拒绝、F401 ITR 保留项/OC 不等价项拒绝、ARM 严格编译、Clang Analyzer、弱符号检查，以及兼容层与官方 GD32 TIMER/RCU 源文件的 Cortex-M4 ELF 链接。

待板级质量门：全部实际使用 TIMER 的 APB 时钟核算；Base 周期/PWM 占空比/OC/IC 波形；Update/CC/Trigger IRQ；normal/circular DMA；主从启动、门控、复位和外部时钟级联；OnePulse；高级 TIMER 主输出；TIM2/TIM5 原 32 位用法的产品级重构。示波器/逻辑分析仪结果完成前，不关闭 Phase 5 板测项。

## 8. Phase 6：ADC（代码完成，待板测）

已实现：ADC1→ADC0 不透明 Instance；Init/DeInit/ConfigChannel；polling/IT/DMA；每次掉电重启后的 14 CK_ADC 等待与前台校准；12/10/8/6 位、对齐、连续/扫描/不连续序列；保守采样时间转换；Timer/EXTI 触发查表与 ADC0 remap；状态/ErrorCode；weak full/half/error Callback；ADC 时钟范围检查。

已通过的代码质量门：单次/连续、scan、外部 Timer trigger、poll timeout、IT、DMA normal/circular、half/full/error、错误 Parent/Channel/request/方向/宽度/模式、不可映射 trigger/edge/sample/channel、ARM 严格编译、Clang Analyzer、weak symbol 检查，以及兼容层与官方 GD32 ADC/DMA/GPIO/RCU 源文件的 Cortex-M4 ELF 链接。

待板级质量门：3.3 V 与实际 PCLK2 下的 CK_ADC；单次/连续/扫描采样；Timer/EXTI 触发波形；DMA normal/circular；校准时序；温度/Vref 绝对采样时间；输入过量程饱和值；GD32 无 OVR 标志情况下的系统级数据丢失监控。示波器/逻辑分析仪和已知电压源结果完成前，不关闭 Phase 6 板测项。

## 9. Phase 7：I2C / SPI（代码完成，待板测）

I2C 已实现：I2C1/2→I2C0/1；≤400 kHz Init/DeInit；7/10 位 master polling/IT；1/2/N 字节 ACK/POS/STOP；7 位 Mem Read/Write 与 repeated START；IsDeviceReady；EV/ER IRQ；weak callback；timeout/BERR/ARLO 后软件复位重配。I2C DMA、Slave/Listen/Sequential 暂不支持。

SPI 已实现：SPI1/2/3→SPI0/1/2；8/16 位 Init/DeInit；polling/IT/DMA TX/RX/TxRx；主机 full-duplex dummy-clock RX；1-line 方向切换；normal/circular half/full/error callback；固定 DMA Channel/request/Parent/方向/宽度校验；TBE/TRANS 收尾与 OVR/MODF/FRE 错误处理。CRC enable 与 master 2-lines RX-only 暂不支持。

已通过的代码质量门：ARM Cortex-M4 `-Wall -Wextra -Werror`；Clang Analyzer；I2C/SPI weak callback 符号；兼容层与官方 GD32 I2C/SPI/DMA/RCU 源文件目标 ELF 链接；I2C polling/Mem/repeated START/IT/错误/ready/timeout recovery Host Mock；SPI 8/16 位 polling/IT/DMA/half/full/error/错误配置 Host Mock；全部旧模块回归。

待板级质量门：I2C 协议分析仪验证 100/400 kHz、7/10 位、1/2/N 字节、repeated START、NACK/ARLO/BERR/timeout 和外部拉低恢复；SPI 逻辑分析仪验证三实例、四种 CPOL/CPHA、8/16 位、主从、1-line、IT/DMA normal/circular、最高 SCK、OVR/MODF/FRE 与 NSS 时序。板测完成前不关闭 Phase 7 板测项。

## 10. Phase 8：RCC / EXTI / FLASH（代码完成，待板测）

RCC 已实现：常用门控 enable/disable；HSI/HSE 开关/旁路；当前或已由原生代码配置的时钟源与 AHB/APB 分频切换；频率/配置查询；Flash wait-state 顺序；SystemCoreClock/Tick 更新；CSS/NMI Callback。STM32 PLLM/N/P/Q 不可等价转换，PLL ON、LSE/LSI 明确失败，产品继续使用 GD32 原生 SystemClock 配置。

EXTI 已实现：独立 Handle、Set/Get/ClearConfig、interrupt/event、rising/falling/both、pending、SWI、IRQ 和 Callback；仅开放目标实际存在的 EXTI0..18，并执行 GPIO 封装与 PD0/PD1 晶振保护。

FLASH 已实现：Unlock/Lock、half-word/word polling/IT、错误与 weak Callback；STM32 sector 0..7 到 GD32 2 KB page 展开；GD32 原生页擦除扩展；默认拒绝并要求 2 KB 对齐的显式 writable region；向量表首个 page、执行页、越界和授权区外保护。mass erase、byte/double-word、option bytes 明确不支持。

已通过的代码质量门：ARM Cortex-M4 `-Wall -Wextra -Werror`；Clang Analyzer；RCC/FLASH weak Callback 符号；不可用 EXTI line 负向编译；兼容层与官方 GD32 RCU/EXTI/FMC/GPIO/DMA/CMSIS system 的目标 ELF 链接；RCC、EXTI、FLASH Host Mock；全部旧模块回归。

待板级质量门：测量 HSI/HSE/既有 PLL、SYSCLK/HCLK/PCLK、Flash wait 和 Tick；验证 EXTI GPIO/内部线、共享 IRQ、事件和双边沿；在链接脚本保留的专用数据区验证 Flash polling/IT、sector-to-page、错误路径、擦写寿命、复位/掉电恢复。完成前不关闭 Phase 8 板测项。

## 11. Phase 9：产品集成

- 重写 GD32 的 startup、linker、system、MSP、IRQ 和所有 `MX_xxx_Init()`。
- 建立应用源码兼容编译清单。
- 板级回归：电源、时钟、GPIO/EXTI、UART、DMA、TIMER、ADC、I2C、SPI、FLASH。
- 输出未支持项冻结清单与版本标签。

## 12. 每模块检查表

| 检查项 | 通过条件 |
|---|---|
| API | 声明、const、参数宽度和返回值与目标 STM32 HAL 用法兼容 |
| Handle | State/Lock/ErrorCode/Parent/DMA 指针生命周期明确 |
| Timeout | 使用无符号 Tick 差值，支持回绕和 HAL_MAX_DELAY |
| IRQ | 只处理已使能且已置位的来源，清标志顺序有依据 |
| Callback | 正常、半完成、错误、Abort 的次数和状态可验证 |
| DMA | 请求映射、方向、宽度、计数、normal/circular 正确 |
| 型号 | 仅使用 GD32F403RET6 实际存在的实例和封装引脚 |
| 编译 | ARM Clang/GCC 至少一种严格编译；发布前补 IAR/ARMClang |
| 板测 | 真实目标板和测量仪器通过；Host Mock 不能代替硬件验证 |
