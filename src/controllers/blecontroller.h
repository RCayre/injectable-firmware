#ifndef BLECONTROLLER_H
#define BLECONTROLLER_H
#include "messages/packet.h"
#include "messages/notification.h"
#include "../controller.h"
#include "../time.h"

#define ADV_REPORT_SIZE 25

typedef enum ControllerState {
	SNIFFING_ADVERTISEMENTS,
	COLLECTING_ADVINTERVAL,
	SNIFFING_CONNECTION,
	INJECTING,
	SIMULATING_SLAVE,
	SYNCHRONIZING_MASTER,
	SIMULATING_MASTER,
	SYNCHRONIZING_MITM,
	PERFORMING_MITM,
	JAMMING_CONNECT_REQ
} ControllerState;

// Connection update definitions
typedef enum {
	UPDATE_TYPE_NONE,
	UPDATE_TYPE_CONNECTION_UPDATE_REQUEST,
	UPDATE_TYPE_CHANNEL_MAP_REQUEST
} BLEConnectionUpdateType;

typedef struct BLEConnectionUpdate {
	BLEConnectionUpdateType type;
	uint16_t instant;
	uint16_t hopInterval;
	uint8_t windowSize;
	uint8_t windowOffset;
	uint8_t channelMap[5];
} BLEConnectionUpdate;


// Sequence numbers definition
typedef struct BLESequenceNumbers {
	uint8_t sn;
	uint8_t nesn;
} BLESequenceNumbers;

// Attack specific definitions
typedef enum BLEAttack {
	BLE_ATTACK_NONE,
	BLE_ATTACK_FRAME_INJECTION,
	BLE_ATTACK_MASTER_HIJACKING,
	BLE_ATTACK_SLAVE_HIJACKING,
	BLE_ATTACK_MITM
} BLEAttack;

typedef struct AttackStatus {
	BLEAttack attack;
	bool running;
	bool injecting;
	bool successful;
	uint8_t payload[64];
	size_t size;
	uint32_t injectionTimestamp;
	uint32_t injectionCounter;
	uint32_t hijackingCounter;
	uint16_t nextInstant;
	BLESequenceNumbers injectedSequenceNumbers;
} AttackStatus;

typedef struct BLEPayload {
	uint8_t payload[64];
	size_t size;
	bool transmitted;
} BLEPayload;

class BLEController : public Controller {
	protected:
		TimeModule *timeModule;
		bool emptyTransmitIndicator;
		bool follow; // follow mode indicator

		// Channels related attributes
		int channel;
		int lastUnmappedChannel;
		int unmappedChannel;
		int numUsedChannels;
		int lastAdvertisingChannel;

		// General parameters
		uint32_t accessAddress;
		uint32_t crcInit;

		// Connection specific parameters
		uint16_t connectionEventCount;
		uint8_t hopIncrement;
		uint16_t hopInterval;
		bool channelsInUse[37];
		int *remappingTable;
		BLEConnectionUpdate connectionUpdate;
		bool sync;
		uint8_t desyncCounter;
		uint16_t latency;
		uint16_t latencyCounter;
		int packetCount;
		int lastPacketCount;

		uint32_t lastMasterTimestamp;
		uint32_t lastSlaveTimestamp;

		// Sequence numbers
		BLESequenceNumbers masterSequenceNumbers;
		BLESequenceNumbers slaveSequenceNumbers;

		BLESequenceNumbers simulatedMasterSequenceNumbers;
		BLESequenceNumbers simulatedSlaveSequenceNumbers;

		// Attack related
		AttackStatus attackStatus;

		ControllerState controllerState;

		BLEPayload slavePayload;
		BLEPayload masterPayload;

		// Advertising interval related
		uint32_t timestampsFirstChannel[ADV_REPORT_SIZE];
		uint32_t timestampsThirdChannel[ADV_REPORT_SIZE];
		int collectedIntervals;

		BLEAddress filter;

	public:
		static int channelToFrequency(int channel);

		BLEController(Radio *radio);
		void start();
		void stop();

		bool whitelistAdvAddress(bool enable, uint8_t a, uint8_t b, uint8_t c,uint8_t d, uint8_t e, uint8_t f);
		bool whitelistInitAddress(bool enable, uint8_t a, uint8_t b, uint8_t c,uint8_t d, uint8_t e, uint8_t f);
		bool whitelistConnection(bool enable,uint8_t a, uint8_t b, uint8_t c,uint8_t d, uint8_t e, uint8_t f,uint8_t ap, uint8_t bp, uint8_t cp,uint8_t dp, uint8_t ep, uint8_t fp);

		void sendInjectionReport(bool status, uint32_t injectionCount);
		void sendAdvIntervalReport(uint32_t interval);
		void sendConnectionReport(ConnectionStatus status);

		void followAdvertisingDevice(uint8_t a,uint8_t b,uint8_t c,uint8_t d,uint8_t e,uint8_t f);
		void calculateAdvertisingIntervals();

		// Hardware configuration manager
		void setHardwareConfiguration(uint32_t accessAddress,uint32_t crcInit);
		void setJammerConfiguration();

		// Follow mode setter
		void setFollowMode(bool follow);

		// Connection specific methods
		void followConnection(uint16_t hopInterval, uint8_t hopIncrement, uint8_t *channelMap,uint32_t accessAddress,uint32_t crcInit,  int masterSCA,uint16_t latency);

		int getChannel();
		void setChannel(int channel);
		int nextChannel();

		void updateHopInterval(uint16_t hopInterval);
		void updateHopIncrement(uint8_t hopIncrement);
		void updateChannelsInUse(uint8_t* channelMap);

		void clearConnectionUpdate();
		void prepareConnectionUpdate(uint16_t instant, uint16_t hopInterval, uint8_t windowSize, uint8_t windowOffset,uint16_t latency);
		void prepareConnectionUpdate(uint16_t instant, uint8_t *channelMap);
		void applyConnectionUpdate();

		void updateMasterSequenceNumbers(uint8_t sn, uint8_t nesn);
		void updateSlaveSequenceNumbers(uint8_t sn, uint8_t nesn);

		void updateSimulatedMasterSequenceNumbers(uint8_t sn, uint8_t nesn);
		void updateSimulatedSlaveSequenceNumbers(uint8_t sn, uint8_t nesn);

		void updateInjectedSequenceNumbers(uint8_t sn, uint8_t nesn);

		void setEmptyTransmitIndicator(bool emptyTransmitIndicator);
		void setFilter(uint8_t a,uint8_t b,uint8_t c,uint8_t d,uint8_t e,uint8_t f);

		// Attack related methods
		void setAttackPayload(uint8_t *payload, size_t size);
		void startAttack(BLEAttack attack);
		void executeAttack();
		void checkAttackSuccess();
		void setSlavePayload(uint8_t *payload, size_t size);
		void setMasterPayload(uint8_t *payload, size_t size);

		void setConnectionJammingConfiguration(uint32_t accessAddress,uint32_t channel);

		bool isSlavePayloadTransmitted();
		bool isMasterPayloadTransmitted();

		// Slave simulation
		void enterSlaveMode();
		void exitSlaveMode();

		// Role simulation callback
		bool slaveRoleCallback(BLEPacket *pkt);
		bool masterRoleCallback(BLEPacket *pkt);

		// Timers callbacks
		bool goToNextChannel();
		bool inject();
		bool connectionLost();

		// Reception callback
		void onReceive(uint32_t timestamp, uint8_t size, uint8_t *buffer, CrcValue crcValue, uint8_t rssi);
		void onJam(uint32_t timestamp);
};
#endif
