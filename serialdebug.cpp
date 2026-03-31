#include "serialdebug.h"
#include "secrets.h"

static float v;

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

void serial_debug_wifi_reestablished_connection(const IPAddress& ip) {
  Serial.println("");
  Serial.print("Re-established connection to WiFi with IP: ");
  Serial.println(ip);
}
