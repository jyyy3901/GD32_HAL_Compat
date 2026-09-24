#include "stm32f4xx_hal.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

uint32_t SystemCoreClock = 8000000U;
uint32_t uwTickPrio = 15U;
static uint32_t frequencies[4] = {8000000U, 8000000U, 4000000U, 8000000U};
static uint32_t source_frequencies[3] = {8000000U, 25000000U, 168000000U};
static uint32_t oscillator_state = GD32_HAL_RCC_OSC_IRC8M_ON;
static uint32_t system_source = GD32_HAL_RCC_SOURCE_IRC8M;
static uint32_t ahb_divider = 1U;
static uint32_t apb1_divider = 2U;
static uint32_t apb2_divider = 1U;
static uint32_t flash_wait;
static uint32_t init_tick_calls;
static uint32_t css_callback_calls;
static uint32_t monitor_pending;
static uint32_t last_error;
static GD32_HAL_RCCResetFlag last_reset_flag;
static uint32_t reset_flag_query_result;
static uint32_t reset_flag_clear_calls;
static char order_log[8];
static size_t order_length;

HAL_StatusTypeDef HAL_InitTick(uint32_t priority)
{
    assert(priority == uwTickPrio);
    ++init_tick_calls;
    return HAL_OK;
}

void SystemCoreClockUpdate(void)
{
    SystemCoreClock = frequencies[GD32_HAL_RCC_CLOCK_AHB];
}

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    (void)detail;
    last_error = (uint32_t)error;
}

void GD32_HAL_RCC_DeInit(void) { system_source = GD32_HAL_RCC_SOURCE_IRC8M; }
int GD32_HAL_RCC_SetIRC8M(int enable, uint32_t calibration)
{
    assert(calibration <= 0x1FU);
    if (enable != 0) oscillator_state |= GD32_HAL_RCC_OSC_IRC8M_ON;
    else oscillator_state &= ~GD32_HAL_RCC_OSC_IRC8M_ON;
    return 0;
}
int GD32_HAL_RCC_SetHXTAL(uint32_t state)
{
    oscillator_state &= ~(GD32_HAL_RCC_OSC_HXTAL_ON |
                          GD32_HAL_RCC_OSC_HXTAL_BYPASS);
    if (state != GD32_HAL_RCC_HXTAL_OFF)
        oscillator_state |= GD32_HAL_RCC_OSC_HXTAL_ON;
    if (state == GD32_HAL_RCC_HXTAL_BYPASS)
        oscillator_state |= GD32_HAL_RCC_OSC_HXTAL_BYPASS;
    return 0;
}
int GD32_HAL_RCC_SetPLL(int enable)
{
    if (enable != 0) oscillator_state |= GD32_HAL_RCC_OSC_PLL_ON;
    else oscillator_state &= ~GD32_HAL_RCC_OSC_PLL_ON;
    return 0;
}
uint32_t GD32_HAL_RCC_GetOscillatorState(void) { return oscillator_state; }
uint32_t GD32_HAL_RCC_GetIRC8MCalibration(void) { return 0x12U; }
uint32_t GD32_HAL_RCC_GetSystemSource(void) { return system_source; }
uint32_t GD32_HAL_RCC_GetSourceFrequency(uint32_t source)
{
    return (source < 3U) ? source_frequencies[source] : 0U;
}
uint32_t GD32_HAL_RCC_GetClockFrequency(GD32_HAL_RCCClock clock)
{
    return frequencies[(uint32_t)clock];
}
uint32_t GD32_HAL_RCC_GetAHBDivider(void) { return ahb_divider; }
uint32_t GD32_HAL_RCC_GetAPB1Divider(void) { return apb1_divider; }
uint32_t GD32_HAL_RCC_GetAPB2Divider(void) { return apb2_divider; }
int GD32_HAL_RCC_ApplyClockConfig(uint32_t source,
                                  uint32_t ahb,
                                  uint32_t apb1,
                                  uint32_t apb2,
                                  int increasing)
{
    order_log[order_length++] = 'A';
    system_source = source;
    ahb_divider = ahb;
    apb1_divider = apb1;
    apb2_divider = apb2;
    frequencies[GD32_HAL_RCC_CLOCK_SYS] = source_frequencies[source];
    frequencies[GD32_HAL_RCC_CLOCK_AHB] = source_frequencies[source] / ahb;
    frequencies[GD32_HAL_RCC_CLOCK_APB1] = frequencies[1] / apb1;
    frequencies[GD32_HAL_RCC_CLOCK_APB2] = frequencies[1] / apb2;
    assert(increasing == 1);
    return 0;
}
void GD32_HAL_RCC_SetFlashWaitState(uint32_t wait_state)
{
    order_log[order_length++] = 'W';
    flash_wait = wait_state;
}
uint32_t GD32_HAL_RCC_GetFlashWaitState(void) { return flash_wait; }
void GD32_HAL_RCC_EnableClockMonitor(int enable) { (void)enable; }
uint32_t GD32_HAL_RCC_GetClockMonitorInterrupt(void) { return monitor_pending; }
void GD32_HAL_RCC_ClearClockMonitorInterrupt(void) { monitor_pending = 0U; }
uint32_t GD32_HAL_RCC_GetResetFlag(GD32_HAL_RCCResetFlag flag)
{
    last_reset_flag = flag;
    return reset_flag_query_result;
}
void GD32_HAL_RCC_ClearResetFlags(void) { ++reset_flag_clear_calls; }

void HAL_RCC_CSSCallback(void)
{
    ++css_callback_calls;
}

static void reset_fixture(void)
{
    frequencies[0] = 4000000U;
    frequencies[1] = 4000000U;
    frequencies[2] = 2000000U;
    frequencies[3] = 4000000U;
    source_frequencies[0] = 8000000U;
    source_frequencies[1] = 25000000U;
    source_frequencies[2] = 168000000U;
    system_source = GD32_HAL_RCC_SOURCE_IRC8M;
    ahb_divider = 1U;
    apb1_divider = 2U;
    apb2_divider = 1U;
    flash_wait = 0U;
    order_length = 0U;
    memset(order_log, 0, sizeof(order_log));
    last_error = 0U;
    last_reset_flag = GD32_HAL_RCC_RESET_FLAG_PIN;
    reset_flag_query_result = 0U;
    reset_flag_clear_calls = 0U;
}

int main(void)
{
    RCC_OscInitTypeDef osc;
    RCC_ClkInitTypeDef clock;
    RCC_ClkInitTypeDef readback;
    uint32_t latency;

    reset_fixture();
    memset(&osc, 0, sizeof(osc));
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI;
    osc.HSEState = RCC_HSE_BYPASS;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = 0x12U;
    osc.PLL.PLLState = RCC_PLL_NONE;
    assert(HAL_RCC_OscConfig(&osc) == HAL_OK);
    assert((oscillator_state & GD32_HAL_RCC_OSC_HXTAL_BYPASS) != 0U);

    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLN = 336U;
    assert(HAL_RCC_OscConfig(&osc) == HAL_ERROR);
    assert(last_error == GD32_HAL_PORT_ERROR_RCC_PLL_MODEL_UNSUPPORTED);

    memset(&clock, 0, sizeof(clock));
    clock.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                      RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clock.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clock.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clock.APB1CLKDivider = RCC_HCLK_DIV2;
    clock.APB2CLKDivider = RCC_HCLK_DIV1;
    assert(HAL_RCC_ClockConfig(&clock, FLASH_LATENCY_1) == HAL_OK);
    assert(order_log[0] == 'W' && order_log[1] == 'A');
    assert(HAL_RCC_GetHCLKFreq() == 8000000U);
    assert(HAL_RCC_GetPCLK1Freq() == 4000000U);
    assert(init_tick_calls != 0U);

    HAL_RCC_GetClockConfig(&readback, &latency);
    assert(readback.AHBCLKDivider == RCC_SYSCLK_DIV1);
    assert(readback.APB1CLKDivider == RCC_HCLK_DIV2);
    assert(latency == FLASH_LATENCY_1);

    source_frequencies[2] = 168000000U;
    clock.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clock.APB1CLKDivider = RCC_HCLK_DIV1;
    assert(HAL_RCC_ClockConfig(&clock, FLASH_LATENCY_2) == HAL_ERROR);
    assert(last_error == GD32_HAL_PORT_ERROR_RCC_CLOCK_LIMIT);

    monitor_pending = 1U;
    HAL_RCC_NMI_IRQHandler();
    assert(monitor_pending == 0U);
    assert(css_callback_calls == 1U);

    reset_flag_query_result = 1U;
    assert(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != 0U);
    assert(last_reset_flag ==
           GD32_HAL_RCC_RESET_FLAG_INDEPENDENT_WATCHDOG);
    reset_flag_query_result = 0U;
    assert(__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) == 0U);
    assert(last_reset_flag == GD32_HAL_RCC_RESET_FLAG_PIN);
    assert(RCC_FLAG_PINRST != RCC_FLAG_PORRST);
    assert(RCC_FLAG_PORRST != RCC_FLAG_SFTRST);
    assert(RCC_FLAG_IWDGRST != RCC_FLAG_WWDGRST);
    assert(RCC_FLAG_WWDGRST != RCC_FLAG_LPWRRST);
    __HAL_RCC_CLEAR_RESET_FLAGS();
    assert(reset_flag_clear_calls == 1U);

    puts("RCC host tests: PASS");
    return 0;
}
