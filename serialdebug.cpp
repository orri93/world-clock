#include "serialdebug.h"

void serial_debug_initiate(long baud) {
  Serial.begin(baud);
  while(!Serial);
  Serial.println("World Clock Serial Debugging Initiated");
  Serial.println("Copyright 2026 Geirmundur Orri Sigurdsson");
}

void serial_debug_wifi_connecting() {
  Serial.print("Connecting to WiFi ");
}

void serial_debug_wifi_connection_success(const IPAddress& ip) {
  Serial.println("");
  Serial.print("Connected to WiFi with IP: ");
  Serial.println(ip);
}

void serial_debug_wifi_lost_connection() {
  Serial.println("Lost connection to WiFi, reconnecting...");
}

void serial_debug_wifi_reconnecting() {
  Serial.println("WiFi still disconnected, retrying...");
}

void serial_debug_wifi_reestablished_connection(const IPAddress& ip) {
  Serial.println("");
  Serial.print("Re-established connection to WiFi with IP: ");
  Serial.println(ip);
}

void serial_debug_rtc_not_connected() {
  Serial.println("RTC is not available");
}

void serial_debug_rtc_not_running() {
  Serial.println("DS3231 found, but oscillator not running, starting the clock...");
}

void serial_debug_time_synced(const ClockTime& time) {
  Serial.println("");
  Serial.print("Time synchronized: ");
  Serial.println(time.epochUtc);
}

void serial_debug_mode_changed(Mode mode) {
  Serial.print("Mode changed to: ");
  switch(mode) {
    case ShowHourMinutes:
      Serial.println("Show Hour Minutes");
      break;
    case ShowDate:
      Serial.println("Show Date");
      break;
    case SetAlarmHour:
      Serial.println("Set Alarm Hour");
      break;
    case SetAlarmMinute:
      Serial.println("Set Alarm Minute");
      break;
    case SetBrightness:
      Serial.println("Set Brightness");
      break;
    default:
      Serial.println("Unknown Mode");
      break;
  }
}

void serial_debug_brightness_changed(uint8_t brightness) {
  Serial.print("Brightness changed to: ");
  Serial.println(brightness);
}

void serial_debug_alarm_changed(uint8_t hour, uint8_t minute) {
  Serial.print("Alarm set to: ");
  if (hour < 10) Serial.print("0");
  Serial.print(hour);
  Serial.print(":");
  if (minute < 10) Serial.print("0");
  Serial.println(minute);
}