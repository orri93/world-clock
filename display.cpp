#include <Arduino.h>

#include "display.h"

#define DISPLAY_COMMUNICATION_DELAY   75
#define DISPLAY_NUMBER_OF_DIGITS       4

Display::Display(uint8_t clk_pin, uint8_t dio_pin) : instance(clk_pin, dio_pin, DISPLAY_COMMUNICATION_DELAY, DISPLAY_NUMBER_OF_DIGITS) {
}

void Display::initiate() {
  instance.displayBegin();
  instance.setBrightness(DISPLAY_BRIGHTNESS, true);
}
