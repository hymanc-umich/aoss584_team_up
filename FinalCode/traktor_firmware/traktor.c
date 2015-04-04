#include "ch.h"
#include "hal.h"
#include "chvt.h"
#include "chprintf.h"
#include <chstreams.h>
#include <string.h>

#include "Drivers/xbeepro.h"

#include <stdlib.h>
#include <stdint.h>

#include "board.h"

static THD_WORKING_AREA(waXbeeRx, 1024);     // GPS thread working area
static THD_WORKING_AREA(waXbeeTxSer, 1024);
static THD_WORKING_AREA(waXbeeTxUsb, 1024);

/* Debug Serial configuration, 460k8, 8N1 */
static SerialConfig serCfg = 
{
   460800,
   0,
   0,
   0,
};

/*===========================================================================*/
/* USB related stuff.                                                        */
/*===========================================================================*/

/*
 * Endpoints to be used for USBD2.
 */
#define USBD1_DATA_REQUEST_EP           1
#define USBD1_DATA_AVAILABLE_EP         1
#define USBD1_INTERRUPT_REQUEST_EP      2

/*
 * Serial over USB Driver structure.
 */
static SerialUSBDriver SDU1;
static xbeePro_t xbee;
/*
 * USB Device Descriptor.
 */
static const uint8_t vcom_device_descriptor_data[18] = {
  USB_DESC_DEVICE       (0x0110,        /* bcdUSB (1.1).                    */
                         0x02,          /* bDeviceClass (CDC).              */
                         0x00,          /* bDeviceSubClass.                 */
                         0x00,          /* bDeviceProtocol.                 */
                         0x40,          /* bMaxPacketSize.                  */
                         0x0483,        /* idVendor (ST).                   */
                         0x5740,        /* idProduct.                       */
                         0x0200,        /* bcdDevice.                       */
                         1,             /* iManufacturer.                   */
                         2,             /* iProduct.                        */
                         3,             /* iSerialNumber.                   */
                         1)             /* bNumConfigurations.              */
};

/*
 * Device Descriptor wrapper.
 */
static const USBDescriptor vcom_device_descriptor = {
  sizeof vcom_device_descriptor_data,
  vcom_device_descriptor_data
};

/* Configuration Descriptor tree for a CDC.*/
static const uint8_t vcom_configuration_descriptor_data[67] = {
  /* Configuration Descriptor.*/
  USB_DESC_CONFIGURATION(67,            /* wTotalLength.                    */
                         0x02,          /* bNumInterfaces.                  */
                         0x01,          /* bConfigurationValue.             */
                         0,             /* iConfiguration.                  */
                         0xC0,          /* bmAttributes (self powered).     */
                         50),           /* bMaxPower (100mA).               */
  /* Interface Descriptor.*/
  USB_DESC_INTERFACE    (0x00,          /* bInterfaceNumber.                */
                         0x00,          /* bAlternateSetting.               */
                         0x01,          /* bNumEndpoints.                   */
                         0x02,          /* bInterfaceClass (Communications
                                           Interface Class, CDC section
                                           4.2).                            */
                         0x02,          /* bInterfaceSubClass (Abstract
                                         Control Model, CDC section 4.3).   */
                         0x01,          /* bInterfaceProtocol (AT commands,
                                           CDC section 4.4).                */
                         0),            /* iInterface.                      */
  /* Header Functional Descriptor (CDC section 5.2.3).*/
  USB_DESC_BYTE         (5),            /* bLength.                         */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x00),         /* bDescriptorSubtype (Header
                                           Functional Descriptor.           */
  USB_DESC_BCD          (0x0110),       /* bcdCDC.                          */
  /* Call Management Functional Descriptor. */
  USB_DESC_BYTE         (5),            /* bFunctionLength.                 */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x01),         /* bDescriptorSubtype (Call Management
                                           Functional Descriptor).          */
  USB_DESC_BYTE         (0x00),         /* bmCapabilities (D0+D1).          */
  USB_DESC_BYTE         (0x01),         /* bDataInterface.                  */
  /* ACM Functional Descriptor.*/
  USB_DESC_BYTE         (4),            /* bFunctionLength.                 */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x02),         /* bDescriptorSubtype (Abstract
                                           Control Management Descriptor).  */
  USB_DESC_BYTE         (0x02),         /* bmCapabilities.                  */
  /* Union Functional Descriptor.*/
  USB_DESC_BYTE         (5),            /* bFunctionLength.                 */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x06),         /* bDescriptorSubtype (Union
                                           Functional Descriptor).          */
  USB_DESC_BYTE         (0x00),         /* bMasterInterface (Communication
                                           Class Interface).                */
  USB_DESC_BYTE         (0x01),         /* bSlaveInterface0 (Data Class
                                           Interface).                      */
  /* Endpoint 2 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD1_INTERRUPT_REQUEST_EP|0x80,
                         0x03,          /* bmAttributes (Interrupt).        */
                         0x0008,        /* wMaxPacketSize.                  */
                         0xFF),         /* bInterval.                       */
  /* Interface Descriptor.*/
  USB_DESC_INTERFACE    (0x01,          /* bInterfaceNumber.                */
                         0x00,          /* bAlternateSetting.               */
                         0x02,          /* bNumEndpoints.                   */
                         0x0A,          /* bInterfaceClass (Data Class
                                           Interface, CDC section 4.5).     */
                         0x00,          /* bInterfaceSubClass (CDC section
                                           4.6).                            */
                         0x00,          /* bInterfaceProtocol (CDC section
                                           4.7).                            */
                         0x00),         /* iInterface.                      */
  /* Endpoint 3 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD1_DATA_AVAILABLE_EP,       /* bEndpointAddress.*/
                         0x02,          /* bmAttributes (Bulk).             */
                         0x0040,        /* wMaxPacketSize.                  */
                         0x00),         /* bInterval.                       */
  /* Endpoint 1 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD1_DATA_REQUEST_EP|0x80,    /* bEndpointAddress.*/
                         0x02,          /* bmAttributes (Bulk).             */
                         0x0040,        /* wMaxPacketSize.                  */
                         0x00)          /* bInterval.                       */
};

/*
 * Configuration Descriptor wrapper.
 */
static const USBDescriptor vcom_configuration_descriptor = {
  sizeof vcom_configuration_descriptor_data,
  vcom_configuration_descriptor_data
};

/*
 * U.S. English language identifier.
 */
static const uint8_t vcom_string0[] = {
  USB_DESC_BYTE(4),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  USB_DESC_WORD(0x0409)                 /* wLANGID (U.S. English).          */
};

/*
 * Vendor string.
 */
static const uint8_t vcom_string1[] = {
  USB_DESC_BYTE(38),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'T', 0, 'e', 0, 'a', 0, 'm', 0, ' ', 0, 'U', 0, 'p', 0, ' ', 0,
  'U', 0, ' ', 0, 'o', 0, 'f', 0, ' ', 0, 'M', 0, 'i', 0, 'c', 0,
  'h', 0, '.', 0
};

/*
 * Device Description string.
 */
static const uint8_t vcom_string2[] = {
  USB_DESC_BYTE(56),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'T', 0, 'r', 0, 'a', 0, 'k', 0, 'T', 0, 'o', 0, 'r', 0, '\n', 0,
  'B', 0, 'a', 0, 'l', 0, 'l', 0, 'o', 0, 'o', 0, 'n', 0, ' ', 0,
  'T', 0, 'r', 0, 'a', 0, 'c', 0, 'k', 0, 'e', 0, 'r', 0, ' ', 0,
  ' ', 0, ' ', 0, ' ', 0
};

/*
 * Serial Number string.
 */
static const uint8_t vcom_string3[] = {
  USB_DESC_BYTE(8),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  '0' + CH_KERNEL_MAJOR, 0,
  '0' + CH_KERNEL_MINOR, 0,
  '0' + CH_KERNEL_PATCH, 0
};

/*
 * Strings wrappers array.
 */
static const USBDescriptor vcom_strings[] = {
  {sizeof vcom_string0, vcom_string0},
  {sizeof vcom_string1, vcom_string1},
  {sizeof vcom_string2, vcom_string2},
  {sizeof vcom_string3, vcom_string3}
};

/*
 * Handles the GET_DESCRIPTOR callback. All required descriptors must be
 * handled here.
 */
static const USBDescriptor *get_descriptor(USBDriver *usbp,
                                           uint8_t dtype,
                                           uint8_t dindex,
                                           uint16_t lang) {

  (void)usbp;
  (void)lang;
  switch (dtype) {
  case USB_DESCRIPTOR_DEVICE:
    return &vcom_device_descriptor;
  case USB_DESCRIPTOR_CONFIGURATION:
    return &vcom_configuration_descriptor;
  case USB_DESCRIPTOR_STRING:
    if (dindex < 4)
      return &vcom_strings[dindex];
  }
  return NULL;
}

/**
 * @brief   IN EP1 state.
 */
static USBInEndpointState ep1instate;

/**
 * @brief   OUT EP1 state.
 */
static USBOutEndpointState ep1outstate;

/**
 * @brief   EP1 initialization structure (both IN and OUT).
 */
static const USBEndpointConfig ep1config = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  sduDataTransmitted,
  sduDataReceived,
  0x0040,
  0x0040,
  &ep1instate,
  &ep1outstate,
  2,
  NULL
};

/**
 * @brief   IN EP2 state.
 */
static USBInEndpointState ep2instate;

/**
 * @brief   EP2 initialization structure (IN only).
 */
static const USBEndpointConfig ep2config = {
  USB_EP_MODE_TYPE_INTR,
  NULL,
  sduInterruptTransmitted,
  NULL,
  0x0010,
  0x0000,
  &ep2instate,
  NULL,
  1,
  NULL
};

/*
 * Handles the USB driver global events.
 */
static void usb_event(USBDriver *usbp, usbevent_t event) {

  switch (event) {
  case USB_EVENT_RESET:
    return;
  case USB_EVENT_ADDRESS:
    return;
  case USB_EVENT_CONFIGURED:
    chSysLockFromISR();

    /* Enables the endpoints specified into the configuration.
       Note, this callback is invoked from an ISR so I-Class functions
       must be used.*/
    usbInitEndpointI(usbp, USBD1_DATA_REQUEST_EP, &ep1config);
    usbInitEndpointI(usbp, USBD1_INTERRUPT_REQUEST_EP, &ep2config);

    /* Resetting the state of the CDC subsystem.*/
    sduConfigureHookI(&SDU1);

    chSysUnlockFromISR();
    return;
  case USB_EVENT_SUSPEND:
    return;
  case USB_EVENT_WAKEUP:
    return;
  case USB_EVENT_STALLED:
    return;
  }
  return;
}

/*
 * USB driver configuration.
 */
static const USBConfig usbcfg = {
  usb_event,
  get_descriptor,
  sduRequestsHook,
  NULL
};

/*
 * Serial over USB driver configuration.
 */
static const SerialUSBConfig serusbcfg = {
  &USBD1,
  USBD1_DATA_REQUEST_EP,
  USBD1_DATA_AVAILABLE_EP,
  USBD1_INTERRUPT_REQUEST_EP
};


/**
 * @brief Initialization routine for OS and peripherals
 */
void initialize(void)
{
    halInit();    	// ChibiOS HAL initialization
    chSysInit();	// ChibiOS System Initialization

    /* 
     * Configure I/O : DEPRECATED
     */
    //boardInit();
    
    chThdSleepMilliseconds(100); // Startup wait (may be a bit long)
    /*
     * Driver Startup
     */
    
    /* Debug Serial Port Startup */
    sdStart(&DBG_SERIAL, &serCfg);	// Activate Debug serial driver
    chprintf((BaseSequentialStream *) &DBG_SERIAL, "==Traktor Ground Station v1==\n");


    // XBee Initialization
    xbeePro_init(&xbee, &COM_SERIAL);
    {
	   chprintf((BaseSequentialStream *) &DBG_SERIAL, "\nERROR: SD Initialization Failed\n");
    }
}

/**
 *
 *
 */
msg_t xbeeRxchangeThread(void *arg)
{
    (void *) arg;
    uint8_t xbeeRx[4];
    xbeeRx[0] = '-';
    while(1)
    {
        // XBee to serial
        sdRead(&COM_SERIAL, xbeeRx, 1); // Blocking Read
        chprintf((BaseSequentialStream *) &DBG_SERIAL, "%c", xbeeRx[0]);
        chprintf((BaseSequentialStream *) &SDU1, "%c", xbeeRx[0]);        // Print to 
        // Serial to XBee
        chThdYield();
        //chThdSleepMicroseconds(100);
    }
}

/**
 *
 *
 */
msg_t xbeeSerTxchangeThread(void *arg)
{
    (void *) arg;
    uint8_t xbeeTx[4];
    while(TRUE)
    {
        sdRead(&COM_SERIAL, xbeeTx, 1);
        chprintf((BaseSequentialStream *) &COM_SERIAL, "%c", xbeeTx[0]);
        chThdYield();
    }
}

/***
 *
 *
 */
msg_t xbeeUsbTxchangeThread(void *arg)
{
    (void *) arg;
    uint8_t xbeeTx;
    while(TRUE)
    {
        xbeeTx = chSequentialStreamGet((BaseSequentialStream *) &SDU1);
        chprintf((BaseSequentialStream *) &COM_SERIAL, "%c", xbeeTx);
        chThdYield();
    }
}
/**
 * Application entry point.
 */
int main(void) 
{
     initialize(); // Initialize OS/Peripherals

     // Startup chirp
     boardSetBuzzer(1);
     boardSetLED(1);
     //chThdSleepMilliseconds(10);
     boardSetBuzzer(0);
     boardSetLED(0);

      /*
       * Initializes a serial-over-USB CDC driver.
       */
      sduObjectInit(&SDU1);
      sduStart(&SDU1, &serusbcfg);

      /*
       * Activates the USB driver and then the USB bus pull-up on D+.
       * Note, a delay is inserted in order to not have to disconnect the cable
       * after a reset.
       */
      usbDisconnectBus(serusbcfg.usbp);
      chThdSleepMilliseconds(1500);
      usbStart(serusbcfg.usbp, &usbcfg);
      usbConnectBus(serusbcfg.usbp);

      /*
       * Stopping and restarting the USB in order to test the stop procedure. The
       * following lines are not usually required.
       */
      chThdSleepMilliseconds(3000);
      usbDisconnectBus(serusbcfg.usbp);
      usbStop(serusbcfg.usbp);
      chThdSleepMilliseconds(1500);
      usbStart(serusbcfg.usbp, &usbcfg);
      usbConnectBus(serusbcfg.usbp);

    chprintf((BaseSequentialStream *) &SDU1, "== TrakTor Ground Station v1 ==\n");
    //char rxBuffer[512];
    uint32_t count = 0;

    chThdCreateStatic(waXbeeRx, sizeof(waXbeeRx), NORMALPRIO, xbeeRxchangeThread, NULL);
    //chThdCreateStatic(waXbeeTxSer, sizeof(waXbeeTxSer), NORMALPRIO, xbeeSerTxchangeThread, NULL);
    //chThdCreateStatic(waXbeeTxUsb, sizeof(waXbeeTxUsb), NORMALPRIO, xbeeUsbTxchangeThread, NULL);
    while (TRUE)
    {
        chThdSleepMilliseconds(250);
        boardToggleLED();
        //chprintf((BaseSequentialStream *) &SDU1, "Pulse %d\n",count++);
        // Read xbee buffer
        // Write out to debug serial
        // Do stuff with LEDs
    }
    return 0;
    }
