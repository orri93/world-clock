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
