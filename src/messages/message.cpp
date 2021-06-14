#include "message.h"

Message::Message(MessageType messageType, size_t payloadSize) {
	this->size = payloadSize + 2 + 1;
	this->buffer = (uint8_t*)malloc(sizeof(uint8_t)*(this->size));
	this->buffer[0] = (uint8_t)((PREAMBLE & 0xFF00) >> 8);
	this->buffer[1] = (uint8_t)(PREAMBLE & 0x00FF);
	this->buffer[2] = (uint8_t)messageType;
	this->payload = &(this->buffer[3]);
}

uint8_t *Message::getBuffer() {
	return this->buffer;
}

size_t Message::getSize() {
	return this->size;
}

Message::~Message() {
	free(this->buffer);
}
