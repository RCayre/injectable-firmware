#include "usb.h"
/* Global instance of USB module */
USBModule* USBModule::instance = NULL;

USBModule::USBModule(CoreCallback inputCallback,Core *coreInstance) {
	this->inputCallback = inputCallback;
	this->coreInstance = coreInstance;
	this->configured = false;
	this->suspended = false;
	this->remoteWakeUpEnabled = false;
	this->suspendStateRequest = false;
	this->systemOffRequest = false;
	instance = this;
}

void USBModule::setConfigured(bool configured) {
	this->configured = configured;
}
void USBModule::setSuspended(bool suspended) {
	this->suspended = suspended;
}

void USBModule::setRemoteWakeUpEnabled(bool rwuEnabled) {
	this->remoteWakeUpEnabled = rwuEnabled;
}
void USBModule::setSuspendStateRequest(bool suspendStateRequest) {
	this->suspendStateRequest = suspendStateRequest;
}
void USBModule::setSystemOffRequest(bool systemOffRequest) {
	this->systemOffRequest = systemOffRequest;
}
bool USBModule::respondSetupData(nrf_drv_usbd_setup_t const * const setup,void const * data, size_t size) {
	/* Check the size against required response size */
	if (size > setup->wLength) {
		size = setup->wLength;
	}
	ret_code_t ret;
	nrf_drv_usbd_transfer_t transfer = {
		.p_data = {.tx = data},
		.size = size
	};
	ret = nrf_drv_usbd_ep_transfer(NRF_DRV_USBD_EPIN0, &transfer);
	return (ret == NRF_SUCCESS);
}

bool USBModule::endPointConfiguration(uint8_t index) {
	if ( index == 1 ) {
		nrf_drv_usbd_ep_dtoggle_clear(NRF_DRV_USBD_EPIN1);
		nrf_drv_usbd_ep_stall_clear(NRF_DRV_USBD_EPIN1);
		nrf_drv_usbd_ep_enable(NRF_DRV_USBD_EPIN1);

		nrf_drv_usbd_ep_dtoggle_clear(NRF_DRV_USBD_EPOUT1);
		nrf_drv_usbd_ep_stall_clear(NRF_DRV_USBD_EPOUT1);
		nrf_drv_usbd_ep_enable(NRF_DRV_USBD_EPOUT1);
		this->setConfigured(true);
		nrf_drv_usbd_setup_clear();
	}
	else if ( index == 0 ) {
		nrf_drv_usbd_ep_disable(NRF_DRV_USBD_EPIN1);
		nrf_drv_usbd_ep_disable(NRF_DRV_USBD_EPOUT1);
		this->setConfigured(false);
		nrf_drv_usbd_setup_clear();
	}
	else {
		return false;
	}
	return true;
}
void USBModule::setupGetStatus(nrf_drv_usbd_setup_t const * const setup) {
	if (setup->bmRequestType == 0x80) { // Device
		if (((setup->wIndex) & 0xff) == 0) {
			this->respondSetupData(
				setup,
				this->remoteWakeUpEnabled ? status_device_resp_rwu : status_device_resp_nrwu,
				sizeof(status_device_resp_nrwu));
			return;
		}
	}
	else if (setup->bmRequestType == 0x81) { // Interface
		// The response is transmitted only if the module is configured
		if (this->configured) {
			if (((setup->wIndex) & 0xff) == 0) {// Only interface 0 supported
				this->respondSetupData(
					setup,
					status_interface_resp,
					sizeof(status_interface_resp));
				return;
			}
		}
	}
	else if (setup->bmRequestType == 0x82) { // End point
		if (((setup->wIndex) & 0xff) == 0) { // Endpoint 0
			this->respondSetupData(
				setup,
				status_ep_active_resp,
				sizeof(status_ep_active_resp));
			return;
		}
		if (this->configured) { // Other endpoints responds if configured
			if (((setup->wIndex) & 0xff) == NRF_DRV_USBD_EPIN1) {
				if (nrf_drv_usbd_ep_stall_check(NRF_DRV_USBD_EPIN1)) {
					this->respondSetupData(
						setup,
						status_ep_halted_resp,
						sizeof(status_ep_halted_resp));
					return;
				}
				else {
					this->respondSetupData(
						setup,
						status_ep_active_resp,
						sizeof(status_ep_active_resp));
					return;
				}
			}
		}
	}
	nrf_drv_usbd_setup_stall();
}

void USBModule::setupClearFeature(nrf_drv_usbd_setup_t const * const setup) {
	if ((setup->bmRequestType) == 0x02) { // standard request, recipient=endpoint
		if ((setup->wValue) == 0) {
			if ((setup->wIndex) == NRF_DRV_USBD_EPIN1) {
				nrf_drv_usbd_ep_stall_clear(NRF_DRV_USBD_EPIN1);
				nrf_drv_usbd_setup_clear();
				return;
			}
		}
	}
	else if ((setup->bmRequestType) ==  0x0) { // standard request, recipient=device
		if (REMOTE_WU) {
			if ((setup->wValue) == 1) {// Feature Wakeup
				this->setRemoteWakeUpEnabled(false);
				nrf_drv_usbd_setup_clear();
				return;
			}
		}
	}
	nrf_drv_usbd_setup_stall();
}

void USBModule::setupSetFeature(nrf_drv_usbd_setup_t const * const setup) {
	if ((setup->bmRequestType) == 0x02) {// standard request, recipient=endpoint
		if ((setup->wValue) == 0) {// Feature HALT
			if ((setup->wIndex) == NRF_DRV_USBD_EPIN1) {
				nrf_drv_usbd_ep_stall(NRF_DRV_USBD_EPIN1);
				nrf_drv_usbd_setup_clear();
				return;
			}
		}
	}
	else if ((setup->bmRequestType) ==  0x0) {// standard request, recipient=device
		if (REMOTE_WU) {
			if ((setup->wValue) == 1) {// Feature Wakeup
				this->setRemoteWakeUpEnabled(true);
				nrf_drv_usbd_setup_clear();
				return;
			}
		}
	}
	nrf_drv_usbd_setup_stall();
}

void USBModule::setupGetDescriptor(nrf_drv_usbd_setup_t const * const setup) {
	//determine which descriptor has been asked for
	switch ((setup->wValue) >> 8) {
		case 1: // Device
			if ((setup->bmRequestType) == 0x80) {
				this->respondSetupData(
					setup,
					descriptor_device,
					sizeof(descriptor_device));
				return;
			}
			break;
		case 2: // Configuration
			if ((setup->bmRequestType) == 0x80) {
				this->respondSetupData(
					setup,
					descriptor_configuration,
					GET_CONFIG_DESC_SIZE);
				return;
			}
			break;
		case 3: // String
			if ((setup->bmRequestType) == 0x80) {
				// Select the string
				switch ((setup->wValue) & 0xFF) {
					case USBD_STRING_LANG_IX:
						this->respondSetupData(
							setup,
							descriptor_string_lang,
							sizeof(descriptor_string_lang));
						return;
					case USBD_STRING_MANUFACTURER_IX:
						this->respondSetupData(
							setup,
							descriptor_string_manuf,
							sizeof(descriptor_string_manuf));
						return;
					case USBD_STRING_PRODUCT_IX:
						this->respondSetupData(
							setup,
							descriptor_string_prod,
							sizeof(descriptor_string_prod));
						return;
					default:
						break;
				}
			}
			break;
		case 4: // Interface
			if ((setup->bmRequestType) == 0x80) {
				// Which interface?
				if ((((setup->wValue) & 0xFF) == 0)) {
					this->respondSetupData(
						setup,
						descriptor_interface_0,
						GET_INTERFACE_DESC_SIZE);
					return;
				}
			}
			break;
		case 5: // Endpoint
			if ((setup->bmRequestType) == 0x80) {
				// Which endpoint?
				if (((setup->wValue) & 0xFF) == 1) {
					this->respondSetupData(
						setup,
						descriptor_in_endpoint_1,
						GET_ENDPOINT_IN_DESC_SIZE);
					return;
				}
			}
			break;
		default:
			break; // Not supported - go to stall
	}

	// Unknown descriptor requested
	nrf_drv_usbd_setup_stall();
}
void USBModule::setupGetConfig(nrf_drv_usbd_setup_t const * const setup) {
	if (this->configured) {
		this->respondSetupData(
			setup,
			config_resp_configured,
			sizeof(config_resp_configured));
	}
	else {
		this->respondSetupData(
			setup,
			config_resp_unconfigured,
			sizeof(config_resp_unconfigured));
	}
}
void USBModule::setupSetConfig(nrf_drv_usbd_setup_t const * const setup) {
	if ((setup->bmRequestType) == 0x00) {
		// accept only 0 and 1
		if (((setup->wIndex) == 0) && ((setup->wLength) == 0) && ((setup->wValue) <= UINT8_MAX)) {
			if (this->endPointConfiguration((uint8_t)(setup->wValue))) {
				nrf_drv_usbd_setup_clear();
				return;
			}
		}
	}
	nrf_drv_usbd_setup_stall();
}

void USBModule::powerUsbEventHandler(nrf_drv_power_usb_evt_t event) {
	USBModule* that = USBModule::instance;
	if (event == NRF_DRV_POWER_USB_EVT_DETECTED) {
		// USB power detected
		if (!nrf_drv_usbd_is_enabled()) {
			nrf_drv_usbd_enable();
		}
	}
	else if (event == NRF_DRV_POWER_USB_EVT_REMOVED) {
		// USB power removed
		that->setConfigured(false);
		if (nrf_drv_usbd_is_started()) {
			nrf_drv_usbd_stop();
		}
		if (nrf_drv_usbd_is_enabled()) {
			nrf_drv_usbd_disable();
		}
	}
	else if (event == NRF_DRV_POWER_USB_EVT_READY) {
		// USB Ready
		if (!nrf_drv_usbd_is_started()) {
			nrf_drv_usbd_start(true);
		}
	}
}

void USBModule::setRx(bool rx) {
	this->rx = rx;
}
void USBModule::usbdEventHandler(nrf_drv_usbd_evt_t const * const p_event) {
	USBModule* that = USBModule::instance;
	switch(p_event->type) {
		case NRF_DRV_USBD_EVT_SUSPEND:
			that->setSuspendStateRequest(true);
			break;
		case NRF_DRV_USBD_EVT_RESUME:
			that->setSuspendStateRequest(false);
			break;
		case NRF_DRV_USBD_EVT_WUREQ:
			that->setSuspendStateRequest(false);
			break;
		case NRF_DRV_USBD_EVT_RESET:
		{
			bool success = that->endPointConfiguration(0);
			if (success) {
				that->setSuspendStateRequest(false);
			}
			break;
		}
		case NRFX_USBD_EVT_EPTRANSFER:
			if (NRF_DRV_USBD_EPIN0 == p_event->data.eptransfer.ep) {
				if (NRF_USBD_EP_OK == p_event->data.eptransfer.status) {
					// Transfer OK
					nrf_drv_usbd_setup_clear();
				}
				else if (NRF_USBD_EP_ABORTED == p_event->data.eptransfer.status) {
					// Transfer aborted
				}
				else {
					// Transfer failed
					nrf_drv_usbd_setup_stall();
				}
			}
			else if (NRF_DRV_USBD_EPOUT0 == p_event->data.eptransfer.ep) {
				if (NRF_USBD_EP_OK == p_event->data.eptransfer.status) {
					// Transfer OK
					nrf_drv_usbd_setup_clear();
				}
				else if (NRF_USBD_EP_ABORTED == p_event->data.eptransfer.status) {
					// Transfer aborted
				}
				else {
					// Transfer failed
					nrf_drv_usbd_setup_stall();
				}
			}
			else if (NRF_DRV_USBD_EPIN1 == p_event->data.eptransfer.ep) {
			// pass
			}
			else if (NRF_DRV_USBD_EPOUT1 == p_event->data.eptransfer.ep) {
				nrf_drv_usbd_ep_transfer( NRF_DRV_USBD_EPOUT1, that->getRxTransfer());
				that->rxSize = nrf_drv_usbd_epout_size_get(NRF_DRV_USBD_EPOUT1);
				(that->coreInstance ->* that->inputCallback)(that->rxBuffer, that->rxSize);
			}
			break;
		case NRFX_USBD_EVT_SETUP:
		{
			nrf_drv_usbd_setup_t setup;
			nrf_drv_usbd_setup_get(&setup);
			switch (setup.bRequest)
			{
				case GET_STATUS:
					that->setupGetStatus(&setup);
					break;
				case CLEAR_FEATURE:
					that->setupClearFeature(&setup);
					break;
				case SET_FEATURE:
					that->setupSetFeature(&setup);
					break;
				case SET_ADDRESS:
					// Handled by hardware, do nothing (don't stall)
					break;
				case GET_DESCRIPTOR:
					that->setupGetDescriptor(&setup);
					break;
				case GET_CONFIG:
					that->setupGetConfig(&setup);
					break;
				case SET_CONFIG:
					that->setupSetConfig(&setup);
					break;
				default:
					// Unknown request
					nrf_drv_usbd_setup_stall();
					break;
			}
			break;
		}
		default:
			break;

	}
}

nrf_drv_usbd_transfer_t* USBModule::getRxTransfer() {
	return &this->rxTransfer;
}


void USBModule::init() {

	/* Initializing power and clock */
	ret_code_t ret = nrf_drv_clock_init();
	APP_ERROR_CHECK(ret);
	ret = nrf_drv_power_init(NULL);
	APP_ERROR_CHECK(ret);
	nrf_drv_clock_hfclk_request(NULL);
	nrf_drv_clock_lfclk_request(NULL);
	while (!(nrf_drv_clock_hfclk_is_running() && nrf_drv_clock_lfclk_is_running()));
	ret = app_timer_init();
	APP_ERROR_CHECK(ret);
	/* Avoid warnings if assertion is disabled */
	//UNUSED_VARIABLE(ret);

	ret = nrf_drv_usbd_init(USBModule::usbdEventHandler);
	APP_ERROR_CHECK(ret);
	/* Configure selected size of the packed on EP0 and EP1*/
	nrf_drv_usbd_ep_max_packet_size_set(NRF_DRV_USBD_EPOUT0, EP0_MAXPACKETSIZE);
	nrf_drv_usbd_ep_max_packet_size_set(NRF_DRV_USBD_EPIN0, EP0_MAXPACKETSIZE);

	nrf_drv_usbd_ep_max_packet_size_set(NRF_DRV_USBD_EPOUT1, EP0_MAXPACKETSIZE);
	nrf_drv_usbd_ep_max_packet_size_set(NRF_DRV_USBD_EPIN1, EP0_MAXPACKETSIZE);


	this->rxTransfer = { .p_data = { .rx = this->rxBuffer  }, .size = 64 };

	if (USBD_POWER_DETECTION) {
		static const nrf_drv_power_usbevt_config_t config = {
			.handler = USBModule::powerUsbEventHandler
		};
		ret = nrf_drv_power_usbevt_init(&config);
		APP_ERROR_CHECK(ret);
	}
	else {
		// No USB power detection enabled, starting USB now
		nrf_delay_us(STARTUP_DELAY);
		if (!nrf_drv_usbd_is_enabled()) {
			nrf_drv_usbd_enable();
			this->endPointConfiguration(0);
		}
		// Wait for regulator Power Up
		while (NRF_DRV_POWER_USB_STATE_CONNECTED == nrf_drv_power_usbstatus_get());

		if (NRF_DRV_POWER_USB_STATE_READY == nrf_drv_power_usbstatus_get()) {
			if (!nrf_drv_usbd_is_started()) {
				nrf_drv_usbd_start(true);
			}
		}
		else {
			nrf_drv_usbd_disable();
		}
	}
}

bool USBModule::sendData(uint8_t *data, uint8_t size) {
	ret_code_t ret;
	nrf_drv_usbd_transfer_t transfer = {
		.p_data = {.tx = data},
		.size = size
	};
	ret = nrf_drv_usbd_ep_transfer(NRF_DRV_USBD_EPIN1, &transfer);
	return (ret == NRF_SUCCESS);
}
bool USBModule::isConfigured() {
	return this->configured;
}

void USBModule::managePower() {
	if (this->systemOffRequest) {
		// Going to system off
		nrf_power_system_off();
	}

	if (this->suspended != this->suspendStateRequest) {
		if (this->suspendStateRequest) {
			nrf_drv_usbd_suspend();
		}
		else {
			this->setSuspended(false);
		}
	}
}
