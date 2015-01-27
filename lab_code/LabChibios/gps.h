#ifndef _GPS_H_
#define _GPS_H_

#include "hal.h"

// NMEA 
#define GPGSTR "$GPG"

void gpsInit(void); // TODO: Serial Driver

void gpsParseNMEA(char *nmeaStr);

uint8_t gpsNMEAChecksum(char *nmeaStr);

#endif