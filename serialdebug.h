#ifndef SERIALDEBUG_H
#define SERIALDEBUG_H

#include <Arduino.h>
#include <WiFi.h>

#include "types.h"
#include "mode.h"

void serial_debug_initiate(long baud);
void serial_debug_startup_summary(bool rtcSupported, bool rtcAvailable, bool rotarySupported, bool webSupported);
void serial_debug_wifi_connecting();
void serial_debug_wifi_connection_success(const IPAddress& ip);
void serial_debug_wifi_lost_connection();
void serial_debug_wifi_reconnecting(int attempt, wl_status_t status);
void serial_debug_wifi_reestablished_connection(const IPAddress& ip);
void serial_debug_sntp_initializing(const char* server1, const char* server2, uint32_t syncIntervalMs);
void serial_debug_sntp_waiting_for_wifi();
void serial_debug_webinterface_started(uint16_t port);

void serial_debug_rtc_not_connected();
void serial_debug_rtc_not_running();

void serial_debug_time_synced(const ClockTime& time);

void serial_debug_mode_changed(Mode mode);
void serial_debug_brightness_changed(uint8_t brightness);
void serial_debug_alarm_changed(uint8_t hour, uint8_t minute);

#endif /* SERIALDEBUG_H */
