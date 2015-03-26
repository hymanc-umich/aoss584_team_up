#include "si70x0.h"

// I2C Configuration for Si70X0 devices
static I2CConfig si70x0_i2c_cfg = 
{
   OPMODE_I2C,
   400000,
   FAST_DUTY_CYCLE_2
};

static systime_t timeout;

/**
 * @brief Initialize Si70x0 Sensor
 * @param s Si70x0 sensor struct
 * @param driver ChibiOS I2C Driver
 * @return 
 */
void si70x0_init(si70x0_t *s, I2CDriver *driver, uint8_t baseAddr)
{
    systime_t timeout = MS2ST(4);
    I2CSensor_init(&s->sensor, driver, baseAddr, timeout);
}

/**
 * 
 */
inline msg_t si70x0_stop(si70x0_t *s, bool stopI2C)
{
    return I2CSensor_stop(&s->sensor, stopI2C);
}

/**
 * @brief Reset the Si70x0 interface
 * @param s Si70x0 struct
 * @return I2C operation status
 */
msg_t si70x0_reset(si70x0_t *s)
{
    uint8_t resetCmd = SI70X0_RESET;
    return I2CSensor_transact(&s->sensor, &resetCmd, 1, NULL, 0);
}

/**
 * 
 */
msg_t si70x0_setResolution(si70x0_t *s, uint8_t resolution)
{
    // TODO: This
    return 0;
}

/**
 * 
 */
msg_t si70x0_heaterEnable(si70x0_t *s, uint8_t heaterOn)
{
    // TODO: This
    return 0;
}

/**
 * 
 */
msg_t si70x0_setHeaterCurrent(si70x0_t *s, uint8_t heaterCurrent)
{
    // TODO: This
    return 0;
}

/**
 * 
 */
msg_t si70x0_readTemperature(si70x0_t *s, float *temp)
{
    uint8_t rx[2];
    uint8_t tempCmd = SI70X0_MEASURE_TEMP_NOHOLD;
    msg_t status = I2CSensor_transact(&s->sensor, &tempCmd, 1, rx, 2);
    if(status == 0)
    {
	int16_t rawTemp = (int16_t *) ((rx[0]<<8) + rx[1]); 
	// Convert raw temp to FP value
	float tempC = (175.72f * rawTemp)/(65536.0)-46.85;
	// TODO: Data validity check
	*temp = tempC;
    }
    return status;
}

/**
 * 
 */
msg_t si70x0_readHumidity(si70x0_t *s, uint16_t *humidity)
{
    uint8_t rx[2];
    uint8_t rhCmd = SI70X0_MEASURE_RH_NOHOLD;
    msg_t status = I2CSensor_transact(&s->sensor, &rhCmd, 1, rx, 2);
    if(status == 0)
    {
	uint16_t rawRH = (rx[0]<<8) + rx[1];
	// Convert raw value to %RH
	int pctRH = (125*rawRH)/65536-6;
	if(pctRH < 0)
	    pctRH = 0;
	else if(pctRH > 100)
	    pctRH = 100;
	*humidity = pctRH;
    }
    return status;
}