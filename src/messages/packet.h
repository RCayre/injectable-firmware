#ifndef PACKET_H
#define PACKET_H
#include "message.h"
#include "helpers.h"
#include "radio_defs.h"

typedef enum PacketType {
	BLE_PACKET_TYPE = 0x00
} PacketType;

class Packet : public Message {
	protected:
		uint8_t *packetPointer;
		size_t packetSize;
		PacketType packetType;
		uint32_t timestamp;
		uint8_t rssi;
		uint8_t channel;
		uint8_t source;
		CrcValue crcValue;
	public:
		Packet(PacketType packetType,uint8_t *packetBuffer, size_t packetSize, uint32_t timestamp, uint8_t source, uint8_t channel, uint8_t rssi, CrcValue crcValue);
		uint8_t *getPacketBuffer();
		size_t getPacketSize();
		void updateSource(uint8_t source);
		uint32_t getTimestamp();
};


typedef enum BLEAdvertisementType {
	ADV_IND = 0,
	ADV_DIRECT_IND = 1,
	ADV_NONCONN_IND = 2,
	SCAN_REQ = 3,
	SCAN_RSP = 4,
	CONNECT_REQ = 5,
	ADV_SCAN_IND = 6,
	ADV_UNKNOWN = 0xFF
} BLEAdvertisementType;

class BLEPacket : public Packet {
	protected:
		uint32_t accessAddress;

	public:
		static void forgeTerminateInd(uint8_t **payload, size_t *size, uint8_t code);
		static void forgeConnectionUpdateRequest(uint8_t **payload,size_t *size, uint8_t winSize, uint16_t winOffset, uint16_t interval, uint16_t latency, uint16_t timeout, uint16_t instant);
		static void forgeChannelMapRequest(uint8_t **payload,size_t *size,uint16_t instant, uint8_t *channelMap);

		BLEPacket(uint32_t accessAddress,uint8_t *packetBuffer, size_t packetSize, uint32_t timestamp, uint32_t timestampRelative, uint8_t source, uint8_t channel,uint8_t rssi,CrcValue crcValue);

		bool isAdvertisement();
		bool isLinkLayerConnectionUpdateRequest();
		bool isLinkLayerChannelMapRequest();
		bool isLinkLayerTerminateInd();
		uint32_t extractAccessAddress();
		uint32_t extractCrcInit();
		uint16_t extractHopInterval();
		uint8_t extractHopIncrement();
		uint16_t extractLatency();
		uint8_t* extractChannelMap();
		uint16_t extractInstant();
		int extractSCA();

		bool checkAdvertiserAddress(BLEAddress address);

		uint8_t extractPayloadLength();

		uint8_t extractWindowSize();
		uint16_t extractWindowOffset();

		uint8_t extractNESN();
		uint8_t extractSN();

		BLEAdvertisementType extractAdvertisementType();
		uint32_t getAccessAddress();
};

#endif
