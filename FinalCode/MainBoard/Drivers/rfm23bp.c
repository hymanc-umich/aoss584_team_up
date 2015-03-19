#include "rfm23bp.h"

static SPIConfig rfm23SpiCfg =
{
    NULL,
    RFM23_CS_PORT,
    RFM23_CS_PIN,
    SPI_CR1_BR_1
};