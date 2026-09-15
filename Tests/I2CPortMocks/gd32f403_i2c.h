#ifndef GD32F403_I2C_PORT_TEST_WRAPPER_H
#define GD32F403_I2C_PORT_TEST_WRAPPER_H

#include_next "gd32f403_i2c.h"

uint32_t GD32_Test_I2C_ReadSTAT0(uint32_t i2c_periph);
uint32_t GD32_Test_I2C_ReadSTAT1(uint32_t i2c_periph);

#undef I2C_STAT0
#undef I2C_STAT1
#define I2C_STAT0(i2c_periph) GD32_Test_I2C_ReadSTAT0(i2c_periph)
#define I2C_STAT1(i2c_periph) GD32_Test_I2C_ReadSTAT1(i2c_periph)

#endif /* GD32F403_I2C_PORT_TEST_WRAPPER_H */
