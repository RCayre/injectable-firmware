#ifndef MESSAGE_H
#define MESSAGE_H
#include <stdint.h>
#include <stdlib.h>

#define PREAMBLE 0x5A17

typedef enum MessageType {
	COMMAND = 0x00,
	RESPONSE = 0x01,
	PACKET = 0x02,
	NOTIFICATION = 0x03
} MessageType;

class Message {
	protected:
		uint8_t *buffer;
		size_t size;
		MessageType messageType;
		uint8_t *payload;

	public:
		Message(MessageType messageType, size_t payloadSize);
		uint8_t *getBuffer();
		size_t getSize();
		~Message();
};

#endif
