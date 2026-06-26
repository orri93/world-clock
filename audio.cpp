#include "audio.h"

#include <Arduino.h>

#define AUDIO_IDLE_LEVEL          128
#define AUDIO_LOW_LEVEL            56
#define AUDIO_HIGH_LEVEL          200
#define AUDIO_UPDATE_INTERVAL_US  125
#define ALARM_AUTO_STOP_MS      60000UL

static uint8_t s_dacPin = 25;
static uint8_t s_soundId = ALARM_SOUND_BEEP;
static bool s_initialized = false;
static bool s_isPlaying = false;
static bool s_squareHigh = false;

static unsigned long s_alarmStartedAtMs = 0;
static unsigned long s_lastSampleAtUs = 0;
static unsigned long s_lastToggleAtUs = 0;

static uint16_t audio_frequency_for_sound(uint8_t soundId, unsigned long elapsedMs) {
  switch (soundId) {
    case ALARM_SOUND_TWO_TONE:
      return ((elapsedMs / 250UL) % 2UL) == 0UL ? 900U : 1400U;

    case ALARM_SOUND_SIREN: {
      const unsigned long phaseMs = elapsedMs % 2000UL;
      if (phaseMs < 1000UL) {
        return static_cast<uint16_t>(500U + (phaseMs * 1300UL) / 1000UL);
      }
      return static_cast<uint16_t>(1800U - ((phaseMs - 1000UL) * 1300UL) / 1000UL);
    }

    case ALARM_SOUND_BEEP:
    default:
      return 1200U;
  }
}

static bool audio_gate_for_sound(uint8_t soundId, unsigned long elapsedMs) {
  switch (soundId) {
    case ALARM_SOUND_TWO_TONE:
      return true;

    case ALARM_SOUND_SIREN:
      return true;

    case ALARM_SOUND_BEEP:
    default:
      return (elapsedMs % 500UL) < 300UL;
  }
}

void audio_initialize(uint8_t dacPin) {
  s_dacPin = dacPin;
  s_initialized = true;
  s_isPlaying = false;
  s_squareHigh = false;
  dacWrite(s_dacPin, AUDIO_IDLE_LEVEL);
}

void audio_set_sound(uint8_t soundId) {
  if (soundId < ALARM_SOUND_COUNT) {
    s_soundId = soundId;
  }
}

uint8_t audio_get_sound() {
  return s_soundId;
}

uint8_t audio_get_sound_count() {
  return ALARM_SOUND_COUNT;
}

void audio_start_alarm() {
  if (!s_initialized) {
    return;
  }

  s_isPlaying = true;
  s_squareHigh = false;
  s_alarmStartedAtMs = millis();
  s_lastSampleAtUs = micros();
  s_lastToggleAtUs = s_lastSampleAtUs;
  dacWrite(s_dacPin, AUDIO_LOW_LEVEL);
}

void audio_stop_alarm() {
  if (!s_initialized) {
    return;
  }

  s_isPlaying = false;
  s_squareHigh = false;
  dacWrite(s_dacPin, AUDIO_IDLE_LEVEL);
}

bool audio_is_playing() {
  return s_isPlaying;
}

void audio_update() {
  if (!s_initialized || !s_isPlaying) {
    return;
  }

  const unsigned long nowMs = millis();
  if ((nowMs - s_alarmStartedAtMs) >= ALARM_AUTO_STOP_MS) {
    audio_stop_alarm();
    return;
  }

  const unsigned long nowUs = micros();
  if ((nowUs - s_lastSampleAtUs) < AUDIO_UPDATE_INTERVAL_US) {
    return;
  }
  s_lastSampleAtUs = nowUs;

  const unsigned long elapsedMs = nowMs - s_alarmStartedAtMs;
  if (!audio_gate_for_sound(s_soundId, elapsedMs)) {
    dacWrite(s_dacPin, AUDIO_IDLE_LEVEL);
    return;
  }

  const uint16_t frequency = audio_frequency_for_sound(s_soundId, elapsedMs);
  if (frequency == 0U) {
    dacWrite(s_dacPin, AUDIO_IDLE_LEVEL);
    return;
  }

  const uint32_t halfPeriodUs = 500000UL / frequency;
  if ((nowUs - s_lastToggleAtUs) >= halfPeriodUs) {
    s_squareHigh = !s_squareHigh;
    s_lastToggleAtUs = nowUs;
  }

  dacWrite(s_dacPin, s_squareHigh ? AUDIO_HIGH_LEVEL : AUDIO_LOW_LEVEL);
}
