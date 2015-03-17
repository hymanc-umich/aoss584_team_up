#include "si70x0.h"

/**
 * @brief Initialize Si70x0 Sensor
 * @param s Si70x0 sensor struct
 * @param driver ChibiOS I2C Driver
 * @
 */
int8_t si70x0_init(Si70x0_t *s, I2CDriver *driver, uint8_t baseAddr)
{
    s->i2c = driver;
    s->baseAddr = baseAddr;
    s->state = SI_70X0_INACTIVE;
    // Initialize I2C
    return 0;
}

/**
 * 
 */
int8_t si70x0_stop(Si70x0_t *s, uint8_t stopI2C)
{
    return 0;
}

/**
 * 
 */
int8_t si70x0_setResolution(Si70x0_t *s, uint8_t resolution)
{
    return 0;
}

/**
 * 
 */
int8_t si70x0_heaterEnable(Si70x0_t *s, uint8_t heaterOn)
{
    return 0;
}

/**
 * 
 */
int8_t si70x0_setHeaterCurrent(Si70x0_t *s, uint8_t heaterCurrent)
{
    return 0;
}

/**
 * 
 */
int8_t si70x0_readTemperature(Si70x0_t *s, int16_t *temp)
{
    return 0;
}

/**
 * 
 */
int8_t si70x0_readHumidity(Si70x0_t *s, uint16_t *humdity)
{
    return 0;
}