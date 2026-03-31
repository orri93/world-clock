#include <Arduino.h>

#define SUPPORT_DS3231

#ifdef SUPPORT_DS3231
#include <Wire.h>
#include <I2C_RTC.h>
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

/* Interval definitions */
#define INTERVAL_WIFI_CONNECT  5000
#define INTERVAL_DISPLAY       1000

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
    SDA  GPIO 21 | DS3231  SDA
    SCL  GPIO 22 | DS3231  SCL

         GPIO 13 | DISPLAY 1 CLK
         GPIO 14 | DISPLAY 1 DIO
         GPIO 15 | DISPLAY 2 CLK
         GPIO 16 | DISPLAY 2 DIO
         GPIO 17 | DISPLAY 3 CLK
         GPIO 18 | DISPLAY 3 DIO

    MISO GPIO 19 | BME280  SDO        5
    MOSI GPIO 23 | BME280  SDI        3
    CS   GPIO  5 | BME280  CSB        2


  ESP32 WROOM 32UE N4
  -------------------

    MCU               | Device
    ------------------+---------------------
    MOSI GPIO 13 (20) | DOGS102 SDA/SI    24
    SCKL GPIO 14 (17) | DOGS102 SCK/SCL   25
    CS   GPIO 15 (21) | DOGS102 CS0/CS    28
         GPIO 16 (25) | DOGS102 CD/A0     26
         GPIO 17 (27) | DOGS102 RST/RESET 27
    MOSI GPIO 23 (36) | BME280  SDI        3
    MISO GPIO 19 (38) | BME280  SDO        5
    SCKL GPIO 18 (35) | BME280  SCK        4
    CS   GPIO  5 (34) | BME280  CSB        2

*/

#define PIN_DISPLAY_1_CLK 13
#define PIN_DISPLAY_1_DIO 14
#define PIN_DISPLAY_2_CLK 15
#define PIN_DISPLAY_2_DIO 16
#define PIN_DISPLAY_3_CLK 17
#define PIN_DISPLAY_3_DIO 18

/* Serial Speed (if undefined no serial output will be generated) */
#define SERIAL_BAUD 115200

/* Serial Debugging include header file */
#ifdef SERIAL_BAUD
#include "serialdebug.h"
#endif

/* WiFi instances */
WiFiClient wificlient;

/* Tick instances */
::gos::atl::Tick<> tick_wifi_connect(INTERVAL_WIFI_CONNECT);
::gos::atl::Tick<> tick_display(INTERVAL_DISPLAY);

#ifdef SUPPORT_DS3231
/* RTC instance */
static DS3231 rtc;
#endif

/* Display instances */
Display display_1(PIN_DISPLAY_1_CLK, PIN_DISPLAY_1_DIO);
Display display_2(PIN_DISPLAY_2_CLK, PIN_DISPLAY_2_DIO);
Display display_3(PIN_DISPLAY_3_CLK, PIN_DISPLAY_3_DIO);

/* Time instances */
ClockTime currentTime;

/* General global variables */
unsigned long current;
unsigned int wifi_status = CONNECTION_STATUS_UNKNOWN;

/* SNTP callback function */
static void time_sync_available(struct timeval *tv);

void setup() {
  /* Initalize general global variables */
  current = 0;

#ifdef SERIAL_BAUD
  serial_debug_initiate(SERIAL_BAUD);
#endif

  /* Initialize the RTC */
#ifdef SUPPORT_DS3231
  rtc.begin();

  if(rtc.isConnected()) {
    time_t now = rtc.getEpoch();
  } else {
  }
#endif

  /* Set SNTP callback function */
  sntp_set_time_sync_notification_cb(time_sync_available);
  /* Use DHCP to get the NTP server */
  sntp_servermode_dhcp(1);
  /* Initialize the SNTP client */
  configTime(0, 0, NTP_SERVER_1, NTP_SERVER_2);

  /* Initalize the WiFi connection */
#ifdef SERIAL_BAUD
  serial_debug_wifi_connecting();
#endif
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
#ifdef SERIAL_BAUD
    Serial.print(".");
#endif
  }
  wifi_status = CONNECTION_STATUS_CONNECTED;

#ifdef SERIAL_BAUD
  serial_debug_wifi_connection_success(WiFi.localIP());
#endif

}

void loop() {
  current = millis();

  /* Maintain the WiFi connection */
  if (WiFi.status() == WL_CONNECTED) {
    if (wifi_status != CONNECTION_STATUS_CONNECTED) {
#ifdef SERIAL_BAUD
      serial_debug_wifi_reestablished_connection(WiFi.localIP());
#endif
      wifi_status = CONNECTION_STATUS_CONNECTED;
    }
  } else {
    if (wifi_status == CONNECTION_STATUS_CONNECTED) {
#ifdef SERIAL_BAUD
      serial_debug_wifi_lost_connection();
#endif
      wifi_status = CONNECTION_STATUS_DISCONNECTED;
    }
    if (tick_wifi_connect.is(current)) {
#ifdef SERIAL_BAUD
      serial_debug_wifi_lost_connection();
#endif
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }
}

void time_sync_available(struct timeval *tv) {
  Serial.println("Got time adjustment from NTP!");
}
