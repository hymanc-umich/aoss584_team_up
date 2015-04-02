#include "tmp275.h"

static uint8_t sensCount = 0;

/**
 * @brief Initialize TMP275 temperature sensor
 * @param tmp TMP275 sensor struct
 * @param driver I2C HAL driver
 * @param baseAddr Base I2C slave address
 */
void tmp275_init(tmp275_t *tmp, I2CDriver *driver, uint8_t baseAddr)
{
    char name[11] = "TMP275-X";
    name[7] = '0' + sensCount++;
    I2CSensor_init(&tmp->sensor, driver, baseAddr, tmp->txBuffer, tmp->rxBuffer, MS2ST(4), name);
    tmp275_writeConfig(tmp, 0);
    tmp275_writeTHigh(tmp, -128);
    tmp275_writeTLow(tmp, 127);
}

/**
 * @brief Stops TMP275 sensor
 * @param tmp TMP275 sensor struct
 * @param stopI2C Stop underlying I2C driver
 * @return I2C driver status message
 */
msg_t tmp275_stop(tmp275_t *tmp, bool stopI2C)
{
    return I2CSensor_stop(&tmp->sensor, stopI2C);
}

/**
 * @brief Write to the configuration register
 * @param tmp TMP275 sensor struct
 * @param value Register value
 * @return I2C driver status message
 */
msg_t tmp275_writeConfig(tmp275_t *tmp, uint8_t value)
{
    tmp->txBuffer[0] = TMP275_CONFIGURATION;
    tmp->txBuffer[1] = value;
    I2CSensor_transact_buf(&tmp->sensor, 2, 0);
}

/**
 * @brief Read formatted temperature
 * @param tmp TMP275 sensor struct
 * @param temp Temperature return value
 * @return I2C driver status
 */
msg_t tmp275_readTemperature(tmp275_t *tmp, float *temp)
{
    tmp->txBuffer[0] = TMP275_TEMPERATURE;
    msg_t status = I2CSensor_transact_buf(&tmp->sensor, 1, 2);
    if(status == 0)
    {
        int16_t rawTemp = (int16_t) ((tmp->rxBuffer[0] << 8) | tmp->rxBuffer[1]);
        if(temp != NULL)
        {
            *temp = (rawTemp)*TMP275_RESOLUTION;
        }
    	//int16_t rawTemp = (int16_t) ((tmp->rxBuffer[0]*256.0) | (tmp->rxBuffer[1]));
    	//*temp = (1.0f*rawTemp)*TMP275_RESOLUTION; // Format temperature
    }
    return status;
    
}

/**
 * @brief Writes the temperature high threshold register
 * @param tmp TMP275 sensor struct
 * @param tempLimit Temperature ADC alert limit
 * @return I2C driver status
 */
msg_t tmp275_writeTHigh(tmp275_t *tmp, int16_t tempLimit)
{
    tmp->txBuffer[0] = (uint8_t)(tempLimit>>8)&0xFF;
    tmp->txBuffer[1] =  (uint8_t)(tempLimit&0xFF);
    return I2CSensor_transact_buf(&tmp->sensor, 2, 0);
}

/**
 * @brief Writes the temperature low threshold register
 * @param tmp TMP275 sensor struct
 * @param tempLimit Temperature ADC alert limit
 * @return I2C driver status
 */
msg_t tmp275_writeTLow(tmp275_t *tmp, int16_t tempLimit)
{
    tmp->txBuffer[0] = (uint8_t) (tempLimit>>8)&0xFF;
    tmp->txBuffer[1] = (uint8_t)(tempLimit&0xFF);
    return I2CSensor_transact_buf(&tmp->sensor, 2, 0);
}