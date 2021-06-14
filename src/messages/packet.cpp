#include "packet.h"

Packet::Packet(PacketType packetType,uint8_t *packetBuffer, size_t packetSize, uint32_t timestamp, uint8_t source, uint8_t channel, uint8_t rssi, CrcValue crcValue) : Message(PACKET,1+4+1+1+1+1+packetSize) {
	this->packetType = packetType;
	this->payload[0] = (uint8_t)(this->packetType);

	this->timestamp = timestamp;
	this->payload[1] = (uint8_t)(timestamp & 0x000000FF);
	this->payload[2] = (uint8_t)((timestamp & 0x0000FF00) >> 8);
	this->payload[3] = (uint8_t)((timestamp & 0x00FF0000) >> 16);
	this->payload[4] = (uint8_t)((timestamp & 0xFF000000) >> 24);

	this->source = source;
	this->payload[5] = source;

	this->channel = channel;
	this->payload[6] = channel;

	this->rssi = rssi;
	this->payload[7] = rssi;

	this->crcValue = crcValue;
	this->payload[8] = (uint8_t)crcValue.validity;

	if (packetBuffer != NULL) {
		for (size_t i=0;i<packetSize;i++) {
			this->payload[9+i] = packetBuffer[i];
		}
	}
	this->packetPointer = &(this->payload[9]);
	this->packetSize = packetSize;
}

uint8_t *Packet::getPacketBuffer() {
	return this->packetPointer;
}
size_t Packet::getPacketSize() {
	return this->packetSize;
}
uint32_t Packet::getTimestamp() {
	return this->timestamp;
}

void Packet::updateSource(uint8_t source) {
	this->source = source;
	this->payload[5] = source;
}

void BLEPacket::forgeTerminateInd(uint8_t **payload,size_t *size, uint8_t code) {
	*size=4;
	*payload = (uint8_t *)malloc(sizeof(uint8_t)*(*size));
	(*payload)[0] = 0x03;
	(*payload)[1] = 0x02;
	(*payload)[2] = 0x02;
	(*payload)[3] = code;
}
void BLEPacket::forgeConnectionUpdateRequest(uint8_t **payload,size_t *size, uint8_t winSize, uint16_t winOffset, uint16_t interval, uint16_t latency, uint16_t timeout, uint16_t instant) {
	*size=14;
	*payload = (uint8_t *)malloc(sizeof(uint8_t)*(*size));
	(*payload)[0] = 0x03;
	(*payload)[1] = 0x0c;
	(*payload)[2] = 0x00;
	(*payload)[3] = winSize;
	(*payload)[4] = (uint8_t)(winOffset & 0x00FF);
	(*payload)[5] = (uint8_t)((winOffset & 0xFF00) >> 8);
	(*payload)[6] = (uint8_t)(interval & 0x00FF);
	(*payload)[7] = (uint8_t)((interval & 0xFF00) >> 8);
	(*payload)[8] = (uint8_t)(latency & 0x00FF);
	(*payload)[9] = (uint8_t)((latency & 0xFF00) >> 8);
	(*payload)[10] = (uint8_t)(timeout & 0x00FF);
	(*payload)[11] = (uint8_t)((timeout & 0xFF00) >> 8);
	(*payload)[12] = (uint8_t)(instant & 0x00FF);
	(*payload)[13] = (uint8_t)((instant & 0xFF00) >> 8);
}
void BLEPacket::forgeChannelMapRequest(uint8_t **payload,size_t *size,uint16_t instant, uint8_t *channelMap) {
	*size=10;
	*payload = (uint8_t *)malloc(sizeof(uint8_t)*(*size));
	(*payload)[0] = 0x03;
	(*payload)[1] = 0x08;
	(*payload)[2] = 0x01;
	(*payload)[3] = channelMap[0];
	(*payload)[4] = channelMap[1];
	(*payload)[5] = channelMap[2];
	(*payload)[6] = channelMap[3];
	(*payload)[7] = channelMap[4];
	(*payload)[8] = (uint8_t)(instant & 0x00FF);
	(*payload)[9] = (uint8_t)((instant & 0xFF00) >> 8);
}


BLEPacket::BLEPacket(uint32_t accessAddress,uint8_t *packetBuffer, size_t packetSize, uint32_t timestamp,  uint32_t timestampRelative, uint8_t source, uint8_t channel,uint8_t rssi, CrcValue crcValue) : Packet(BLE_PACKET_TYPE, NULL, 4+4+packetSize+3, timestamp,source,channel,rssi,crcValue) {
	this->payload[9] = (uint8_t)(timestampRelative & 0x000000FF);
	this->payload[10] = (uint8_t)((timestampRelative & 0x0000FF00) >> 8);
	this->payload[11] = (uint8_t)((timestampRelative & 0x00FF0000) >> 16);
	this->payload[12] = (uint8_t)((timestampRelative & 0xFF000000) >> 24);
	this->packetPointer = &(this->payload[13]);
	this->accessAddress = accessAddress;
	this->packetPointer[0] = (accessAddress & 0x000000FF);
	this->packetPointer[1] = (accessAddress & 0x0000FF00) >> 8;
	this->packetPointer[2] = (accessAddress & 0x00FF0000) >> 16;
	this->packetPointer[3] = (accessAddress & 0xFF000000) >> 24;

	for (size_t i=0;i<packetSize;i++) {
		this->packetPointer[4+i] = packetBuffer[i];
	}
	this->packetPointer[4+packetSize] = bytewise_bit_swap((crcValue.value & 0xFF0000) >> 16);
	this->packetPointer[4+packetSize+1] = bytewise_bit_swap((crcValue.value & 0x00FF00) >> 8);
	this->packetPointer[4+packetSize+2] = bytewise_bit_swap(crcValue.value & 0x0000FF);
}

bool BLEPacket::checkAdvertiserAddress(BLEAddress address) {
	if (this->isAdvertisement()) {
		if (address.bytes[0] == 0xFF && address.bytes[1] == 0xFF && address.bytes[2] == 0xFF && address.bytes[3] == 0xFF && address.bytes[4] == 0xFF && address.bytes[5] == 0xFF) {
			return true;
		}
		else if (this->extractAdvertisementType() == ADV_IND || this->extractAdvertisementType() == ADV_DIRECT_IND || this->extractAdvertisementType() == SCAN_RSP) {
			return (this->packetPointer[6] == address.bytes[5] &&
			this->packetPointer[7] == address.bytes[4] &&
			this->packetPointer[8] == address.bytes[3] &&
			this->packetPointer[9] == address.bytes[2] &&
			this->packetPointer[10] == address.bytes[1] &&
			this->packetPointer[11] == address.bytes[0] );
		}
		else if (this->extractAdvertisementType() == CONNECT_REQ || this->extractAdvertisementType() == SCAN_REQ) {
			return (this->packetPointer[12] == address.bytes[5] &&
			this->packetPointer[13] == address.bytes[4] &&
			this->packetPointer[14] == address.bytes[3] &&
			this->packetPointer[15] == address.bytes[2] &&
			this->packetPointer[16] == address.bytes[1] &&
			this->packetPointer[17] == address.bytes[0] );
		}
		else return false;
	}
	return false;
}

bool BLEPacket::isAdvertisement() {
	return ((this->packetPointer[0] == 0xd6) && (this->packetPointer[1] == 0xbe) && (this->packetPointer[2] == 0x89) && (this->packetPointer[3] == 0x8e));
}

bool BLEPacket::isLinkLayerConnectionUpdateRequest() {
	return (!this->isAdvertisement() && this->packetSize > 7 && this->packetPointer[6] == 0x00);
}

bool BLEPacket::isLinkLayerTerminateInd() {
	return (!this->isAdvertisement() && this->packetSize > 7 && this->packetPointer[6] == 0x02);
}

bool BLEPacket::isLinkLayerChannelMapRequest() {
	return (!this->isAdvertisement() && this->packetSize > 7 && this->packetPointer[5] == 0x08 && this->packetPointer[6] == 0x01);
}

uint32_t BLEPacket::extractAccessAddress() {
	return *((uint32_t *)&this->packetPointer[18]);
}

uint32_t BLEPacket::extractCrcInit() {
	return (this->packetPointer[22] | (this->packetPointer[23] << 8) | (this->packetPointer[24] << 16));
}
uint16_t BLEPacket::extractLatency() {
	if (this->isAdvertisement() && this->extractAdvertisementType() == CONNECT_REQ) {
		return (this->packetPointer[30] | (this->packetPointer[31] << 8));
	}
	else if (this->isLinkLayerConnectionUpdateRequest()) {
		return (this->packetPointer[12] | (this->packetPointer[13] << 8));
	}
	else return 0x0000;
}
uint16_t BLEPacket::extractHopInterval() {
	if (this->isAdvertisement() && this->extractAdvertisementType() == CONNECT_REQ) {
		return (this->packetPointer[28] | (this->packetPointer[29] << 8));
	}
	else if (this->isLinkLayerConnectionUpdateRequest()) {
		return (this->packetPointer[10] | (this->packetPointer[11] << 8));
	}
	else return 0x0000;
}
uint8_t BLEPacket::extractHopIncrement() {
	if (this->isAdvertisement() && this->extractAdvertisementType() == CONNECT_REQ) {
		return this->packetPointer[39] & 0x1F;
	}
	else return 0x00;
}

int BLEPacket::extractSCA() {
	if (this->isAdvertisement() && this->extractAdvertisementType() == CONNECT_REQ) {
		uint8_t sca = (this->packetPointer[39] & 0xE0) >> 5;
		int ppm = 0;
		switch (sca) {
			case 0:
				ppm = 500; // 251
				break;
			case 1:
				ppm = 250; // 151
				break;
			case 2:
				ppm = 150; // 101
				break;
			case 3:
				ppm = 100; // 76
				break;
			case 4:
				ppm = 75; // 51
				break;
			case 5:
				ppm = 50; // 31
				break;
			case 6:
				ppm = 30; // 21
				break;
			case 7:
				ppm = 20; // 0
				break;
			default:
				ppm = 0;
		}
		return ppm;
	}
	else return 0;
}

uint8_t* BLEPacket::extractChannelMap() {
	if (this->isAdvertisement() && this->extractAdvertisementType() == CONNECT_REQ) {
		return &this->packetPointer[34];
	}
	else if (this->isLinkLayerChannelMapRequest()) {
		return &this->packetPointer[7];
	}
	else return NULL;
}

uint16_t BLEPacket::extractInstant() {
	if (this->isLinkLayerChannelMapRequest()) {
		return (this->packetPointer[12] | (this->packetPointer[13] << 8));
	}
	else if (this->isLinkLayerConnectionUpdateRequest()) {
		return (this->packetPointer[16] | (this->packetPointer[17] << 8));
	}
	else return 0x0000;
}

uint8_t BLEPacket::extractWindowSize() {
	if (this->isLinkLayerConnectionUpdateRequest()) {
		return this->packetPointer[7];
	}
	else return 0x00;
}

uint16_t BLEPacket::extractWindowOffset() {
	if (this->isLinkLayerConnectionUpdateRequest()) {
		return (this->packetPointer[8] | (this->packetPointer[9] << 8));
	}
	else return 0x0000;
}


uint8_t BLEPacket::extractPayloadLength() {
	return this->packetPointer[5];
}


uint8_t BLEPacket::extractSN() {
	if (!this->isAdvertisement()) {
		return (this->packetPointer[4] & 0x08) >> 3;
	}
	else return 0x00;
}

uint8_t BLEPacket::extractNESN() {
	if (!this->isAdvertisement()) {
		return (this->packetPointer[4] & 0x04) >> 2;
	}
	else return 0x00;
}

uint32_t BLEPacket::getAccessAddress() {
	return (((this->packetPointer[3]) << 24) & 0xFF000000) | (((this->packetPointer[2]) << 16) & 0x00FF0000) | (((this->packetPointer[1]) << 8) & 0x0000FF00) | (((this->packetPointer[0]) & 0x000000FF));
}

BLEAdvertisementType BLEPacket::extractAdvertisementType() {
	BLEAdvertisementType type;
	if ((this->packetPointer[4] & 0x0F) == 0) type = ADV_IND;
	else if ((this->packetPointer[4] & 0x0F) == 1) type = ADV_DIRECT_IND;
	else if ((this->packetPointer[4] & 0x0F) == 2) type = ADV_NONCONN_IND;
	else if ((this->packetPointer[4] & 0x0F) == 3) type = SCAN_REQ;
	else if ((this->packetPointer[4] & 0x0F) == 4) type = SCAN_RSP;
	else if ((this->packetPointer[4] & 0x0F) == 5 && this->packetPointer[5] == 0x22) type = CONNECT_REQ;
	else if ((this->packetPointer[4] & 0x0F) == 6) type = ADV_SCAN_IND;
	else type = ADV_UNKNOWN;
	return type;
}
