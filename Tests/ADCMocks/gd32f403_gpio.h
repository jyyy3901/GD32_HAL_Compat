#ifndef GD32F403_GPIO_H
#define GD32F403_GPIO_H
#include "gd32f403.h"
#define GPIO_ADC0_ETRGROU_REMAP 0U
void gpio_pin_remap_config(uint32_t remap, ControlStatus enable);
#endif
