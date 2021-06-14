#include <stdbool.h>
#include <stdint.h>
#include "core.h"

void initInput(int pin) {
    NRF_GPIO->PIN_CNF[pin] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                                        | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                                        | (GPIO_PIN_CNF_PULL_Pulldown << GPIO_PIN_CNF_PULL_Pos)
                                        | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                                        | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
}

void initPPI(int pin) {
   NRF_GPIOTE->CONFIG[0] =  (GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos)
                       | (pin << GPIOTE_CONFIG_PSEL_Pos)
                       | (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);

   NRF_GPIOTE->EVENTS_IN[0] = 0;

    NRF_PPI->CH[0].EEP = (uint32_t)&(NRF_GPIOTE->EVENTS_IN[0]);
    NRF_PPI->CH[0].TEP = (uint32_t)&(NRF_TIMER1->TASKS_CLEAR);
    NRF_PPI->CHEN= 0x00000001;
}

int main(void) {
	Core core;
	core.init();
	//initInput(22);
	//initPPI(22);
	core.loop();
}
