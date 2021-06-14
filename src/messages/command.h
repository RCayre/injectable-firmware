#ifndef COMMAND_H
#define COMMAND_H
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include "radio_defs.h"

typedef enum CommandType {
	GET_VERSION = 0x0000,
	SELECT_CONTROLLER = 0x0001,
	ENABLE_CONTROLLER = 0x0002,
	DISABLE_CONTROLLER = 0x0003,
	GET_CHANNEL = 0x0004,
	SET_CHANNEL = 0x0005,
	SET_FILTER = 0x0006,
	SET_FOLLOW_MODE = 0x0007,
	START_ATTACK = 0x0008,
	SEND_PAYLOAD = 0x0009
} CommandType;

class Command {
	private:
		uint8_t *buffer;
		size_t size;
		uint8_t *parameters;
	public:
		Command(uint8_t *buffer, size_t size);
		~Command();
		CommandType getCommandType();
		uint8_t *getParameters();
		uint8_t *getBuffer();
		size_t getParametersSize();
};

class Parameters {
	protected:
		uint8_t size;
		uint8_t* buffer;
	public:
		Parameters(uint8_t *buffer, uint8_t size);
};

class ProtocolParameters : public Parameters {
	private:
		Protocol protocol;
	public:
		ProtocolParameters(uint8_t *buffer, uint8_t size);
		Protocol getProtocol();
};

class ChannelParameters : public Parameters {
	private:
		uint8_t channel;
	public:
		ChannelParameters(uint8_t *buffer, uint8_t size);
		uint8_t getChannel();
};

class BooleanParameters : public Parameters {
	private:
		bool boolean;
	public:
		BooleanParameters(uint8_t *buffer, uint8_t size);
		bool getBoolean();
};

class PayloadParameters : public Parameters {
	private:
		uint8_t payloadDirection;
		size_t payloadSize;
		uint8_t *payloadContent;

	public:
		PayloadParameters(uint8_t *buffer, uint8_t size);
		size_t getPayloadSize();
		uint8_t getPayloadDirection();
		uint8_t *getPayloadContent();
};
#endif
