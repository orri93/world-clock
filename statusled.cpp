#include "statusled.h"

namespace {

enum class StatusLedState : uint8_t {
  Online,
  Offline,
  TryingToConnect,
  HardwareProblem
};

constexpr unsigned long TRYING_TO_CONNECT_WINDOW_MS = 1600UL;

uint8_t g_greenPin = 0;
uint8_t g_redPin = 0;
bool g_isInitialized = false;

void status_led_set_duty(uint8_t greenDuty, uint8_t redDuty) {
  ledcWrite(g_greenPin, greenDuty);
  ledcWrite(g_redPin, redDuty);
}

uint8_t status_led_sine(unsigned long nowMs, uint16_t periodMs, uint8_t minDuty, uint8_t maxDuty) {
  float angle = (2.0f * PI * static_cast<float>(nowMs % periodMs)) / static_cast<float>(periodMs);
  float normalized = 0.5f * (sinf(angle) + 1.0f);
  float duty = static_cast<float>(minDuty) + normalized * static_cast<float>(maxDuty - minDuty);
  return static_cast<uint8_t>(duty);
}

StatusLedState status_led_get_state(
  unsigned long nowMs,
  bool wifiConnected,
  bool hardwareProblemDetected,
  unsigned long lastWifiConnectAttemptMs) {
  if (hardwareProblemDetected) {
    return StatusLedState::HardwareProblem;
  }

  if (wifiConnected) {
    return StatusLedState::Online;
  }

  if ((nowMs - lastWifiConnectAttemptMs) < TRYING_TO_CONNECT_WINDOW_MS) {
    return StatusLedState::TryingToConnect;
  }

  return StatusLedState::Offline;
}

}  // namespace

void status_led_begin(uint8_t greenPin, uint8_t redPin, uint32_t pwmFreq, uint8_t pwmBits) {
  g_greenPin = greenPin;
  g_redPin = redPin;

  ledcAttach(g_greenPin, pwmFreq, pwmBits);
  ledcAttach(g_redPin, pwmFreq, pwmBits);
  status_led_set_duty(0, 0);

  g_isInitialized = true;
}

void status_led_update(
  unsigned long nowMs,
  bool wifiConnected,
  bool hardwareProblemDetected,
  unsigned long lastWifiConnectAttemptMs) {
  if (!g_isInitialized) {
    return;
  }

  switch (status_led_get_state(nowMs, wifiConnected, hardwareProblemDetected, lastWifiConnectAttemptMs)) {
    case StatusLedState::Online: {
      // 1 second green breathing, intentionally never fully dark.
      uint8_t green = status_led_sine(nowMs, 1000, 22, 150);
      status_led_set_duty(green, 0);
      break;
    }

    case StatusLedState::TryingToConnect: {
      // Faster amber pulse (green + red) while active connect/reconnect attempts run.
      uint8_t pulse = status_led_sine(nowMs, 550, 8, 120);
      status_led_set_duty(pulse, static_cast<uint8_t>(pulse / 2));
      break;
    }

    case StatusLedState::Offline: {
      // Slow red breathing when disconnected and idle between retries.
      uint8_t red = status_led_sine(nowMs, 1500, 4, 100);
      status_led_set_duty(0, red);
      break;
    }

    case StatusLedState::HardwareProblem: {
      // Two short red flashes every 1.2s.
      unsigned long phase = nowMs % 1200UL;
      bool isOn = (phase < 90UL) || (phase >= 180UL && phase < 270UL);
      status_led_set_duty(0, isOn ? 220 : 0);
      break;
    }
  }
}
