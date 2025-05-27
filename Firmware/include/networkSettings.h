#ifndef NETWORK_SETTINGS_H
#define NETWORK_SETTINGS_H

#include <string>
#include "NTPClient.h"
#include "WiFiUdp.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

extern const char *ssid;
extern const char *password;
extern const String CREDENTIALS_API_KEY;

//Web/Server address to read/write from 
extern const char *host;
extern const int httpsPort; //HTTPS= 443 and HTTP = 80

//SHA1 finger print of certificate use web browser to view and copys
extern const char fingerprint[];

extern WiFiUDP ntpUDP;
extern NTPClient timeClient;//(ntpUDP, "north-america.pool.ntp.org", utcOffsetInSeconds);

bool setup_Wifi();
String format_stock_price(String stockprice);
String wifi_stock_price(String stockTicker);
#endif