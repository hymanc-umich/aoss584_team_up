/**
 * gps.c
 * NMEA-0183 GPS Interface
 * AOSS 584, Winter 2015
 * University of Michigan
 * Author: Cody Hyman
 */

#include "gps.h"
#include <string.h>
#include <stdlib.h>
#include "chbsem.h"

#define DEBUG_GPS
#define DEBUG_SERIAL &SD2

/**
 * @brief GPS Receiver interface struct
 */
struct gpsReceiver
{
   UARTDriver *gpsUart; 		// UART Driver
   char buffer[2][GPS_BUFFER_SIZE]; 	// Double receive buffer
   uint8_t bufferCount;			// Receiver buffer counter
   uint8_t activeBuffer; 		// Double buffer switch
   gpsLocation_t location;		// Location storage
   mutex_t dataMutex;			// Data mutex
};
typedef struct gpsReceiver gpsReceiver_t;

typedef struct gpsThread gpsThread_t;

static gpsReceiver_t GPS;

#ifdef DEBUG_GPS
    static SerialDriver *DEBUG = DEBUG_SERIAL;
#endif

binary_semaphore_t gpsSem;

/** GPS UART Callbacks **/

/**
 * @brief Double buffer selector
 * @return Active receive buffer
 */
static char * gpsGetActiveRxBuffer(void)
{
    return GPS.buffer[GPS.activeBuffer];
}

/**
 * @brief Add character to GPS buffer
 * @param c Character to add
 * @return Success 
 */
static int8_t gpsAddToBuffer(char c)
{
    if(GPS.bufferCount >= GPS_BUFFER_SIZE)
	return -1;
    char * buffer = gpsGetActiveRxBuffer();
    buffer[GPS.bufferCount++] = c;
    return 0;
}

/**
 * @brief Toggles the active GPS receive buffer
 */
static inline void gpsToggleBuffer(void)
{
    GPS.activeBuffer ^= 1; 	// Toggle double buffer
    GPS.bufferCount = 0; 	// Reset buffer count
}

/**
 * @brief GPS Tx buffer empty
 */
static void gpsTxbufEnd(UARTDriver *uartp)
{
   (void) uartp;// TODO: Nothing? 
}

/**
 * @brief GPS Tx complete
 */
static void gpsTxC(UARTDriver *uartp)
{
    (void) uartp;
}

/**
 * @brief GPS Rx character received callback
 */
static void gpsRxChar(UARTDriver *uartp, uint16_t c)
{
    (void) uartp; // TODO: Check character and place in buffer
    if(c == '\n')
    {
	gpsAddToBuffer('\0');
	chBSemSignal(&gpsSem);
    }
    else
    {
	gpsAddToBuffer(c);
    }
}

/**
 * @brief GPS RxBuffer full callback
 */
static void gpsRxEnd(UARTDriver *uartp)
{
    (void) uartp;// Add to buffer
}

/**
 * @brief GPS UART Rx Error callback
 */
static void gpsRxErr(UARTDriver *uartp, uartflags_t e)
{
    (void) uartp;
    (void) e;
}


/*
 * UART driver configuration structure.
 */
static UARTConfig serGpsCfg = {
  gpsTxbufEnd,
  gpsTxC,
  gpsRxEnd,
  gpsRxChar,
  gpsRxErr,
  NMEA_0183_BAUD,
  0,
  0,	// No LIN
  0
};


/**
 * @brief GPS interface startup
 * @param gpsUART GPS UART driver
 * @return Initialization success status
 */
int8_t gpsStart(UARTDriver *gpsUart)
{
    if(gpsUart == NULL)
	return -1;
    GPS.gpsUart = gpsUart;
    chMtxObjectInit(&(GPS.dataMutex));
    uartStart(GPS.gpsUart, &serGpsCfg); // Start GPS Serial port
    return 0;
}

/**
 * @brief Stops GPS receiver
 * @param gps GPS receiver to stop
 * @return Stop success flag
 */
int8_t gpsStop(gpsReceiver_t *gps)
{
    uartStop(gps->gpsUart);
    return 0; 
}

/**
 * @brief Parses NMEA 
 * @param nmeaStr NMEA Sentence string to parse
 * @return NMEA GPS Sentence type
 */
sentenceType_t gpsParseNMEAType(char *nmeaStr)
{
    char *stypeIndex;
    if(nmeaStr[0] == NMEA_START_CHAR)
    {
	if(strncmp(nmeaStr+1, NMEA_GPS, 2) == 0)
	{
	    stypeIndex = nmeaStr + 3;
	    if(strncmp(stypeIndex, NMEA_GPS_ALM, 3) == 0)
	    {
		return GPS_ALM;
	    }
	    else if(strncmp(stypeIndex, NMEA_FIX_DATA, 3) == 0)
	    {
		return FIX_DATA;
	    }
	    else if(strncmp(stypeIndex, NMEA_GEO_POS,3) == 0)
	    {
		return GEO_POS;
	    }
	    else if(strncmp(stypeIndex, NMEA_ACT_SAT, 3) == 0)
	    {
		return ACT_SAT;
	    }
	    else if(strncmp(stypeIndex, NMEA_SAT_VIEW, 3) == 0)
	    {
		return SAT_VIEW;
	    }
	    else if(strncmp(stypeIndex, NMEA_HEADING, 3) == 0)
	    {
		return HEADING;
	    }
	    else if(strncmp(stypeIndex, NMEA_DATE_TIME, 3) == 0)
	    {
		return DATE_TIME;
	    }
	    else
	    {
		return OTHER;
	    }
	}
	else
	{
	    return OTHER;
	}
    }
    else
	return UNDEF;
}

/**
 * @brief Parse GPS GGA Sentence for location
 * @param nmeaGGAstr NMEA GPS Fix ($GPGGA) string
 * @param loc Location to store to
 * @return Parse success status
 */
int8_t gpsParseFix(char *nmeaGGAStr, gpsLocation_t *loc)
{
    // Verify GGA Sentence
    if(strncmp(nmeaGGAStr+3, NMEA_FIX_DATA, 3)) 
	return -1;
    
    char buffer[16];
    // Parse sentence
    char * itok = strtok(nmeaGGAStr,",");
    uint8_t i = 0;
    chMtxLock(&(GPS.dataMutex));
    while((itok != NULL) && (i < 8))
    {
	switch(i)
	{
	    case 1: // Time
		// HHMMSS
		strncpy(buffer, itok, 6);
		loc->time = atoi(buffer);
		break;
	    case 2: // Latitude
		strncpy(buffer, itok, 4);
		strncpy(buffer+4, itok+5,2);
		loc->latitude = atoi(buffer);
		break;
	    case 3: // Latitude Hemisphere
		if(strncmp(itok, "S", 1) == 0)
		    loc->latitude = -1 * loc->latitude;
		break;
	    case 4: // Longitude
		strncpy(buffer, itok, 5);
		strncpy(buffer+5, itok+6, 2);
		loc->longitude = atoi(buffer);
		break;
	    case 5: // Longitude Hemisphere
		if(strncmp(itok, "W", 1) == 0)
		    loc->longitude = -1 * loc->latitude;
		break;
	    case 6: // Quality Indicator (Do nothing for now)
		break;
	    case 7: // Satellite count
		strncpy(buffer, itok, 1);
		loc->satCount = atoi(buffer);
		break;
	    default:
		break;
	}
	itok = strtok(NULL,",");
	i++;
	// 
    }
    chMtxUnlock(&(GPS.dataMutex));
    return 0;
}

/**
 * @brief Computes the NMEA-0183 sentence checksum
 */
uint8_t gpsNMEAChecksum(char *nmeaStr)
{
    uint8_t checksum = 0;
    nmeaStr++; // Skip initial character
    while(*nmeaStr)
	checksum ^= *nmeaStr++;
    return checksum;
}

/**
 * @brief GPS Data Collection Thread
 * @param arg GPS thread structure pointer
 */
msg_t gpsThread(void *arg)
{
    gpsThread_t *thread = (gpsThread_t *) arg;
    msg_t message;
    chBSemObjectInit(&gpsSem, false);
    // GPS Loop
    while(thread->running)
    {
	chBSemWait(&gpsSem); 		// Wait for new NMEA string
	chBSemReset(&gpsSem, false);	// Clear semaphore
	// Parse Fix data if sentence is correct
	if(gpsParseNMEAType(gpsGetActiveRxBuffer()) == FIX_DATA)
	{
	    gpsParseFix(gpsGetActiveRxBuffer(), &(GPS.location));
	}
    }
    gpsStop(&GPS);
    return message;
}

/**
 * @brief Thread safe accessor for GPS latitude
 * @return GPS latitude (integerized)
 */
int32_t gpsGetLatitude(void)
{
    chMtxLock((&GPS.dataMutex));
    int32_t latitude = GPS.location.latitude;
    chMtxUnlock((&GPS.dataMutex));
    return latitude;
}

/**
 * @brief Thread safe accessor for GPS longitude
 * @return GPS longitude (integerized)
 */
int32_t gpsGetLongitude(void)
{
    chMtxLock(&(GPS.dataMutex));
    int32_t longitude = GPS.location.longitude;
    chMtxUnlock(&(GPS.dataMutex));
    return longitude;
}

/**
 * @brief Thread safe accessor for GPS altitude
 * @return GPS altitude (integerized)
 */
int32_t gpsGetAltitude(void)
{
    chMtxLock(&(GPS.dataMutex));
    int32_t altitude = GPS.location.altitude;
    chMtxUnlock(&(GPS.dataMutex));
    return altitude;
}

/**
 * @brief Thread safe accessor for GPS time
 * @return GPS time (hhmmss)
 */
uint32_t gpsGetTime(void)
{
    chMtxLock(&(GPS.dataMutex));
    uint32_t time = GPS.location.time;
    chMtxUnlock(&(GPS.dataMutex));
    return time;
}

/**
 * @brief Thread safe accessor for GPS satellite count
 * @return Number of visible GPS satellites
 */
uint8_t gpsGetSatellites(void)
{
    chMtxLock(&(GPS.dataMutex));
    uint8_t satCount = GPS.location.satCount;
    chMtxUnlock(&(GPS.dataMutex));
    return satCount;
}

/**
 * @brief Thread safe accessor to copy all location data
 * @param dest Destination location container
 */
void gpsGetLocation(gpsLocation_t *dest)
{
    chMtxLock(&(GPS.dataMutex));
    dest->latitude  = GPS.location.latitude;
    dest->longitude = GPS.location.longitude;
    dest->altitude  = GPS.location.altitude;
    dest->time      = GPS.location.time;
    dest->satCount  = GPS.location.satCount;
    chMtxUnlock(&(GPS.dataMutex));
}
