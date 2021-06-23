/* Device descriptor */
#define EP0_MAXPACKETSIZE NRF_DRV_USBD_EPSIZE
#define USBD_DEVICE_DESCRIPTOR \
	0x12,                        /* bLength | size of descriptor                                                  */\
	0x01,                        /* bDescriptorType | descriptor type                                             */\
	0x00, 0x02,                  /* bcdUSB | USB spec release (ver 2.0)                                           */\
	0xFF,                        /* bDeviceClass ¦ class code (each interface specifies class information)        */\
	0xFF,                        /* bDeviceSubClass ¦ device sub-class (must be set to 0 because class code is 0) */\
	0xFF,                        /* bDeviceProtocol | device protocol (no class specific protocol)                */\
	EP0_MAXPACKETSIZE,           /* bMaxPacketSize0 | maximum packet size (64 bytes)                              */\
	0x17, 0x5A,                  /* vendor ID  (0x1915 Nordic)                                                    */\
	0x00, 0x00,                  /* product ID (0x520A nRF52 HID mouse on nrf_drv)                                */\
	0x01, 0x00,                  /* bcdDevice | final device release number in BCD Format                         */\
	USBD_STRING_MANUFACTURER_IX, /* iManufacturer | index of manufacturer string                                  */\
	USBD_STRING_PRODUCT_IX,      /* iProduct | index of product string                                            */\
	USBD_STRING_SERIAL_IX,       /* iSerialNumber | Serial Number string                                          */\
	0x01                         /* bNumConfigurations | number of configurations                                 */


/* String descriptors */
#define USBD_STRING_LANG_IX  0x00
#define USBD_STRING_LANG \
	0x04,         /* length of descriptor                   */\
	0x03,         /* descriptor type                        */\
	0x09,         /*                                        */\
	0x04          /* Supported LangID = 0x0409 (US-English) */

#define USBD_STRING_MANUFACTURER_IX  0x01
#define USBD_STRING_MANUFACTURER \
	30,           /* length of descriptor (? bytes)   */\
	0x03,         /* descriptor type                  */\
	'M', 0x00,    /* Define Unicode String "Nordic Semiconductor  */\
	'i', 0x00, \
	'r', 0x00, \
	'a', 0x00, \
	'g', 0x00, \
	'e', 0x00, \
	' ', 0x00, \
	'T', 0x00, \
	'o', 0x00, \
	'o', 0x00, \
	'l', 0x00, \
	'k', 0x00, \
	'i', 0x00, \
	't', 0x00

#define USBD_STRING_PRODUCT_IX  0x02
#define USBD_STRING_PRODUCT \
	23,           /* length of descriptor (? bytes)         */\
	0x03,         /* descriptor type                        */\
	'B', 0x00,    /* generic unicode string for all devices */\
	'u', 0x00, \
	't', 0x00, \
	't', 0x00, \
	'e', 0x00, \
	'R', 0x00, \
	'F', 0x00, \
	'l', 0x00, \
	'y', 0x00

#define USBD_STRING_SERIAL_IX  0x00

/* Configuration descriptors */
#define DEVICE_SELF_POWERED 1
#define REMOTE_WU           1

#define USBD_CONFIG_DESCRIPTOR_SIZE   9
#define USBD_CONFIG_DESCRIPTOR_FULL_SIZE   (9 + (9 + 7 + 7))
#define USBD_CONFIG_DESCRIPTOR  \
	0x09,         /* bLength | length of descriptor                                             */\
	0x02,         /* bDescriptorType | descriptor type (CONFIGURATION)                          */\
	USBD_CONFIG_DESCRIPTOR_FULL_SIZE, 0x00,    /* wTotalLength | total length of descriptor(s)  */\
	0x01,         /* bNumInterfaces                                                             */\
	0x01,         /* bConfigurationValue                                                        */\
	0x00,         /* index of string Configuration | configuration string index (not supported) */\
	0x80| (((DEVICE_SELF_POWERED) ? 1U:0U)<<6) | (((REMOTE_WU) ? 1U:0U)<<5), /* bmAttributes    */\
	49            /* maximum power in steps of 2mA (98mA)                                       */

#define USBD_INTERFACE0_DESCRIPTOR  \
	0x09,         /* bLength                                                                          */\
	0x04,         /* bDescriptorType | descriptor type (INTERFACE)                                    */\
	0x00,         /* bInterfaceNumber                                                                 */\
	0x00,         /* bAlternateSetting                                                                */\
	0x02,         /* bNumEndpoints | number of endpoints (2)                                          */\
	0xFF,         /* bInterfaceClass | interface class (3..defined by USB spec: HID)                  */\
	0xFF,         /* bInterfaceSubClass |interface sub-class (0.. no boot interface)                  */\
	0xFF,         /* bInterfaceProtocol | interface protocol (1..defined by USB spec: mouse)          */\
	0x00          /* interface string index (not supported)                                           */

#define USBD_ENDPOINT1_IN_DESCRIPTOR  \
	0x07,         /* bLength | length of descriptor (7 bytes)                                     */\
	0x05,         /* bDescriptorType | descriptor type (ENDPOINT)                                 */\
	0x81,         /* bEndpointAddress | endpoint address (IN endpoint, endpoint 1)                */\
	0x02,         /* bmAttributes | endpoint attributes (bulk)                                  peut être 3 ?  */\
	0x40,0x00,    /* bMaxPacketSizeLowByte,bMaxPacketSizeHighByte | maximum packet size (8 bytes) */\
	0x00          /* bInterval | polling interval                                                 */

#define USBD_ENDPOINT1_OUT_DESCRIPTOR  \
	0x07,         /* bLength | length of descriptor (7 bytes)                                     */\
	0x05,         /* bDescriptorType | descriptor type (ENDPOINT)                                 */\
	0x01,         /* bEndpointAddress | endpoint address (OUT endpoint, endpoint 1)                */\
	0x02,         /* bmAttributes | endpoint attributes (bulk)                                  peut être 3 ?  */\
	0x40,0x00,    /* bMaxPacketSizeLowByte,bMaxPacketSizeHighByte | maximum packet size (8 bytes) */\
	0x00          /* bInterval | polling interval                                                 */




static const uint8_t descriptor_device[] = {
	USBD_DEVICE_DESCRIPTOR
};

static const uint8_t descriptor_configuration[] = {
	USBD_CONFIG_DESCRIPTOR,
	USBD_INTERFACE0_DESCRIPTOR,
	USBD_ENDPOINT1_IN_DESCRIPTOR,
	USBD_ENDPOINT1_OUT_DESCRIPTOR
};
static const uint8_t descriptor_string_lang[] = {
	USBD_STRING_LANG
};
static const uint8_t descriptor_string_manuf[] = {
	USBD_STRING_MANUFACTURER
};
static const uint8_t descriptor_string_prod[] = {
	USBD_STRING_PRODUCT
};

static const uint8_t config_resp_configured[]   = {1};
static const uint8_t config_resp_unconfigured[] = {0};

static const uint8_t status_device_resp_nrwu[] = {
	((DEVICE_SELF_POWERED) ? 1 : 0), //LSB first: self-powered, no remoteWk
	0
};
static const uint8_t status_device_resp_rwu[]  = {
	((DEVICE_SELF_POWERED) ? 1 : 0) | 2, //LSB first: self-powered, remoteWk
	0
};

static const uint8_t status_interface_resp[] = {0, 0};
static const uint8_t status_ep_halted_resp[] = {1, 0};
static const uint8_t status_ep_active_resp[] = {0, 0};


#define GET_CONFIG_DESC_SIZE    sizeof(descriptor_configuration)
#define GET_INTERFACE_DESC_SIZE 9
#define GET_ENDPOINT_IN_DESC_SIZE  7
#define GET_ENDPOINT_OUT_DESC_SIZE  7

#define descriptor_interface_0 \
	&descriptor_configuration[9]
#define descriptor_in_endpoint_1 \
	&descriptor_configuration[9+GET_INTERFACE_DESC_SIZE]
#define descriptor_out_endpoint_1 \
	&descriptor_configuration[9+GET_INTERFACE_DESC_SIZE+GET_ENDPOINT_IN_DESC_SIZE]


#define GET_STATUS 0x00
#define CLEAR_FEATURE 0x01
#define SET_FEATURE 0x03
#define SET_ADDRESS 0x05
#define GET_DESCRIPTOR 0x06
#define GET_CONFIG 0x08
#define SET_CONFIG 0x09
