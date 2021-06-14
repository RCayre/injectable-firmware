#include "core.h"

// Global instance of Core
Core* Core::instance = NULL;


void Core::handleInputData(uint8_t *buffer, size_t size) {
	// Commands are received from Host
	uint8_t *command = (uint8_t *)malloc(sizeof(uint8_t)*size);
	for (size_t i=0;i<size;i++) command[i] = buffer[i];
	if (size >= 5 && command[0] == 0x5a && command[1] == 0x17 ) {
		this->handleCommand(new Command(command,size));
	}
}

Core::Core() {
	instance = this;
	this->ledModule = new LedModule();
	this->timeModule = new TimeModule();
	this->usbModule = new USBModule((CoreCallback)&Core::handleInputData,this);
	this->radio = new Radio();
}

LedModule* Core::getLedModule() {
	return (this->ledModule);
}

USBModule* Core::getUSBModule() {
	return (this->usbModule);
}

TimeModule* Core::getTimeModule() {
	return (this->timeModule);
}

Radio* Core::getRadioModule() {
	return (this->radio);
}

void Core::init() {
	this->usbModule->init();
	this->messageQueue.size = 0;
	this->messageQueue.firstElement = NULL;
	this->messageQueue.lastElement = NULL;

	this->bleController = new BLEController(this->getRadioModule());

	this->currentController = NULL;
	this->radio->setController(this->currentController);
}

bool Core::selectController(Protocol controller) {
	if (controller == BLE_PROTOCOL) {
		this->radio->disable();
		this->currentController = this->bleController;
		this->radio->setController(this->currentController);
		return true;
	}
	else {
		this->radio->disable();
		this->currentController = NULL;
		this->radio->setController(NULL);
		return false;
	}
}

void Core::sendDebug(const char *message) {
	this->pushMessageToQueue(new DebugNotification(message));
}

void Core::sendDebug(uint8_t *buffer, uint8_t size) {
	this->pushMessageToQueue(new DebugNotification(buffer,size));
}

void Core::pushMessageToQueue(Message *msg) {
	MessageQueueElement *element = (MessageQueueElement*)malloc(sizeof(MessageQueueElement));
	element->message = msg;
	element->nextElement = NULL;
	if (this->messageQueue.size == 0) {
		this->messageQueue.firstElement = element;
		this->messageQueue.lastElement = element;
	}
	else {
		// We insert the message at the end of the queue
		this->messageQueue.lastElement->nextElement = element;
		this->messageQueue.lastElement = element;
	}
	this->messageQueue.size = this->messageQueue.size + 1;
}
Message* Core::popMessageFromQueue() {
	if (this->messageQueue.size == 0) return NULL;
	else {
		MessageQueueElement* element = this->messageQueue.firstElement;
		Message* msg = element->message;
		this->messageQueue.firstElement = element->nextElement;
		this->messageQueue.size = this->messageQueue.size - 1;
		free(element);
		return msg;
	}
}

void Core::sendMessage(Message *msg) {
	this->usbModule->sendData(msg->getBuffer(),msg->getSize());
	delete msg;
}

void Core::handleCommand(Command *cmd) {
	if (cmd->getCommandType() == GET_VERSION) {
		this->pushMessageToQueue(new GetVersionResponse(VERSION_MAJOR,VERSION_MINOR));
	}
	else if (cmd->getCommandType() == SELECT_CONTROLLER) {
		ProtocolParameters parameters(cmd->getParameters(),cmd->getParametersSize());
		bool success = this->selectController(parameters.getProtocol());
		this->pushMessageToQueue(new SelectControllerResponse(success));
	}
	else if (cmd->getCommandType() == ENABLE_CONTROLLER) {
		this->pushMessageToQueue(new EnableControllerResponse(this->currentController != NULL));
		if (this->currentController != NULL) {
			this->currentController->start();
		}
	}

	else if (cmd->getCommandType() == DISABLE_CONTROLLER) {
		bool success = false;
		if (this->currentController != NULL) {
			this->currentController->stop();
			success = true;
		}
		this->pushMessageToQueue(new DisableControllerResponse(success));
	}
	else if (cmd->getCommandType() == GET_CHANNEL) {
		uint8_t channel = 0xFF;
		if (this->currentController == this->bleController) {
			channel = this->bleController->getChannel();
		}
		this->pushMessageToQueue(new GetChannelResponse(channel));
	}
	else if (cmd->getCommandType() == SET_CHANNEL) {
		ChannelParameters parameters(cmd->getParameters(),cmd->getParametersSize());
		bool status = false;
		if (this->currentController == this->bleController) {
			if (parameters.getChannel() >= 0 && parameters.getChannel() <= 39) {
				this->bleController->setChannel(parameters.getChannel());
				status = true;
			}
		}

		this->pushMessageToQueue(new SetChannelResponse(status));
	}
	else if (cmd->getCommandType() == SET_FILTER) {
		bool status = false;
		if (this->currentController == this->bleController) {
			this->bleController->setFilter(cmd->getParameters()[0],cmd->getParameters()[1],cmd->getParameters()[2],cmd->getParameters()[3],cmd->getParameters()[4],cmd->getParameters()[5]);
			status = true;
		}
		this->pushMessageToQueue(new SetFilterResponse(status));
	}

	else if (cmd->getCommandType() == SET_FOLLOW_MODE) {
		BooleanParameters parameters(cmd->getParameters(),cmd->getParametersSize());
		bool status = false;
		if (this->currentController == this->bleController) {
			this->bleController->setFollowMode(parameters.getBoolean());
			status = true;
		}
		this->pushMessageToQueue(new SetFollowModeResponse(status));
	}
	else if (cmd->getCommandType() == START_ATTACK) {
		bool status = false;
		if (this->currentController == this->bleController) {
			if (cmd->getParameters()[0] == 0x01) {
				this->bleController->startAttack(BLE_ATTACK_FRAME_INJECTION);
				status = true;
			}
			if (cmd->getParameters()[0] == 0x02) {
				this->bleController->startAttack(BLE_ATTACK_SLAVE_HIJACKING);
				status = true;
			}
			else if (cmd->getParameters()[0] == 0x03) {
				this->bleController->startAttack(BLE_ATTACK_MASTER_HIJACKING);
				status = true;
			}
			else if (cmd->getParameters()[0] == 0x04) {
				this->bleController->startAttack(BLE_ATTACK_MITM);
				status = true;
			}
		}
		this->pushMessageToQueue(new StartAttackResponse(status));
	}
	else if (cmd->getCommandType() == SEND_PAYLOAD) {
		PayloadParameters parameters(cmd->getParameters(),cmd->getParametersSize());
		//bool status = false;
		if (this->currentController == this->bleController) {
			if (parameters.getPayloadDirection() == 0x00) {
				this->bleController->setAttackPayload(parameters.getPayloadContent(),parameters.getPayloadSize());
				this->pushMessageToQueue(new SendPayloadResponse(0x00,true));
			}
			else if (parameters.getPayloadDirection() == 0x01) {
				if (this->bleController->isMasterPayloadTransmitted()) {
					this->bleController->setMasterPayload(parameters.getPayloadContent(),parameters.getPayloadSize());
				}
			}
			else if (parameters.getPayloadDirection() == 0x02) {
				if (this->bleController->isMasterPayloadTransmitted()) {
					this->bleController->setSlavePayload(parameters.getPayloadContent(),parameters.getPayloadSize());
				}
			}
		}
	}

	delete cmd;
}

void Core::loop() {
	while (true) {
		this->usbModule->managePower();
		// Responses, Notifications and Packets are transmitted to Host
		Message *msg = this->popMessageFromQueue();
		while (msg != NULL) {
			this->sendMessage(msg);
			msg = this->popMessageFromQueue();
		}

		// Even if we miss an event enabling USB, USB event would wake us up.
		__WFE();
		// Clear SEV flag if CPU was woken up by event
		__SEV();
		__WFE();
	}
}
