#ifndef NOTIFICATION_H
#define NOTIFICATION_H
#include "message.h"
#include <string.h>

typedef enum NotificationType {
	DEBUG_NOTIFICATION = 0x00,
	INJECTION_REPORT_NOTIFICATION = 0x01,
	ADVINTERVAL_REPORT_NOTIFICATION = 0x02,
	CONNECTION_REPORT_NOTIFICATION = 0x03
} NotificationType;

typedef enum ConnectionStatus {
	CONNECTION_STARTED = 0x00,
	CONNECTION_LOST = 0x01,
	ATTACK_STARTED = 0x02,
	ATTACK_SUCCESS = 0x03,
	ATTACK_FAILURE = 0x04
} ConnectionStatus;

class Notification : public Message {
	protected:
		uint8_t *parameters;
		NotificationType notificationType;

	public:
		Notification(NotificationType notificationType,size_t parametersSize);
};

class DebugNotification : public Notification {
	public:
		DebugNotification(uint8_t *buffer,uint8_t size);
		DebugNotification(const char* message);
};

class InjectionReportNotification : public Notification {
	public:
		InjectionReportNotification(bool status, uint32_t injectionCount);
};

class AdvIntervalReportNotification : public Notification {
	public:
		AdvIntervalReportNotification(uint32_t interval);
};

class ConnectionNotification : public Notification {
	public:
		ConnectionNotification(ConnectionStatus status);
};

#endif
