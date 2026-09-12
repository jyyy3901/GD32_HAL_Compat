#include "gd32_hal_port.h"
#include <stddef.h>

static int GD32_HAL_RCC_MapAHBDivider(uint32_t divider, uint32_t *value)
{
    if (value == NULL)
    {
        return -1;
    }

    switch (divider)
    {
        case 1U: *value = RCU_AHB_CKSYS_DIV1; break;
        case 2U: *value = RCU_AHB_CKSYS_DIV2; break;
        case 4U: *value = RCU_AHB_CKSYS_DIV4; break;
        case 8U: *value = RCU_AHB_CKSYS_DIV8; break;
        case 16U: *value = RCU_AHB_CKSYS_DIV16; break;
        case 64U: *value = RCU_AHB_CKSYS_DIV64; break;
        case 128U: *value = RCU_AHB_CKSYS_DIV128; break;
        case 256U: *value = RCU_AHB_CKSYS_DIV256; break;
        case 512U: *value = RCU_AHB_CKSYS_DIV512; break;
        default: return -1;
    }

    return 0;
}

static int GD32_HAL_RCC_MapAPBDivider(uint32_t divider,
                                      int apb2,
                                      uint32_t *value)
{
    if (value == NULL)
    {
        return -1;
    }

    switch (divider)
    {
        case 1U:
            *value = (apb2 != 0) ? RCU_APB2_CKAHB_DIV1 : RCU_APB1_CKAHB_DIV1;
            break;
        case 2U:
            *value = (apb2 != 0) ? RCU_APB2_CKAHB_DIV2 : RCU_APB1_CKAHB_DIV2;
            break;
        case 4U:
            *value = (apb2 != 0) ? RCU_APB2_CKAHB_DIV4 : RCU_APB1_CKAHB_DIV4;
            break;
        case 8U:
            *value = (apb2 != 0) ? RCU_APB2_CKAHB_DIV8 : RCU_APB1_CKAHB_DIV8;
            break;
        case 16U:
            *value = (apb2 != 0) ? RCU_APB2_CKAHB_DIV16 : RCU_APB1_CKAHB_DIV16;
            break;
        default:
            return -1;
    }

    return 0;
}

static uint32_t GD32_HAL_RCC_DecodeAHBDivider(uint32_t bits)
{
    static const uint16_t dividers[16] = {
        1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U,
        2U, 4U, 8U, 16U, 64U, 128U, 256U, 512U
    };
    return dividers[(bits >> 4U) & 0x0FU];
}

static uint32_t GD32_HAL_RCC_DecodeAPBDivider(uint32_t bits)
{
    static const uint8_t dividers[8] = {1U, 1U, 1U, 1U, 2U, 4U, 8U, 16U};
    return dividers[bits & 0x07U];
}

int GD32_HAL_RCC_SetInstanceClock(uintptr_t stm32_instance,
                                  GD32_HAL_ResourceKind kind,
                                  int enable)
{
    const GD32_HAL_Resource *resource =
        GD32_HAL_ResolveInstance(stm32_instance, kind);

    if (resource == NULL)
    {
        GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_INVALID_INSTANCE,
                           (uint32_t)stm32_instance);
        return -1;
    }
    if (enable != 0)
    {
        rcu_periph_clock_enable((rcu_periph_enum)resource->gd32_clock);
    }
    else
    {
        if (kind == GD32_HAL_RESOURCE_ADC)
        {
            GD32_HAL_ADC_InvalidateCalibration(resource->gd32_instance);
        }
        rcu_periph_clock_disable((rcu_periph_enum)resource->gd32_clock);
    }
    return 0;
}

int GD32_HAL_RCC_SetCommonClock(GD32_HAL_RCCCommonClock clock, int enable)
{
    rcu_periph_enum periph;

    switch (clock)
    {
        case GD32_HAL_RCC_COMMON_AF: periph = RCU_AF; break;
        case GD32_HAL_RCC_COMMON_PMU: periph = RCU_PMU; break;
        case GD32_HAL_RCC_COMMON_CRC: periph = RCU_CRC; break;
        default:
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_RCC_INVALID_CONFIG,
                               (uint32_t)clock);
            return -1;
    }
    if (enable != 0)
    {
        rcu_periph_clock_enable(periph);
    }
    else
    {
        rcu_periph_clock_disable(periph);
    }
    return 0;
}

void GD32_HAL_RCC_DeInit(void)
{
    GD32_HAL_ADC_InvalidateCalibration(GD32_HAL_ADC0_ADDRESS);
    rcu_deinit();
}

int GD32_HAL_RCC_SetIRC8M(int enable, uint32_t calibration)
{
    if (calibration > 0x1FU)
    {
        return -1;
    }

    if (enable != 0)
    {
        rcu_irc8m_adjust_value_set(calibration);
        rcu_osci_on(RCU_IRC8M);
        return (rcu_osci_stab_wait(RCU_IRC8M) == SUCCESS) ? 0 : -1;
    }

    /* 当前系统时钟直接或间接依赖 IRC8M 时，拒绝关闭。 */
    if ((GD32_HAL_RCC_GetSystemSource() == GD32_HAL_RCC_SOURCE_IRC8M) ||
        (GD32_HAL_RCC_GetSystemSource() == GD32_HAL_RCC_SOURCE_PLL))
    {
        return -2;
    }

    rcu_osci_off(RCU_IRC8M);
    return 0;
}

int GD32_HAL_RCC_SetHXTAL(uint32_t state)
{
    const uint32_t source = GD32_HAL_RCC_GetSystemSource();

    if ((state != GD32_HAL_RCC_HXTAL_OFF) &&
        (state != GD32_HAL_RCC_HXTAL_ON) &&
        (state != GD32_HAL_RCC_HXTAL_BYPASS))
    {
        return -1;
    }

    if ((state == GD32_HAL_RCC_HXTAL_OFF) &&
        ((source == GD32_HAL_RCC_SOURCE_HXTAL) ||
         (source == GD32_HAL_RCC_SOURCE_PLL)))
    {
        return -2;
    }

    if (state == GD32_HAL_RCC_HXTAL_OFF)
    {
        rcu_osci_off(RCU_HXTAL);
        rcu_osci_bypass_mode_disable(RCU_HXTAL);
        return 0;
    }

    if ((source == GD32_HAL_RCC_SOURCE_HXTAL) ||
        (source == GD32_HAL_RCC_SOURCE_PLL))
    {
        const uint32_t current_bypass = RCU_CTL & RCU_CTL_HXTALBPS;
        const uint32_t requested_bypass =
            (state == GD32_HAL_RCC_HXTAL_BYPASS) ? RCU_CTL_HXTALBPS : 0U;
        if (current_bypass != requested_bypass)
        {
            return -2;
        }
    }
    else
    {
        rcu_osci_off(RCU_HXTAL);
        if (state == GD32_HAL_RCC_HXTAL_BYPASS)
        {
            rcu_osci_bypass_mode_enable(RCU_HXTAL);
        }
        else
        {
            rcu_osci_bypass_mode_disable(RCU_HXTAL);
        }
    }

    rcu_osci_on(RCU_HXTAL);
    return (rcu_osci_stab_wait(RCU_HXTAL) == SUCCESS) ? 0 : -1;
}

int GD32_HAL_RCC_SetPLL(int enable)
{
    if (enable != 0)
    {
        rcu_osci_on(RCU_PLL_CK);
        return (rcu_osci_stab_wait(RCU_PLL_CK) == SUCCESS) ? 0 : -1;
    }

    if (GD32_HAL_RCC_GetSystemSource() == GD32_HAL_RCC_SOURCE_PLL)
    {
        return -2;
    }

    rcu_osci_off(RCU_PLL_CK);
    return 0;
}

uint32_t GD32_HAL_RCC_GetOscillatorState(void)
{
    uint32_t state = 0U;

    if ((RCU_CTL & RCU_CTL_IRC8MEN) != 0U)
    {
        state |= GD32_HAL_RCC_OSC_IRC8M_ON;
    }
    if ((RCU_CTL & RCU_CTL_HXTALEN) != 0U)
    {
        state |= GD32_HAL_RCC_OSC_HXTAL_ON;
    }
    if ((RCU_CTL & RCU_CTL_HXTALBPS) != 0U)
    {
        state |= GD32_HAL_RCC_OSC_HXTAL_BYPASS;
    }
    if ((RCU_CTL & RCU_CTL_PLLEN) != 0U)
    {
        state |= GD32_HAL_RCC_OSC_PLL_ON;
    }
    return state;
}

uint32_t GD32_HAL_RCC_GetIRC8MCalibration(void)
{
    return (RCU_CTL & RCU_CTL_IRC8MADJ) >> 3U;
}

uint32_t GD32_HAL_RCC_GetSystemSource(void)
{
    switch (rcu_system_clock_source_get())
    {
        case RCU_SCSS_HXTAL:
            return GD32_HAL_RCC_SOURCE_HXTAL;
        case RCU_SCSS_PLL:
            return GD32_HAL_RCC_SOURCE_PLL;
        case RCU_SCSS_IRC8M:
        default:
            return GD32_HAL_RCC_SOURCE_IRC8M;
    }
}

uint32_t GD32_HAL_RCC_GetSourceFrequency(uint32_t source)
{
    if (source == GD32_HAL_RCC_SOURCE_IRC8M)
    {
        return IRC8M_VALUE;
    }
    if (source == GD32_HAL_RCC_SOURCE_HXTAL)
    {
        return HXTAL_VALUE;
    }
    if ((source == GD32_HAL_RCC_SOURCE_PLL) &&
        (GD32_HAL_RCC_GetSystemSource() == GD32_HAL_RCC_SOURCE_PLL))
    {
        return rcu_clock_freq_get(CK_SYS);
    }
    return 0U;
}

uint32_t GD32_HAL_RCC_GetClockFrequency(GD32_HAL_RCCClock clock)
{
    switch (clock)
    {
        case GD32_HAL_RCC_CLOCK_SYS: return rcu_clock_freq_get(CK_SYS);
        case GD32_HAL_RCC_CLOCK_AHB: return rcu_clock_freq_get(CK_AHB);
        case GD32_HAL_RCC_CLOCK_APB1: return rcu_clock_freq_get(CK_APB1);
        case GD32_HAL_RCC_CLOCK_APB2: return rcu_clock_freq_get(CK_APB2);
        default: return 0U;
    }
}

uint32_t GD32_HAL_RCC_GetAHBDivider(void)
{
    return GD32_HAL_RCC_DecodeAHBDivider(RCU_CFG0 & RCU_CFG0_AHBPSC);
}

uint32_t GD32_HAL_RCC_GetAPB1Divider(void)
{
    return GD32_HAL_RCC_DecodeAPBDivider((RCU_CFG0 & RCU_CFG0_APB1PSC) >> 8U);
}

uint32_t GD32_HAL_RCC_GetAPB2Divider(void)
{
    return GD32_HAL_RCC_DecodeAPBDivider((RCU_CFG0 & RCU_CFG0_APB2PSC) >> 11U);
}

int GD32_HAL_RCC_ApplyClockConfig(uint32_t source,
                                  uint32_t ahb_divider,
                                  uint32_t apb1_divider,
                                  uint32_t apb2_divider,
                                  int increasing)
{
    uint32_t ahb_value;
    uint32_t apb1_value;
    uint32_t apb2_value;
    uint32_t source_value;
    uint32_t expected_status;
    uint32_t timeout;

    if ((GD32_HAL_RCC_MapAHBDivider(ahb_divider, &ahb_value) != 0) ||
        (GD32_HAL_RCC_MapAPBDivider(apb1_divider, 0, &apb1_value) != 0) ||
        (GD32_HAL_RCC_MapAPBDivider(apb2_divider, 1, &apb2_value) != 0))
    {
        return -1;
    }

    switch (source)
    {
        case GD32_HAL_RCC_SOURCE_IRC8M:
            if (rcu_flag_get(RCU_FLAG_IRC8MSTB) == RESET) return -1;
            source_value = RCU_CKSYSSRC_IRC8M;
            expected_status = RCU_SCSS_IRC8M;
            break;
        case GD32_HAL_RCC_SOURCE_HXTAL:
            if (rcu_flag_get(RCU_FLAG_HXTALSTB) == RESET) return -1;
            source_value = RCU_CKSYSSRC_HXTAL;
            expected_status = RCU_SCSS_HXTAL;
            break;
        case GD32_HAL_RCC_SOURCE_PLL:
            if (rcu_flag_get(RCU_FLAG_PLLSTB) == RESET) return -1;
            source_value = RCU_CKSYSSRC_PLL;
            expected_status = RCU_SCSS_PLL;
            break;
        default:
            return -1;
    }

    /* 升频前先降低 APB 负载；降频后再放宽 APB 分频。 */
    if (increasing != 0)
    {
        rcu_apb1_clock_config(apb1_value);
        rcu_apb2_clock_config(apb2_value);
        rcu_ahb_clock_config(ahb_value);
        rcu_system_clock_source_config(source_value);
    }
    else
    {
        rcu_system_clock_source_config(source_value);
        rcu_ahb_clock_config(ahb_value);
        rcu_apb1_clock_config(apb1_value);
        rcu_apb2_clock_config(apb2_value);
    }

    timeout = GD32_HAL_RCC_SWITCH_TIMEOUT_CYCLES;
    while ((rcu_system_clock_source_get() != expected_status) && (timeout > 0U))
    {
        --timeout;
    }
    return (timeout > 0U) ? 0 : -1;
}

void GD32_HAL_RCC_SetFlashWaitState(uint32_t wait_state)
{
    const uint32_t was_locked = FMC_CTL0 & FMC_CTL0_LK;

    fmc_unlock();
    fmc_wscnt_set(wait_state);
    FMC_WSEN |= FMC_WSEN_WSEN;
    if (was_locked != 0U)
    {
        fmc_lock();
    }
}

uint32_t GD32_HAL_RCC_GetFlashWaitState(void)
{
    return FMC_WS & FMC_WS_WSCNT;
}

void GD32_HAL_RCC_EnableClockMonitor(int enable)
{
    if (enable != 0)
    {
        rcu_hxtal_clock_monitor_enable();
    }
    else
    {
        rcu_hxtal_clock_monitor_disable();
    }
}

uint32_t GD32_HAL_RCC_GetClockMonitorInterrupt(void)
{
    return (rcu_interrupt_flag_get(RCU_INT_FLAG_CKM) != RESET) ? 1U : 0U;
}

void GD32_HAL_RCC_ClearClockMonitorInterrupt(void)
{
    rcu_interrupt_flag_clear(RCU_INT_FLAG_CKM_CLR);
}
