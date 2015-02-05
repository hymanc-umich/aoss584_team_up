/**
 * gps.c
 * NMEA-0183 GPS Interface
 * AOSS 584, Winter 2015
 * University of Michigan
 * Author: Cody Hyman
 */

#ifndef _GPS_H_
#define _GPS_H_

#include "ch.h"
#include "hal.h"

/* NMEA Sentence Definitions */
// NMEA Start Character
#define NMEA_START_CHAR		'$'
#define NMEA_START 		"$"
// NMEA Talker Identifiers
#define NMEA_GPS		"GP"
// NMEA Sentence Identifiers
#define NMEA_GPS_ALM		"ALM"
#define NMEA_FIX_DATA		"GGA"
#define NMEA_GEO_POS		"GLL"
#define NMEA_ACT_SAT		"GSA"
#define NMEA_SAT_VIEW		"GSV"
#define NMEA_HEADING		"HDT"
#define NMEA_DATE_TIME		"ZDA"

#define GPS_BUFFER_SIZE 82 	// Maximum NMEA0183 length

#define NMEA_0183_BAUD 4800	// NMEA0183 baud rate

/**
 * @brief NMEA GPS Sentence types
 */
typedef enum
{
    GPS_ALM,
    FIX_DATA,
    GEO_POS,
    ACT_SAT,
    SAT_VIEW,
    HEADING,
    DATE_TIME,
    OTHER,
    UNDEF
}sentenceType_t;

/**
 * @brief GPS Location container
 */
struct gpsLocation
{
    uint32_t 	time;		// Time hhmmssss
    int32_t 	latitude;	// Latitude
    int32_t 	longitude;	// Longitude
    int32_t 	altitude;	// Altitude
    uint8_t 	satCount;	// Satellite count
};
typedef struct gpsLocation gpsLocation_t;

/**
 * @brief GPS thread structure
 */
struct gpsThread
{
    uint16_t sleepTime;
    bool running;
};
typedef struct gpsThread gpsThread_t;

/**  Function Declarations */
int8_t gpsStart(UARTDriver *gpsUart); 
sentenceType_t gpsParseNMEAType(char *nmeaStr);
int8_t gpsParseFix(char *nmeaGGAStr, gpsLocation_t *loc);
uint8_t gpsNMEAChecksum(char *nmeaStr);
msg_t gpsThread(void *arg);

// Thread safe data accessors
int32_t gpsGetLatitude(void);
int32_t gpsGetLongitude(void);
int32_t gpsGetAltitude(void);
uint32_t gpsGetTime(void);
uint8_t gpsGetSatellites(void);
void gpsGetLocation(gpsLocation_t *dest);
#endif