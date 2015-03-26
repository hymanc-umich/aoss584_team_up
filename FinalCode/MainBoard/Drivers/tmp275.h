#ifndef _TMP275_H_
#define _TMP275_H_

#include "i2c_sensor.h"

// TMP275 registers
#define TMP275_TEMPERATURE 	0x00
#define TMP275_CONFIGURATION 	0x01
#define TMP275_TLOW		0x02
#define TMP275_THIGH		0x03

// Resolution settings
#define TMP275_RES_BM		0b11
#define TMP275_RESOLUTION	(0.5f/(2*TMP275_RES_BM))

// Configuration register bits
#define TMP275_OS		7	// One-shot
#define TMP275_R1		6	// Resolution bit 1
#define TMP275_R0		5	// Resolution bit 0
#define TMP275_F1		4	// Fault queue bit 1
#define TMP275_F0		3	// Fault queue bit 0
#define TMP275_POL		2	// Alert polarity
#define TMP275_TM		1	// Thermostat mode
#define TMP275_SD		0	// Shutdown

typedef struct
{
    I2CSensor_t sensor;
}tmp275_t;

void tmp275_init(tmp275_t *tmp, I2CDriver *driver, uint8_t baseAddr);
msg_t tmp275_stop(tmp275_t *tmp, bool stopI2C);
msg_t tmp275_writeConfig(tmp275_t *tmp, uint8_t value);
msg_t tmp275_readTemperature(tmp275_t *tmp, float *temp);
msg_t tmp275_writeTHigh(tmp275_t *tmp, int16_t tempLimit);
msg_t tmp275_writeTLow(tmp275_t *tmp, int16_t tempLimit);
#endif