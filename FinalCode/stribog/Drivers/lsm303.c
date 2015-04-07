#include "lsm303.h"


static uint8_t sensCount = 0;

/**
 * 
 */
msg_t lsm303_init(lsm303_t *lsm, I2CDriver *driver)
{
    systime_t timeout = MS2ST(5);
    char nameA[11] = "LSM303_A-X";
    char nameM[11] = "LSM303_M-X";
    nameA[9] = '0' + sensCount;
    nameM[9] = '0' + sensCount++;
    I2CSensor_init(&lsm->accSensor, driver, LSM303_ACC_ADDR, lsm->txaBuffer, lsm->rxaBuffer, timeout, nameA);
    I2CSensor_init(&lsm->magSensor, driver, LSM303_MAG_ADDR, lsm->txmBuffer, lsm->rxmBuffer, timeout, nameM);

    // Enable Accelerometer
    lsm->txaBuffer[0] = CTRL_REG1_A;
    lsm->txaBuffer[1] = 0x27;
    msg_t status = I2CSensor_transact_buf(&lsm->accSensor, 2, 0);
    /*if(status != MSG_OK)
    {
        lsm->accSensor.state = INACTIVE;
        return status;
    }*/
    // Enable Magnetometer
    lsm->txmBuffer[0] = CTRL_REG1_M;
    lsm->txmBuffer[1] = 0x00;
    status = I2CSensor_transact_buf(&lsm->magSensor, 2, 0);
    /*if(status != MSG_OK)
    {
        lsm->accSensor.state = INACTIVE;
    }*/
    return status;
}

/**
 * 
 */
msg_t lsm303_readAcceleration(lsm303_t *lsm, uint8_t nread)
{
    uint8_t i;
    msg_t status;
    for(i = 0; i < nread; i++)
    {
        lsm->txaBuffer[0] = OUT_X_L_A | 0x80;
        status = I2CSensor_transact_buf(&lsm->accSensor, 1, 6);
        if(status == MSG_OK)
        {
            int16_t xa = (int16_t) ((lsm->rxaBuffer[1] << 8) | lsm->rxaBuffer[0]);
            int16_t ya = (int16_t) ((lsm->rxaBuffer[3] << 8) | lsm->rxaBuffer[2]);
            int16_t za = (int16_t) ((lsm->rxaBuffer[5] << 8) | lsm->rxaBuffer[4]);
        }
    }
    return status;
}

/**
 * 
 */
msg_t lsm303_readMagnetometer(lsm303_t *lsm, uint8_t nread)
{
    uint8_t i;
    msg_t status;
    for(i = 0; i < nread; i++)
    {
        lsm->txmBuffer[0] = OUT_X_L_M | 0x80;
        status = I2CSensor_transact_buf(&lsm->magSensor, 1, 6);
        if(status == MSG_OK)
        {
            int16_t xa = (int16_t) ((lsm->rxmBuffer[1] << 8) | lsm->rxmBuffer[0]);
            int16_t ya = (int16_t) ((lsm->rxmBuffer[3] << 8) | lsm->rxmBuffer[2]);
            int16_t za = (int16_t) ((lsm->rxmBuffer[5] << 8) | lsm->rxmBuffer[4]);
        }
    }
    return status;
}