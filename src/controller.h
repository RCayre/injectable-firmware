#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <stdint.h>
#include "radio_defs.h"
#include "radio.h"
#include "messages/packet.h"

class Radio;

class Controller {
	protected:
		Radio *radio;

	public:
		Controller(Radio* radio);
		void addPacket(Packet* packet);
		virtual void start() = 0;
		virtual void stop() = 0;
		virtual void onReceive(uint32_t timestamp,uint8_t size,uint8_t *buffer,CrcValue crcValue,uint8_t rssi) = 0;
		virtual void onJam(uint32_t timestamp) = 0;
};

#endif
