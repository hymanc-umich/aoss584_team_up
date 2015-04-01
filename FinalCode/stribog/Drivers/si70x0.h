#ifndef _SI70X0_H_
#define _SI70X0_H_

#include "i2c_sensor.h"

#define SI70X0_TIMEOUT				50
#define SI7020_DEFAULT_ADDRESS 		0b10000000

// Si7020 Commands
#define SI70X0_MEASURE_RH_HOLD		0xE5
#define SI70X0_MEASURE_RH_NOHOLD	0xF5
#define SI70X0_MEASURE_TEMP_HOLD	0xE3
#define SI70X0_MEASURE_TEMP_NOHOLD	0xF3
#define SI70X0_READ_PREV_TEMP		0xE0
#define SI70X0_RESET				0xFE
#define SI70X0_W_USER_REG1			0xE6
#define SI70X0_R_USER_REG1			0xE7
#define SI70X0_W_HEATER_CTL			0x51
#define SI70X0_R_HEATER_CTL			0x11
#define SI70X0_R_ID_BYTE_1			0xFA0F
#define SI70X0_R_ID_BYTE_2			0xFCC9
#define SI70X0_R_FIRMWARE_VERSION	0x84B8

// Resolution defines
#define SI70X0_RES_H12T14			0b00
#define SI70X0_RES_H8T12			0b01
#define SI70X0_RES_H10T13			0b10
#define SI70X0_RES_H11T11			0b11 

typedef enum 
{
	H8T12,
	H10T13,
	H11T11,
	H12T14
}si70x0_resolution;

typedef struct
{
    I2CSensor_t sensor;
    si70x0_resolution res;
    bool heater;
    uint8_t txBuffer[4];
    uint8_t rxBuffer[10];
}si70x0_t;

void si70x0_init(si70x0_t *s, I2CDriver *driver, uint8_t baseAddr);
msg_t si70x0_stop(si70x0_t *s, bool stopI2C);
msg_t si70x0_reset(si70x0_t *s);
msg_t si70x0_setResolution(si70x0_t *s, si70x0_resolution resolution);
msg_t si70x0_heaterEnable(si70x0_t *s, bool heaterOn);
msg_t si70x0_setHeaterCurrent(si70x0_t *s, uint8_t heaterCurrent);
msg_t si70x0_readTemperature(si70x0_t *s, float *temp);
msg_t si70x0_readHumidity(si70x0_t *s, float *humidity);

#endif