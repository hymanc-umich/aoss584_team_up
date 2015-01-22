/*
 * FAT SD Card Interface
 * Cody Hyman
 * 
 * 
 */

#ifndef _SD_FAT_H_
#define _SD_FAT_H_

#include <string.h>

#include "ch.h"
#include "hal.h"

#include "ff.h"

// Default Values
#define SD_DEFAULT_MMCD &MMCD1
#define SD_DEFAULT_SPID &SPID1

// TODO: Define default ports/pins
#define SD_DEFAULT_CLK_PORT
#define SD_DEFAULT_MOSI_PORT
#define SD_DEFAULT_MISO_PORT
#define SD_DEFAULT_CS_PORT
#define SD_DEFAULT_CD_PORT
#define SD_DEFAULT_WP_PORT

#define SD_DEFAULT_CLK_PIN
#define SD_DEFAULT_MOSI_PIN
#define SD_DEFAULT_MISO_PIN
#define SD_DEFAULT_CS_PIN
#define SD_DEFAULT_CD_PIN
#define SD_DEFAULT_WP_PIN

/**
 * @brief SD/MMC SPI-Mode 
 */
typedef struct
{
    // File System Interface
    FATFS *FS;		// FAT Filesystem
    bool fsReady;	// Filesystem ready flag
    
    // High-Level Driver
    MMCDriver *MMCD; 	// MMC Driver
    
    // Physical Layer Driver
    SPIDriver *SPID;	// Physical Layer Driver
    SPIConfig *hsCfg;	// Low Speed Configuration
    SPIConfig *lsCfg;	// High Speed Configuration
    
    // SD/MMC SPI Mode I/O mapping
    ioportid_t CLKPort;		// Clock Port
    ioportid_t MOSIPort;	// Slave-In Port
    ioportid_t MISOPort; 	// Slave-Out Port
    ioportid_t CSPort;		// Chip Select Port
    ioportid_t CDPort;		// Card Detect Port
    ioportid_t WPPort;		// Write Protect Port
    uint8_t CLKPin;		// Clock Pin
    uint8_t MOSIPin;		// Slave-In Pin
    uint8_t MISOPin;		// Slave-Out Pin
    uint8_t CSPin; 		// Chip Select Pin
    uint8_t CDPin;		// Card Detect Pin
    uint8_t WPPin;		// Write Protect Pin
}sd_t;

bool sdfatCardInserted(sd_t *sd);
bool sdfatCardProtected(sd_t *sd);
FRESULT sdfatScanFiles(BaseChannel *ch, char *path);

//void sdfatInsertHandler(eventid_t id);
//void sdfatRemoveHandler(eventid_t id);

bool sdFatInitializeCard(sd_t *sd, bool acceptDefaults);

#endif