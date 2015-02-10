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

#define NMEA_0183_BAUD 9600	// NMEA0183 baud rate (device dependent)

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
 * @brief GPS Location string container
 */
struct gpsLocationStr
{
    char time[9];
    char latitude[15];
    char longitude[15];
    char altitude[8];
    char satCount[3];
};
typedef struct gpsLocationStr gpsLocation_t;

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
int8_t gpsParseFixStr(char *nmeaGGAStr, gpsLocation_t *loc);
uint8_t gpsNMEAChecksum(char *nmeaStr);
msg_t gpsThread(void *arg);

// Thread safe data accessors
void gpsGetLatitude(char *dest);
void gpsGetLongitude(char *dest);
void gpsGetAltitude(char *dest);
void gpsGetTime(char *dest);
void gpsGetSatellites(char *dest);
void gpsGetLocation(gpsLocation_t *dest);
#endif