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
	float vdiff = (v-3.3f)*5;
	return (1.0f/( (1.13/2.13) - vdiff/25.0f ) - 2)/0.00385f;
	//return (1000.0f/((1.13/2.13)-(v-3.3f)/25.0f ) - 2000.0f) / 0.00385f;

	//float vbridge = (v-RTD_V0)/RTD_AMPGAIN;
	//return ( (2.0f * vbridge) / (RTD_VS - 2*vbridge) ) * RTD_SENS;
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