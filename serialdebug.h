#ifndef SERIALDEBUG_H
#define SERIALDEBUG_H

#include <Arduino.h>

void serial_debug_initiate(long baud);
void serial_debug_wifi_connecting();
void serial_debug_wifi_connection_success(const IPAddress& ip);
void serial_debug_wifi_lost_connection();
void serial_debug_wifi_reestablished_connection(const IPAddress& ip);

#endif /* SERIALDEBUG_H */
