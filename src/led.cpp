#include "led.h"

LedModule::LedModule() {
	bsp_board_init(BSP_INIT_LEDS);
	this->state[LED1] = false;
	this->state[LED2] = false;
	this->ledColor = BLUE;
}

void LedModule::on(int num) {
	if (num == LED1) {
		bsp_board_led_on(LED1);
		this->state[LED1] = true;
	}
	else {
		if (this->ledColor == RED) {
			bsp_board_led_on(LED2_RED);
		}
		else if (this->ledColor == GREEN) {
			bsp_board_led_on(LED2_GREEN);
		}
		else if (this->ledColor == BLUE) {
			bsp_board_led_on(LED2_BLUE);
		}
		else if (this->ledColor == YELLOW) {
			bsp_board_led_on(LED2_RED);
			bsp_board_led_on(LED2_GREEN);
		}
		else if (this->ledColor == PURPLE) {
			bsp_board_led_on(LED2_RED);
			bsp_board_led_on(LED2_BLUE);
		}
		else if (this->ledColor == CYAN) {
			bsp_board_led_on(LED2_GREEN);
			bsp_board_led_on(LED2_BLUE);
		}
		this->state[LED2] = true;
	}
}

void LedModule::off(int num) {
	if (num == LED1) {
		bsp_board_led_off(LED1);
		this->state[LED1] = false;
	}
	else {
		bsp_board_led_off(LED2_RED);
		bsp_board_led_off(LED2_GREEN);
		bsp_board_led_off(LED2_BLUE);
		this->state[LED2] = false;
	}
}

void LedModule::toggle(int num) {
	if (num == LED1) {
		if (this->state[LED1]) this->off(LED1);
		else this->on(LED1);
	}
	else {
		if (this->state[LED2]) this->off(LED2);
		else this->on(LED2);
	}
}

bool LedModule::isOn(int num) {
	bool ret;
	if (num == LED1) ret = this->state[LED1];
	else ret = this->state[LED2];
	return ret;
}

void LedModule::setColor(LedColor color) {
	if (color != this->ledColor && this->state[LED2]) {
		this->off(LED2);
		this->ledColor = color;
		this->on(LED2);
	}
	this->ledColor = color;
}
