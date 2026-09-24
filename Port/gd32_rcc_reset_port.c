#include "gd32_hal_port.h"
#include "gd32f403_rcu.h"

uint32_t GD32_HAL_RCC_GetResetFlag(GD32_HAL_RCCResetFlag flag)
{
    rcu_flag_enum target_flag;

    switch (flag)
    {
        case GD32_HAL_RCC_RESET_FLAG_PIN:
            target_flag = RCU_FLAG_EPRST;
            break;
        case GD32_HAL_RCC_RESET_FLAG_POWER:
            target_flag = RCU_FLAG_PORRST;
            break;
        case GD32_HAL_RCC_RESET_FLAG_SOFTWARE:
            target_flag = RCU_FLAG_SWRST;
            break;
        case GD32_HAL_RCC_RESET_FLAG_INDEPENDENT_WATCHDOG:
            target_flag = RCU_FLAG_FWDGTRST;
            break;
        case GD32_HAL_RCC_RESET_FLAG_WINDOW_WATCHDOG:
            target_flag = RCU_FLAG_WWDGTRST;
            break;
        case GD32_HAL_RCC_RESET_FLAG_LOW_POWER:
            target_flag = RCU_FLAG_LPRST;
            break;
        default:
            GD32_HAL_ErrorHook(GD32_HAL_PORT_ERROR_RCC_INVALID_CONFIG,
                               (uint32_t)flag);
            return 0U;
    }

    return (rcu_flag_get(target_flag) != RESET) ? 1U : 0U;
}

void GD32_HAL_RCC_ClearResetFlags(void)
{
    rcu_all_reset_flag_clear();
}
