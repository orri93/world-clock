#ifndef CONVERT_H
#define CONVERT_H

#include "types.h"

// Convert a UTC Unix epoch to a LocalTime struct for the given TIMEZONE_* ID.
// DST is handled automatically for DST-aware zones. Returns isAvailable=false on failure.
LocalTime convertUtcToLocal(uint32_t epochUtc, int timezoneId);

#endif
