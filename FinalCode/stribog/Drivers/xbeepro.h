#ifndef _XBEEPRO_H_
#define _XBEEPRO_H_

#include "ch.h"
#include "hal.h"

// Default Destination Address
#define XBEE_DEFAULT_DESTINATION_ADDRESS_H
#define XBEE_DEFAULT_DESTINATION_ADDRESS_L	

#define XBEE_LINEEND				'\r'
#define XBEE_OK						"OK"

// AT Command Mode
#define XBEE_AT_COMMAND_MODE 		"+++"
#define XBEE_AT_PREFIX				"AT"

// Special AT Commands
#define XBEE_AT_APPLY_CHANGES		"AC"
#define XBEE_AT_SW_RESET	 		"FR"
#define XBEE_AT_RESTORE_DEFAULTS 	"RE"
#define XBEE_AT_WRITE				"WR"
// AT MAC/PHY Level Commands
#define XBEE_AT_AVAILABLE_FREQ		"AF"
#define XBEE_AT_CHANNEL_MASK		"CM"
#define XBEE_AT_MINIMUM_FREQ_CNT	"MF"
#define XBEE_AT_PREAMBLE_ID			"HP"
#define XBEE_AT_NETWORK_ID			"ID"
#define XBEE_AT_BROADCAST_MULTI_TX	"MT"
#define XBEE_AT_POWER_LEVEL			"PL"
#define XBEE_AT_UNICAST_MAC_RETRY	"RR"
#define XBEE_AT_ENERGY_DETECT		"ED"
// AT Diagnostic Commands
#define XBEE_AT_BYTES_TX			"BC"
#define XBEE_AT_RSSI				"DB"
#define XBEE_AT_RX_ERR_COUNT		"ER"
#define XBEE_AT_GOOD_PKTS_RX		"GD"
#define XBEE_AT_MAC_ACK_TIMEOUTS	"EA"
#define XBEE_AT_TX_ERRORS			"TR"
#define XBEE_AT_MAC_UNI_TX_COUNT	"UA"
#define XBEE_AT_MAC_UNI_ONEHOP_TIME	"%H"
#define XBEE_AT_MAC_BRD_ONEHOP_TIME	"%8"

// AT Network Commands
#define XBEE_AT_NODE_MSG_OPTIONS	"CE"
#define XBEE_AT_BROADCAST_HOPS		"BH"
#define XBEE_AT_NETWORK_HOPS		"NH"
#define XBEE_AT_NETWORK_DELAY_SLOTS	"NN"
#define XBEE_AT_MESH_UNI_RETRIES	"MR"

// AT Addressing Commands
#define XBEE_AT_SERIAL_NUMBER_HIGH	"SH"
#define XBEE_AT_SERIAL_NUMBER_LOW	"SL"
#define XBEE_AT_DEST_ADDR_HIGH		"DH"
#define XBEE_AT_DEST_ADDR_LOW		"DL"
#define XBEE_AT_TX_OPTIONS			"TO"
#define XBEE_AT_NODE_ID				"NI"
#define XBEE_AT_NODE_DISC_TIMEOUT	"NT"
#define XBEE_AT_NODE_DISC_OPTIONS	"NO"
#define XBEE_AT_CLUSTER_ID			"CI"
#define XBEE_AT_DEST_ENDPOINT		"DE"
#define XBEE_AT_SRC_ENDPOINT		"SE"
// AT Address Discovery/Configuration Commands
#define XBEE_AT_AGG_SUPPORT			"AG"
#define XBEE_AT_DISCOVER_NODE		"DN"
#define XBEE_AT_NETWORK_DISCOVER	"ND"
#define XBEE_AT_FIND_NEIGHBORS		"FN"

// AT Security Commands
#define XBEE_AT_SECURITY_ENABLE		"EE"
#define XBEE_AT_AES_ENCRYPTION_KEY	"KY"

// AT Serial Interfacing
#define XBEE_AT_BAUD_RATE			"BD"
	#define XBEE_BAUD_1K2			0
	#define XBEE_BAUD_2K4			1
	#define XBEE_BAUD_4K8			2
	#define XBEE_BAUD_9K6			3
	#define XBEE_BAUD_19K2			4
	#define XBEE_BAUD_38K4			5
	#define XBEE_BAUD_57K6			6
	#define XBEE_BAUD_115K2			7
	#define XBEE_BAUD_230K4			8
#define XBEE_AT_PARITY				"NB"
	#define XBEE_PARITY_NONE		0
	#define XBEE_PARITY_EVEN		1
	#define XBEE_PARITY_ODD			2
#define XBEE_AT_STOP_BITS			"SB"
	#define XBEE_SB_ONE				0
	#define XBEE_SB_TWO				1
#define XBEE_AT_PACKET_TIMEOUT		"RO"
#define XBEE_FLOW_CTL_THRESHOLD		"FT"
#define XBEE_API_MODE				"AP"
	#define XBEE_API_TRANSPARENT	0
	#define XBEE_API_WO_ESCAPES		1
	#define XBEE_API_W_ESCAPES 		2
#define XBEE_API_OPTIONS			"AO"

// AT I/O Settings
#define XBEE_AT_COMM_PUSHBUTTON		"CB"
#define XBEE_AT_DIO0_CONFIG			"D0"
#define XBEE_AT_DIO1_CONFIG			"D1"
#define XBEE_AT_DIO2_CONFIG			"D2"
#define XBEE_AT_DIO3_CONFIG			"D3"
#define XBEE_AT_DIO4_CONFIG			"D4"
#define XBEE_AT_DIO5_CONFIG			"D5"
#define XBEE_AT_DIO6_CONFIG			"D6"
#define XBEE_AT_DIO7_CONFIG			"D7"
#define XBEE_AT_DIO8_CONFIG			"D8"
#define XBEE_AT_DIO9_CONFIG			"D9"
#define XBEE_AT_DIO10_CONFIG		"P0"
#define XBEE_AT_DIO11_CONFIG		"P1"
#define XBEE_AT_DIO12_CONFIG		"P2"
#define XBEE_AT_DIO13_CONFIG		"P3"
#define XBEE_AT_DIO14_CONFIG		"P4"
#define XBEE_AT_PULL_DIRECTION		"PD"
#define XBEE_AT_PULLUP_RESISTOR		"PR"
#define XBEE_AT_PWM0_DUTY_CYCLE		"M0"
#define XBEE_AT_PWM1_DUTY_CYCLE		"M1"
#define XBEE_AT_ASSOC_LED_BLINK		"LT"
#define XBEE_AT_RSSI_PWM_TIMER		"RP"

// AT I/O Sampling
#define XBEE_AT_ANALOG_VREF			"AV"
	#define XBEE_VREF_1V25			0
	#define XBEE_VREF_2V5 			1
#define XBEE_AT_DIO_CHANGE_DET		"IC"
#define XBEE_AT_SLEEP_SAMPLE_RATE	"IF"
#define XBEE_AT_IO_SAMPLE_RATE		"IR"
#define XBEE_AT_FORCE_SAMPLE		"IS"
#define XBEE_AT_TEMPERATURE			"TP"
#define XBEE_AT_SUPPLY_VOLTAGE		"%V"

// AT Sleep Commands
#define XBEE_AT_SLEEP_MODE			"SM"
#define XBEE_AT_SLEEP_OPTIONS		"SO"
#define XBEE_AT_SLEEP_PERIODS		"SN"
#define XBEE_AT_SLEEP_PERIOD 		"SP"
#define XBEE_AT_WAKE_TIME			"ST"
#define XBEE_AT_WAKE_HOST			"WH"
// AT Sleep Diagnostics
#define XBEE_AT_SLEEP_STATUS		"SS"
#define XBEE_AT_OP_SLEEP_PERIOD		"OS"
#define XBEE_AT_OP_WAKE_PERIOD		"OW"
#define XBEE_AT_NUM_MISS_SYNC		"MS"
#define XBEE_AT_MISS_SYNC_COUNT		"SQ"

// AT Command Options
#define XBEE_AT_COMMAND_CHARACTER	"CC"
#define XBEE_AT_EXIT_COMMAND_MODE	"CN"
#define XBEE_AT_CMD_MODE_TIMEOUT	"CT"
#define XBEE_AT_GUARD_TIMES			"GT"

// AT Firmware Commands


typedef enum 
{
	XBEE_UNKNOWN,
	XBEE_IDLE,
	XBEE_AT_COMMAND,
}xbee_state;

typedef struct
{
	SerialDriver *ser;
	xbee_state state;
}xbeePro_t;

void xbeePro_init(xbeePro_t *xb, SerialDriver *driver);
int8_t xbeePro_enterCommandMode(xbeePro_t *xb);
int8_t xbeePro_sendMessage(xbeePro_t *xb, char *message);
int8_t xbeePro_sendATCommand(xbeePro_t *xb, char *command, char *arg);
#endif