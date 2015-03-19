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
 * 
 */
static ms5607_transmit(ms5607_t *m, uint8_t *txb, uint8_t txc, uint8_t *rxb, uint8_t rxc)
{    
    i2cAcquireBus(m->i2c);
    msg_t status = i2cMasterTransmitTimeout(m->i2c, m->addr, txb, txc, rxb, rxc, timeout);
    i2cReleaseBus(m->i2c);
    return status;
}

/**
 * @brief Initialize an MS5607 sensor
 * @param m MS5607 struct
 * @param driver I2C driver
 * @param baseAddr MS5607 slave address
 * @return I2C communication status
 */
msg_t ms5607_init(ms5607_t *m, I2CDriver *driver, uint8_t baseAddr)
{
    m->state = INACTIVE;
    m->i2c = driver;
    m->addr = baseAddr;
    
    timeout = MS2ST(4); // Initialize global timeout
    // TODO: Read out cal coefficients
    return 0;
}

/**
 * 
 */
msg_t ms5607_stop(ms5607_t *m, uint8_t stopI2C)
{
    m->state = INACTIVE;
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