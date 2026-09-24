#include "stm32f4xx_hal.h"
#include "gd32f403_rcu.h"
#include <assert.h>
#include <stdio.h>

static rcu_flag_enum last_vendor_flag;
static FlagStatus vendor_flag_status;
static uint32_t clear_calls;
static GD32_HAL_PortError last_error;

FlagStatus rcu_flag_get(rcu_flag_enum flag)
{
    last_vendor_flag = flag;
    return vendor_flag_status;
}

void rcu_all_reset_flag_clear(void)
{
    ++clear_calls;
}

void GD32_HAL_ErrorHook(GD32_HAL_PortError error, uint32_t detail)
{
    (void)detail;
    last_error = error;
}

static void expect_mapping(GD32_HAL_RCCResetFlag semantic,
                           rcu_flag_enum expected)
{
    vendor_flag_status = SET;
    assert(GD32_HAL_RCC_GetResetFlag(semantic) == 1U);
    assert(last_vendor_flag == expected);
    vendor_flag_status = RESET;
    assert(GD32_HAL_RCC_GetResetFlag(semantic) == 0U);
    assert(last_vendor_flag == expected);
}

int main(void)
{
    expect_mapping(GD32_HAL_RCC_RESET_FLAG_PIN, RCU_FLAG_EPRST);
    expect_mapping(GD32_HAL_RCC_RESET_FLAG_POWER, RCU_FLAG_PORRST);
    expect_mapping(GD32_HAL_RCC_RESET_FLAG_SOFTWARE, RCU_FLAG_SWRST);
    expect_mapping(GD32_HAL_RCC_RESET_FLAG_INDEPENDENT_WATCHDOG,
                   RCU_FLAG_FWDGTRST);
    expect_mapping(GD32_HAL_RCC_RESET_FLAG_WINDOW_WATCHDOG,
                   RCU_FLAG_WWDGTRST);
    expect_mapping(GD32_HAL_RCC_RESET_FLAG_LOW_POWER, RCU_FLAG_LPRST);

    GD32_HAL_RCC_ClearResetFlags();
    assert(clear_calls == 1U);

    last_error = GD32_HAL_PORT_ERROR_NONE;
    assert(GD32_HAL_RCC_GetResetFlag((GD32_HAL_RCCResetFlag)99) == 0U);
    assert(last_error == GD32_HAL_PORT_ERROR_RCC_INVALID_CONFIG);

    puts("RCC reset Port host tests: PASS");
    return 0;
}
