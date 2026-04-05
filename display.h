#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

#include <TM1637plus.h>

#include "types.h"

#define DISPLAY_BRIGHTNESS 7

class Display {
  public:
    Display(uint8_t clk_pin, uint8_t dio_pin);
    void initiate();
    void setBrightness(uint8_t brightness);

    void showUnavailable();
    void showTime(const LocalTime &time);
    void showDate(const LocalTime &time);
    void showAlarm(uint8_t hour, uint8_t minute);
    void showBrightness(uint8_t brightness);

  private:
    TM1637plus_modelX instance;
};

#endif
