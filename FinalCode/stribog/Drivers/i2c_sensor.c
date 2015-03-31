#include "i2c_sensor.h"

/**
 * @brief Initializer for I2C sensor struct
 * @param sensor I2C sensor struct
 * @param driver I2C Driver
 * @param baseAddress Slave address
 * @param timeout 
 */
void I2CSensor_init(I2CSensor_t *sensor, I2CDriver *driver, uint8_t baseAddress, systime_t timeout)
{
    sensor->i2c = driver;
    sensor->address = baseAddress;
    sensor->timeout = timeout;
}

/**
 * @brief Stops I2C sensor
 * @param sensor I2C sensor struct
 * @param stopI2C Stop underlying I2C driver flag
 * @return Status of stopping I2C sensor
 */
msg_t I2CSensor_stop(I2CSensor_t *sensor, bool stopI2C)
{
    msg_t status = 0;
    sensor->state = INACTIVE; // set state to inactive
    if(stopI2C) // Stop underlying I2C driver
    {
        I2cStop(&(sensor->i2c));
    }
    return status;
}

/**
 * @brief Write-Read I2C transaction
 * @param sensor I2C sensor struct
 * @param txb Transmit data buffer
 * @param txc Number of transmit bytes
 * @param rxb Receive data buffer
 * @param rxc Number of expected receive data bytes
 * @return I2C driver status message
 */
msg_t I2CSensor_transact(I2CSensor_t *sensor, uint8_t *txb, uint8_t txc, uint8_t *rxb, uint8_t rxc)
{
    if(sensor->state != INACTIVE)
    {
    	i2cAcquireBus(sensor->i2c);
    	msg_t status = i2cMasterTransmitTimeout(
    	    sensor->i2c, 
    	    sensor->address, 
    	    txb, txc, rxb, rxc, sensor->timeout);
	   i2cReleaseBus(sensor->i2c);
        return status;
    }
    return -100;
}
