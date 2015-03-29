/*
    ChibiOS/RT - Copyright (C) 2006-2014 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "hal.h"

#include "sdmmcsimple.h"

#if HAL_USE_PAL || defined(__DOXYGEN__)
/**
 * @brief   PAL setup.
 * @details Digital I/O ports static configuration as defined in @p board.h.
 *          This variable is used by the HAL when initializing the PAL driver.
 */
const PALConfig pal_default_config =
{
#if STM32_HAS_GPIOA
  {VAL_GPIOA_MODER, VAL_GPIOA_OTYPER, VAL_GPIOA_OSPEEDR, VAL_GPIOA_PUPDR,
   VAL_GPIOA_ODR,   VAL_GPIOA_AFRL,   VAL_GPIOA_AFRH},
#endif
#if STM32_HAS_GPIOB
  {VAL_GPIOB_MODER, VAL_GPIOB_OTYPER, VAL_GPIOB_OSPEEDR, VAL_GPIOB_PUPDR,
   VAL_GPIOB_ODR,   VAL_GPIOB_AFRL,   VAL_GPIOB_AFRH},
#endif
#if STM32_HAS_GPIOC
  {VAL_GPIOC_MODER, VAL_GPIOC_OTYPER, VAL_GPIOC_OSPEEDR, VAL_GPIOC_PUPDR,
   VAL_GPIOC_ODR,   VAL_GPIOC_AFRL,   VAL_GPIOC_AFRH},
#endif
#if STM32_HAS_GPIOD
  {VAL_GPIOD_MODER, VAL_GPIOD_OTYPER, VAL_GPIOD_OSPEEDR, VAL_GPIOD_PUPDR,
   VAL_GPIOD_ODR,   VAL_GPIOD_AFRL,   VAL_GPIOD_AFRH},
#endif
#if STM32_HAS_GPIOE
  {VAL_GPIOE_MODER, VAL_GPIOE_OTYPER, VAL_GPIOE_OSPEEDR, VAL_GPIOE_PUPDR,
   VAL_GPIOE_ODR,   VAL_GPIOE_AFRL,   VAL_GPIOE_AFRH},
#endif
#if STM32_HAS_GPIOF
  {VAL_GPIOF_MODER, VAL_GPIOF_OTYPER, VAL_GPIOF_OSPEEDR, VAL_GPIOF_PUPDR,
   VAL_GPIOF_ODR,   VAL_GPIOF_AFRL,   VAL_GPIOF_AFRH},
#endif
#if STM32_HAS_GPIOG
  {VAL_GPIOG_MODER, VAL_GPIOG_OTYPER, VAL_GPIOG_OSPEEDR, VAL_GPIOG_PUPDR,
   VAL_GPIOG_ODR,   VAL_GPIOG_AFRL,   VAL_GPIOG_AFRH},
#endif
#if STM32_HAS_GPIOH
  {VAL_GPIOH_MODER, VAL_GPIOH_OTYPER, VAL_GPIOH_OSPEEDR, VAL_GPIOH_PUPDR,
   VAL_GPIOH_ODR,   VAL_GPIOH_AFRL,   VAL_GPIOH_AFRH},
#endif
#if STM32_HAS_GPIOI
  {VAL_GPIOI_MODER, VAL_GPIOI_OTYPER, VAL_GPIOI_OSPEEDR, VAL_GPIOI_PUPDR,
   VAL_GPIOI_ODR,   VAL_GPIOI_AFRL,   VAL_GPIOI_AFRH}
#endif
};
#endif

/**
 * @brief   Early initialization code.
 * @details This initialization must be performed just after stack setup
 *          and before any other initialization.
 */
void __early_init(void) 
{
  stm32_clock_init();
}

#if HAL_USE_SDC || defined(__DOXYGEN__)
/**
 * @brief   SDC card detection.
 */
bool sdc_lld_is_card_inserted(SDCDriver *sdcp) 
{
  (void)sdcp;
  /* TODO: Fill the implementation.*/
  return TRUE;
}

/**
 * @brief   SDC card write protection detection.
 */
bool sdc_lld_is_write_protected(SDCDriver *sdcp) 
{
  (void)sdcp;
  /* TODO: Fill the implementation.*/
  return FALSE;
}
#endif /* HAL_USE_SDC */

#if HAL_USE_MMC_SPI || defined(__DOXYGEN__)
/**
 * @brief   MMC_SPI card detection.
 */
bool mmc_lld_is_card_inserted(MMCDriver *mmcp) 
{
  (void)mmcp;
  return palReadPad(SD_CD_PORT, SD_CD_PIN);
}
/**
 * @brief   MMC_SPI card write protection detection.
 */
bool mmc_lld_is_write_protected(MMCDriver *mmcp) 
{
  (void)mmcp;
  return FALSE;
}
#endif

/**
 * @brief   Board-specific I/O initialization
 */
void boardInit(void)
{
    // SD SPI
    palSetPadMode(SD_CS_PORT, SD_CS_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(SD_CD_PORT, SD_CD_PIN, PAL_MODE_INPUT_PULLUP);
    palSetPadMode(SD_SCK_PORT, SD_SCK_PIN, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(SD_MOSI_PORT, SD_MOSI_PIN, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
    palSetPadMode(SD_MISO_PORT, SD_MISO_PIN, PAL_MODE_ALTERNATE(5));
    
    // Serial I/O Cfg
    palSetPadMode(DBG_PORT, DBG_TX_PIN, PAL_MODE_ALTERNATE(8)); // Debug Tx
    palSetPadMode(DBG_PORT, DBG_RX_PIN, PAL_MODE_ALTERNATE(8));	// Debug Rx
    palSetPadMode(GPS_PORT, GPS_TX_PIN, PAL_MODE_ALTERNATE(7)); // GPS Tx
    palSetPadMode(GPS_PORT, GPS_RX_PIN, PAL_MODE_ALTERNATE(7)); // GPS Rx
    palSetPadMode(COM_PORT, COM_TX_PIN, PAL_MODE_ALTERNATE(7)); // COM Tx
    palSetPadMode(COM_PORT, COM_RX_PIN, PAL_MODE_ALTERNATE(7)); // COM Rx
    
    // I2C
    palSetPadMode(II2C_PORT, II2C_SDA_PIN, PAL_MODE_ALTERNATE(4));
    palSetPadMode(II2C_PORT, II2C_SCL_PIN, PAL_MODE_ALTERNATE(4));
    palSetPadMode(EI2C_SDA_PORT, EI2C_SDA_PIN, PAL_MODE_ALTERNATE(4));
    palSetPadMode(EI2C_SCL_PORT, EI2C_SCL_PIN, PAL_MODE_ALTERNATE(4));
    
    // Audio beacon
    palSetPadMode(PIEZO_PORT, PIEZO_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
    
    // Trigger- LED
    palSetPadMode(LED_TRIG_PORT ,LED_TRIG_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
}

/**
 * 
 */
void boardSetBuzzer(bool on)
{
    if(on)
	palSetPad(PIEZO_PORT, PIEZO_PIN);
    else
	palClearPad(PIEZO_PORT, PIEZO_PIN);
}

/**
 * 
 */
void boardSetLED(bool on)
{
    if(on)
	palSetPad(LED_TRIG_PORT, LED_TRIG_PIN);
    else
	palClearPad(LED_TRIG_PORT, LED_TRIG_PIN);
}