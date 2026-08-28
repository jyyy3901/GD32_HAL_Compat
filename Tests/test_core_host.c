#include "stm32f4xx_hal.h"
#include <assert.h>
#include <stdio.h>

SysTick_Type mock_systick;
SCB_Type mock_scb;
MPU_Type mock_mpu;
uint32_t SystemCoreClock = 168000000U;
uint32_t mock_priority_group;
uint32_t mock_irq_priority[8];
uint32_t mock_irq_pending[8];
uint32_t mock_irq_active[8];
uint32_t mock_irq_enabled[8];

static uint32_t mock_port_init_count;
static uint32_t mock_port_deinit_count;
static const uint32_t mock_uid[3] = {0x11223344U, 0x55667788U, 0x99AABBCCU};

void SystemCoreClockUpdate(void)
{
}

int GD32_HAL_PortInit(void)
{
    ++mock_port_init_count;
    return 0;
}

int GD32_HAL_PortDeInit(void)
{
    ++mock_port_deinit_count;
    return 0;
}

uint32_t GD32_HAL_ReadUIDWord(uint8_t index)
{
    return (index < 3U) ? mock_uid[index] : 0U;
}

static void test_hal_and_tick(void)
{
    GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE, 0x1234U);
    assert(gd32HalLastPortError == GD32_HAL_PORT_ERROR_INVALID_INSTANCE);
    assert(gd32HalLastPortErrorDetail == 0x1234U);

    assert(HAL_Init() == HAL_OK);
    assert(mock_port_init_count == 1U);
    assert(HAL_NVIC_GetPriorityGrouping() == NVIC_PRIORITYGROUP_4);
    assert(SysTick->LOAD == 167999U);
    assert(HAL_GetTickFreq() == HAL_TICK_FREQ_1KHZ);
    assert(HAL_GetUIDw0() == mock_uid[0]);
    assert(HAL_GetUIDw1() == mock_uid[1]);
    assert(HAL_GetUIDw2() == mock_uid[2]);

    GD32_HAL_SysTickIRQHandler();
    assert(HAL_GetTick() == 1U);

    assert(HAL_SetTickFreq(HAL_TICK_FREQ_100HZ) == HAL_OK);
    assert(SysTick->LOAD == 1679999U);
    GD32_HAL_SysTickIRQHandler();
    assert(HAL_GetTick() == 11U);

    assert(HAL_SetTickFreq(HAL_TICK_FREQ_10HZ) == HAL_OK);
    assert(SysTick->LOAD == 2099999U);
    assert((SysTick->CTRL & SysTick_CTRL_CLKSOURCE_Msk) == 0U);
    GD32_HAL_SysTickIRQHandler();
    assert(HAL_GetTick() == 111U);

    assert(HAL_SetTickFreq((HAL_TickFreqTypeDef)2U) == HAL_ERROR);

    HAL_SuspendTick();
    assert((SysTick->CTRL & SysTick_CTRL_TICKINT_Msk) == 0U);
    HAL_ResumeTick();
    assert((SysTick->CTRL & SysTick_CTRL_TICKINT_Msk) != 0U);

    assert(HAL_DeInit() == HAL_OK);
    assert(mock_port_deinit_count == 1U);
    assert(SysTick->CTRL == 0U);
    assert(SysTick->LOAD == 0U);
}

static void test_nvic_and_mpu(void)
{
    uint32_t preempt;
    uint32_t sub;
    MPU_Region_InitTypeDef region = {0};

    HAL_NVIC_SetPriority(TEST_IRQn, 2U, 1U);
    HAL_NVIC_GetPriority(TEST_IRQn, NVIC_PRIORITYGROUP_3, &preempt, &sub);
    assert(preempt == 2U);
    assert(sub == 1U);

    HAL_NVIC_EnableIRQ(TEST_IRQn);
    assert(mock_irq_enabled[1] == 1U);
    HAL_NVIC_SetPendingIRQ(TEST_IRQn);
    assert(HAL_NVIC_GetPendingIRQ(TEST_IRQn) == 1U);
    HAL_NVIC_ClearPendingIRQ(TEST_IRQn);
    assert(HAL_NVIC_GetPendingIRQ(TEST_IRQn) == 0U);
    HAL_NVIC_DisableIRQ(TEST_IRQn);
    assert(mock_irq_enabled[1] == 0U);

    region.Enable = MPU_REGION_ENABLE;
    region.Number = MPU_REGION_NUMBER2;
    region.BaseAddress = 0x20000000U;
    region.Size = MPU_REGION_SIZE_1KB;
    region.AccessPermission = MPU_REGION_FULL_ACCESS;
    region.IsCacheable = MPU_ACCESS_CACHEABLE;
    region.IsBufferable = MPU_ACCESS_BUFFERABLE;
    HAL_MPU_ConfigRegion(&region);
    assert(MPU->RNR == MPU_REGION_NUMBER2);
    assert(MPU->RBAR == 0x20000000U);
    assert((MPU->RASR & MPU_RASR_ENABLE_Msk) != 0U);

    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
    assert((MPU->CTRL & MPU_CTRL_ENABLE_Msk) != 0U);
    assert((SCB->SHCSR & SCB_SHCSR_MEMFAULTENA_Msk) != 0U);
    HAL_MPU_Disable();
    assert(MPU->CTRL == 0U);
}

int main(void)
{
    test_hal_and_tick();
    test_nvic_and_mpu();
    puts("Core, SysTick, NVIC and MPU host tests: PASS");
    return 0;
}
