#include <Arduino.h>

#include "display.h"
#include "format.h"

#define DISPLAY_COMMUNICATION_DELAY   75
#define DISPLAY_NUMBER_OF_DIGITS       4
#define DISPLAY_COLON_SEGMENT_MASK   0x80

Display::Display(uint8_t clk_pin, uint8_t dio_pin) : instance(clk_pin, dio_pin, DISPLAY_COMMUNICATION_DELAY, DISPLAY_NUMBER_OF_DIGITS) {
}

void Display::initiate() {
  instance.displayBegin();
  instance.setBrightness(DISPLAY_BRIGHTNESS, true);
}

void Display::setBrightness(uint8_t brightness) {
  const uint8_t safeBrightness = brightness > DISPLAY_BRIGHTNESS ? DISPLAY_BRIGHTNESS : brightness;
  instance.setBrightness(safeBrightness, true);
}

void Display::showUnavailable() {
  uint8_t unavailable = instance.encodeCharacter('-');
  for (uint8_t i = 0; i < DISPLAY_NUMBER_OF_DIGITS; i++) {
    instance.setSegments(&unavailable, 1, i);
  }
}

void Display::showTime(const LocalTime &time) {
  if (!time.isAvailable) {
    showUnavailable();
    return;
  }

  uint8_t digits[DISPLAY_NUMBER_OF_DIGITS];
  uint8_t segments[DISPLAY_NUMBER_OF_DIGITS];
  formatTimeDigits(digits, time);

  for (uint8_t i = 0; i < DISPLAY_NUMBER_OF_DIGITS; i++) {
    segments[i] = instance.encodeCharacter(static_cast<char>('0' + digits[i]));
  }

  // Blink center colon/dot at 1 Hz even when refreshing faster than 1 second.
  const bool colonOn = ((millis() / 1000UL) % 2UL) == 0UL;
  if (colonOn) {
    segments[1] |= DISPLAY_COLON_SEGMENT_MASK;
  }

  instance.setSegments(segments, DISPLAY_NUMBER_OF_DIGITS, 0);
}

void Display::showDate(const LocalTime &time) {
  if (!time.isAvailable) {
    showUnavailable();
    return;
  }

  uint8_t digits[DISPLAY_NUMBER_OF_DIGITS];
  uint8_t segments[DISPLAY_NUMBER_OF_DIGITS];
  formatDateDigits(digits, time);

  for (uint8_t i = 0; i < DISPLAY_NUMBER_OF_DIGITS; i++) {
    segments[i] = instance.encodeCharacter(static_cast<char>('0' + digits[i]));
  }

  // Keep the center separator on to visually split MM and DD.
  segments[1] |= DISPLAY_COLON_SEGMENT_MASK;

  instance.setSegments(segments, DISPLAY_NUMBER_OF_DIGITS, 0);
}

void Display::showBrightness(uint8_t brightness) {
  const uint8_t safeBrightness = brightness > DISPLAY_BRIGHTNESS ? DISPLAY_BRIGHTNESS : brightness;

  uint8_t segments[DISPLAY_NUMBER_OF_DIGITS];
  segments[0] = instance.encodeCharacter(' ');
  segments[1] = instance.encodeCharacter(' ');
  segments[2] = instance.encodeCharacter(static_cast<char>('0' + (safeBrightness / 10)));
  segments[3] = instance.encodeCharacter(static_cast<char>('0' + (safeBrightness % 10)));

  instance.setSegments(segments, DISPLAY_NUMBER_OF_DIGITS, 0);
}
