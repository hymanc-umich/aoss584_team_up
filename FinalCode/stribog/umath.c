#include "umath.h"

/**
 * @param power Positive power
 */
int32_t pow(int base, int power)
{
    if(power == 0)
	   return 1;
    if(power < 0)
        return 0;
    int32_t i;
    int32_t result = 1;
    for(i = 0; i < power; i++)
    {
	result *= base;
    }
    return result;
}

/**
 * 
 */
float powf(float base, int power)
{
    if(power == 0)
    {
	return 1;
    }
    int32_t i;
    float result = 1;
    if(power > 0)
    {
	for(i = 0; i < power; i++)
	{
	    result *= base;
	}
	return result;
    }
    for(i = 0; i < power; i++)
    {
	result /= base;
    }
    return result;
}