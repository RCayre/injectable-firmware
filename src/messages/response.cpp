#include "response.h"

Response::Response(ResponseType responseType,size_t parametersSize) : Message(RESPONSE,parametersSize+2) {
	this->responseType = responseType;
	this->payload[0] = (uint8_t)((this->responseType & 0xFF00) >> 8);
	this->payload[1] = (uint8_t)(this->responseType & 0x00FF);
	this->parameters = &(this->payload[2]);
}

GetVersionResponse::GetVersionResponse(uint8_t major, uint8_t minor) : Response(GET_VERSION,2) {
	this->parameters[0] = major;
	this->parameters[1] = minor;
}

SelectControllerResponse::SelectControllerResponse(bool status) : Response(SELECT_CONTROLLER,1) {
	this->parameters[0] = (status ? 0 : 1);
}

EnableControllerResponse::EnableControllerResponse(bool status) : Response(ENABLE_CONTROLLER,1) {
	this->parameters[0] = (status ? 0 : 1);
}

DisableControllerResponse::DisableControllerResponse(bool status) : Response(DISABLE_CONTROLLER,1) {
	this->parameters[0] = (status ? 0 : 1);
}

GetChannelResponse::GetChannelResponse(uint8_t channel) : Response(GET_CHANNEL,1) {
	this->parameters[0] = channel;
}

SetChannelResponse::SetChannelResponse(bool status) : Response(SET_CHANNEL,1) {
	this->parameters[0] = (status ? 0 : 1);
}

SetFilterResponse::SetFilterResponse(bool status) : Response(SET_FILTER,1) {
	this->parameters[0] = (status ? 0 : 1);
}

SetFollowModeResponse::SetFollowModeResponse(bool status) : Response(SET_FOLLOW_MODE,1) {
	this->parameters[0] = (status ? 0 : 1);
}

StartAttackResponse::StartAttackResponse(bool status) : Response(START_ATTACK,1) {
	this->parameters[0] = (status ? 0 : 1);
}

SendPayloadResponse::SendPayloadResponse(uint8_t direction,bool status) : Response(SEND_PAYLOAD,2) {
	this->parameters[0] = direction;
	this->parameters[1] = (status ? 0 : 1);
}
