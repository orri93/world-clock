#include "format.h"

void formatTimeDigits(uint8_t *digits, const LocalTime &time) {
  if (digits != nullptr) {
    digits[0] = time.hour / 10;
    digits[1] = time.hour % 10;
    digits[2] = time.minute / 10;
    digits[3] = time.minute % 10;
  }
}

void formatDateDigits(uint8_t *digits, const LocalTime &time) {
  if (digits != nullptr) {
    digits[0] = time.month / 10;
    digits[1] = time.month % 10;
    digits[2] = time.day / 10;
    digits[3] = time.day % 10;
  }
}

void formatAlarmDigits(uint8_t *digits, uint8_t hour, uint8_t minute) {
  if (digits != nullptr) {
    digits[0] = hour / 10;
    digits[1] = hour % 10;
    digits[2] = minute / 10;
    digits[3] = minute % 10;
  }
}

void formatNumberDigits(uint8_t *digits, uint16_t value) {
  if (digits != nullptr) {
    const uint16_t safeValue = value > 9999 ? 9999 : value;
    digits[0] = static_cast<uint8_t>(safeValue / 1000);
    digits[1] = static_cast<uint8_t>((safeValue / 100) % 10);
    digits[2] = static_cast<uint8_t>((safeValue / 10) % 10);
    digits[3] = static_cast<uint8_t>(safeValue % 10);
  }
}
