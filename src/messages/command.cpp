#include "command.h"

Command::Command(uint8_t *buffer, size_t size) {
	this->buffer = buffer;
	this->size = size;
	this->parameters = &buffer[5];
}
Command::~Command() {
	free(this->buffer);
}
CommandType Command::getCommandType() {
	return (CommandType)((this->buffer[3] << 8) | this->buffer[4]);
}

uint8_t *Command::getParameters() {
	return this->parameters;
}

uint8_t *Command::getBuffer() {
	return this->buffer;
}

size_t Command::getParametersSize() {
	return this->size-4;
}

Parameters::Parameters(uint8_t* buffer,uint8_t size) {
	this->size = size;
	this->buffer = buffer;
}

ProtocolParameters::ProtocolParameters(uint8_t* buffer,uint8_t size) : Parameters(buffer,size) {
	if (buffer[0] == 0x00) {
		this->protocol = BLE_PROTOCOL;
	}
	else if (buffer[0] == 0x01) {
		this->protocol = DOT15D4_PROTOCOL;
	}
	else if (buffer[0] == 0x02) {
		this->protocol = ESB_PROTOCOL;
	}
	else if (buffer[0] == 0x03) {
		this->protocol = GENERIC_PROTOCOL;
	}
}

Protocol ProtocolParameters::getProtocol() {
	return this->protocol;
}


ChannelParameters::ChannelParameters(uint8_t* buffer,uint8_t size) : Parameters(buffer,size) {
	this->channel = buffer[0];
}

uint8_t ChannelParameters::getChannel() {
	return this->channel;
}

BooleanParameters::BooleanParameters(uint8_t* buffer,uint8_t size) : Parameters(buffer,size) {
	this->boolean = (buffer[0] != 0x00);
}

bool BooleanParameters::getBoolean() {
	return this->boolean;
}


PayloadParameters::PayloadParameters(uint8_t* buffer,uint8_t size) : Parameters(buffer,size) {
	this->payloadDirection = buffer[0];
	this->payloadSize = (size_t)buffer[1];
	this->payloadContent = &(buffer[2]);
}

size_t PayloadParameters::getPayloadSize() {
	return this->payloadSize;
}

uint8_t PayloadParameters::getPayloadDirection() {
	return this->payloadDirection;
}

uint8_t *PayloadParameters::getPayloadContent() {
	return this->payloadContent;
}
