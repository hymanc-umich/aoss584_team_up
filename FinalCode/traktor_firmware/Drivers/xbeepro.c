#include "xbeepro.h"
#include "chprintf.h"

/* XBee Default Serial configuration, 9600bps, 8N1 */
static SerialConfig xbeeSerCfg = 
{
   9600,
   0,
   0,
   0,
};

/**
 * @brief Initializes XBee Pro for serial operation
 * @param xb XBee device struct
 * @param driver XBee serial driver
 */
void xbeePro_init(xbeePro_t *xb, SerialDriver *driver)
{
	xb->ser = driver;
	sdStart(driver, &xbeeSerCfg); // Start serial port driver
	xb->state = XBEE_IDLE;
	// TODO Set destination address?
}

/**
 * @brief Enters XBee temporary AT command mode
 * @param xb XBee pro device struct
 * @return Status of command mode entry
 */
int8_t xbeePro_enterCommandMode(xbeePro_t *xb)
{
	if(xb->state != XBEE_UNKNOWN)
		chprintf((BaseSequentialStream *) xb->ser, "+++");
	// TODO: Wait for OK
	return 0;
}

/**
 * @brief Sends a serial message to the XBee with a newline
 * @param xb XBee device struct
 * @param message Message string to send
 * @return Status of message send
 */
int8_t xbeePro_sendMessage(xbeePro_t *xb, char *message)
{
	if(xb->state != XBEE_UNKNOWN)
		chprintf((BaseSequentialStream *) xb->ser, "%s\n", message);
	return 0;
}

/**
 * @brief Send formatted AT command to XBee (requires AT command mode on)
 * @param xb XBee device struct
 * @param command AT command ID (2 characters)
 * @param arg AT command argument
 * @return Status of command execution
 */
int8_t xbeePro_sendATCommand(xbeePro_t *xb, char *command, char *arg)
{
	if(xb->state != XBEE_UNKNOWN)
		if(xbeePro_enterCommandMode(xb) == 0)
			chprintf((BaseSequentialStream *) xb->ser, "AT%s %s\r", command, arg);
		// TODO: Wait for OK
	return 0;
}

/**
 *
 *
 */
int8_t xbeePro_read(xbeePro_t *xb, char *buffer, uint16_t maxChars)
{
	// TODO: Read XBee serial
	return 0;
}