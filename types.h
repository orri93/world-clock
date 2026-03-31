#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

typedef struct ClockTime {
  uint32_t epochUtc;  // single source of truth
} ClockTime;

typedef struct LocalTime {
  uint16_t year;
  uint8_t month, day, hour, minute;
  bool isDst;
  int16_t offsetMinutes;
} LocalTime;

typedef struct DisplayTime {
  uint8_t hours;
  uint8_t minutes;
} DisplayTime;

#endif
