/**
 * MPXM2102 Pressure Sensor Conversion Library
 * Author: Cody Hyman
 */

#include "mpxm2102.h"

/**
 * @brief Convert MPXM2102 raw ADC count to voltage
 * @param raw Raw ADC value
 * @return Amplified MPXM2102 output voltage
 */
inline float mpxmRawtoV(uint16_t raw)
{
	return raw * MPXM2102_VPERC;
}

/**
 * @brief Convert raw voltage into 
 * @param v Amplified MPXM2102 output voltage
 * @return MPXM2102 measured pressure
 */
inline float mpxmVToPressure(float v)
{
	return (v/MPXM2102_MAXV)*MPXM2102_MAXP;
}

/**
 * @brief Convert MPXM2102 raw ADC count to pressure
 * @param raw Raw ADC value
 * @return MPXM2102 measured pressure
 */
inline float mpxmRawToPressure(uint16_t raw)
{
	return mpxmVToPressure(mpxmRawToV(raw));
}