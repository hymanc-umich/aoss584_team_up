#include "si70x0.h"
#include "chprintf.h"
#include "../board.h"

static uint8_t sensCount = 0;

/**
 * @brief Initialize Si70x0 Sensor
 * @param s Si70x0 sensor struct
 * @param driver ChibiOS I2C Driver
 * @return 
 */
void si70x0_init(si70x0_t *s, I2CDriver *driver, uint8_t baseAddr)
{
    char name[11] = "Si70x0-X";
    name[7] = '0' + sensCount++;
    systime_t timeout = MS2ST(SI70X0_TIMEOUT);
    I2CSensor_init(&s->sensor, driver, 0b1000000, s->txBuffer, s->rxBuffer, timeout, name);
    si70x0_setResolution(s, H11T11);
}

/**
 * @brief Stop Si70x0 sensor
 * @param s Si70x0 struct
 * @param stopI2C stop underlying I2C driver
 * @return I2C operation status
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
    s->txBuffer[0] = SI70X0_RESET;
    //uint8_t resetCmd = SI70X0_RESET;
    // TODO: Reset settings
    return I2CSensor_transact_buf(&s->sensor, 1, 0);
   // return I2CSensor_transact(&(s->sensor), &resetCmd, 1, rxBuffer, 0);
}

/**
 * @brief Set Si70x0 ADC resolution
 * @param s Si70x0 struct
 * @param resolution Resolution setting
 * @return I2C operation status
 */
msg_t si70x0_setResolution(si70x0_t *s, si70x0_resolution resolution)
{
    uint8_t res_bits;
    switch(resolution)
    {
        case H8T12:
            res_bits = SI70X0_RES_H8T12;
            break;
        case H10T13:
            res_bits = SI70X0_RES_H10T13;
            break;
        case H11T11:
            res_bits = SI70X0_RES_H11T11;
            break;
        case H12T14:
            res_bits = SI70X0_RES_H12T14;
            break;
        default:
            res_bits = SI70X0_RES_H12T14;
            break;
    }
    // Read old userreg
    s->txBuffer[0] = SI70X0_R_USER_REG1;
    msg_t status = I2CSensor_transact_buf(&(s->sensor), 1, 1);
    if(status == MSG_OK)
    {
        uint8_t userReg = s->rxBuffer[0];  
        userReg &= ~0x3;
        userReg |= res_bits;
        s->txBuffer[0] = SI70X0_W_USER_REG1;
        s->txBuffer[1] = userReg;
        status = I2CSensor_transact_buf(&(s->sensor), 2, 0); // Write modifications
    }
    return status;
}

/**
 * @brief Toggle Si70x0 integrated heater
 * @param s Si70x0 struct
 * @param heaterOn Heater on/off command
 * @return I2C operation status
 */
msg_t si70x0_heaterEnable(si70x0_t *s, bool heaterOn)
{
    s->txBuffer[0] = SI70X0_R_USER_REG1;
    msg_t status = I2CSensor_transact_buf(&(s->sensor), 1, 1);
    if(status == MSG_OK)
    {
        uint8_t userReg = s->rxBuffer[0];
        if(heaterOn)
        {
            userReg |= 1<<2;
        }
        else
        {
            userReg &= ~(1<<2);
        }
        s->txBuffer[0] = SI70X0_W_USER_REG1;
        s->txBuffer[1] = userReg;
        status = I2CSensor_transact_buf(&(s->sensor), 2, 0);
        if(status == MSG_OK)
        {
            s->heater = heaterOn;
        }
    }
    return status;
}

/**
 * @brief Set heater current
 * @param s Si70x0 struct
 * @param heaterCurrent Heater current level (0-15)
 * @return I2C operation status
 */
msg_t si70x0_setHeaterCurrent(si70x0_t *s, uint8_t heaterCurrent)
{
    s->txBuffer[0] = SI70X0_W_HEATER_CTL;
    s->txBuffer[1] = heaterCurrent & 0x0F;
    return I2CSensor_transact_buf(&(s->sensor), 2, 0);
}

/**
 * @brief Read formatted temperature from Si70x0 (Deg C)
 * @param s Si70x0 struct
 * @param temp Temperature return pointer
 * @return I2C operation status
 */
msg_t si70x0_readTemperature(si70x0_t *s, float *temp)
{
    s->txBuffer[0] = SI70X0_MEASURE_TEMP_HOLD;
    msg_t status = I2CSensor_transact_buf(&s->sensor, 1, 2);
    if(status == MSG_OK)
    {
    	int16_t rawTemp = (int16_t) ((s->rxBuffer[0]<<8) + s->rxBuffer[1]); 
    	// Convert raw temp to FP value
    	float tempC = (175.72f * rawTemp)/(65536.0)-46.85;
    	// TODO: Data validity check
    	*temp = tempC;
    }
    return status;
}

/**
 * @brief Read formatted humidity from Si70x0 (%RH)
 * @param s Si70x0 struct
 * @param humidity Humidity return pointer
 * @return I2C operation status
 */
msg_t si70x0_readHumidity(si70x0_t *s, float *humidity)
{
    s->txBuffer[0] = SI70X0_MEASURE_RH_HOLD;
    msg_t status = I2CSensor_transact_buf(&s->sensor, 1, 2);
    if(status == MSG_OK)
    {
        //chprintf((BaseSequentialStream *) &DBG_SERIAL, "SI70X0:HREAD_SUCCESS\n");
    	uint16_t rawRH = (s->rxBuffer[0]<<8) + s->rxBuffer[1];
    	// Convert raw value to %RH
    	int pctRH = ((125.0f*rawRH)/65536)-6.0f;
    	if(pctRH < 0)
    	    pctRH = 0;
    	else if(pctRH > 100)
    	    pctRH = 100.0f;
    	*humidity = pctRH;
    }
    else
    {
        //chprintf((BaseSequentialStream *) &DBG_SERIAL, "SI70X0:HREAD_FAIL(%d)\n", status);
    }
    return status;
}