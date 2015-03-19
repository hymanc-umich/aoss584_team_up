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
 * @brief I2C Exchange with timeout
 * @param s Si70x0 device struct
 * @param txb Transmit buffer
 * @param txc Number of bytes to transmit
 * @param rxb Receive buffer
 * @param rxc Number of bytes to receive
 * @return Status of read
 */
static msg_t si70x0_transmit(Si70x0_t *s, uint8_t *txb, uint8_t txc, uint8_t *rxb, uint8_t rxc)
{
    i2cAcquireBus(s->i2c);
    msg_t status = i2cMasterTransmitTimeout(s->i2c, s->addr, txb, txc, rxb, rxc, timeout);
    i2cReleaseBus(s->i2c);
    return status;
}

/**
 * @brief Initialize Si70x0 Sensor
 * @param s Si70x0 sensor struct
 * @param driver ChibiOS I2C Driver
 * @
 */
int8_t si70x0_init(Si70x0_t *s, I2CDriver *driver, uint8_t baseAddr)
{
    s->i2c = driver;
    s->addr = baseAddr;
    s->state = SI_70X0_INACTIVE;
    // Initialize I2C
    timeout = MS2ST(4); // Initialize global timeout
    return 0;
}

/**
 * 
 */
int8_t si70x0_stop(Si70x0_t *s, uint8_t stopI2C)
{
    // TODO: This
    return 0;
}

/**
 * @brief Reset the Si70x0 interface
 * @param s Si70x0 struct
 * @return I2C operation status
 */
msg_t si70x0_reset(Si70x0_t *s)
{
    uint8_t resetCmd = SI70X0_RESET;
    return si70x0_transmit(s, &resetCmd, 1, NULL, 0);
}

/**
 * 
 */
msg_t si70x0_setResolution(Si70x0_t *s, uint8_t resolution)
{
    // TODO: This
    return 0;
}

/**
 * 
 */
msg_t si70x0_heaterEnable(Si70x0_t *s, uint8_t heaterOn)
{
    // TODO: This
    return 0;
}

/**
 * 
 */
msg_t si70x0_setHeaterCurrent(Si70x0_t *s, uint8_t heaterCurrent)
{
    // TODO: This
    return 0;
}

/**
 * 
 */
msg_t si70x0_readTemperature(Si70x0_t *s, float *temp)
{
    uint8_t rx[2];
    uint8_t tempCmd = SI70X0_MEASURE_TEMP_NOHOLD;
    msg_t status = si70x0_transmit(s, &tempCmd, 1, rx, 2);
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
msg_t si70x0_readHumidity(Si70x0_t *s, uint16_t *humidity)
{
    uint8_t rx[2];
    uint8_t rhCmd = SI70X0_MEASURE_RH_NOHOLD;
    msg_t status = si70x0_transmit(s, &rhCmd, 1, rx, 2);
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