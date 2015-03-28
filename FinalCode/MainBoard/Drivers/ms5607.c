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
    I2CSensor_init(&(m->sensor), driver, baseAddr, MS2ST(20));
    uint8_t promBuffer[2];
    uint8_t i;
    for(i=1; i<6; i++)
    {
	uint8_t txCmd = MS5607_READ_PROM(i);
	I2CSensor_transact(&(m->sensor), &txCmd, 1, promBuffer, 2); // Read out cal coefficients
	m->cal[i-1] = (promBuffer[0]<<8) + promBuffer[1];
    }
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
    return I2CSensor_transact(&m->sensor, &resetCmd, 1, NULL, 0);
}

/**
 * 
 */
msg_t ms5607_readPressure(ms5607_t *m, float *pressure)
{
    uint8_t regAddr = MS5607_PRESSURE_CONVERT; // TODO: This
    uint8_t pressureData[3];
    // Start conversion
    msg_t status = I2CSensor_transact(&(m->sensor), &regAddr, 1, NULL, 0);
    chThdSleepMilliseconds(10);     // Wait for conversion
    regAddr = MS5607_READ_ADC_RESULT;
    status |= I2CSensor_transact(&(m->sensor), &regAddr, 1, pressureData, 3);
    int32_t T2, OFF2, SENS2;
    if(status == 0)
    {
	// Temperature compensation
	if(m->lastTemp < 20) // Low temp
	{
	    if(m->lastTemp < -15) // Very low temp
	    {
		
	    }
	    else
	    {
		
	    }
	}
	else
	{
	  T2 = 0;
	  OFF2 = 0;
	  SENS2 = 0;
	}
	// TODO: Convert to actual pressure
	if(pressure != NULL)
	{
	    
	}
    }
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