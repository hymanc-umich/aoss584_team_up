#include "lsm303.h"

static systime_t timeout;

/**
 * 
 */
static msg_t lsm303_transmit_acc(lsm303_t *lsm, uint8_t *txb, uint8_t txc, uint8_t *rxb, uint8_t rxc)
{
    
    return 0;
}

/**
 * 
 */
static msg_t lsm303_transmit_mag(lsm303_t *lsm, uint8_t *txb, uint8_t txc, uint8_t *rxb, uint8_t rxc)
{
    
    return 0;
}




/**
 * 
 */
msg_t lsm303_init(lsm303_t *lsm, I2CDriver *driver, uint8_t accAddr, uint8_t magAddr)
{
    lsm->state = INACTIVE;
    lsm->i2c = driver;
    lsm->accAddr = accAddr;
    lsm->magAddr = magAddr;
    
    timeout = MS2ST(4); // Initialize global timeout
    return 0;
}

/**
 * 
 */
msg_t lsm303_readAcceleration(lsm303_t *lsm, uint8_t nread)
{
    return 0;
}

/**
 * 
 */
msg_t lsm303_readMagnetometer(lsm303_t *lsm, uint8_t nread)
{
    return 0;
}