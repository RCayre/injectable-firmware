#ifndef CORE_H
#define CORE_H

#include <stdlib.h>
#include "version.h"
#include "messages/response.h"
#include "messages/command.h"
#include "messages/notification.h"
#include "messages/messageQueue.h"
#include "led.h"
#include "usb.h"
#include "time.h"
#include "radio.h"
#include "controller.h"
#include "controllers/blecontroller.h"

class USBModule;

class Core {
	private:
		LedModule *ledModule;
		USBModule *usbModule;
		TimeModule *timeModule;
		Radio *radio;
		MessageQueue messageQueue;

		BLEController *bleController;

		Controller* currentController;

	public:
		static Core *instance;

		Core();
		LedModule *getLedModule();
		USBModule *getUSBModule();
		TimeModule *getTimeModule();
		Radio *getRadioModule();

		void sendDebug(const char* message);
		void sendDebug(uint8_t *buffer, uint8_t size);

		bool selectController(Protocol controller);

		void pushMessageToQueue(Message *msg);
		Message* popMessageFromQueue();

		void handleInputData(uint8_t *buffer, size_t size);
		void sendMessage(Message *msg);
		void handleCommand(Command *cmd);
		void init();
		void loop();

};
#endif
