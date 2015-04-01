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

static uint8_t sensCount = 0;

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
	char name[11] = "MS5607-X";
	name[7] = '0' + sensCount++;
    I2CSensor_init(&(m->sensor), driver, baseAddr, m->txBuffer, m->rxBuffer, MS2ST(20), name);
    uint8_t i;
    for(i=1; i<6; i++)
    {
		m->txBuffer[0] = MS5607_READ_PROM(i);
		I2CSensor_transact_buf(&(m->sensor), 1, 2); // Read out cal coefficients
		m->cal[i-1] = (m->rxBuffer[0]<<8) + m->rxBuffer[1];
    }
    return 0;
}

/**
 * @brief Stop the MS5607 driver
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
    m->txBuffer[0] = MS5607_RESET;
    return I2CSensor_transact_buf(&m->sensor, 1, 0);
}

/**
 * @brief Read calibrated pressure and temperature from MS5607
 * @param m MS5607 device struct
 * @param pressure Formatted pressure return
 * @param temperature Formatted temperature return
 * @return I2C status
 */
msg_t ms5607_readPressureTemperature(ms5607_t *m, float *pressure, float *temperature)
{ 
    uint8_t pressureData[3];
    uint8_t tempData[3];
    // Start temperature conversion
    m->txBuffer[0] = MS5607_TEMPERATURE_CONVERT;
    msg_t status = I2CSensor_transact_buf(&(m->sensor), 1, 0);
    chThdSleepMilliseconds(11);
    m->txBuffer[0] = MS5607_READ_ADC_RESULT;
    status |= I2CSensor_transact(&(m->sensor), m->txBuffer, 1, tempData, 3);
    // Start pressure conversion
    m->txBuffer[0] = MS5607_PRESSURE_CONVERT;
    status |= I2CSensor_transact_buf(&(m->sensor), 1, 0);
    chThdSleepMilliseconds(11);     // Wait for conversion
    m->txBuffer[0] = MS5607_READ_ADC_RESULT;
    status |= I2CSensor_transact(&(m->sensor), m->txBuffer, 1, pressureData, 3);
    
    if(status == 0)
    {
		int32_t traw = (tempData[0] << 16) + (tempData[1] << 8) + tempData[2];
		int32_t praw = (pressureData[0] << 16) + (pressureData[1] << 8) + pressureData[2];
		int64_t off, sens;
		int32_t T2 = 0;
		int32_t dT = traw - (m->cal[4]<<8);
		
		int32_t temp = 2000 + (dT*m->cal[5]>>23);
		int32_t press;
		
		off = (m->cal[1]<<17) + ((m->cal[3]*dT)>>6);
		sens = (m->cal[0]<<16) + ((m->cal[2]*dT)>>7);
		// 2nd order Temperature compensation
		if(temp < 20000) // Low temp
		{
		    int32_t t2k = (temp-2000);
		    t2k *= t2k;
		    T2 = dT*dT/0x80000000;
		    off -= 61*t2k/16;
		    sens -= 2*t2k;
		    if(temp < -15000) // Very low temp
		    {
			int32_t t1k5 = (temp-1500);
			t1k5 *= t1k5;
			off -= 15*t1k5;
			sens -= 8*t1k5;
		    }
		}
		temp -= T2;
		press = ((praw * sens >> 21)- off)>>15;
		
		// Convert to C and mbar
		m->lastTemp = (float) temp*0.01f;
		m->lastPress = (float) press*0.01f;
		if(pressure != NULL)
		{
		    *pressure = m->lastPress;
		}
		if(temperature != NULL)
		{
		    *temperature = m->lastTemp;
		}
    }
    return 0;
}
