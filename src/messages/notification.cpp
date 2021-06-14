#include "notification.h"

Notification::Notification(NotificationType notificationType,size_t parametersSize) : Message(NOTIFICATION,parametersSize+1) {
	this->notificationType = notificationType;
	this->payload[0] = (uint8_t)(this->notificationType);
	this->parameters = &(this->payload[1]);
}

DebugNotification::DebugNotification(uint8_t *buffer,uint8_t size) : Notification(DEBUG_NOTIFICATION,size) {
	for (size_t i=0;i<size;i++) this->parameters[i] = buffer[i];
}

DebugNotification::DebugNotification(const char *message) : Notification(DEBUG_NOTIFICATION, strlen(message)) {
	for (size_t i=0;i<strlen(message);i++) {
		this->parameters[i] = (uint8_t)message[i];
	}
}

InjectionReportNotification::InjectionReportNotification(bool status, uint32_t injectionCount) : Notification(INJECTION_REPORT_NOTIFICATION, 1 + 4) {
	this->parameters[0] = (status ? 0x00 : 0x01);
	this->parameters[1] = (uint8_t)((injectionCount & 0xFF000000) >> 24);
	this->parameters[2] = (uint8_t)((injectionCount & 0x00FF0000) >> 16);
	this->parameters[3] = (uint8_t)((injectionCount & 0x0000FF00) >> 8);
	this->parameters[4] = (uint8_t)(injectionCount & 0x000000FF);
}

AdvIntervalReportNotification::AdvIntervalReportNotification(uint32_t interval) : Notification(ADVINTERVAL_REPORT_NOTIFICATION, 4) {
	this->parameters[0] = (uint8_t)((interval & 0xFF000000) >> 24);
	this->parameters[1] = (uint8_t)((interval & 0x00FF0000) >> 16);
	this->parameters[2] = (uint8_t)((interval & 0x0000FF00) >> 8);
	this->parameters[3] = (uint8_t)(interval & 0x000000FF);
}

ConnectionNotification::ConnectionNotification(ConnectionStatus status) : Notification(CONNECTION_REPORT_NOTIFICATION, 1) {
	this->parameters[0] = (uint8_t)status;
}
