#include "serialdebug.h"

/* Prefix all logs with uptime so field debugging can correlate events. */
static void printPrefix(const char *level) {
  Serial.print("[");
  Serial.print(millis());
  Serial.print(" ms] ");
  Serial.print(level);
  Serial.print(" ");
}

static const char* wifiStatusToString(wl_status_t status) {
  switch (status) {
    case WL_IDLE_STATUS:     return "IDLE";
    case WL_NO_SSID_AVAIL:   return "NO_SSID";
    case WL_SCAN_COMPLETED:  return "SCAN_COMPLETED";
    case WL_CONNECTED:       return "CONNECTED";
    case WL_CONNECT_FAILED:  return "CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "CONNECTION_LOST";
    case WL_DISCONNECTED:    return "DISCONNECTED";
    default:                 return "UNKNOWN";
  }
}

void serial_debug_initiate(long baud) {
  Serial.begin(baud);
  const unsigned long waitStartMs = millis();
  while (!Serial && (millis() - waitStartMs) < 2000UL) {
    delay(10);
  }
  printPrefix("[INFO]");
  Serial.println("World Clock serial debugging initialized.");
  printPrefix("[INFO]");
  Serial.println("Copyright 2026 Geirmundur Orri Sigurdsson");
}

void serial_debug_startup_summary(bool rtcSupported, bool rtcAvailable, bool rotarySupported, bool webSupported) {
  printPrefix("[INFO]");
  Serial.print("Features: RTC=");
  Serial.print(rtcSupported ? (rtcAvailable ? "enabled+detected" : "enabled+missing") : "disabled");
  Serial.print(", Rotary=");
  Serial.print(rotarySupported ? "enabled" : "disabled");
  Serial.print(", Web=");
  Serial.println(webSupported ? "enabled" : "disabled");
}

void serial_debug_wifi_connecting() {
  printPrefix("[INFO]");
  Serial.println("Connecting to WiFi.");
}

void serial_debug_wifi_connection_success(const IPAddress& ip) {
  printPrefix("[INFO]");
  Serial.print("WiFi connected, IP address: ");
  Serial.println(ip);
}

void serial_debug_wifi_lost_connection() {
  printPrefix("[WARN]");
  Serial.println("WiFi connection lost.");
}

void serial_debug_wifi_reconnecting(int attempt, wl_status_t status) {
  printPrefix("[WARN]");
  Serial.print("WiFi retry #");
  Serial.print(attempt);
  Serial.print(" (status=");
  Serial.print(wifiStatusToString(status));
  Serial.println(").");
}

void serial_debug_wifi_reestablished_connection(const IPAddress& ip) {
  printPrefix("[INFO]");
  Serial.print("WiFi re-established, IP address: ");
  Serial.println(ip);
}

void serial_debug_sntp_initializing(const char* server1, const char* server2, uint32_t syncIntervalMs) {
  printPrefix("[INFO]");
  Serial.print("Initializing SNTP with servers ");
  Serial.print(server1);
  Serial.print(" and ");
  Serial.print(server2);
  Serial.print(", sync interval=");
  Serial.print(syncIntervalMs);
  Serial.println(" ms.");
}

void serial_debug_sntp_waiting_for_wifi() {
  printPrefix("[WARN]");
  Serial.println("SNTP initialization deferred until WiFi is connected.");
}

void serial_debug_webinterface_started(uint16_t port) {
  printPrefix("[INFO]");
  Serial.print("Web interface listening on port ");
  Serial.print(port);
  Serial.println(".");
}

void serial_debug_rtc_not_connected() {
  printPrefix("[WARN]");
  Serial.println("RTC not detected on I2C bus.");
}

void serial_debug_rtc_not_running() {
  printPrefix("[WARN]");
  Serial.println("RTC oscillator stopped; starting clock.");
}

void serial_debug_time_synced(const ClockTime& time) {
  printPrefix("[INFO]");
  Serial.print("Time synchronized from NTP, epoch=");
  Serial.println(time.epochUtc);
}

void serial_debug_mode_changed(Mode mode) {
  printPrefix("[INFO]");
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
    case ShowWiFiStatus:
      Serial.println("Show WiFi Status");
      break;
    default:
      Serial.println("Unknown Mode");
      break;
  }
}

void serial_debug_brightness_changed(uint8_t brightness) {
  printPrefix("[INFO]");
  Serial.print("Brightness changed to: ");
  Serial.println(brightness);
}

void serial_debug_alarm_changed(uint8_t hour, uint8_t minute) {
  printPrefix("[INFO]");
  Serial.print("Alarm set to: ");
  if (hour < 10) Serial.print("0");
  Serial.print(hour);
  Serial.print(":");
  if (minute < 10) Serial.print("0");
  Serial.println(minute);
}
