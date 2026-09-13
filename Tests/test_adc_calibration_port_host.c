#include "gd32_hal_port.h"
#include "gd32f403_adc.h"
#include <assert.h>
#include <stdio.h>

uint32_t SystemCoreClock = 100U;
volatile uint32_t mock_adc_ctl0;
volatile uint32_t mock_adc_ctl1;
volatile uint32_t mock_rcu_cfg0;

static int mock_auto_complete;
static uint32_t mock_enable_count;
static uint32_t mock_disable_count;
static uint32_t mock_reset_calibration_count;
static uint32_t mock_calibration_count;
static GD32_HAL_PortError mock_error;

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    (void)detail;
    mock_error = error;
}

void mock_adc_nop(void)
{
    if (mock_auto_complete != 0)
    {
        if ((mock_adc_ctl1 & ADC_CTL1_RSTCLB) != 0U)
        {
            ++mock_reset_calibration_count;
            mock_adc_ctl1 &= ~ADC_CTL1_RSTCLB;
        }
        if ((mock_adc_ctl1 & ADC_CTL1_CLB) != 0U)
        {
            ++mock_calibration_count;
            mock_adc_ctl1 &= ~ADC_CTL1_CLB;
        }
    }
}

uint32_t rcu_clock_freq_get(uint32_t clock)
{
    (void)clock;
    return 50U;
}

void adc_enable(uint32_t adc)
{
    (void)adc;
    ++mock_enable_count;
    mock_adc_ctl1 |= ADC_CTL1_ADCON;
}

void adc_disable(uint32_t adc)
{
    (void)adc;
    ++mock_disable_count;
    mock_adc_ctl1 &= ~ADC_CTL1_ADCON;
}

#define MOCK_VOID1(name, t1) void name(t1 a) { (void)a; }
#define MOCK_VOID2(name, t1, t2) void name(t1 a, t2 b) { (void)a; (void)b; }
#define MOCK_VOID3(name, t1, t2, t3) \
    void name(t1 a, t2 b, t3 c) { (void)a; (void)b; (void)c; }
#define MOCK_VOID4(name, t1, t2, t3, t4) \
    void name(t1 a, t2 b, t3 c, t4 d) \
    { (void)a; (void)b; (void)c; (void)d; }

MOCK_VOID1(adc_deinit, uint32_t)
MOCK_VOID2(adc_resolution_config, uint32_t, uint32_t)
MOCK_VOID2(adc_data_alignment_config, uint32_t, uint32_t)
MOCK_VOID3(adc_special_function_config, uint32_t, uint32_t, ControlStatus)
MOCK_VOID3(adc_channel_length_config, uint32_t, uint8_t, uint32_t)
MOCK_VOID3(adc_discontinuous_mode_config, uint32_t, uint8_t, uint8_t)
MOCK_VOID3(adc_external_trigger_source_config, uint32_t, uint8_t, uint32_t)
MOCK_VOID3(adc_external_trigger_config, uint32_t, uint8_t, ControlStatus)
MOCK_VOID2(adc_flag_clear, uint32_t, uint32_t)
MOCK_VOID1(adc_dma_mode_disable, uint32_t)
MOCK_VOID1(adc_dma_mode_enable, uint32_t)
void adc_tempsensor_vrefint_disable(void) {}
void adc_tempsensor_vrefint_enable(void) {}
MOCK_VOID4(adc_routine_channel_config, uint32_t, uint8_t, uint8_t, uint32_t)
MOCK_VOID2(adc_software_trigger_enable, uint32_t, uint8_t)
MOCK_VOID2(adc_interrupt_enable, uint32_t, uint32_t)
MOCK_VOID2(adc_interrupt_disable, uint32_t, uint32_t)
MOCK_VOID1(rcu_adc_clock_config, uint32_t)
MOCK_VOID1(rcu_periph_clock_enable, uint32_t)
MOCK_VOID2(gpio_pin_remap_config, uint32_t, ControlStatus)

FlagStatus adc_flag_get(uint32_t adc, uint32_t flag)
{
    (void)adc;
    (void)flag;
    return RESET;
}

uint16_t adc_routine_data_read(uint32_t adc)
{
    (void)adc;
    return 0U;
}

static void reset_mocks(void)
{
    mock_adc_ctl0 = 0U;
    mock_adc_ctl1 = 0U;
    mock_rcu_cfg0 = 0U;
    mock_auto_complete = 1;
    mock_enable_count = 0U;
    mock_disable_count = 0U;
    mock_reset_calibration_count = 0U;
    mock_calibration_count = 0U;
    mock_error = GD32_HAL_PORT_ERROR_NONE;
    GD32_HAL_ADC_InvalidateCalibration(GD32_HAL_ADC0_ADDRESS);
}

int main(void)
{
    reset_mocks();
    assert(GD32_HAL_ADC_EnableAndCalibrate(GD32_HAL_ADC0_ADDRESS) == 0);
    assert(mock_enable_count == 1U);
    assert(mock_reset_calibration_count == 1U);
    assert(mock_calibration_count == 1U);
    assert(GD32_HAL_ADC_IsCalibrationValid(GD32_HAL_ADC0_ADDRESS) != 0);

    assert(GD32_HAL_ADC_EnableAndCalibrate(GD32_HAL_ADC0_ADDRESS) == 0);
    assert(mock_reset_calibration_count == 1U);
    assert(mock_calibration_count == 1U);

    mock_adc_ctl1 &= ~ADC_CTL1_ADCON;
    assert(GD32_HAL_ADC_EnableAndCalibrate(GD32_HAL_ADC0_ADDRESS) == 0);
    assert(mock_enable_count == 2U);
    assert(mock_reset_calibration_count == 2U);
    assert(mock_calibration_count == 2U);

    mock_adc_ctl1 &= ~ADC_CTL1_ADCON;
    mock_auto_complete = 0;
    assert(GD32_HAL_ADC_EnableAndCalibrate(GD32_HAL_ADC0_ADDRESS) != 0);
    assert(GD32_HAL_ADC_IsCalibrationValid(GD32_HAL_ADC0_ADDRESS) == 0);
    assert(GD32_HAL_ADC_IsEnabled(GD32_HAL_ADC0_ADDRESS) == 0);
    assert(mock_disable_count == 1U);
    assert(mock_error == GD32_HAL_PORT_ERROR_ADC_CALIBRATION_TIMEOUT);

    puts("ADC calibration Port host tests: PASS");
    return 0;
}
