#include "time.h"
#include "bsp.h"
TimeModule *TimeModule::instance = NULL;

TimeModule::TimeModule() {
	instance = this;

	// Enable High frequency clock if it is disabled
	if (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0UL) {
		NRF_CLOCK->TASKS_HFCLKSTART = 1UL;
		while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0UL);
	}
	NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer;
	NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
	NRF_TIMER1->PRESCALER = PRESCALER_VALUE;

	NRF_TIMER1->INTENCLR = TIMER_INTENCLR_COMPARE0_Msk
		| TIMER_INTENCLR_COMPARE1_Msk
		| TIMER_INTENCLR_COMPARE2_Msk
		| TIMER_INTENCLR_COMPARE3_Msk;

	NVIC_SetPriority(TIMER1_IRQn, 1);

	NRF_TIMER1->TASKS_CLEAR = 1;
	NRF_TIMER1->TASKS_START = 1;


}
void TimeModule::setBLEHopInterval(uint16_t hopInterval) {
	this->hopInterval = hopInterval;
}
uint16_t TimeModule::getBLEHopInterval() {
	return this->hopInterval;
}
uint32_t TimeModule::getLastBLEAnchorPoint() {
	return  this->lastAnchorPoint;
}

uint32_t TimeModule::getTimestamp() {
	NRF_TIMER1->TASKS_CAPTURE[0] = 1UL;
	uint32_t ticks = NRF_TIMER1->CC[0];
	return ticks;
}

void TimeModule::followBLEConnection(uint16_t hopInterval,int masterSCA, CtrlCallback channelCallback,CtrlCallback injectionCallback, CtrlCallback masterCallback, Controller *controllerInstance) {
	this->hopInterval = hopInterval;
	this->channelCallback = channelCallback;
	this->injectionCallback = injectionCallback;
	this->masterCallback = masterCallback;
	this->controllerInstance = controllerInstance;
	this->switchDelay = 250;

	this->window = 0;
	this->masterSCA = masterSCA;
	this->slaveSCA = 20;//500;
	this->injection = false;

	this->masterTime = 6 * 1250UL;
	this->masterMode = false;
}

void TimeModule::setBLEAnchorPoint(uint32_t timestamp) {
	NVIC_DisableIRQ(TIMER1_IRQn);
	NRF_TIMER1->CC[1] = timestamp + this->hopInterval*1250UL - this->switchDelay;
	this->lastAnchorPoint = timestamp;
	// Estimate the window widening value
	this->window = (int)(((double)(this->masterSCA+this->slaveSCA) / 1000000.0) * (this->hopInterval * 1250)) + 32;

	NRF_TIMER1->CC[2] = timestamp + this->hopInterval*1250UL - this->window;
	NRF_TIMER1->INTENCLR = TIMER_INTENCLR_COMPARE0_Msk
		| TIMER_INTENCLR_COMPARE1_Msk
		| TIMER_INTENCLR_COMPARE2_Msk
		| TIMER_INTENCLR_COMPARE3_Msk;

	NRF_TIMER1->CC[3] = timestamp + this->masterTime;

	if (this->injection && this->masterMode) {
		NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE1_Msk | TIMER_INTENSET_COMPARE2_Msk | TIMER_INTENSET_COMPARE3_Msk;
	}
	else if (this->injection) {
		NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE1_Msk | TIMER_INTENSET_COMPARE2_Msk;
	}
	else if (this->masterMode) {
		NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE1_Msk | TIMER_INTENSET_COMPARE3_Msk;
	}
	else {
		NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE1_Msk;
	}

	NVIC_ClearPendingIRQ(TIMER1_IRQn);
	NVIC_EnableIRQ(TIMER1_IRQn);

}
void TimeModule::exitBLEConnection() {
	NVIC_DisableIRQ(TIMER1_IRQn);
	NRF_TIMER1->CC[0] = 0;
	NRF_TIMER1->CC[1] = 0;
	NRF_TIMER1->CC[2] = 0;
	NRF_TIMER1->CC[3] = 0;
	this->lastAnchorPoint = 0;
	this->injection = false;
	this->masterMode = false;
	NRF_TIMER1->INTENCLR = TIMER_INTENCLR_COMPARE0_Msk
		| TIMER_INTENCLR_COMPARE1_Msk
		| TIMER_INTENCLR_COMPARE2_Msk
		| TIMER_INTENCLR_COMPARE3_Msk;
	NVIC_ClearPendingIRQ(TIMER1_IRQn);
	NVIC_EnableIRQ(TIMER1_IRQn);
}

void TimeModule::startBLEInjection(){
	this->injection = true;
}

void TimeModule::stopBLEInjection(){
	this->injection = false;
}

void TimeModule::enterBLEMasterMode() {
	this->masterMode = true;
}

void TimeModule::exitBLEMasterMode() {
	this->masterMode = false;
}

void TimeModule::setBLEMasterTime(uint32_t time) {
	this->masterTime = time;
}

extern "C" void TIMER1_IRQHandler(void) {
	if (NRF_TIMER1->EVENTS_COMPARE[1]) {
		NRF_TIMER1->EVENTS_COMPARE[1] = 0UL;

		NRF_TIMER1->TASKS_CAPTURE[0] = 1UL;
		uint32_t now = NRF_TIMER1->CC[0];

		bool estimate = (TimeModule::instance->controllerInstance ->* TimeModule::instance->channelCallback)();
		if (estimate) {
			NRF_TIMER1->CC[1] = now + (TimeModule::instance->getBLEHopInterval() * 1250UL) - 11;
			TimeModule::instance->lastAnchorPoint = now + TimeModule::instance->switchDelay - 20;
			NRF_TIMER1->INTENSET |= TIMER_INTENSET_COMPARE1_Msk;
		}
		else {
			NRF_TIMER1->CC[1] = 0;
			NRF_TIMER1->INTENCLR |= TIMER_INTENCLR_COMPARE1_Msk;
		}
	}

	if (NRF_TIMER1->EVENTS_COMPARE[2] && TimeModule::instance->injection) {
		NRF_TIMER1->EVENTS_COMPARE[2] = 0UL;
		(TimeModule::instance->controllerInstance ->* TimeModule::instance->injectionCallback)();
	}
	if (NRF_TIMER1->EVENTS_COMPARE[3] && TimeModule::instance->masterMode){
		NRF_TIMER1->EVENTS_COMPARE[3] = 0UL;
		NRF_TIMER1->TASKS_CAPTURE[0] = 1UL;
		uint32_t now = NRF_TIMER1->CC[0];
		(TimeModule::instance->controllerInstance ->* TimeModule::instance->masterCallback)();
		NRF_TIMER1->CC[3] = now + (TimeModule::instance->getBLEHopInterval() * 1250UL) - 11;
		NRF_TIMER1->INTENSET |= TIMER_INTENSET_COMPARE3_Msk;
	}
}
