#ifndef RADIO_DEFS_H
#define RADIO_DEFS_H

#include <stdint.h>
#include <stddef.h>

typedef enum RadioMode {
	MODE_NORMAL = 0x00,
	MODE_JAMMER = 0x01,
	MODE_IPS_SLAVE = 0x02,
	MODE_IPS_MASTER = 0x03
} RadioMode;


typedef enum RadioState {
	NONE = 0x00,
	RX = 0x01,
	TX = 0x02,
	JAM_RX = 0x03,
	JAM_TX = 0x04
} RadioState;

typedef enum Protocol {
	BLE_PROTOCOL = 0x00,
	DOT15D4_PROTOCOL = 0x01,
	ESB_PROTOCOL = 0x02,
	GENERIC_PROTOCOL = 0x03
} Protocol;

typedef enum Endianness {
	BIG = 0x00,
	LITTLE = 0x01
} Endianness;

typedef struct Preamble {
	uint8_t pattern[5];
	uint8_t size;
} Preamble;

typedef enum TxPower {
	NEG40_DBM = -40,
	NEG30_DBM = -30,
	NEG20_DBM = -20,
	NEG16_DBM = -16,
	NEG12_DBM = -12,
	NEG8_DBM = -8,
	NEG4_DBM = -4,
	POS0_DBM = 0,
	POS4_DBM = 4,
	POS8_DBM = 8,
} TxPower;

typedef enum Phy {
	ESB_1MBITS,
	ESB_2MBITS,
	BLE_1MBITS,
	BLE_2MBITS,
	DOT15D4_NATIVE,
	DOT15D4_WAZABEE
} Phy;

typedef enum Whitening {
	HARDWARE_WHITENING,
	SOFTWARE_WHITENING,
	NO_WHITENING
} Whitening;

typedef enum Crc {
	HARDWARE_CRC,
	SOFTWARE_CRC,
	NO_CRC
} Crc;

typedef struct Header {
	uint8_t s0;
	uint8_t length;
	uint8_t s1;
} Header;


typedef enum CrcValidity {
	VALID_CRC = 0x00,
	INVALID_CRC = 0x01,
	UNKNOWN_CRC = 0xFF
} CrcValidity;

typedef struct CrcValue {
	CrcValidity validity;
	uint32_t value;
} CrcValue;

typedef struct BLEAddress {
	uint8_t bytes[6];
} BLEAddress;

typedef struct Prefixes {
	uint8_t prefixes[3];
	size_t number;
} Prefixes;

#define ANYWHERE 0xFF

typedef struct JammingPattern JammingPattern;

typedef struct JammingPattern {
	uint8_t *pattern;
	uint8_t *mask;
	size_t size;
	uint8_t position;
	JammingPattern* next;
} JammingPattern;

typedef struct JammingPatternsQueue {
	size_t size;
	JammingPattern* first;
} JammingPatternsQueue;

#endif
