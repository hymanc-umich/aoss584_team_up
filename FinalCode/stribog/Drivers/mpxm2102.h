/**
 * MPXM2102 Pressure Sensor Conversion Library
 * Author: Cody Hyman
 */

#ifndef _MPXM2102_H_
#define _MPXM2102_H_

#include <stdint.h>

#define MPXM2102_MAXV	1.253f	// Amplified output (V) at 1atm
#define MPXM2102_MAXP	101.3f   // 

#define MPXM2102_VPERC	0.000805664062f	// Volts per count

float mpxmRawtoV(uint16_t raw);
float mpxmVToPressure(float v);
float mpxmRawToPressure(uint16_t raw);

#endif