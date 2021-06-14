#include "controller.h"
#include "core.h"


Controller::Controller(Radio *radio) {
	this->radio = radio;
}

void Controller::addPacket(Packet* packet) {
	Core::instance->pushMessageToQueue(packet);
}
