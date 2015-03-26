#include "rfm23bp.h"

static SPIConfig rfm23SpiCfg =
{
    NULL,
    RFM23_CS_PORT,
    RFM23_CS_PIN,
    SPI_CR1_BR_1
};

/**
 * 
 */
msg_t rfm23_init(rfm23_t *rfm, SPIDriver *driver)
{
    rfm->spi = driver;
    return 0;
}

/**
 * @brief Toggle RFM23 transmitter power (active low)
 * @param rfm RFM23 device struct
 * @param en On/Off state of transmitter
 */
void rfm23_TXEN(rfm23_t *rfm, bool en)
{
    if(en)
    {
	rfm23_RXEN(rfm, FALSE);
	palClearPad(RFM23_PCTL_PORT, RFM23_TXON_PIN);
	return;
    }
    palSetPad(RFM23_PCTL_PORT, RFM23_TXON_PIN);
    
}

/**
 * @brief Toggle RFM23 receiver power (active low)
 * @param rfm RFM23 device struct
 * @param en On/Off state of transmitter
 */
void rfm23_RXEN(rfm23_t *rfm ,bool en)
{
    if(en)
    {
	rfm23_TXEN(rfm, FALSE);
	palClearPad(RFM23_PCTL_PORT, RFM23_RXON_PIN);
	return;
    }
    palSetPad(RFM23_PCTL_PORT, RFM23_RXON_PIN);
}

/**
 * @brief Active low radio shutdown
 * @param rfm RFM23 device struct
 * @param shutdown 0=On, 1=Shutdown
 */
void rfm23_SHUTDOWN(rfm23_t *rfm, bool shutdown)
{
    if(shutdown)
    {
	palSetPad(RFM23_PCTL_PORT, RFM23_SDN_PIN);
	return;
    }
    palClearPad(RFM23_PCTL_PORT, RFM23_SDN_PIN);
}

/**
 * 
 */
msg_t rfm23_readRegister(rfm23_t *rfm, uint8_t addr, uint8_t *data)
{
    // TODO: this
    return 0;
}

/**
 * 
 */
msg_t rfm23_writeRegister(rfm23_t *rfm, uint8_t addr, uint8_t value)
{
    // TODO: this
    return 0;
}

/**
 * 
 */
msg_t rfm23_readBurst(rfm23_t *rfm, uint8_t startAddr, uint8_t *data, uint8_t count)
{
    return 0;
}

/**
 * 
 */
msg_t rfm23_writeBurst(rfm23_t *rfm, uint8_t startAddr, uint8_t *data, uint8_t count)
{
    return 0;
}

/**
 * 
 */
msg_t rfm23_sendByte(rfm23_t *rfm, uint8_t byte)
{
    return 0;
}
