#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>

#define ALARM_SOUND_BEEP       0
#define ALARM_SOUND_TWO_TONE   1
#define ALARM_SOUND_SIREN      2
#define ALARM_SOUND_COUNT      3

void audio_initialize(uint8_t dacPin);
void audio_set_sound(uint8_t soundId);
uint8_t audio_get_sound();
uint8_t audio_get_sound_count();

void audio_start_alarm();
void audio_stop_alarm();
bool audio_is_playing();

void audio_update();

#endif
