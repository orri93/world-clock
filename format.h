#ifndef FORMAT_H
#define FORMAT_H

#include "types.h"

// Fill digits[4] with { H/10, H%10, M/10, M%10 } from time.hour / time.minute.
void formatTimeDigits(uint8_t *digits, const LocalTime &time);

// Fill digits[4] with { Mo/10, Mo%10, D/10, D%10 } from time.month / time.day.
void formatDateDigits(uint8_t *digits, const LocalTime &time);

// Fill digits[4] with { H/10, H%10, M/10, M%10 } from the given hour and minute values.
void formatAlarmDigits(uint8_t *digits, uint8_t hour, uint8_t minute);

#endif
