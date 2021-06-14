#include "bsp.h"

#ifndef LED_H
#define LED_H

#define LED1 0
#define LED2 1

#define LED2_RED 1
#define LED2_GREEN 2
#define LED2_BLUE 3


typedef enum LedColor {
	RED,
	GREEN,
	BLUE,
	YELLOW,
	PURPLE,
	CYAN
} LedColor;

class LedModule
{
	private:
		bool state[2];
		LedColor ledColor;
	public:
		LedModule();

		void on(int num);
		void off(int num);
		void toggle(int num);		
		bool isOn(int num);
		void setColor(LedColor color);
};
#endif
