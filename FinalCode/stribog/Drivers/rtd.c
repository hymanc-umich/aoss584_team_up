#include "rtd.h"

/**
 * @brief Convert raw RTD ADC measurement into raw voltage
 * @param raw Raw RTD ADC measurement
 * @return Raw RTD amplifier output in volts
 */
inline float RTD_rawToV(uint16_t raw)
{
	return raw * RTD_VPERC;
}

/**
 * @brief Convert raw RTD amplified voltage into temperature
 * @param v Raw RTD amplified voltage (w/ offset)
 * @return RTD temperature in C
 */
float RTD_vToTemp(float v)
{
	float vbridge = (v-RTD_V0)/RTD_AMPGAIN;
	return ( (2.0f * vbridge) / (RTD_VS - 2*vbridge) ) * RTD_SENS;
}

/**
 * @brief Convert raw RTD ADC measurement into temperature
 * @param raw Raw ADC measurement
 * @return RTD temperature in C
 */
inline float RTD_rawToTemp(uint16_t raw)
{
	return RTD_vToTemp(RTD_rawToV(raw));
}