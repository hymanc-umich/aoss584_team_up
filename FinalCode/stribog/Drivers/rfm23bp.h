/**
 * RFM23BP Radio Module Driver for ChibiOS
 * Author: Cody Hyman
 */

#include "ch.h"
#include "hal.h"

#include "board.h"
/** I/O Defines: Set up for provided board **/


/** Register Definitions **/
#define RFM23_DEV_TYPE		0x00
#define RFM23_DEV_VERS		0x01
#define RFM23_DEV_STATUS	0x02
#define RFM23_INT_STATUS1	0x03
#define RFM23_INT_STATUS2	0x04
#define RFM23_INT_ENABLE1	0x05
#define RFM23_INT_ENABLE2	0x06
#define RFM23_OP_FNC_CTL1	0x07
#define RFM23_OP_FNC_CTL2	0x08
#define RFM23_XO_LOAD_CAP	0x09
#define RFM23_MCU_OUT_CLK	0x0A
#define RFM23_GPIO0_CFG		0x0B
#define RFM23_GPIO1_CFG		0x0C
#define RFM23_GPIO2_CFG		0x0D
#define RFM23_IO_PORT_CFG	0x0E
#define RFM23_ADC_CFG		0x0F
#define RFM23_ADC_AMP_OFFSET	0x10
#define RFM23_ADC_VALUE		0x11
#define RFM23_TMP_SNS_CTL	0x12
#define RFM23_TMP_VAL_OFFSET	0x13
#define RFM23_WKUP_PERIOD1	0x14
#define RFM23_WKUP_PERIOD2	0x15
#define RFM23_WKUP_PERIOD3	0x16
#define RFM23_WKUP_VALUE1	0x17
#define RFM23_WKUP_VALUE2	0x18
#define RFM23_LDC_MODE_DUR	0x19
#define RFM23_LO_BATT_THRESH	0x1A
#define RFM23_BATT_VOLTAGE	0x1B
#define RFM23_IF_FILTER_BW	0x1C
#define RFM23_AFC_LOOP_GS_OVERRIDE		0x1D
#define RFM23_AFC_TIMING_CTL	0x1E
#define RFM23_CLK_RECOVERY_OVERRIDE		0x1F
#define RFM23_CLK_RECOVERY_OVERSAMPLE_RATIO	0x20
#define RFM23_CLK_RECOVERY_OFFSET2	0x21
#define RFM23_CLK_RECOVERY_OFFSET1	0x22
#define RFM23_CLK_RECOVERY_OFFSET0	0x23
#define RFM23_CLK_RECOVERY_TIMING_LOOP_GAIN1	0x24
#define RFM23_CLK_RECOVERY_TIMING_LOOP_GAIN0	0x25
#define RFM23_RSSI		0x26
#define RFM23_RSSI_CLCH_THRESHOLD	0x27
#define RFM23_ANT_DIVERSITY1		0x28
#define RFM23_ANT_DIVERSITY2		0x29
#define RFM23_AFC_LIMITER		0x2A
#define RFM23_AFC_CORRECTION_READ	0x2B
#define RFM23_OOK_COUNTER_VALUE1	0x2C
#define RFM23_OOK_COUNTER_VALUE2	0x2D
#define RFM23_SLICER_PEAK_HOLD		0x2E
#define RFM23_DATA_ACCESS_CONTROL	0x30
#define RFM23_EZMAC_STATUS		0x31
#define RFM23_HEADER_CTL1		0x32
#define RFM23_HEADER_CTL2		0x33
#define RFM23_PREAMBLE_LENGTH		0x34
#define RFM23_PREAMBLE_DETECTION_CTL	0x35
#define RFM23_SYNC_WORD3		0x36
#define RFM23_SYNC_WORD2		0x37
#define RFM23_SYNC_WORD1		0x38
#define RFM23_SYNC_WORD0		0x39
#define RFM23_TRANSMIT_HEADER3		0x3A
#define RFM23_TRANSMIT_HEADER2		0x3B
#define RFM23_TRANSMIT_HEADER1		0x3C
#define RFM23_TRANSMIT_HEADER0		0x3D
#define RFM23_TRANSMIT_PKT_LENGTH	0x3E
#define RFM23_CHECK_HEADER3		0x3F
#define RFM23_CHECK_HEADER2		0x40
#define RFM23_CHECK_HEADER1		0x41
#define RFM23_CHECK_HEADER0		0x42
#define RFM23_HEADER_ENABLE3		0x43
#define RFM23_HEADER_ENABLE2		0x44
#define RFM23_HEADER_ENABLE1		0x45
#define RFM23_HEADER_ENABLE0		0x46
#define RFM23_RECEIVED_HEADER3		0x47
#define RFM23_RECEIVED_HEADER2		0x48
#define RFM23_RECEIVED_HEADER1		0x49
#define RFM23_RECEIVED_HEADER0		0x4A
#define RFM23_RECEIVED_PKT_LENGTH	0x4B
#define RFM23_ADC8_CTL			0x4F
#define RFM23_CH_FILTER_COEFF_ADDR	0x60
#define RFM23_XO_CTL_TEST		0x62
#define RFM23_AGC_OVERRIDE1		0x69
#define RFM23_TX_POWER			0x6D
#define RFM23_TX_DATA_RATE1		0x6E
#define RFM23_TX_DATA_RATE0		0x6F
#define RFM23_MOD_MODE_CTL1		0x70
#define RFM23_MOD_MODE_CTL2		0x71
#define RFM23_FREQ_DEV			0x72
#define RFM23_FREQ_OFFSET1		0x73
#define RFM23_FREQ_OFFSET2		0x74
#define RFM23_FREQ_BAND_SELECT		0x75
#define RFM23_NOM_CARRIER_FREQ1		0x76
#define RFM23_NOM_CARRIER_FREQ2		0x77
#define RFM23_FH_CHANNEL_SEL		0x79
#define RFM23_FH_STEP_SIZE		0x7A
#define RFM23_TX_FIFO_CTL1		0x7C
#define RFM23_TX_FIFO_CTL2		0x7D
#define RFM23_RX_FIFO_CTL		0x7E
#define RFM23_FIFO_ACCESS		0x7F

typedef enum
{
    INACTIVE,
    IDLE,
    SHUTDOWN,
    TX_MODE,
    RX_MODE,
    TRX_MODE
}rfm23_state_t;

typedef struct
{
    SPIDriver *spi;
    rfm23_state_t state;
    // TODO: SW Tx/Rx buffers
}rfm23_t;

msg_t rfm23_init(rfm23_t *rfm, SPIDriver *driver);
void rfm23_TXEN(rfm23_t *rfm, bool en);
void rfm23_RXEN(rfm23_t *rfm ,bool en);
void rfm23_SHUTDOWN(rfm23_t *rfm, bool shutdown);
msg_t rfm23_readRegister(rfm23_t *rfm, uint8_t addr, uint8_t *data);
msg_t rfm23_writeRegister(rfm23_t *rfm, uint8_t addr, uint8_t value);
msg_t rfm23_readBurst(rfm23_t *rfm, uint8_t startAddr, uint8_t *data, uint8_t count);
msg_t rfm23_writeBurst(rfm23_t *rfm, uint8_t startAddr, uint8_t *data, uint8_t count);
msg_t rfm23_sendByte(rfm23_t *rfm, uint8_t byte);

msg_t rfm23_setTxPower(rfm23_t *rfm, uint8_t powerLevel);
int8_t rfm23_getTxPower(rfm23_t *rfm);

int16_t rfm23_getRSSI(rfm23_t *rfm);
float	rfm23_getTemperature(rfm23_t *rfm);
