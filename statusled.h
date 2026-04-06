#ifndef STATUSLED_H
#define STATUSLED_H

#include <Arduino.h>

void status_led_begin(uint8_t greenPin, uint8_t redPin, uint32_t pwmFreq = 5000, uint8_t pwmBits = 8);

void status_led_update(
  unsigned long nowMs,
  bool wifiConnected,
  bool hardwareProblemDetected,
  unsigned long lastWifiConnectAttemptMs);

#endif
