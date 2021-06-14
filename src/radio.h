#ifndef RADIO_H
#define RADIO_H
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "nrf.h"
#include "controller.h"
#include "radio_defs.h"
#include "helpers.h"

#define MAX_PACKET_SIZE 257

class Controller;

class Radio
{
	private:
		int frequency;
		int channel;
		bool ready;
		Protocol protocol;
		RadioState state;

		Prefixes prefixes;

		bool fastRampUpTime;
		RadioMode mode;

		Endianness endianness;
		Preamble preamble;
		TxPower txPower;
		bool rssi;
		Whitening whitening;
		uint8_t whiteningDataIv;
		Phy phy;

		Crc crc;
		uint8_t crcSize;
		uint32_t crcPoly;
		uint32_t crcInit;
		bool crcSkipAddress;

		uint8_t interFrameSpacing;
		Header header;

		uint8_t expandPayloadLength;
		uint8_t payloadLength;

		BLEAddress filter;
		bool filterEnabled;

		bool autoTXafterRXenabled;

		bool jammingPatternsEnabled;
		uint8_t jammingPatternsCounter;
		JammingPatternsQueue* jammingPatternsQueue;

		Controller *controller;

		void initJammingPatternsQueue();


		bool generateTxPowerRegister();
		bool generateModeRegister();
		bool generateModeCnf0Register();
		bool generateFrequencyRegister();
		bool generateBaseAndPrefixRegisters();
		bool generatePcnf0Register();
		bool generatePcnf1Register();
		bool generateDataWhiteIvRegister();
		bool generateCrcRegisters();

	public:
		static Radio *instance;
		uint32_t currentTimestamp;

		uint8_t rxBuffer[MAX_PACKET_SIZE];
		uint8_t txBuffer[MAX_PACKET_SIZE];

		Radio();

		Controller* getController();
		bool setController(Controller *controller);

		bool enableJammingPatterns();
		bool disableJammingPatterns();
		bool setJammingPatternsCounter(uint8_t counter);
		uint8_t getJammingPatternsCounter();

		void addJammingPattern(uint8_t* pattern, uint8_t* mask, size_t size, uint8_t position);
		bool resetJammingPatternsQueue();
		bool removeJammingPattern(uint8_t* pattern, uint8_t* mask, size_t size, uint8_t position);
		bool checkJammingPattern(JammingPattern* pattern, uint8_t *buffer, size_t size);
		bool checkJammingPatterns(uint8_t *buffer, size_t size);

		bool setPrefixes();
		bool setPrefixes(uint8_t a);
		bool setPrefixes(uint8_t a,uint8_t b);
		bool setPrefixes(uint8_t a,uint8_t b,uint8_t c);

		bool enableFilter(BLEAddress address);
		bool isFilterEnabled();
		bool disableFilter();

		RadioState getState();
		bool setState(RadioState state);

		bool getFastRampUpTime();
		bool setFastRampUpTime(bool fastRampUpTime);

		Protocol getProtocol();
		bool setProtocol(Protocol protocol);

		Endianness getEndianness();
		bool setEndianness(Endianness endianness);

		Preamble getPreamble();
		bool setPreamble(uint8_t* pattern, uint8_t size);
		bool setPreamble(Preamble preamble);

		TxPower getTxPower();
		bool setTxPower(TxPower txPower);
		bool setTxPower(int txPower);

		bool isAutoTXafterRXenabled();
		bool enableAutoTXafterRX();
		bool disableAutoTXafterRX();

		bool updateTXBuffer(uint8_t *data, uint8_t size);

		bool isRssiEnabled();
		bool enableRssi();
		bool disableRssi();

		Phy getPhy();
		bool setPhy(Phy phy);

		Whitening getWhitening();
		bool setWhitening(Whitening whitening);

		uint8_t getWhiteningDataIv();
		bool setWhiteningDataIv(uint8_t iv);

		Crc getCrc();
		bool setCrc(Crc crc);

		uint8_t getCrcSize();
		bool setCrcSize(uint8_t crcSize);

		uint32_t getCrcInit();
		bool setCrcInit(uint32_t init);

		uint32_t getCrcPoly();
		bool setCrcPoly(uint32_t poly);

		bool getCrcSkipAddress();
		bool setCrcSkipAddress(bool skip);

		uint8_t getInterFrameSpacing();
		bool setInterFrameSpacing(uint8_t ifs);

		Header getHeader();
		bool setHeader(uint8_t s0, uint8_t length, uint8_t s1);
		bool setHeader(Header header);

		uint8_t getExpandPayloadLength();
		bool setExpandPayloadLength(uint8_t expandPayloadLength);

		uint8_t getPayloadLength();
		bool setPayloadLength(uint8_t payloadLength);

		bool setFrequency(int channel);
		int getFrequency();

		RadioMode getMode();
		bool setMode(RadioMode mode);

		bool disable();
		bool enable();
		bool reload();

		bool fastFrequencyChange(int frequency,uint8_t iv);
		bool setChannel(int channel);
		int getChannel();

		bool send(uint8_t *data,int size, int frequency, uint8_t channel);
};

#endif
