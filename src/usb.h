#ifndef USB_H
#define USB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "nrf.h"
#include "nrf_drv_usbd.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"
#include "nrf_drv_power.h"
#include "app_timer.h"
#include "app_error.h"
#include "usb_descriptors.h"
#include "boards.h"
#include "core.h"

class Core;

typedef void (Core::*CoreCallback) (uint8_t*, size_t);

#define STARTUP_DELAY 100

#ifndef USBD_POWER_DETECTION
	#define USBD_POWER_DETECTION true
#endif

class USBModule {
	private:
		uint8_t rxBuffer[64];
		nrf_drv_usbd_transfer_t rxTransfer;
		size_t rxSize;

		bool rx;

		bool configured;
		bool suspended;
		bool remoteWakeUpEnabled;
		bool suspendStateRequest;
		bool systemOffRequest;
	public:
		CoreCallback inputCallback;
		Core *coreInstance;
		static USBModule *instance;

		USBModule(CoreCallback inputCallback,Core *coreInstance);

		void init();
		void managePower();
		bool isConfigured();
		size_t receive(uint8_t *buffer);

		/* Transfer related */
		nrf_drv_usbd_transfer_t *getRxTransfer();
		void setRx(bool rx);
		bool sendData(uint8_t *data, uint8_t size);

		/* Flags setters */
		void setConfigured(bool configured);
		void setSuspended(bool suspended);
		void setRemoteWakeUpEnabled(bool rwuEnabled);
		void setSuspendStateRequest(bool suspendStateRequest);
		void setSystemOffRequest(bool systemOffRequest);

		/* Handlers */
		static void powerUsbEventHandler(nrf_drv_power_usb_evt_t event);
		static void usbdEventHandler(nrf_drv_usbd_evt_t const * const p_event);

		/* Setup and configuration */
		bool endPointConfiguration(uint8_t index);
		void setupGetStatus(nrf_drv_usbd_setup_t const * const setup);
		void setupClearFeature(nrf_drv_usbd_setup_t const * const setup);
		void setupSetFeature(nrf_drv_usbd_setup_t const * const setup);
		void setupGetDescriptor(nrf_drv_usbd_setup_t const * const setup);
		void setupGetConfig(nrf_drv_usbd_setup_t const * const setup);
		void setupSetConfig(nrf_drv_usbd_setup_t const * const setup);
		bool respondSetupData(nrf_drv_usbd_setup_t const * const setup,void const * data, size_t size);
};
#endif
