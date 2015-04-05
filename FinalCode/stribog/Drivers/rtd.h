/**
 * Analog 2-RTD bridge driver
 * Author: Cody Hyman
 */

#ifndef _RTD_H_
#define _RTD_H_

#include <stdint.h>

#define RTD_VS		5.00f 		    // RTD bridge supply voltage (V)
#define RTD_VPERC 	0.000805664062f	// Volts per count (V)
#define RTD_AMPGAIN	46.0f			// RTD amplifier gain (V/V)
#define RTD_SENS	0.000385f		// RTD sensitivity (K^-1)
#define RTD_T0 		25.0f			// RTD null temp (C)
#define RTD_V0		3.00f			// Null amplifier voltage at T0 (V)

float RTD_rawToV(uint16_t raw);
float RTD_vToTemp(float v);
float RTD_rawToTemp(uint16_t raw);

#endif