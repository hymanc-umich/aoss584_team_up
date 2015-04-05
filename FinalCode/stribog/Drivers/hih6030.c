#include "hih6030.h"

/**
 * 
 */
void hih6030_init(hih6030_t *hih, I2CDriver *driver, uint8_t baseAddr)
{
    I2CSensor_init(&(hih->sensor), driver, baseAddr, hih->txBuffer, hih->rxBuffer, MS2ST(4), "HIH6030");
    hih->measureFlag = FALSE;
}

/**
 * 
 */
msg_t hih6030_stop(hih6030_t *hih, bool stopI2C)
{
    return I2CSensor_stop(&(hih->sensor), stopI2C);
}

/**
 *
 *
 */
msg_t hih6030_measurementRequest(hih6030_t *hih)
{
	hih->txBuffer[0] = 0x00;
	msg_t status = I2CSensor_transact_buf(&(hih->sensor),1,0);
	if(status == MSG_OK)
		hih->measureFlag = TRUE;
	return status;
}

/**
 * 
 */
msg_t hih6030_read(hih6030_t *hih, float *humidity, float *temperature)
{
	if(!(hih->measureFlag)) // Request measurement if none has been made
	{
		hih->txBuffer[0] = 0x00;
		I2CSensor_transact_buf(&(hih->sensor),1,0); // Measurement request
		chThdSleepMilliseconds(37);
	}
	msg_t status = I2CSensor_transact_buf(&(hih->sensor), 0, 4); // Data fetch
	uint16_t rawH, rawT;
	// Check I2C status
	if(status == MSG_OK)
	{
		// Check status bits
		if(!(hih->rxBuffer[0] & 0b10000000)) // Device not in command mode
		{
			// Get raw temperature and humidity
			rawH = ((hih->rxBuffer[0] &0x3F)<<8) + hih->rxBuffer[1];
			rawT = (hih->rxBuffer[2]<<6) + (hih->rxBuffer[3]>>2);
			// Get compensated temperature

			// Convert to floating point
			if(humidity != NULL)
			{
				*humidity = (float) (100.0f*rawH)/(16382.0f);
			}
			if(temperature != NULL)
			{
				*temperature = (float) (rawT * 165.0)/(16382.0f)-40.0f;
			}
			return status;
		}
	}	
    return status;
}

