#ifndef GD32F403_H
#define GD32F403_H

#include <stdint.h>

#define __IO volatile
#define __weak
#define __NVIC_PRIO_BITS 4U
#define __MPU_PRESENT 1U

typedef enum
{
    SysTick_IRQn = -1,
    TEST_IRQn = 0,
    ADC0_1_IRQn = 18,
    TIMER0_BRK_TIMER8_IRQn = 24,
    TIMER0_UP_TIMER9_IRQn = 25,
    TIMER0_TRG_CMT_TIMER10_IRQn = 26,
    TIMER0_Channel_IRQn = 27,
    TIMER1_IRQn = 28,
    TIMER2_IRQn = 29,
    TIMER3_IRQn = 30,
    I2C0_EV_IRQn = 31,
    I2C0_ER_IRQn = 32,
    I2C1_EV_IRQn = 33,
    I2C1_ER_IRQn = 34,
    SPI0_IRQn = 35,
    SPI1_IRQn = 36,
    USART0_IRQn = 37,
    USART1_IRQn = 38,
    USART2_IRQn = 39,
    TIMER7_BRK_TIMER11_IRQn = 43,
    TIMER7_UP_TIMER12_IRQn = 44,
    TIMER7_TRG_CMT_TIMER13_IRQn = 45,
    TIMER7_Channel_IRQn = 46,
    TIMER4_IRQn = 50,
    SPI2_IRQn = 51,
    UART3_IRQn = 52,
    UART4_IRQn = 53,
    TIMER5_IRQn = 54,
    TIMER6_IRQn = 55
} IRQn_Type;

typedef struct
{
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
} SysTick_Type;

typedef struct
{
    volatile uint32_t RESERVED[9];
    volatile uint32_t SHCSR;
} SCB_Type;

typedef struct
{
    volatile uint32_t TYPE;
    volatile uint32_t CTRL;
    volatile uint32_t RNR;
    volatile uint32_t RBAR;
    volatile uint32_t RASR;
} MPU_Type;

extern SysTick_Type mock_systick;
extern SCB_Type mock_scb;
extern MPU_Type mock_mpu;
extern uint32_t SystemCoreClock;
extern uint32_t mock_priority_group;
extern uint32_t mock_irq_priority[8];
extern uint32_t mock_irq_pending[8];
extern uint32_t mock_irq_active[8];
extern uint32_t mock_irq_enabled[8];

#define SysTick (&mock_systick)
#define SCB (&mock_scb)
#define MPU (&mock_mpu)

#define SysTick_CTRL_ENABLE_Msk (1UL << 0U)
#define SysTick_CTRL_TICKINT_Msk (1UL << 1U)
#define SysTick_CTRL_CLKSOURCE_Msk (1UL << 2U)
#define SCB_SHCSR_MEMFAULTENA_Msk (1UL << 16U)
#define MPU_CTRL_ENABLE_Msk (1UL << 0U)
#define MPU_CTRL_HFNMIENA_Msk (1UL << 1U)
#define MPU_CTRL_PRIVDEFENA_Msk (1UL << 2U)
#define MPU_RASR_ENABLE_Pos 0U
#define MPU_RASR_SIZE_Pos 1U
#define MPU_RASR_SRD_Pos 8U
#define MPU_RASR_B_Pos 16U
#define MPU_RASR_C_Pos 17U
#define MPU_RASR_S_Pos 18U
#define MPU_RASR_TEX_Pos 19U
#define MPU_RASR_AP_Pos 24U
#define MPU_RASR_XN_Pos 28U
#define MPU_RASR_ENABLE_Msk (1UL << MPU_RASR_ENABLE_Pos)

static inline void NVIC_SetPriorityGrouping(uint32_t group)
{
    mock_priority_group = group;
}

static inline uint32_t NVIC_GetPriorityGrouping(void)
{
    return mock_priority_group;
}

static inline uint32_t NVIC_EncodePriority(uint32_t group, uint32_t preempt, uint32_t sub)
{
    (void)group;
    return ((preempt & 0xFU) << 4U) | (sub & 0xFU);
}

static inline void NVIC_DecodePriority(uint32_t priority,
                                       uint32_t group,
                                       uint32_t *preempt,
                                       uint32_t *sub)
{
    (void)group;
    *preempt = (priority >> 4U) & 0xFU;
    *sub = priority & 0xFU;
}

static inline void NVIC_SetPriority(IRQn_Type irqn, uint32_t priority)
{
    mock_irq_priority[((int32_t)irqn) + 1] = priority;
}

static inline uint32_t NVIC_GetPriority(IRQn_Type irqn)
{
    return mock_irq_priority[((int32_t)irqn) + 1];
}

static inline void NVIC_EnableIRQ(IRQn_Type irqn)
{
    mock_irq_enabled[((int32_t)irqn) + 1] = 1U;
}

static inline void NVIC_DisableIRQ(IRQn_Type irqn)
{
    mock_irq_enabled[((int32_t)irqn) + 1] = 0U;
}

static inline void NVIC_SetPendingIRQ(IRQn_Type irqn)
{
    mock_irq_pending[((int32_t)irqn) + 1] = 1U;
}

static inline uint32_t NVIC_GetPendingIRQ(IRQn_Type irqn)
{
    return mock_irq_pending[((int32_t)irqn) + 1];
}

static inline void NVIC_ClearPendingIRQ(IRQn_Type irqn)
{
    mock_irq_pending[((int32_t)irqn) + 1] = 0U;
}

static inline uint32_t NVIC_GetActive(IRQn_Type irqn)
{
    return mock_irq_active[((int32_t)irqn) + 1];
}

static inline void NVIC_SystemReset(void)
{
}

static inline uint32_t SysTick_Config(uint32_t ticks)
{
    if ((ticks == 0U) || ((ticks - 1U) > 0xFFFFFFU))
    {
        return 1U;
    }
    SysTick->LOAD = ticks - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
    return 0U;
}

#define __DMB() ((void)0U)
#define __DSB() ((void)0U)
#define __ISB() ((void)0U)
#define __SEV() ((void)0U)
#define __WFE() ((void)0U)

extern uint32_t mock_primask;

static inline uint32_t __get_PRIMASK(void)
{
    return mock_primask;
}

static inline void __disable_irq(void)
{
    mock_primask = 1U;
}

static inline void __enable_irq(void)
{
    mock_primask = 0U;
}

void SystemCoreClockUpdate(void);

#define GPIO_BASE 0x40010800UL
#define GPIO_PIN_0 (1UL << 0U)
#define GPIO_PIN_1 (1UL << 1U)
#define GPIO_PIN_2 (1UL << 2U)
#define GPIO_PIN_3 (1UL << 3U)
#define GPIO_PIN_4 (1UL << 4U)
#define GPIO_PIN_5 (1UL << 5U)
#define GPIO_PIN_6 (1UL << 6U)
#define GPIO_PIN_7 (1UL << 7U)
#define GPIO_PIN_8 (1UL << 8U)
#define GPIO_PIN_9 (1UL << 9U)
#define GPIO_PIN_10 (1UL << 10U)
#define GPIO_PIN_11 (1UL << 11U)
#define GPIO_PIN_12 (1UL << 12U)
#define GPIO_PIN_13 (1UL << 13U)
#define GPIO_PIN_14 (1UL << 14U)
#define GPIO_PIN_15 (1UL << 15U)
#define GPIO_PIN_ALL 0xFFFFUL

#define REG32(addr) (*(volatile uint32_t *)(uintptr_t)(addr))

typedef enum
{
    DISABLE = 0,
    ENABLE = !DISABLE
} EventStatus, ControlStatus;

typedef enum
{
    RESET = 0,
    SET = 1
} FlagStatus;

#define DMA0 0x40020000UL
#define DMA1 0x40020400UL

typedef enum
{
    DMA_CH0 = 0,
    DMA_CH1,
    DMA_CH2,
    DMA_CH3,
    DMA_CH4,
    DMA_CH5,
    DMA_CH6
} dma_channel_enum;

typedef struct
{
    uint32_t periph_addr;
    uint32_t periph_width;
    uint32_t memory_addr;
    uint32_t memory_width;
    uint32_t number;
    uint32_t priority;
    uint8_t periph_inc;
    uint8_t memory_inc;
    uint8_t direction;
} dma_parameter_struct;

#define DMA_CHXCTL_CHEN  (1UL << 0U)
#define DMA_CHXCTL_FTFIE (1UL << 1U)
#define DMA_CHXCTL_HTFIE (1UL << 2U)
#define DMA_CHXCTL_ERRIE (1UL << 3U)

#define DMA_PERIPHERAL_WIDTH_8BIT  0x00000000U
#define DMA_PERIPHERAL_WIDTH_16BIT 0x00000100U
#define DMA_PERIPHERAL_WIDTH_32BIT 0x00000200U
#define DMA_MEMORY_WIDTH_8BIT      0x00000000U
#define DMA_MEMORY_WIDTH_16BIT     0x00000400U
#define DMA_MEMORY_WIDTH_32BIT     0x00000800U
#define DMA_PERIPH_INCREASE_DISABLE 0U
#define DMA_PERIPH_INCREASE_ENABLE  1U
#define DMA_MEMORY_INCREASE_DISABLE 0U
#define DMA_MEMORY_INCREASE_ENABLE  1U
#define DMA_PERIPHERAL_TO_MEMORY 0U
#define DMA_MEMORY_TO_PERIPHERAL 1U
#define DMA_PRIORITY_LOW       0x00000000U
#define DMA_PRIORITY_MEDIUM    0x00001000U
#define DMA_PRIORITY_HIGH      0x00002000U
#define DMA_PRIORITY_ULTRA_HIGH 0x00003000U
#define DMA_FLAG_FTF (1UL << 0U)
#define DMA_FLAG_HTF (1UL << 1U)
#define DMA_FLAG_ERR (1UL << 2U)
#define DMA_INT_FTF  (1UL << 0U)
#define DMA_INT_HTF  (1UL << 1U)
#define DMA_INT_ERR  (1UL << 2U)

volatile uint32_t *mock_dma_ctl_address(uint32_t controller,
                                         dma_channel_enum channel);
#define DMA_CHCTL(CONTROLLER, CHANNEL) \
    (*mock_dma_ctl_address((CONTROLLER), (CHANNEL)))

void dma_deinit(uint32_t dma_periph, dma_channel_enum channelx);
void dma_struct_para_init(dma_parameter_struct *init_struct);
void dma_init(uint32_t dma_periph,
              dma_channel_enum channelx,
              dma_parameter_struct *init_struct);
void dma_circulation_enable(uint32_t dma_periph, dma_channel_enum channelx);
void dma_circulation_disable(uint32_t dma_periph, dma_channel_enum channelx);
void dma_memory_to_memory_enable(uint32_t dma_periph, dma_channel_enum channelx);
void dma_memory_to_memory_disable(uint32_t dma_periph, dma_channel_enum channelx);
void dma_channel_enable(uint32_t dma_periph, dma_channel_enum channelx);
void dma_channel_disable(uint32_t dma_periph, dma_channel_enum channelx);
void dma_periph_address_config(uint32_t dma_periph,
                               dma_channel_enum channelx,
                               uint32_t address);
void dma_memory_address_config(uint32_t dma_periph,
                               dma_channel_enum channelx,
                               uint32_t address);
void dma_transfer_number_config(uint32_t dma_periph,
                                dma_channel_enum channelx,
                                uint32_t number);
uint32_t dma_transfer_number_get(uint32_t dma_periph,
                                 dma_channel_enum channelx);
FlagStatus dma_flag_get(uint32_t dma_periph,
                        dma_channel_enum channelx,
                        uint32_t flag);
void dma_flag_clear(uint32_t dma_periph,
                    dma_channel_enum channelx,
                    uint32_t flag);
void dma_interrupt_enable(uint32_t dma_periph,
                          dma_channel_enum channelx,
                          uint32_t source);
void dma_interrupt_disable(uint32_t dma_periph,
                           dma_channel_enum channelx,
                           uint32_t source);

#endif /* GD32F403_H */
