#ifndef RESPONSE_H
#define RESPONSE_H
#include "command.h"
#include "message.h"

#define ResponseType CommandType

class Response : public Message {
	protected:
		uint8_t *parameters;
		ResponseType responseType;

	public:
		Response(ResponseType responseType,size_t parametersSize);
};

class GetVersionResponse : public Response {
	public:
		GetVersionResponse(uint8_t major, uint8_t minor);
};

class SelectControllerResponse : public Response {
	public:
		SelectControllerResponse(bool status);
};

class EnableControllerResponse : public Response {
	public:
		EnableControllerResponse(bool status);
};

class DisableControllerResponse : public Response {
	public:
		DisableControllerResponse(bool status);
};

class GetChannelResponse : public Response {
	public:
		GetChannelResponse(uint8_t channel);
};

class SetChannelResponse : public Response {
	public:
		SetChannelResponse(bool status);
};

class SetFilterResponse : public Response {
	public:
		SetFilterResponse(bool status);
};

class SetFollowModeResponse : public Response {
	public:
		SetFollowModeResponse(bool status);
};

class StartAttackResponse : public Response {
	public:
		StartAttackResponse(bool status);
};

class SendPayloadResponse : public Response {
	public:
		SendPayloadResponse(uint8_t direction,bool status);
};


#endif
