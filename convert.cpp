#include "convert.h"

#include <time.h>

#include "constdef.h"

static int dayOfWeekSundayZero(int year, int month, int day) {
  // Sakamoto's algorithm: returns 0=Sunday..6=Saturday
  static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (month < 3) {
    year -= 1;
  }
  return (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
}

static int nthSundayOfMonth(int year, int month, int nth) {
  const int firstDow = dayOfWeekSundayZero(year, month, 1);
  const int firstSundayDay = 1 + ((7 - firstDow) % 7);
  return firstSundayDay + ((nth - 1) * 7);
}

static int64_t daysFromCivil(int year, unsigned month, unsigned day) {
  year -= month <= 2;
  const int era = (year >= 0 ? year : year - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(year - era * 400);
  const unsigned doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return static_cast<int64_t>(era) * 146097 + static_cast<int64_t>(doe) - 719468;
}

static int64_t epochFromUtcDateTime(int year, int month, int day, int hour, int minute, int second) {
  return daysFromCivil(year, static_cast<unsigned>(month), static_cast<unsigned>(day)) * 86400LL +
    static_cast<int64_t>(hour) * 3600LL +
    static_cast<int64_t>(minute) * 60LL +
    static_cast<int64_t>(second);
}

static bool isHoustonDst(uint32_t epochUtc) {
  const time_t utc = static_cast<time_t>(epochUtc);
  struct tm utcTm = {};
  if (gmtime_r(&utc, &utcTm) == nullptr) {
    return false;
  }

  const int year = utcTm.tm_year + 1900;
  const int dstStartDay = nthSundayOfMonth(year, 3, 2);   // 2nd Sunday in March
  const int dstEndDay = nthSundayOfMonth(year, 11, 1);    // 1st Sunday in November

  // US Central: DST starts 02:00 local standard (UTC-6) => 08:00 UTC
  // US Central: DST ends   02:00 local daylight (UTC-5) => 07:00 UTC
  const int64_t dstStartUtc = epochFromUtcDateTime(year, 3, dstStartDay, 8, 0, 0);
  const int64_t dstEndUtc = epochFromUtcDateTime(year, 11, dstEndDay, 7, 0, 0);
  const int64_t nowUtc = static_cast<int64_t>(epochUtc);

  return nowUtc >= dstStartUtc && nowUtc < dstEndUtc;
}

static int16_t getOffsetMinutes(uint32_t epochUtc, int timezoneId, bool *isDst) {
  if (isDst != nullptr) {
    *isDst = false;
  }

  switch (timezoneId) {
    case TIMEZONE_ICELAND:
      return 0;

    case TIMEZONE_BANGKOK:
      return 7 * 60;

    case TIMEZONE_HOUSTON: {
      const bool dst = isHoustonDst(epochUtc);
      if (isDst != nullptr) {
        *isDst = dst;
      }
      return dst ? -5 * 60 : -6 * 60;
    }

    default:
      return 0;
  }
}

LocalTime convertUtcToLocal(uint32_t epochUtc, int timezoneId) {
  LocalTime localTime = {};

  bool isDst = false;
  const int16_t offsetMinutes = getOffsetMinutes(epochUtc, timezoneId, &isDst);
  const int64_t localEpochRaw = static_cast<int64_t>(epochUtc) + static_cast<int64_t>(offsetMinutes) * 60LL;
  const time_t localEpoch = static_cast<time_t>(localEpochRaw);

  struct tm localTm = {};
  if (gmtime_r(&localEpoch, &localTm) == nullptr) {
    localTime.isAvailable = false;
    return localTime;
  }

  localTime.year = static_cast<uint16_t>(localTm.tm_year + 1900);
  localTime.month = static_cast<uint8_t>(localTm.tm_mon + 1);
  localTime.day = static_cast<uint8_t>(localTm.tm_mday);
  localTime.hour = static_cast<uint8_t>(localTm.tm_hour);
  localTime.minute = static_cast<uint8_t>(localTm.tm_min);
  localTime.isDst = isDst;
  localTime.offsetMinutes = offsetMinutes;
  localTime.isAvailable = true;

  return localTime;
}
