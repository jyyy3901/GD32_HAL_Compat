#ifndef STM32F401XE_H
#define STM32F401XE_H

#include "stm32_hal_cmsis_bridge.h"
#include "stm32_hal_instance_map.h"
#include "stm32f401_register_compat.h"
#include "stm32f401_register_bits.h"

#ifndef SET_BIT
#define SET_BIT(REG, BIT) ((REG) |= (BIT))
#endif
#ifndef CLEAR_BIT
#define CLEAR_BIT(REG, BIT) ((REG) &= ~(BIT))
#endif
#ifndef READ_BIT
#define READ_BIT(REG, BIT) ((REG) & (BIT))
#endif
#ifndef CLEAR_REG
#define CLEAR_REG(REG) ((REG) = 0U)
#endif
#ifndef WRITE_REG
#define WRITE_REG(REG, VAL) ((REG) = (VAL))
#endif
#ifndef READ_REG
#define READ_REG(REG) (REG)
#endif
#ifndef MODIFY_REG
#define MODIFY_REG(REG, CLEARMASK, SETMASK) \
    WRITE_REG((REG), (((READ_REG(REG)) & ~(CLEARMASK)) | (SETMASK)))
#endif

__STATIC_INLINE uint32_t STM32_COMPAT_PositionVal(uint32_t value)
{
    uint32_t position = 0U;
    if (value == 0U)
    {
        return 32U;
    }
    while ((value & 1U) == 0U)
    {
        value >>= 1U;
        ++position;
    }
    return position;
}

#ifndef POSITION_VAL
#define POSITION_VAL(VAL) STM32_COMPAT_PositionVal((uint32_t)(VAL))
#endif

#endif /* STM32F401XE_H */
