#include "ms5607.h"

/**
 * 
 */
static I2CConfig ms5607_i2c_cfg = 
{
   OPMODE_I2C,
   400000,
   FAST_DUTY_CYCLE_2
};

static systime_t timeout;


/**
 * @brief Initialize an MS5607 sensor
 * @param m MS5607 struct
 * @param driver I2C driver
 * @param baseAddr MS5607 slave address
 * @return I2C communication status
 */
msg_t ms5607_init(ms5607_t *m, I2CDriver *driver, uint8_t baseAddr)
{
    I2CSensor_init(&m->sensor, driver, baseAddr, MS2ST(4));

    // TODO: Read out cal coefficients
    return 0;
}

/**
 * 
 */
msg_t ms5607_stop(ms5607_t *m, bool stopI2C)
{
    I2CSensor_stop(&m->sensor, stopI2C);
    return 0;
}

/**
 * @brief Reset the MS5607 over the bus
 * @param m MS5607 struct
 * @return I2C communication status
 */
msg_t ms5607_reset(ms5607_t *m)
{
    uint8_t resetCmd = MS5607_RESET;
    return ms5607_transmit(m, &resetCmd, 1, NULL, 0);
}

/**
 * 
 */
msg_t ms5607_readPressure(ms5607_t *m, float *pressure)
{
    // TODO: This
    return 0;
}

/**
 * 
 */
msg_t ms5607_readTemperature(ms5607_t *m, float *temperature)
{
    // TODO: This
    return 0;
}

/**
 * 
 */
msg_t ms5607_readPROMValues(ms5607_t *m)
{
    // TODO: This
    return 0;
}