#ifndef SERIALDEBUG_H
#define SERIALDEBUG_H

#include <Arduino.h>

#include "types.h"
#include "mode.h"

void serial_debug_initiate(long baud);
void serial_debug_wifi_connecting();
void serial_debug_wifi_connection_success(const IPAddress& ip);
void serial_debug_wifi_lost_connection();
void serial_debug_wifi_reestablished_connection(const IPAddress& ip);

void serial_debug_rtc_not_connected();
void serial_debug_rtc_not_running();

void serial_debug_time_synced(const ClockTime& time);

void serial_debug_mode_changed(Mode mode);
void serial_debug_brightness_changed(uint8_t brightness);

#endif /* SERIALDEBUG_H */
