#include <Arduino.h>

#define SUPPORT_DS3231
//#define SUPPORT_DS3232
#define SUPPORT_ROTARY_ENCODER
#define SUPPORT_WEB_INTERFACE

#if defined(SUPPORT_DS3231) && defined(SUPPORT_DS3232)
#error "Only one of SUPPORT_DS3231 or SUPPORT_DS3232 may be defined"
#endif

#if defined(SUPPORT_DS3231) || defined(SUPPORT_DS3232)
#define RTC_SUPPORT
#endif

#ifdef RTC_SUPPORT
#include <Wire.h>
#include <I2C_RTC.h>
#endif
#ifdef SUPPORT_ROTARY_ENCODER
#include <RotaryEncoder.h>
#endif
#include <WiFi.h>
#include <time.h>
#include <sntp.h>
#include <TM1637plus.h>

/* Other include header files for miscellaneous libraries */
#include <gatltick.h>

/* Local include header files */
#include "constdef.h"
#include "configuration.h"
#include "secrets.h"
#include "display.h"
#include "convert.h"
#include "types.h"
#include "mode.h"
#ifdef SUPPORT_WEB_INTERFACE
#include "webinterface.h"
#endif

#define SNTP_SYNC_INTERVAL    60000  // 60 seconds

/* Interval definitions */
#define INTERVAL_WIFI_CONNECT  5000
#define INTERVAL_DISPLAY        100

/* ESP32-WROOM-32 I2C Pins

  Pins defined in the esp32 code
  ------------------------------
  SDA:  GPIO 21
  SCL:  GPIO 22

*/

/* Wiring
   ======


  ESP32 Dev Kit C 32UE
  --------------------

    MCU          | Device
    -------------+---------------------
    SDA  GPIO 21 | DS3231  SDA        7
    SCL  GPIO 22 | DS3231  SCL        8
         GPIO 27 | DS3231  INT/SQW    9
         GPIO 13 | DISPLAY 1 CLK
         GPIO 14 | DISPLAY 1 DIO
         GPIO 16 | DISPLAY 2 CLK
         GPIO 17 | DISPLAY 2 DIO
         GPIO 18 | DISPLAY 3 CLK
         GPIO 19 | DISPLAY 3 DIO
         GPIO 23 | Rotary  CLK
         GPIO 32 | Rotary  DIO
         GPIO 33 | Rotary  SW
         GPIO 25 | DAC Audio


  ESP32 WROOM 32UE N4
  -------------------

    MCU               | Device
    ------------------+---------------------
    SDA  GPIO 21 | DS3231  SDA        7
    SCL  GPIO 22 | DS3231  SCL        8
         GPIO 27 | DS3231  INT/SQW    9
         GPIO 13 | DISPLAY 1 CLK
         GPIO 14 | DISPLAY 1 DIO
         GPIO 16 | DISPLAY 2 CLK
         GPIO 17 | DISPLAY 2 DIO
         GPIO 18 | DISPLAY 3 CLK
         GPIO 19 | DISPLAY 3 DIO
         GPIO 23 | Rotary  CLK
         GPIO 32 | Rotary  DIO
         GPIO 33 | Rotary  SW
         GPIO 25 | DAC Audio

*/

/* TM1637 display pins */
#define PIN_DISPLAY_1_CLK 13
#define PIN_DISPLAY_1_DIO 14
#define PIN_DISPLAY_2_CLK 16
#define PIN_DISPLAY_2_DIO 17
#define PIN_DISPLAY_3_CLK 18
#define PIN_DISPLAY_3_DIO 19

/* Rotary encoder pins */
#define PIN_ROTARY_CLK    23
#define PIN_ROTARY_DT     32
#define PIN_ROTARY_SW     33

/* DAC Audio output pin */
#define PIN_DAC_AUDIO     25

/* Serial Speed (if undefined no serial output will be generated) */
#define SERIAL_BAUD 115200

/* Serial Debugging include header file */
#ifdef SERIAL_BAUD
#include "serialdebug.h"
#endif

// use FOUR3 mode when CLK, DT signals are always HIGH in latch position.
// use FOUR0 mode when CLK, DT signals are always LOW in latch position.
// use TWO03 mode when CLK, DT signals are both LOW or HIGH in latch position.
#define ENCODER_LATCH_MODE RotaryEncoder::LatchMode::FOUR3

#define ENCODER_SW_DEBOUNCE_MS 25

/* Tick instances */
::gos::atl::Tick<> tick_wifi_connect(INTERVAL_WIFI_CONNECT);
::gos::atl::Tick<> tick_display(INTERVAL_DISPLAY);

#ifdef SUPPORT_DS3231
/* RTC instance */
static DS3231 rtc;
#endif
#ifdef SUPPORT_DS3232
/* RTC instance */
static DS3232 rtc;
#endif

/* Current operating mode */
static Mode currentMode = Mode::ShowHourMinutes;

/* Brightness and alarm are device-wide settings accessible by both the
   rotary encoder and the web interface (whichever features are enabled). */
static uint8_t brightness = 7, lastReportedBrightness = 7;
static uint8_t alarmHour = 7, alarmMinute = 0, lastReportedAlarmHour = 7, lastReportedAlarmMinute = 0;

#ifdef SUPPORT_ROTARY_ENCODER
static RotaryEncoder *rotaryencoder = nullptr;
static bool buttonRawState = HIGH;
static bool buttonStableState = HIGH;
static unsigned long buttonLastChangeMs = 0;
#endif

/* Display instances */
Display display_1(PIN_DISPLAY_1_CLK, PIN_DISPLAY_1_DIO);
Display display_2(PIN_DISPLAY_2_CLK, PIN_DISPLAY_2_DIO);
Display display_3(PIN_DISPLAY_3_CLK, PIN_DISPLAY_3_DIO);

/* Time instances */
ClockTime currentTime;
LocalTime icelandLocalTime;
LocalTime houstonLocalTime;
LocalTime bangkokLocalTime;

/* General global variables */
unsigned long current;
unsigned int wifi_status = CONNECTION_STATUS_UNKNOWN;

#ifdef SUPPORT_ROTARY_ENCODER
IRAM_ATTR void encoderPositionChanged();
#endif

/* SNTP callback function */
static void time_sync_available(struct timeval *tv);
static void initialize_sntp_if_needed();

static bool is_rtc_available = false;
static bool is_sntp_initialized = false;

/* One-time initialization: serial debug output, TM1637 displays, I2C RTC,
   rotary encoder with interrupts, and non-blocking WiFi connect.
   SNTP is started lazily in loop() once WiFi is confirmed connected. */
void setup() {
  /* Initalize general global variables */
  current = 0;

  currentTime.epochUtc = 0;
  currentTime.isAvailable = false;

#ifdef SERIAL_BAUD
  serial_debug_initiate(SERIAL_BAUD);
#endif

  /* Initialize the displays */
  display_1.initiate();
  display_2.initiate();
  display_3.initiate();

  /* Initialize the RTC */
#ifdef RTC_SUPPORT
  rtc.begin();
  if(rtc.isConnected()) {
    if (!rtc.isRunning()) {
#ifdef SERIAL_BAUD
      serial_debug_rtc_not_running();
#endif
      rtc.startClock();
    }
    is_rtc_available = true;
    currentTime.epochUtc = rtc.getEpoch();
    currentTime.isAvailable = true;
  } else {
#ifdef SERIAL_BAUD
      serial_debug_rtc_not_connected();
#endif
  }
#endif

#ifdef SUPPORT_ROTARY_ENCODER
  rotaryencoder = new RotaryEncoder(PIN_ROTARY_CLK, PIN_ROTARY_DT);
  rotaryencoder->setPosition(brightness);
  pinMode(PIN_ROTARY_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_ROTARY_CLK), encoderPositionChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ROTARY_DT), encoderPositionChanged, CHANGE);
#endif

  /* Initalize the WiFi connection */
#ifdef SERIAL_BAUD
  serial_debug_wifi_connecting();
#endif

  // Do not block startup waiting for WiFi; loop() will maintain reconnect attempts.
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (WiFi.status() == WL_CONNECTED) {
    wifi_status = CONNECTION_STATUS_CONNECTED;
    initialize_sntp_if_needed();
#ifdef SERIAL_BAUD
    serial_debug_wifi_connection_success(WiFi.localIP());
#endif
  } else {
    wifi_status = CONNECTION_STATUS_DISCONNECTED;
  }

  /* Start the HTTP web interface (listens on port 80). */
#ifdef SUPPORT_WEB_INTERFACE
  webinterface_initiate(
    &currentTime,
    &icelandLocalTime, &houstonLocalTime, &bangkokLocalTime,
    &alarmHour, &alarmMinute,
    &brightness,
    &display_1, &display_2, &display_3);
#endif

}

/* Main loop, called repeatedly by the Arduino runtime.
   Responsibilities (in order):
     1. Process any pending HTTP requests from the web interface.
     2. Debounce the rotary encoder push button and advance the display mode on press.
     3. Read the encoder value and apply it to the active setting (brightness / alarm).
     4. Monitor WiFi and trigger reconnect attempts on a timed interval.
     5. Refresh the three TM1637 displays on a 100 ms tick. */
void loop() {
  current = millis();

#ifdef SUPPORT_WEB_INTERFACE
  webinterface_handle();
#endif

  /* Sample the rotary encoder push button and debounce it */
#ifdef SUPPORT_ROTARY_ENCODER
  int buttonSample = digitalRead(PIN_ROTARY_SW);
  if (buttonSample != buttonRawState) {
    buttonRawState = buttonSample;
    buttonLastChangeMs = millis();
  }
  if ((millis() - buttonLastChangeMs) >= ENCODER_SW_DEBOUNCE_MS && buttonStableState != buttonRawState) {
    buttonStableState = buttonRawState;

    if (buttonStableState == LOW) {
      currentMode = static_cast<Mode>((currentMode + 1) % ModeCount);
#ifdef SERIAL_BAUD
      serial_debug_mode_changed(currentMode);
#endif
      // Initialize encoder position for new mode
      if (currentMode == SetBrightness) {
        rotaryencoder->setPosition(brightness);
      } else if (currentMode == SetAlarmHour) {
        rotaryencoder->setPosition(alarmHour);
      } else if (currentMode == SetAlarmMinute) {
        rotaryencoder->setPosition(alarmMinute);
      }
    }
  }

  switch(currentMode) {
    case SetBrightness: {
      int rawPosition = rotaryencoder->getPosition();
      brightness = static_cast<uint8_t>(constrain(rawPosition, 0, 7));
      // Force encoder position back into valid range if it went out of bounds
      if (rawPosition != static_cast<int>(brightness)) {
        rotaryencoder->setPosition(brightness);
      }
      if (brightness != lastReportedBrightness) {
        lastReportedBrightness = brightness;
#ifdef SERIAL_BAUD
        serial_debug_brightness_changed(brightness);
#endif
        display_1.setBrightness(brightness);
        display_2.setBrightness(brightness);
        display_3.setBrightness(brightness);
      }
      break;
    }

    case SetAlarmHour: {
      int rawPosition = rotaryencoder->getPosition();
      alarmHour = static_cast<uint8_t>(constrain(rawPosition, 0, 23));
      if (rawPosition != static_cast<int>(alarmHour)) {
        rotaryencoder->setPosition(alarmHour);
      }
      if (alarmHour != lastReportedAlarmHour) {
        lastReportedAlarmHour = alarmHour;
#ifdef SERIAL_BAUD
        serial_debug_alarm_changed(alarmHour, alarmMinute);
#endif
      }
      break;
    }

    case SetAlarmMinute: {
      int rawPosition = rotaryencoder->getPosition();
      alarmMinute = static_cast<uint8_t>(constrain(rawPosition, 0, 59));
      if (rawPosition != static_cast<int>(alarmMinute)) {
        rotaryencoder->setPosition(alarmMinute);
      }
      if (alarmMinute != lastReportedAlarmMinute) {
        lastReportedAlarmMinute = alarmMinute;
#ifdef SERIAL_BAUD
        serial_debug_alarm_changed(alarmHour, alarmMinute);
#endif
      }
      break;
    }
  }
#endif

  /* Maintain the WiFi connection */
  if (WiFi.status() == WL_CONNECTED) {
    if (wifi_status != CONNECTION_STATUS_CONNECTED) {
#ifdef SERIAL_BAUD
      serial_debug_wifi_reestablished_connection(WiFi.localIP());
#endif
      wifi_status = CONNECTION_STATUS_CONNECTED;
    }
    initialize_sntp_if_needed();
  } else {
    if (wifi_status == CONNECTION_STATUS_CONNECTED) {
#ifdef SERIAL_BAUD
      serial_debug_wifi_lost_connection();
#endif
      wifi_status = CONNECTION_STATUS_DISCONNECTED;
    }
    if (tick_wifi_connect.is(current)) {
#ifdef SERIAL_BAUD
      serial_debug_wifi_reconnecting();
#endif
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }

  /* Update the Displays */
  if (tick_display.is(current)) {

    switch(currentMode) {
      case ShowHourMinutes:
        /* Read the current time from the RTC if available */
#ifdef RTC_SUPPORT
        if (is_rtc_available) {
          currentTime.epochUtc = rtc.getEpoch();
          currentTime.isAvailable = true;
        }
#endif

        if (currentTime.isAvailable) {
          icelandLocalTime = convertUtcToLocal(currentTime.epochUtc, TIMEZONE_ICELAND);
          houstonLocalTime = convertUtcToLocal(currentTime.epochUtc, TIMEZONE_HOUSTON);
          bangkokLocalTime = convertUtcToLocal(currentTime.epochUtc, TIMEZONE_BANGKOK);
          display_1.showTime(houstonLocalTime);
          display_2.showTime(bangkokLocalTime);
          display_3.showTime(icelandLocalTime);
        } else {
          display_1.showUnavailable();
          display_2.showUnavailable();
          display_3.showUnavailable();
        }
        break;

      case ShowDate:
        /* Read the current time from the RTC if available */
#ifdef RTC_SUPPORT
        if (is_rtc_available) {
          currentTime.epochUtc = rtc.getEpoch();
          currentTime.isAvailable = true;
        }
#endif

        if (currentTime.isAvailable) {
          icelandLocalTime = convertUtcToLocal(currentTime.epochUtc, TIMEZONE_ICELAND);
          houstonLocalTime = convertUtcToLocal(currentTime.epochUtc, TIMEZONE_HOUSTON);
          bangkokLocalTime = convertUtcToLocal(currentTime.epochUtc, TIMEZONE_BANGKOK);
          display_1.showDate(houstonLocalTime);
          display_2.showDate(bangkokLocalTime);
          display_3.showDate(icelandLocalTime);
        } else {
          display_1.showUnavailable();
          display_2.showUnavailable();
          display_3.showUnavailable();
        }
        break;

      case SetAlarmHour:
      case SetAlarmMinute:
        display_1.showAlarm(alarmHour, alarmMinute);
        // Indicate that alarm is being set on displays 2 and 3
        display_2.showUnavailable();
        display_3.showUnavailable();
        break;

      case SetBrightness:
        display_1.showBrightness(brightness);
        // Indicate that brightness is being set on displays 2 and 3
        display_2.showUnavailable();
        display_3.showUnavailable();
        break;

      default:
        break;
    }
  }
}

/* Configures and starts SNTP time synchronization against the configured NTP servers.
   Idempotent — safe to call on every loop iteration; exits immediately after the
   first successful initialization. WiFi must be connected before calling. */
static void initialize_sntp_if_needed() {
  if (is_sntp_initialized) {
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  sntp_set_time_sync_notification_cb(time_sync_available);
  sntp_servermode_dhcp(1);
  sntp_set_sync_interval(SNTP_SYNC_INTERVAL);
  configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2);
  is_sntp_initialized = true;
}

/* Interrupt Service Routine: fires on any CHANGE edge of the encoder CLK or DT pins.
   Must stay minimal — only forward the signal to the library's tick handler.
   IRAM_ATTR ensures the function is placed in IRAM so it is reachable during a cache miss. */
#ifdef SUPPORT_ROTARY_ENCODER
IRAM_ATTR void encoderPositionChanged() {
  rotaryencoder->tick();
}
#endif

/* SNTP callback: invoked by the ESP32 SNTP library when a new time is received from an
   NTP server. Updates the global epoch and writes the new time back to the RTC so that
   the RTC stays accurate even across reboots without network access. */
void time_sync_available(struct timeval *tv) {
  if (tv != nullptr) {
    currentTime.epochUtc = tv->tv_sec;
    currentTime.isAvailable = true;

#ifdef RTC_SUPPORT
    if (is_rtc_available) {
      rtc.setEpoch(currentTime.epochUtc);
    }
#endif
#ifdef SERIAL_BAUD
    serial_debug_time_synced(currentTime);
#endif
  }
}
