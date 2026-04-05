#ifndef FORMAT_H
#define FORMAT_H

#include "types.h"

void formatTimeDigits(uint8_t *digits, const LocalTime &time);
void formatDateDigits(uint8_t *digits, const LocalTime &time);
void formatAlarmDigits(uint8_t *digits, uint8_t hour, uint8_t minute);

#endif
