#ifndef GD32F403_LIBOPT_H
#define GD32F403_LIBOPT_H

/*
 * Test-only SPL module selection.  Official firmware packages keep this file
 * in each application project rather than in the library root.  Product
 * projects must provide their own selection header.
 */
#include "gd32f403_rcu.h"
#include "gd32f403_adc.h"
#include "gd32f403_dma.h"
#include "gd32f403_exti.h"
#include "gd32f403_fmc.h"
#include "gd32f403_gpio.h"
#include "gd32f403_i2c.h"
#include "gd32f403_pmu.h"
#include "gd32f403_spi.h"
#include "gd32f403_timer.h"
#include "gd32f403_usart.h"
#include "gd32f403_misc.h"

#endif /* GD32F403_LIBOPT_H */
