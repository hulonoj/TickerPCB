#include <string>
#include "NTPClient.h"
#include "WiFiUdp.h"
#include "max6955.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <credentials.h>

//Web/Server address to read/write from 
const char *host = "alphavantage.co";
const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80

unsigned static long initTime = millis();

const long utcOffsetInSeconds = -14400;

extern bool GLOBAL_WIFI_CONNECTED;

//Time info
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org", utcOffsetInSeconds);

String format_stock_price(String stockprice)
{
  if(stockprice == "-2" || stockprice == ""){
     return "Invalid";
  }

  if(stockprice == "-1"){
    return "Timeout";
  }
  
  double price;
  const char* input_price = stockprice.c_str();
  sscanf(input_price, "%lf",&price);
  Serial.print("Rounding price: ");
  price = ceil(price * 100.0) / 100.0;
  
  Serial.println(price);
  String answer = String(price);
  return answer;
}

String wifi_stock_price(String stockTicker){
  WiFiClientSecure httpsClient;    //Declare object of class WiFiClient

  Serial.print("Checking ");
  Serial.println(host);
  Serial.printf("Using fingerprint '%s'\n", CREDENTIALS_FINGERPRINT);
  httpsClient.setFingerprint(CREDENTIALS_FINGERPRINT);
  httpsClient.setTimeout(15000); // 15 Seconds
  delay(500);
  
  Serial.print("HTTPS Connecting");
  int r=0; //retry counter
  while((!httpsClient.connect(host, httpsPort)) && (r < 100)){
      delay(75);
      Serial.print(".");
      r++;
  }
  if(r==100) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected to web");
  }
  //////////////Actually get data
  
  String getData, Link;

  //GET Data
  Link = "/query?function=GLOBAL_QUOTE&symbol=" + stockTicker + "&apikey=" + CREDENTIALS_ALPHA_VANTAGE_APIKEY;

  Serial.print("requesting URL: ");
  Serial.println(host+Link);

  httpsClient.print(String("GET ") + Link + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +               
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  unsigned long timeout = millis();
  while (httpsClient.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      httpsClient.stop();
      return "-1";
    }
  }
  
  char status[32] = {0};
  httpsClient.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(status);
    Serial.print("Unexpected HTTP response");
    return "-1";
  }


  Serial.println("reply was:");
  Serial.println("==========");
  //Skip Headers
  char endOfHeaders[] = "\r\n\r\n";
  httpsClient.find(endOfHeaders);
  String line = "";
  while (httpsClient.connected()) {
    if (httpsClient.available()) {
      char c = httpsClient.read();
      line += c;
      Serial.print(c);
    }
  }
  Serial.println();
  
  
  //String line = httpsClient.readStringUntil('\n');
  Serial.println(line);
  int priceIndex;
  priceIndex = line.indexOf("price\":");

  if(priceIndex < 0){
    return "-2";
  }

  priceIndex += 9;
  Serial.println(priceIndex);
  line.remove(0, priceIndex);
  priceIndex = line.indexOf("\",");
  Serial.println(priceIndex);
  line.remove(priceIndex);
  Serial.println(line);
  
  return format_stock_price(line);
  //writeDisplay(line);
    
  //delay(10000);  //GET Data at every 2 seconds
}

bool setup_Wifi(){
  //Set up wifi
  writeDisplay("WIFI O");
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA); 
  WiFi.begin(CREDENTIALS_WIFI_SSID, CREDENTIALS_WIFI_PASSWORD); //Connect to your WiFi router

  initTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis()-initTime < 6000) {
    delay(250);
    writeDisplay("WIFI 0");
    delay(250);
    writeDisplay("WIFI O");
    Serial.println("Connecting");
  }

  if(WiFi.status() != WL_CONNECTED){
    writeDisplay("FAILURE");
    return false;
  }else{
    writeDisplay("SUCCESS");
    GLOBAL_WIFI_CONNECTED = true;
    delay(500);
  }
  Serial.println("CONNECTED TO WIFI");
  timeClient.begin();
  return true;
}