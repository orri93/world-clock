#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

#include <TM1637plus.h>

#define DISPLAY_BRIGHTNESS 7

class Display {
  public:
    Display(uint8_t clk_pin, uint8_t dio_pin);
    void initiate();

  private:
    TM1637plus_modelX instance;
};

#endif
