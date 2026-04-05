#ifndef WEBINTERFACE_H
#define WEBINTERFACE_H

#include <Arduino.h>
#include "types.h"
#include "display.h"

/* Initialize the HTTP web server on port 80 and register all route handlers.
   Pointers to shared state are stored internally; they must remain valid for
   the entire lifetime of the program.  Call once from setup().

   Routes exposed:
     GET  /               — HTML management page
     GET  /status         — JSON snapshot of times, alarm, and brightness
     POST /set/alarm      — body: hour=<0-23>&minute=<0-59>
     POST /set/brightness — body: value=<0-7>                             */
void webinterface_initiate(
    ClockTime *clockTime,
    LocalTime *icelandTime,
    LocalTime *houstonTime,
    LocalTime *bangkokTime,
    uint8_t   *alarmHour,
    uint8_t   *alarmMinute,
    uint8_t   *brightness,
    Display   *display1,
    Display   *display2,
    Display   *display3
);

/* Process one pending HTTP request. Call once per loop() iteration. */
void webinterface_handle();

#endif /* WEBINTERFACE_H */
