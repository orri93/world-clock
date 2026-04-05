#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

#include <TM1637plus.h>

#include "types.h"

#define DISPLAY_BRIGHTNESS 7

class Display {
  public:
    Display(uint8_t clk_pin, uint8_t dio_pin);

    // Initialize the TM1637 driver and apply the default brightness.
    void initiate();

    // Set display brightness (0 = dimmost, 7 = brightest). Values above 7 are clamped to 7.
    void setBrightness(uint8_t brightness);

    // Fill all four digits with '-' to indicate data is not yet available.
    void showUnavailable();

    // Show HH:MM with a 1 Hz blinking colon. Falls back to showUnavailable() if time.isAvailable is false.
    void showTime(const LocalTime &time);

    // Show MMDD with a fixed center separator. Falls back to showUnavailable() if time.isAvailable is false.
    void showDate(const LocalTime &time);

    // Show HH:MM for an alarm time with a fixed center separator.
    void showAlarm(uint8_t hour, uint8_t minute);

    // Show a two-digit brightness level (00-07) on the right two digits, left two digits blank.
    void showBrightness(uint8_t brightness);

  private:
    TM1637plus_modelX instance;
};

#endif
