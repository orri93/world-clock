#ifndef CONVERT_H
#define CONVERT_H

#include "types.h"

LocalTime convertUtcToLocal(uint32_t epochUtc, int timezoneId);

#endif
