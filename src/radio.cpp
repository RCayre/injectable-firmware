#include "radio.h"
#include "bsp.h"
// Global instance of Radio
Radio* Radio::instance = NULL;

Radio::Radio() {
	this->setProtocol(GENERIC_PROTOCOL);
	this->ready = false;
	this->state = NONE;
	this->txPower = POS0_DBM;
	this->rssi = false;
	this->autoTXafterRXenabled = false;
	this->controller = NULL;
	this->interFrameSpacing = 0;
	this->filterEnabled = false;
	this->jammingPatternsEnabled = false;
	this->jammingPatternsCounter = 0;
	this->initJammingPatternsQueue();
	instance = this;
}

void Radio::initJammingPatternsQueue() {
	this->jammingPatternsQueue = (JammingPatternsQueue*) malloc(sizeof(JammingPatternsQueue));
	this->jammingPatternsQueue->size = 0;
	this->jammingPatternsQueue->first = NULL;
}


bool Radio::enableJammingPatterns() {
	this->jammingPatternsEnabled = true;
	return true;
}

bool  Radio::disableJammingPatterns() {
	this->jammingPatternsEnabled = false;
	return true;
}

bool  Radio::setJammingPatternsCounter(uint8_t counter) {
	this->jammingPatternsCounter = counter;
	return true;
}

uint8_t  Radio::getJammingPatternsCounter() {
	return this->jammingPatternsCounter;
}

void Radio::addJammingPattern(uint8_t* pattern, uint8_t* mask, size_t size, uint8_t position) {
	JammingPattern* current = (JammingPattern*)malloc(sizeof(JammingPattern));
	current->pattern = (uint8_t*)malloc(sizeof(uint8_t)*size);
	for (size_t i=0;i<size;i++) current->pattern[i] = pattern[i];
	current->mask = (uint8_t*)malloc(sizeof(uint8_t)*size);
	for (size_t i=0;i<size;i++) current->mask[i] = mask[i];
	current->size = size;
	current->position = position;
	current->next = this->jammingPatternsQueue->first;
	this->jammingPatternsQueue->size++;
	this->jammingPatternsQueue->first = current;
}

bool Radio::resetJammingPatternsQueue() {
	JammingPattern* remove = this->jammingPatternsQueue->first;
	JammingPattern* current = remove;
	while (remove != NULL) {
		current = remove->next;
		free(remove->pattern);
		free(remove->mask);
		free(remove);
		remove = current;
	}
	this->jammingPatternsQueue->first = NULL;
	this->jammingPatternsQueue->size = 0;
	return true;
}

bool Radio::removeJammingPattern(uint8_t* pattern, uint8_t* mask, size_t size, uint8_t position) {
	JammingPattern* current = this->jammingPatternsQueue->first;
	JammingPattern* remove;
	if (current == NULL) {
		return false;
	}
	else {
		if (size == current->size && position == current->position && compareBuffers(pattern,current->pattern,size) && compareBuffers(mask,current->mask,size)) {
			this->jammingPatternsQueue->first = current->next;
			this->jammingPatternsQueue->size--;
			free(current->pattern);
			free(current->mask);
			free(current);
			return true;
		}
		else {
			while (current->next != NULL) {
				if (size == current->next->size && position == current->next->position && compareBuffers(pattern,current->next->pattern,size) && compareBuffers(mask,current->next->mask,size)) {
					remove = current->next;
					current->next = current->next->next;
					this->jammingPatternsQueue->size--;
					free(remove->pattern);
					free(remove->mask);
					free(remove);
					return true;
				}
				current = current->next;
			}
			return false;
		}
	}
	return false;
}

bool Radio::checkJammingPattern(JammingPattern* pattern, uint8_t *buffer, size_t size) {
	if (pattern->position == 0xFF) {
		bool match = true;
		for (size_t pos=0;pos<size-pattern->size;pos++) {
			match = true;
			for (size_t i=0;i<pattern->size;i++) {
				match = match && ((buffer[i+pos] & pattern->mask[i]) == pattern->pattern[i]);
			}
			if (match) return true;
		}
		return false;
	}
	else {
		bool match = true;
		for (size_t i=0;i<pattern->size;i++) {
			match = match && ((buffer[i+pattern->position] & pattern->mask[i]) == pattern->pattern[i]);
		}
		return match;
	}
	return false;
}

bool Radio::checkJammingPatterns(uint8_t *buffer, size_t size) {
	bool found = false;
	JammingPattern* pattern = this->jammingPatternsQueue->first;
	while (pattern != NULL) {
		found = checkJammingPattern(pattern,buffer,size);
		if (!found) pattern = pattern->next;
		else break;
	}
	return found;
}

Controller* Radio::getController(){
	return this->controller;
}

bool Radio::setController(Controller *controller){
	this->controller = controller;
	return true;
}


RadioState Radio::getState() {
	return this->state;
}
bool Radio::setState(RadioState state) {
	this->state = state;
	return true;
}

Protocol Radio::getProtocol() {
	return this->protocol;
}
bool Radio::setProtocol(Protocol protocol) {
	this->protocol = protocol;
	return true;
}

Endianness Radio::getEndianness() {
	return this->endianness;
}

bool Radio::getFastRampUpTime() {
	return this->fastRampUpTime;
}
bool Radio::setFastRampUpTime(bool fastRampUpTime) {
	this->fastRampUpTime = fastRampUpTime;
	return true;
}

bool Radio::setEndianness(Endianness endianness) {
	this->endianness = endianness;
	return true;
}

Preamble Radio::getPreamble() {
	return this->preamble;
}

bool Radio::setPreamble(uint8_t* pattern, uint8_t size) {
	for (int i=0;i<5;i++) (this->preamble).pattern[i] = 0x00;
	for (int i=0;i<size;i++) {
		(this->preamble).pattern[i] = pattern[i];
	}
	(this->preamble).size = size;
	return true;
}

bool Radio::setPreamble(Preamble preamble) {
	this->preamble = preamble;
	return true;
}

TxPower Radio::getTxPower() {
	return this->txPower;
}

bool Radio::setTxPower(TxPower txPower) {

	this->txPower = txPower;
	return true;
}

bool Radio::setTxPower(int txPower) {
	bool success = false;
	switch (txPower) {
		case -40:
			success = this->setTxPower(NEG40_DBM);
			break;
		case -30:
			success = this->setTxPower(NEG30_DBM);
			break;
		case -20:
			success = this->setTxPower(NEG20_DBM);
			break;
		case -16:
			success = this->setTxPower(NEG16_DBM);
			break;
		case -12:
			success = this->setTxPower(NEG12_DBM);
			break;
		case -8:
			success = this->setTxPower(NEG8_DBM);
			break;
		case -4:
			success = this->setTxPower(NEG4_DBM);
			break;
		case 0:
			success = this->setTxPower(POS0_DBM);
			break;
		case 4:
			success = this->setTxPower(POS4_DBM);
			break;
		case 8:
			success = this->setTxPower(POS8_DBM);
			break;
		default:
			success = false;
	}
	return success;
}

bool Radio::isRssiEnabled() {
	return this->rssi;
}

bool Radio::enableRssi() {
	this->rssi = true;
	return true;
}
bool Radio::disableRssi() {
	this->rssi = false;
	return true;
}

Phy Radio::getPhy() {
	return this->phy;
}

bool Radio::setPhy(Phy phy) {
	this->phy = phy;
	return true;
}

Whitening Radio::getWhitening() {
	return this->whitening;
}

bool Radio::setWhitening(Whitening whitening) {
	this->whitening = whitening;
	return true;
}

uint8_t Radio::getWhiteningDataIv() {
	return this->whiteningDataIv;
}
bool Radio::setWhiteningDataIv(uint8_t iv) {
	this->whiteningDataIv = iv;
	return true;
}

Crc Radio::getCrc() {
	return this->crc;
}
bool Radio::setCrc(Crc crc) {
	this->crc = crc;
	return true;
}

uint8_t Radio::getCrcSize() {
	return this->crcSize;
}

bool Radio::setCrcSize(uint8_t crcSize) {
	this->crcSize = crcSize;
	return true;
}

uint32_t Radio::getCrcInit() {
	return this->crcInit;
}

bool Radio::setCrcInit(uint32_t init) {
	this->crcInit = init;
	return true;
}

uint32_t Radio::getCrcPoly() {
	return this->crcPoly;
}

bool Radio::setCrcPoly(uint32_t poly) {
	this->crcPoly = poly;
	return true;
}

bool Radio::getCrcSkipAddress() {
	return crcSkipAddress;
}
bool Radio::setCrcSkipAddress(bool skip) {
	this->crcSkipAddress = skip;
	return true;
}


bool Radio::isAutoTXafterRXenabled() {
	return this->autoTXafterRXenabled;
}
bool Radio::enableAutoTXafterRX() {
	this->autoTXafterRXenabled = true;
	return true;
}

bool Radio::disableAutoTXafterRX() {
	this->autoTXafterRXenabled = false;
	return true;
}
uint8_t Radio::getInterFrameSpacing() {
	return this->interFrameSpacing;
}
bool Radio::setInterFrameSpacing(uint8_t ifs) {
	this->interFrameSpacing = ifs;
	return true;
}

Header Radio::getHeader() {
	return this->header;
}
bool Radio::setHeader(uint8_t s0, uint8_t length, uint8_t s1) {
	(this->header).s0 = s0;
	(this->header).length = length;
	(this->header).s1 = s1;
	return true;
}

bool Radio::setHeader(Header header) {
	this->header = header;
	return true;
}

uint8_t Radio::getPayloadLength() {
	return this->payloadLength;
}

bool Radio::setPayloadLength(uint8_t payloadLength) {
	this->payloadLength = payloadLength;
	return true;
}

uint8_t Radio::getExpandPayloadLength() {
	return this->expandPayloadLength;
}

bool Radio::setExpandPayloadLength(uint8_t expandPayloadLength) {
	this->expandPayloadLength = expandPayloadLength;
	return true;
}

bool Radio::setFrequency(int frequency) {
	this->frequency = frequency;
	return true;
}

int Radio::getFrequency() {
	return this->frequency;
}

RadioMode Radio::getMode() {
	return this->mode;
}

bool Radio::setMode(RadioMode mode) {
	this->mode = mode;
	return true;
}

bool Radio::enableFilter(BLEAddress address) {
	for (int i=0;i<6;i++) this->filter.bytes[i] = address.bytes[i];
	this->filterEnabled = true;
	return true;
}

bool Radio::isFilterEnabled() {
	return this->filterEnabled;
}

bool Radio::disableFilter() {
	for (int i=0;i<6;i++) this->filter.bytes[i] = 0;
	this->filterEnabled = false;
	return true;
}


bool Radio::setPrefixes() {
	this->prefixes.number = 0;
	this->prefixes.prefixes[0] = 0x00;
	this->prefixes.prefixes[1] = 0x00;
	this->prefixes.prefixes[2] = 0x00;
	return true;
}
bool Radio::setPrefixes(uint8_t a) {
	this->prefixes.number = 1;
	this->prefixes.prefixes[0] = a;
	this->prefixes.prefixes[1] = 0x00;
	this->prefixes.prefixes[2] = 0x00;
	return true;
}
bool Radio::setPrefixes(uint8_t a,uint8_t b) {
	this->prefixes.number = 2;
	this->prefixes.prefixes[0] = a;
	this->prefixes.prefixes[1] = b;
	this->prefixes.prefixes[2] = 0x00;
	return true;
}
bool Radio::setPrefixes(uint8_t a,uint8_t b,uint8_t c) {
	this->prefixes.number = 3;
	this->prefixes.prefixes[0] = a;
	this->prefixes.prefixes[1] = b;
	this->prefixes.prefixes[2] = c;
	return true;
}


bool Radio::disable() {
	bool success = false;
	if (NRF_RADIO->STATE > 0)
	{
		NVIC_ClearPendingIRQ(RADIO_IRQn);
		NVIC_DisableIRQ(RADIO_IRQn);
		NRF_RADIO->EVENTS_DISABLED = 0;
		NRF_RADIO->TASKS_DISABLE = 1;
		while (NRF_RADIO->EVENTS_DISABLED == 0);
		success = true;
	}
	return success;
}
bool Radio::generateTxPowerRegister() {

	switch (this->txPower) {
		case NEG40_DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg40dBm << RADIO_TXPOWER_TXPOWER_Pos);
			return true;
		case NEG30_DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg30dBm << RADIO_TXPOWER_TXPOWER_Pos);
			return true;
		case NEG20_DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg20dBm << RADIO_TXPOWER_TXPOWER_Pos);
			return true;
		case NEG16_DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg16dBm << RADIO_TXPOWER_TXPOWER_Pos);
			return true;
		case NEG12_DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg12dBm << RADIO_TXPOWER_TXPOWER_Pos);
			return true;
		case NEG8_DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg8dBm << RADIO_TXPOWER_TXPOWER_Pos);
			return true;
		case NEG4_DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Neg4dBm << RADIO_TXPOWER_TXPOWER_Pos);
			return true;
		case POS0_DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos);
			return true;
		case POS4_DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Pos4dBm << RADIO_TXPOWER_TXPOWER_Pos);
			return true;
		case POS8_DBM:
			NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Pos8dBm << RADIO_TXPOWER_TXPOWER_Pos);
			return true;
	}
	return false;
}

bool Radio::generateModeCnf0Register() {
	if (this->fastRampUpTime) {
		NRF_RADIO->MODECNF0 |= 1;//| (1 << 8) /* glitch :) ?*/;
	}
	else {
		NRF_RADIO->MODECNF0 &= 0;
	}
	return true;
}

bool Radio::generateFrequencyRegister() {
	bool success = false;
	if (this->frequency >= 0 && this->frequency <= 100) {
		NRF_RADIO->FREQUENCY = this->frequency;
		success = true;
	}
	return success;
}
bool Radio::generateModeRegister() {
	bool success = true;
	switch (this->phy) {
		case ESB_1MBITS:
			NRF_RADIO->MODE = (RADIO_MODE_MODE_Nrf_1Mbit << RADIO_MODE_MODE_Pos);
			break;
		case ESB_2MBITS:
			NRF_RADIO->MODE = (RADIO_MODE_MODE_Nrf_2Mbit << RADIO_MODE_MODE_Pos);
			break;
		case BLE_1MBITS:
			NRF_RADIO->MODE = (RADIO_MODE_MODE_Ble_1Mbit << RADIO_MODE_MODE_Pos);
			break;
		case DOT15D4_WAZABEE:
		case BLE_2MBITS:
			NRF_RADIO->MODE = (RADIO_MODE_MODE_Ble_2Mbit << RADIO_MODE_MODE_Pos);
			break;
		case DOT15D4_NATIVE:
			NRF_RADIO->MODE = (RADIO_MODE_MODE_Ieee802154_250Kbit << RADIO_MODE_MODE_Pos);
			break;
		default:
			success = false;
			break;
	}
	return success;
}

bool Radio::generateBaseAndPrefixRegisters() {
	bool success = false;

	NRF_RADIO->TXADDRESS = 0;
	NRF_RADIO->RXADDRESSES = 1;
	if (this->phy == DOT15D4_NATIVE) {
		NRF_RADIO->SFD = 0xA7;
		NRF_RADIO->MHRMATCHCONF = 0;
		NRF_RADIO->MHRMATCHMAS = 0;
		return true;
	}
	if (this->endianness == BIG) {
		if ((this->preamble).size == 5) {
			NRF_RADIO->BASE0 = bytewise_bit_swap(
				((uint32_t)(this->preamble).pattern[3])<<24 |
				((uint32_t)(this->preamble).pattern[0]) |
				((uint32_t)(this->preamble).pattern[2])<<16  |
				((uint32_t)(this->preamble).pattern[1])<<8
			);
			NRF_RADIO->PREFIX0 = bytewise_bit_swap((this->preamble).pattern[4]) & RADIO_PREFIX0_AP0_Msk;

			if (this->prefixes.number > 0) {
				NRF_RADIO->BASE1 = bytewise_bit_swap(
					((uint32_t)(this->preamble).pattern[3])<<24 |
					((uint32_t)(this->preamble).pattern[0]) |
					((uint32_t)(this->preamble).pattern[2])<<16  |
					((uint32_t)(this->preamble).pattern[1])<<8
				);
				for (size_t i=0;i<this->prefixes.number;i++) {
					NRF_RADIO->PREFIX0 |= ((bytewise_bit_swap((this->prefixes).prefixes[i]) & 0xFF) << (8*(i+1)));
					NRF_RADIO->RXADDRESSES |= 1 << (i+1);
				}
			}
			success = true;
		}
		else if ((this->preamble).size == 4) {
			NRF_RADIO->BASE0 = bytewise_bit_swap(
				((uint32_t)(this->preamble).pattern[2])<<24 |
				((uint32_t)0x00) |
				((uint32_t)(this->preamble).pattern[1])<<16  |
				((uint32_t)(this->preamble).pattern[0])<<8
			);


			NRF_RADIO->PREFIX0 = bytewise_bit_swap((this->preamble).pattern[3]) & RADIO_PREFIX0_AP0_Msk;

			if (this->prefixes.number > 0) {
				NRF_RADIO->BASE1 = bytewise_bit_swap(
					((uint32_t)(this->preamble).pattern[2])<<24 |
					((uint32_t)0x00) |
					((uint32_t)(this->preamble).pattern[1])<<16  |
					((uint32_t)(this->preamble).pattern[0])<<8
				);
				for (size_t i=0;i<this->prefixes.number;i++) {
					NRF_RADIO->PREFIX0 |= ((bytewise_bit_swap((this->prefixes).prefixes[i]) & 0xFF) << (8*(i+1)));
					NRF_RADIO->RXADDRESSES |= 1 << (i+1);
				}
			}
			success = true;
		}
		else if ((this->preamble).size == 3) {
			NRF_RADIO->BASE0 = bytewise_bit_swap(
				((uint32_t)(this->preamble).pattern[1])<<24 |
				((uint32_t)0x00) |
				((uint32_t)(this->preamble).pattern[0])<<16  |
				((uint32_t)0x00)<<8
			);


			NRF_RADIO->PREFIX0 = bytewise_bit_swap((this->preamble).pattern[2]) & RADIO_PREFIX0_AP0_Msk;
			if (this->prefixes.number > 0) {
				NRF_RADIO->BASE1 = bytewise_bit_swap(
					((uint32_t)(this->preamble).pattern[1])<<24 |
					((uint32_t)0x00) |
					((uint32_t)(this->preamble).pattern[0])<<16  |
					((uint32_t)0x00)<<8
				);
				for (size_t i=0;i<this->prefixes.number;i++) {
					NRF_RADIO->PREFIX0 |= ((bytewise_bit_swap((this->prefixes).prefixes[i]) & 0xFF) << (8*(i+1)));
					NRF_RADIO->RXADDRESSES |= 1 << (i+1);
				}
			}
			success = true;
		}
		else if ((this->preamble).size == 2) {
			NRF_RADIO->BASE0 = bytewise_bit_swap(
				((uint32_t)(this->preamble).pattern[0])<<24 |
				((uint32_t)0x00) |
				((uint32_t)0x00)<<16  |
				((uint32_t)0x00)<<8
			);


			NRF_RADIO->PREFIX0 = bytewise_bit_swap((this->preamble).pattern[1]) & RADIO_PREFIX0_AP0_Msk;
			if (this->prefixes.number > 0) {
				NRF_RADIO->BASE1 = bytewise_bit_swap(
					((uint32_t)(this->preamble).pattern[0])<<24 |
					((uint32_t)0x00) |
					((uint32_t)0x00)<<16  |
					((uint32_t)0x00)<<8
				);
				for (size_t i=0;i<this->prefixes.number;i++) {
					NRF_RADIO->PREFIX0 |= ((bytewise_bit_swap((this->prefixes).prefixes[i]) & 0xFF) << (8*(i+1)));
					NRF_RADIO->RXADDRESSES |= 1 << (i+1);
				}
			}
			success = true;
		}
		else if ((this->preamble).size == 1) {
			NRF_RADIO->BASE0 = bytewise_bit_swap(
				((uint32_t)0x00)<<24 |
				((uint32_t)0x00) |
				((uint32_t)0x00)<<16  |
				((uint32_t)0x00)<<8
			);


			NRF_RADIO->PREFIX0 = bytewise_bit_swap((this->preamble).pattern[0]) & RADIO_PREFIX0_AP0_Msk;
			if (this->prefixes.number > 0) {
				NRF_RADIO->BASE1 = bytewise_bit_swap(
					((uint32_t)0x00)<<24 |
					((uint32_t)0x00) |
					((uint32_t)0x00)<<16  |
					((uint32_t)0x00)<<8
				);
				for (size_t i=0;i<this->prefixes.number;i++) {
					NRF_RADIO->PREFIX0 |= ((bytewise_bit_swap((this->prefixes).prefixes[i]) & 0xFF) << (8*(i+1)));
					NRF_RADIO->RXADDRESSES |= 1 << (i+1);
				}
			}
			success = true;
		}
	}
	else {
		if ((this->preamble).size == 1) {
			NRF_RADIO->BASE0 = 0x00000000;
			NRF_RADIO->PREFIX0 = (this->preamble).pattern[0];
			if (this->prefixes.number > 0) {
				NRF_RADIO->BASE1 = (this->preamble).pattern[0];
				for (size_t i=0;i<this->prefixes.number;i++) {
					NRF_RADIO->PREFIX0 |= ((((this->prefixes).prefixes[i]) & 0xFF) << (8*(i+1)));
					NRF_RADIO->RXADDRESSES |= 1 << (i+1);
				}
			}
			success = true;
		}
		else if ((this->preamble).size == 2) {
			NRF_RADIO->BASE0 = (
				(((uint32_t)(this->preamble).pattern[1]) << 24) |
				(((uint32_t)0x00) << 16) |
				(((uint32_t)0x00) << 8) |
				(((uint32_t)0x00))
			);
			NRF_RADIO->PREFIX0 = (this->preamble).pattern[0] & RADIO_PREFIX0_AP0_Msk;
			if (this->prefixes.number > 0) {
				NRF_RADIO->BASE1 = (
					(((uint32_t)(this->preamble).pattern[1]) << 24) |
					(((uint32_t)0x00) << 16) |
					(((uint32_t)0x00) << 8) |
					(((uint32_t)0x00))
				);
				for (size_t i=0;i<this->prefixes.number;i++) {
					NRF_RADIO->PREFIX0 |= ((((this->prefixes).prefixes[i]) & 0xFF) << (8*(i+1)));
					NRF_RADIO->RXADDRESSES |= 1 << (i+1);
				}
			}
			success = true;
		}
		else if ((this->preamble).size == 3) {
			NRF_RADIO->BASE0 = (
				(((uint32_t)(this->preamble).pattern[1]) << 24) |
				(((uint32_t)(this->preamble).pattern[2]) << 16) |
				(((uint32_t)0x00) << 8) |
				(((uint32_t)0x00))
			);
			NRF_RADIO->PREFIX0 = (this->preamble).pattern[0] & RADIO_PREFIX0_AP0_Msk;
			if (this->prefixes.number > 0) {
				NRF_RADIO->BASE1 = (
					(((uint32_t)(this->preamble).pattern[1]) << 24) |
					(((uint32_t)(this->preamble).pattern[2]) << 16) |
					(((uint32_t)0x00) << 8) |
					(((uint32_t)0x00))
				);
				for (size_t i=0;i<this->prefixes.number;i++) {
					NRF_RADIO->PREFIX0 |= ((((this->prefixes).prefixes[i]) & 0xFF) << (8*(i+1)));
					NRF_RADIO->RXADDRESSES |= 1 << (i+1);
				}
			}
			success = true;
		}
		else if ((this->preamble).size == 4) {
			NRF_RADIO->BASE0 = (
				(((uint32_t)(this->preamble).pattern[1]) << 24) |
				(((uint32_t)(this->preamble).pattern[2]) << 16) |
				(((uint32_t)(this->preamble).pattern[3]) << 8) |
				(((uint32_t)0x00))
			);
			NRF_RADIO->PREFIX0 = (this->preamble).pattern[0] & RADIO_PREFIX0_AP0_Msk;
			if (this->prefixes.number > 0) {
				NRF_RADIO->BASE1 = (
					(((uint32_t)(this->preamble).pattern[1]) << 24) |
					(((uint32_t)(this->preamble).pattern[2]) << 16) |
					(((uint32_t)(this->preamble).pattern[3]) << 8) |
					(((uint32_t)0x00))
				);
				for (size_t i=0;i<this->prefixes.number;i++) {
					NRF_RADIO->PREFIX0 |= ((((this->prefixes).prefixes[i]) & 0xFF) << (8*(i+1)));
					NRF_RADIO->RXADDRESSES |= 1 << (i+1);
				}
			}

			success = true;
		}
		else if ((this->preamble).size == 5) {
			NRF_RADIO->BASE0 = (
				(((uint32_t)(this->preamble).pattern[1]) << 24) |
				(((uint32_t)(this->preamble).pattern[2]) << 16) |
				(((uint32_t)(this->preamble).pattern[3]) << 8) |
				(((uint32_t)(this->preamble).pattern[4]))
			);
			NRF_RADIO->PREFIX0 = (this->preamble).pattern[0] & RADIO_PREFIX0_AP0_Msk;
			if (this->prefixes.number > 0) {
				NRF_RADIO->BASE1 = (
					(((uint32_t)(this->preamble).pattern[1]) << 24) |
					(((uint32_t)(this->preamble).pattern[2]) << 16) |
					(((uint32_t)(this->preamble).pattern[3]) << 8) |
					(((uint32_t)(this->preamble).pattern[4]))
				);
				for (size_t i=0;i<this->prefixes.number;i++) {
					NRF_RADIO->PREFIX0 |= ((((this->prefixes).prefixes[i]) & 0xFF) << (8*(i+1)));
					NRF_RADIO->RXADDRESSES |= 1 << (i+1);
				}
			}
			success = true;
		}
	}

	return success;
}

bool Radio::generatePcnf0Register() {
	if (this->phy == DOT15D4_NATIVE) {
		NRF_RADIO->PCNF0 = (8 << RADIO_PCNF0_LFLEN_Pos) |
		(RADIO_PCNF0_PLEN_32bitZero << RADIO_PCNF0_PLEN_Pos) |
		(RADIO_PCNF0_CRCINC_Include << RADIO_PCNF0_CRCINC_Pos);
		return true;
	}
	NRF_RADIO->PCNF0 =  ((this->header).s0 << RADIO_PCNF0_S0LEN_Pos)
	| ((this->header).length << RADIO_PCNF0_LFLEN_Pos)
	| ((this->header).s1 << RADIO_PCNF0_S1LEN_Pos);
	return true;
}

bool Radio::generatePcnf1Register() {
	if (this->phy == DOT15D4_NATIVE) {
		NRF_RADIO->PCNF1 = (128UL << RADIO_PCNF1_MAXLEN_Pos);
		return true;
	}
	NRF_RADIO->PCNF1 = (((this->whitening == HARDWARE_WHITENING ? RADIO_PCNF1_WHITEEN_Enabled : RADIO_PCNF1_WHITEEN_Disabled) << RADIO_PCNF1_WHITEEN_Pos)  & RADIO_PCNF1_WHITEEN_Msk) |
	(((this->endianness == LITTLE ? RADIO_PCNF1_ENDIAN_Little : RADIO_PCNF1_ENDIAN_Big) << RADIO_PCNF1_ENDIAN_Pos) & RADIO_PCNF1_ENDIAN_Msk)  |
	((((this->preamble).size-1) << RADIO_PCNF1_BALEN_Pos) & RADIO_PCNF1_BALEN_Msk ) |
	((this->expandPayloadLength << RADIO_PCNF1_STATLEN_Pos) & RADIO_PCNF1_STATLEN_Msk) |
	((this->payloadLength << RADIO_PCNF1_MAXLEN_Pos) & RADIO_PCNF1_MAXLEN_Msk);
	return true;
}
bool Radio::generateDataWhiteIvRegister() {
	bool success = true;
	if (this->whitening == HARDWARE_WHITENING) {
		NRF_RADIO->DATAWHITEIV = this->whiteningDataIv & 0x3F;
	}
	return success;
}
bool Radio::generateCrcRegisters() {
	bool success = true;
	if (this->crc == HARDWARE_CRC) {
		uint8_t crcsize = 0x00;
		switch (this->crcSize) {
			case 0:
				crcsize = 0x00;
				break;
			case 1:
				crcsize = RADIO_CRCCNF_LEN_One;
				break;
			case 2:
				crcsize = RADIO_CRCCNF_LEN_Two;
				break;
			case 3:
				crcsize = RADIO_CRCCNF_LEN_Three;
				break;
			default:
				crcsize = 0x00;
				break;
		}
		NRF_RADIO->CRCCNF = ((crcsize << RADIO_CRCCNF_LEN_Pos) << RADIO_CRCCNF_LEN_Pos)|
		((this->crcSkipAddress ? RADIO_CRCCNF_SKIP_ADDR_Skip : RADIO_CRCCNF_SKIP_ADDR_Include) << RADIO_CRCCNF_SKIP_ADDR_Pos);
		NRF_RADIO->CRCINIT = this->crcInit;
		NRF_RADIO->CRCPOLY = this->crcPoly;
	}
	else {
		NRF_RADIO->CRCCNF = 0x0;
		NRF_RADIO->CRCINIT = 0xFFFF;
		NRF_RADIO->CRCPOLY = 0x11021;
	}
	return success;
}

bool Radio::fastFrequencyChange(int frequency,uint8_t iv) {
	/* Go listening on the new channel. */

	NVIC_DisableIRQ(RADIO_IRQn);
	NRF_RADIO->EVENTS_DISABLED = 0;
	NRF_RADIO->TASKS_DISABLE = 1;
	while (NRF_RADIO->EVENTS_DISABLED == 0);

	NRF_RADIO->FREQUENCY = frequency;
	this->frequency = frequency;
	NRF_RADIO->DATAWHITEIV = iv;
	this->whiteningDataIv = iv;

	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_EnableIRQ(RADIO_IRQn);

	// enable receiver
	NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk | (this->autoTXafterRXenabled ? RADIO_SHORTS_DISABLED_TXEN_Msk : RADIO_SHORTS_DISABLED_RXEN_Msk);

	// enable receiver (once enabled, it will listen)
	NRF_RADIO->EVENTS_READY = 0;
	NRF_RADIO->EVENTS_END = 0;
	NRF_RADIO->TASKS_RXEN = 1;
	return true;
}
bool Radio::enable() {
	bool success = true;
	this->disable();

	NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
	NRF_CLOCK->TASKS_HFCLKSTART = 1;
	while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);

	success = this->generateTxPowerRegister();
	if (success) success = this->generateModeRegister();
	if (success) success = this->generateModeCnf0Register();
	if (success) success = this->generateFrequencyRegister();
	if (success) success = this->generateBaseAndPrefixRegisters();
	if (success) success = this->generateCrcRegisters();
	if (success) success = this->generatePcnf0Register();
	if (success) success = this->generatePcnf1Register();
	if (success) success = this->generateDataWhiteIvRegister();

	if (success) {
		NRF_RADIO->TIFS = this->interFrameSpacing;
		NRF_RADIO->PACKETPTR = (uint32_t)(this->rxBuffer);
		if (this->mode == MODE_NORMAL) {
			if (this->isFilterEnabled()) {
				NRF_RADIO->DAB[0] = ((uint32_t)(this->filter.bytes[2] << 24) |
				(uint32_t)(this->filter.bytes[3] << 16) |
				(uint32_t)(this->filter.bytes[4] << 8) |
				(uint32_t)(this->filter.bytes[5]));
				NRF_RADIO->DAP[0] = (uint32_t)(this->filter.bytes[0] << 8) | (uint32_t)(this->filter.bytes[1]);
				NRF_RADIO->DACNF = 1;
				NRF_RADIO->INTENSET = 0x00000008 | 1 << 5;
			}
			else {
				NRF_RADIO->DACNF = 0;
				NRF_RADIO->INTENSET = 0x00000008;
			}
			NVIC_ClearPendingIRQ(RADIO_IRQn);
			NVIC_EnableIRQ(RADIO_IRQn);

			NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk | (this->autoTXafterRXenabled ? RADIO_SHORTS_DISABLED_TXEN_Msk : RADIO_SHORTS_DISABLED_RXEN_Msk);
			if (this->rssi) {
				NRF_RADIO->SHORTS |= RADIO_SHORTS_ADDRESS_RSSISTART_Msk;
			}

			NRF_RADIO->EVENTS_END = 0;
			NRF_RADIO->EVENTS_READY = 0;
			NRF_RADIO->TASKS_RXEN = 1;

			this->state = RX;

		}
		else if (this->mode == MODE_JAMMER) {
			NRF_RADIO->INTENSET = 0x00000008;
			NVIC_ClearPendingIRQ(RADIO_IRQn);
			NVIC_EnableIRQ(RADIO_IRQn);

			NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk | RADIO_SHORTS_DISABLED_TXEN_Msk;
			if (this->rssi) {
				NRF_RADIO->SHORTS |= RADIO_SHORTS_ADDRESS_RSSISTART_Msk;
			}
			if (this->jammingPatternsEnabled) {
				NRF_RADIO->BCC = this->jammingPatternsCounter;//8+6*2*8;
				NRF_RADIO->SHORTS |= RADIO_SHORTS_ADDRESS_BCSTART_Msk;
				NRF_RADIO->INTENSET |= 1 << 10; // enable BCMATCH event
			}
			NRF_RADIO->EVENTS_READY = 0;
			NRF_RADIO->EVENTS_END = 0;
			NRF_RADIO->TASKS_RXEN = 1;
			this->state = JAM_RX;
		}
	}
	return success;
}

bool Radio::reload() {
	this->disable();
	return this->enable();
}

bool Radio::updateTXBuffer(uint8_t *data, uint8_t size) {
	for (int i=0;i<size;i++) {
		this->txBuffer[i] = data[i];
	}
	return true;
}

bool Radio::send(uint8_t *data,int size,int frequency, uint8_t channel) {
	NVIC_DisableIRQ(RADIO_IRQn);
	NRF_RADIO->SHORTS = 0;

	NRF_RADIO->EVENTS_DISABLED = 0;
	NRF_RADIO->TASKS_DISABLE = 1;
	while(NRF_RADIO->EVENTS_DISABLED == 0);

	NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk | RADIO_SHORTS_DISABLED_RXEN_Msk;
	if (this->rssi) {
		NRF_RADIO->SHORTS |= RADIO_SHORTS_ADDRESS_RSSISTART_Msk;
	}
	NRF_RADIO->FREQUENCY = frequency;
	NRF_RADIO->DATAWHITEIV = channel;
	NRF_RADIO->PACKETPTR = (uint32_t)data;

	// Turn on the transmitter, and wait for it to signal that it's ready to use.
	this->state = TX;
	NRF_RADIO->INTENSET =  0x00000008;

	NVIC_EnableIRQ(RADIO_IRQn);
	NRF_RADIO->EVENTS_READY = 0;
	NRF_RADIO->EVENTS_END = 0;
	NRF_RADIO->TASKS_TXEN = 1;


	return false;
}

static uint8_t jamBuffer[] = {0x55,0x55,0x55,0x55};
extern "C" void RADIO_IRQHandler(void) {

	if (NRF_RADIO->EVENTS_READY) {
		NRF_RADIO->EVENTS_READY = 0;
		NRF_RADIO->TASKS_START = 1;
	}

	if (NRF_RADIO->EVENTS_BCMATCH) {
		NRF_RADIO->EVENTS_BCMATCH = 0;
		bsp_board_led_invert(0);
		if (Radio::instance->checkJammingPatterns(Radio::instance->rxBuffer,Radio::instance->getJammingPatternsCounter()/8)) {
			NRF_RADIO->TASKS_STOP = 1;
			Radio::instance->reload();
		}
	}
	if (NRF_RADIO->EVENTS_END && (Radio::instance->isFilterEnabled() ? NRF_RADIO->EVENTS_DEVMATCH : 1)) {
		NRF_RADIO->EVENTS_END = 0;
		if (Radio::instance->isFilterEnabled()) NRF_RADIO->EVENTS_DEVMATCH = 0;
		NRF_TIMER1->TASKS_CAPTURE[0] = 1UL;
		uint32_t now = NRF_TIMER1->CC[0];

		Controller *controller = Radio::instance->getController();
		if (controller != NULL) {
			if (Radio::instance->getMode() == MODE_NORMAL) {
				if (Radio::instance->getState() == TX) {
					//LedManager::instance->toggle(LED1);
					NRF_RADIO->PACKETPTR = (uint32_t)Radio::instance->rxBuffer;
					Radio::instance->setState(RX);
				}
				else if (Radio::instance->getState() == RX) {
					uint8_t bufferSize = 0;
					if (Radio::instance->getHeader().s0 != 0) {
						bufferSize += 1;
					}
					if (Radio::instance->getHeader().s1 != 0) {
						bufferSize += 1;
					}
					if (Radio::instance->getHeader().length != 0) {
						bufferSize += 1+(Radio::instance->getHeader().s0 == 0 ? Radio::instance->rxBuffer[0] : Radio::instance->rxBuffer[1]);
					}
					else {
						bufferSize += Radio::instance->getPayloadLength();
					}
					if (bufferSize <= 2+Radio::instance->getPayloadLength()) {
						uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t)*bufferSize);
						memcpy(buffer,Radio::instance->rxBuffer,bufferSize);
						CrcValue crcValue;
						crcValue.validity = UNKNOWN_CRC;
						uint8_t rssi = 0x00;
						if (Radio::instance->getCrc() == HARDWARE_CRC) {
							crcValue.validity = (NRF_RADIO->CRCSTATUS == 1 ? VALID_CRC : INVALID_CRC);
							crcValue.value = NRF_RADIO->RXCRC;
						}
						if (Radio::instance->isRssiEnabled()) {
							rssi = NRF_RADIO->RSSISAMPLE;
						}
						Phy p = Radio::instance->getPhy();

						Radio::instance->currentTimestamp = now - (Radio::instance->getPreamble().size+bufferSize)  * 4 * (p == BLE_2MBITS || p == ESB_2MBITS ? 1 : 2) - 100;
						controller->onReceive(Radio::instance->currentTimestamp,bufferSize,buffer,crcValue, rssi);

						free(buffer);
						if (Radio::instance->isAutoTXafterRXenabled()) {
							NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk | RADIO_SHORTS_DISABLED_RXEN_Msk;
							NRF_RADIO->PACKETPTR = (uint32_t)(Radio::instance->txBuffer);
							Radio::instance->setState(TX);
						}
						if (Radio::instance->isRssiEnabled()) {
							NRF_RADIO->SHORTS |= RADIO_SHORTS_ADDRESS_RSSISTART_Msk;
						}

					}
				}
			}
			else if (Radio::instance->getMode() == MODE_JAMMER) {
				if (Radio::instance->getState() == JAM_RX) {
					NRF_RADIO->PACKETPTR = (uint32_t)jamBuffer;
					NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk | RADIO_SHORTS_DISABLED_RXEN_Msk | RADIO_SHORTS_ADDRESS_BCSTART_Msk;
					Radio::instance->setState(JAM_TX);

				}
				else if (Radio::instance->getState() == JAM_TX) {
					NRF_RADIO->PACKETPTR = (uint32_t)Radio::instance->rxBuffer;
					NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk | RADIO_SHORTS_DISABLED_TXEN_Msk | RADIO_SHORTS_ADDRESS_BCSTART_Msk;
					Radio::instance->setState(JAM_RX);
					controller->onJam(now);
				}
			}
		}
		NRF_RADIO->TASKS_START = 1;
	}

}
