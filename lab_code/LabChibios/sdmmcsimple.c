#include "sdmmcsimple.h"
#include "chprintf.h"
#include "evtimer.h"

/**
 * @brief Opaque SD card struct
 */
/*
struct sdmmc
{
    MMCDriver *mmcd;	// HAL MMC-SPI Driver
    SPIDriver *spid;	// HAL SPI Driver
    FATFS *filesys;	// FAT Filesystem
    bool mmcReady; 	// MMC Driver ready flag
    bool fsReady;	// Filesystem ready flag    
};*/

static sdmmc_t *SD;

static SerialDriver *serialPort;

/** Low Speed SPI Configuration */
static SPIConfig lsCfg =
{
    NULL,
    SD_CS_PORT,
    SD_CS_PIN,
    SPI_CR1_BR_2 | SPI_CR1_BR_1
};

/** High Speed SPI Configuration */
static SPIConfig hsCfg =
{
    NULL,
    SD_CS_PORT,
    SD_CS_PIN,
    0
};

static MMCConfig mmcCfg =
{
    &SD_SPID,
    &lsCfg,
    &hsCfg
};
    
/**
 * @brief Checks if an SD card is inserted
 */
bool mmc_lld_is_card_inserted(MMCDriver *mmcd)
{
    return palReadPad(SD_CD_PORT, SD_CD_PIN);
}

/**
 * @brief Checks if an SD write protect flag is on
 */
bool mmc_lld_is_write_protected(MMCDriver *mmcd)
{
    return !palReadPad(SD_WP_PORT, SD_WP_PIN);
}

/**
 * @brief Event handler for SD card insertion
 * @param id Event id
 */
/*
static void sdmmcInsertHandler(eventid_t id)
{
    FRESULT err;
    (void) id;
    if(mmcConnect(SD->mmcd))
    {
	return;
    }
    err = f_mount(SD->filesys, "/", 0);
    if(err != FR_OK)
    {
	mmcDisconnect(SD->mmcd);
	return;
    }
    SD->fsReady = true;
}
*/

/**
 * @brief Event handler for SD card removal
 * @param id Event ID
 */
/*
static void sdmmcRemoveHandler(eventid_t id)
{
    (void) id;
    SD->fsReady = false;
}
*/

/** Event Handlers */
/*
static const evhandler_t evhdnSd[] = 
{
    sdmmcInsertHandler,
    sdmmcRemoveHandler
};
struct event_listener_t elIns, elRem;
*/

/**
 * @brief Initializes an SD card
 */
int8_t sdmmcInitialize(sdmmc_t *sd, MMCDriver *mld, SerialDriver *sp)
{    
    serialPort = sp;
    // Initialze SD object
    sd->mmcd = mld;
    sd->spid = &SD_SPID;
    sd->filesys = NULL;
    sd->fsReady = false;
    
    SD = sd;
    
    int8_t status;
    
    // MMCSPI I/O Initialization
    palSetPadMode(SD_CS_PORT, SD_CS_PIN, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(SD_CD_PORT, SD_CD_PIN, PAL_MODE_INPUT_PULLUP);
    palSetPadMode(SD_SCK_PORT, SD_SCK_PIN, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
    palSetPadMode(SD_MOSI_PORT, SD_MOSI_PIN, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
    palSetPadMode(SD_MISO_PORT, SD_MISO_PIN, PAL_MODE_INPUT);
    
    // Initialize MMC Driver
    /*mmcObjectInit(sd->mmcd, sd->spid, 
		  &lsCfg, &hsCfg, 
		  sdmmcCardProtected, sdmmcCardInserted);*/
    mmcObjectInit(sd->mmcd);
    mmcStart(sd->mmcd, &mmcCfg);

    // Try to connect
    if(mmc_lld_is_card_inserted(sd->mmcd))
    {
	chprintf((BaseSequentialStream *) serialPort, "SD/MMC:Card Found\n");
	palClearPad(SD_CS_PORT, SD_CS_PIN);
	chThdSleepMilliseconds(100); // wait
	FRESULT err;
	status = mmcConnect(sd->mmcd);
	if(status == HAL_FAILED)
	{
	    chprintf((BaseSequentialStream *) serialPort, "SD/MMC:Connect Error,ERR%02d\n", status);
	    return status;
	}
	err = f_mount(sd->filesys, "/", 0);
	if(err != FR_OK)
	{
	    chprintf((BaseSequentialStream *) serialPort, "SD/MMC:File System Mount Error\n");
	    mmcDisconnect(sd->mmcd);
	    return -2;
	}
	else
	    chprintf((BaseSequentialStream *) serialPort, "SD/MMC:File System Mounted\n");
	sd->fsReady = true;
    }
    else
    {
	chprintf((BaseSequentialStream *) serialPort, "SD/MMC:No SD Card Found\n");
	return -3;
    }
    
    return 0;
}

/**
 * @brief Accessor to the HAL MMC driver
 * @param sd SD/MMC card object to check
 * @return Pointer to MMC driver, NULL if initialization incomplete
 */
MMCDriver * sdmmGetMMCDriver(sdmmc_t *sd)
{
    if(sd->mmcReady)
	return sd->mmcd;
    return NULL;
}

/**
 * @brief Accessor to the FAT filesystem object
 * @param sd SD/MMC card object to check
 * @return Pointer to SD/MMC filesystem, returns NULL if not mounted
 */
inline FATFS * sdmmcGetFS(sdmmc_t *sd)
{
    if(sd->fsReady)
	return sd->filesys; 
    return NULL;
}

/**
 * @brief Returns the filesystem mounting status
 * @param sd SD/MMC card object to check
 * @return Status of FAT file system being mounted
 */
inline bool sdmmcFSMounted(sdmmc_t *sd)
{
   return sd->fsReady;  
}