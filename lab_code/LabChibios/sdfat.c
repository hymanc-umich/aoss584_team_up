#include "sdfat.h"

volatile static sd_t SDC; // Main SD Card Instance

/**
 * @brief SD card insertion event handler
 */
static void sdfatInsertHandler(eventid_t id)
{
   FRESULT err;
   (void) id;
   if(mmcConnect(SDC.MMCD))
   {
       return;
   }
   err = f_mount(0, SD.MMC_FS);
   if(err != FR_OK)
   {
       mmcDisconnect(SDC.MMCD);
       return;
   }
   SD.fsReady = true;  
}

/**
 * @brief SD card removal event handler
 */
static void sdfatRemoveHandler(eventid_t id)
{
    (void) id;
    SDC.fsReady = FALSE;
}

/**
 * @brief Checks for SD Card inserted
 */
inline bool sdfatCardInserted(void)
{
    return palReadPad(SDC.CDPort, SDC.CDPin);
}

/**
 * @brief Checks the write-protected SD card flag
 */
inline bool sdfatCardProtected(void)
{
    return !palReadPad(SDC.WPPort, SDC.WPPin);
}

/**
 * @brief FAT SD Card Initialization Routine
 * @param sd SD Card struct to initialize
 * @param acceptDefaults
 * @return S
 */
bool sdFatInitializeCard(sd_t *sd, bool acceptDefaults)
{
    if(!acceptDefaults)
	if( (sd->MMCD == NULL) || (sd->SPID == NULL) || (sd->lsCfg == NULL) ||
	    (sd->hsCfg == NULL) || (sd->CLKPort == NULL) || 
	    (sd->MOSIPort == 0) || (sd->MISOPort == 0) ||
	    (sd->CSPort == 0) || (sd->CDPort == 0) ||
	    (sd->WPPort == 0)
	)
	    return false;
    // Default Value Checks/Assignment
    if(sd->MMCD == NULL) 
	sd->MMCD = DEFAULT_MMCD;
    if(sd->SPID == NULL) 
	sd->SPID = DEFAULT_SPID;
    if(sd->CLKPort == NULL) 
    {
	sd->CLKPort = SD_DEFAULT_CLK_PORT;
	sd->CLKPin  = SD_DEFAULT_CLK_PIN;
    }
    if(sd->MOSIPort == NULL)
    {
	sd->MOSIPort = SD_DEFAULT_MOSI_PORT;
	sd->MOSIPin  = SD_DEFAULT_MOSI_PIN;
    }
    if(sd->MISOPort == NULL)
    {
	sd->MISOPort = SD_DEFAULT_MISO_PORT;
	sd->MISOPin  = SD_DEFAULT_MISO_PIN;
    }
    if(sd->CSPort == NULL)
    {
	sd->CSPort = SD_DEFAULT_CS_PORT;
	sd->CSPin  = SD_DEFAULT_CS_PIN;
    }
    if(sd->CDPort == NULL)
    {
	sd->CDPort = SD_DEFAULT_CD_PORT;
	sd->CDPin  = SD_DEFAULT_CD_PIN;
    }
    if(sd->WPPort == NULL)
    {
	sd->WPPort = SD_DEFAULT_WP_PORT;
	sd->WPPin  = SD_DEFAULT_WP_PIN;
    }
    if(sd->lsCfg == NULL)
	sd->lsCfg = 
	{
	    NULL, 
	    sd->CSPort,
	    sd->CSPin,
	    SPI_CR1_BR2 | SPI_CR1_BR_1
	};
    if(sd->hsCfg == NULL)
	sd->hsCfg = 
	{
	    NULL, 
	    sd->CSPort, 
	    sd->CSPin, 
	    0
	};

    // Event Handlers
    static const evhandler_t evhdnSd[] = 
    {
	sdfatInsertHandler,
	sdfatRemoveHandler
    };
    struct EventListener elIns, elRem;
    
    // MMCSPI I/O Initialization
    palSetPadMode(sd->CSPort, sd->CSPin, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(sd->CDPort, sd->CDPin, PAL_MODE_INPUT_PULLUP);
    palSetPadMode(sd->CLKPort, sd->CLKPin, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
    palSetPadMode(sd->MOSIPort, sd->MOSIPin, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
    palSetPadMode(sd->MISOPort, sd->MISOPin, PAL_MODE_INPUT);
    
    // Initialize MMC Driver
    mmcObjectInit(sd->MMCD, sd->SPID, 
		  sd->lsCfg, sd->hsCfg, 
		  sdfatCardProtected, sdfatCardInserted);
    mmcStart(sd->MMCD, NULL);
    
    // Register insert/remove events
    chEvtRegister(sd->MMCD.inserted_event, &elIns, 0);
    chEvtRegister(sd->MMCD.removed_event, &elRem, 1);
    
    SDC = *sd;
    return true;
}