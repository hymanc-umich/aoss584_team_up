/*
 * Simple SD/MMC over SPI Utility
 * Cody Hyman
 */

#ifndef _SDMMC_SIMPLE_H_
#define _SDMMC_SIMPLE_H_

#include "ch.h"
#include "hal.h"
#include "ff.h"

// Hardware configuration
#define SD_SPID SPID1

#define SD_SCK_PORT	GPIOA
#define SD_MOSI_PORT	GPIOA
#define SD_MISO_PORT	GPIOA
#define SD_CS_PORT	GPIOB
#define SD_CD_PORT	GPIOC
#define SD_WP_PORT	GPIOA

#define SD_SCK_PIN	5
#define SD_MOSI_PIN	7
#define SD_MISO_PIN	6
#define SD_CS_PIN	6
#define SD_CD_PIN	7
#define SD_WP_PIN	9

extern MMCDriver MMCD1;

struct sdmmc
{
    MMCDriver *mmcd;	// HAL MMC-SPI Driver
    SPIDriver *spid;	// HAL SPI Driver
    FATFS *filesys;	// FAT Filesystem
    bool mmcReady; 	// MMC Driver ready flag
    bool fsReady;	// Filesystem ready flag    
};

typedef struct sdmmc sdmmc_t;

int8_t sdmmcInitialize(sdmmc_t *sd, MMCDriver *mld, SerialDriver *sp);
bool sdmmcCardInserted(sdmmc_t *sd);
bool sdmmcCardProtected(sdmmc_t *sd);
MMCDriver * sdmmGetMMCDriver(sdmmc_t *sd);
FATFS * sdmmcGetFS(sdmmc_t *sd);
bool sdmmcFSMounted(sdmmc_t *sd);

bool mmc_lld_is_card_inserted(MMCDriver *mmcd);
#endif