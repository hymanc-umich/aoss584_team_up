/**
 * lsm9ds0.h
 * ST LSM9DS0 IMU Driver for ChibiOS and STM32F4
 * Author: Cody Hyman
 */

#ifndef _LSM9DS0_H_
#define _LSM9DS0_H_

#include "ch.h"
#include "hal.h"

#include <stdint.h>
#include <stdfix.h>
#include <stdio,h>

// Target Device Indices
#define IMU_XM 	0
#define IMU_G	1

/* 
*  LSM9DS0 I2C address options:
*  1. SA0 at Vdd:   accel/mag add = 0x3A;  gyro add = 0xD6
*  2. SA0 at GND:   accel/mag add = 0x3C;  gyro add = 0xD4
*/
#define XM_ADDR_SA0_0	0x3C
#define XM_ADDR_SA0_1 	0x3A
#define G_ADDR_SA0_0	0xD4
#define G_ADDR_SA0_1	0xD6

#define IMU_I2C 	I2C_1		// BBB I2C2 if I2C1 is off
#define IMU_XM_ADDR	XM_ADDR_SA0_1	// SA is grounded
#define IMU_G_ADDR	G_ADDR_SA0_1	// SA is grounded

/* LSM9DS0 Register Definitions */
// 16-bit register
#define REG_L(base) base 
#define REG_H(base) base+1

// XYZ 6-sequential register
#define REG_XL(base) base
#define REG_XH(base) base + 0x01
#define REG_YL(base) base + 0x02
#define REG_YH(base) base + 0x03
#define REG_ZL(base) base + 0x04
#define REG_ZH(base) base + 0x05

/* Gyrometer I2C Register Definitions */
#define WHO_AM_I_G 	0x0F
#define CTRL_REG1_G 	0x20
#define CTRL_REG2_G 	0x21
#define CTRL_REG3_G 	0x22
#define CTRL_REG4_G 	0x23
#define CTRL_REG5_G 	0x24
#define REFERENCE_G 	0x25
#define STATUS_REG_G 	0x27
// Gyro Data Registers
#define OUT_X_L_G 	0x28
#define OUT_X_H_G 	0x29
#define OUT_Y_L_G 	0x2A
#define OUT_Y_H_G 	0x2B
#define OUT_Z_L_G 	0x2C
#define OUT_Z_H_G 	0x2D
#define GYRO_X_BASE 	OUT_X_L_G
#define GYRO_Y_BASE	OUT_Y_L_G
#define GYRO_Z_BASE	OUT_Z_L_G
#define GYRO_BASE	GYRO_X_BASE

#define FIFO_CTRL_REG_G 0x2E
#define FIFO_SRC_REG_G 	0x2F
#define INT1_CFG_G 	0x30
#define INT1_SRC_G 	0x31
#define INT1_TSH_XH_G 	0x32
#define INT1_TSH_XL_G 	0x33
#define INT1_TSH_YH_G 	0x34
#define INT1_TSH_YL_G 	0x35
#define INT1_TSH_ZH_G 	0x36
#define INT1_TSH_ZL_G 	0x37
#define INT1_DURATION_G 0x38

/* Magnetometer I2C Register Definitions */
#define TEMP_BASE 	0x05
#define OUT_TEMP_L_XM 	0x05
#define OUT_TEMP_H_XM 	0x06
#define STATUS_REG_M 	0x07

// Magnetometer Data Registers
#define OUT_X_L_M 	0x08
#define OUT_X_H_M 	0x09
#define OUT_Y_L_M 	0x0A
#define OUT_Y_H_M 	0x0B
#define OUT_Z_L_M 	0x0C
#define OUT_Z_H_M 	0x0D
#define MAG_X_BASE 	0x08
#define MAG_Y_BASE 	0x0A
#define MAG_Z_BASE 	0x0C
#define MAG_BASE	MAG_X_BASE

#define WHO_AM_I_XM 	0x0F
#define INT_CTRL_REG_M 	0x12
#define INT_SRC_REG_M 	0x13
#define INT_THS_L_M 	0x14
#define INT_THS_H_M	0x15

// Magnetometer Offset Registers
#define OFFSET_X_L_M	0x16
#define OFFSET_X_H_M	0x17
#define OFFSET_Y_L_M 	0x18
#define OFFSET_Y_H_M	0x19
#define OFFSET_Z_L_M	0x1A
#define OFFSET_Z_H_M	0x1B
#define MAG_OFF_X_BASE 	OFFSET_X_L_M
#define MAG_OFF_Y_BASE 	OFFSET_Y_L_M
#define MAG_OFF_Z_BASE 	OFFSET_Z_H_M

#define REFERENCE_X	0x1C
#define REFERENCE_Y	0x1D
#define REFERENCE_Z	0x1E

#define CTRL_REG0_XM	0x1F
#define CTRL_REG1_XM	0x20
#define CTRL_REG2_XM	0x21
#define CTRL_REG3_XM	0x22
#define CTRL_REG4_XM	0x23
#define CTRL_REG5_XM	0x24
#define CTRL_REG6_XM	0x25
#define CTRL_REG7_XM	0x26

/* Accelerometer I2C Register Definitions */
#define STATUS_REG_A	0x27

// Accelerometer Data Registers
#define OUT_X_L_A	0x28
#define OUT_X_H_A	0x29
#define OUT_Y_L_A	0x2A
#define OUT_Y_H_A	0x2B
#define OUT_Z_L_A	0x2C
#define OUT_Z_H_A	0x2D
#define OUT_X_A		OUT_X_L_A
#define OUT_Y_A		OUT_Y_L_A
#define OUT_Z_A		OUT_Z_L_A
#define OUT_A_BASE	ACC_X_BASE

#define FIFO_CTRL_REG	0x2E
#define FIFO_SRC_REG	0x2F
#define INT_GEN_1_REG	0x30
#define INT_GEN_1_SRC	0x31
#define INT_GEN_1_THS	0x32
#define INT_GEN_1_DURATION 0x33
#define INT_GEN_2_REG	0x34
#define INT_GEN_2_SRC	0x35
#define INT_GEN_2_THS	0x36
#define INT_GEN_2_DURATION 0x37
#define CLICK_CFG	0x38
#define CLICK_SRC	0x39
#define CLICK_THS	0x3A
#define TIME_LIMIT	0x3B
#define TIME_LATENCY 	0x3C
#define TIME_WINDOW	0x3D
#define ACT_THS		0x3E
#define ACT_DUR		0x3F
/* End of Register Definitions */

#define CTRL1_G_DR1 	(1<<7)
#define CTRL1_G_DR0 	(1<<6)
#define CTRL1_G_BW1 	(1<<5)
#define CTRL1_G_BW0 	(1<<4)
#define CTRL1_G_PD  	(1<<3)
#define CTRL1_G_ZEN 	(1<<2)
#define CTRL1_G_XEN 	(1<<1)
#define CTRL1_G_YEN 	(1<<0)

/* CTRL_REG4_G */
// Gyro full scale selection (CTRL_REG4_G)
#define GFS_245DPS 	(0b00 << 4)
#define GFS_500DPS 	(0b01 << 4)
#define GFS_2000DPS 	(0b11 << 4)
// Gyro Self-test mode (CTRL_REG4_G)
#define GST_NORMAL	(0b00 << 1)
#define GST_X_POSITIVE	(0b01 << 1)
#define GST_X_NEGATIVE	(0b11 << 1)

/* CTRL_REG2_XM */
// Accelerometer full scale selection (CTRL_REG2_XM)
#define AFS_2G 		(0b000 << 3)
#define AFS_4G 		(0b001 << 3)
#define AFS_6G 		(0b010 << 3)
#define AFS_8G		(0b011 << 3)
#define AFS_16G		(0b100 << 3)
// Accelerometer AA Filter Bandwidth (CTRL_REG2_XM)
#define ABW_50HZ	(0b11 << 6)
#define ABW_194HZ	(0b01 << 6)
#define ABW_362HZ	(0b10 << 6)
#define ABW_773HZ	(0b00 << 6)
// Accelerometer Self-Test Mode (CTRL_REG2_XM)
#define AST_NORMAL	(0b00 << 1)
#define AST_POSITIVE	(0b01 << 1)
#define AST_NEGATIVE	(0b10 << 1)

/* CTRL_REG6_XM */
// Magnetometer full scale selection (CTRL_REG6_XM)
#define MFS_2G		(0b00 << 5)
#define MFS_4G		(0b01 << 5)
#define MFS_8G		(0b10 << 5)
#define MFS_12G		(0b11 << 5)

/* Sensitivities */
// Accelerometer Full Scale Sensitivities (g=9.806m/s^2)
#define ACC_SENSITIVITY_2G	0.000061
#define ACC_SENSITIVITY_4G	0.000122
#define ACC_SENSITIVITY_6G	0.000183
#define ACC_SENSITIVITY_8G	0.000244
#define ACC_SENSITIVITY_16G	0.000732
// Gyrometer Full Scale Sensitivities (Degrees Per Second)
#define GYRO_SENSITIVITY_245DPS	 0.00875
#define GYRO_SENSITIVITY_500DPS	 0.01750
#define GYRO_SENSITIVITY_2000DPS 0.07000
// Magnetometer Full Scale Sensitivities (Gauss)
#define MAG_SENSITIVITY_2G	0.00008
#define MAG_SENSITIVITY_4G	0.00016
#define MAG_SENSITIVITY_8G	0.00032
#define MAG_SENSITIVITY_12G	0.00048
// Temperature Sensitivity (C)
#define TEMP_SENSITIVITY	0.125
#define A_GRAVITY_MPS	9.80665

typedef struct
{
    float
}accelData_t;

typedef struct
{
    I2CDriver *i2c;
    int16_t
}lsm9ds0_t;

int8_t lsmInitialize(I2CDriver *drv);
float lsmGetAccel();
float lsmGetAngularRate();
float lsmGetMag();
#endif