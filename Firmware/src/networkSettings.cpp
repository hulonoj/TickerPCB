#include <string>
#include "NTPClient.h"
#include "WiFiUdp.h"
#include "max6955.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <credentials.h>

//Web/Server address to read/write from 
const char *host = "www.alphavantage.co"; // Offers 25 free Stock price API calls per 24 hours.
const int httpsPort = 443;  //HTTPS= 443 and HTTP = 80

unsigned static long initTime = millis();

const long ESTOffsetInSecondsFromUTC = -5 * 3600; //3600 * -5 = EST

extern bool GLOBAL_WIFI_CONNECTED;

//Time info

bool isDSTActive() {
  time_t now = timeClient.getEpochTime();
  struct tm *t = gmtime(&now);
  int m = t->tm_mon + 1, d = t->tm_mday, dow = t->tm_wday;
  
  if (m < 3 || m > 11) return false;
  if (m > 3 && m < 11) return true;
  if (m == 3) return (d - dow >= 8);
  if (m == 11) return (d - dow < 1);
  return false;
}

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org", ESTOffsetInSecondsFromUTC + (isDSTActive() ? 3600 : 0));

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

  writeDisplay("Loading");
  Serial.print("Checking ");
  Serial.println(host);
  //Serial.printf("Using fingerprint '%s'\n", CREDENTIALS_FINGERPRINT);
  httpsClient.setInsecure();
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
    return "HttpFail";
  }
  else {
    Serial.println("Connected to web");
  }
  //////////////Actually get data
  
  String getData, Link;

  //GET Data
  Link = "/query?function=GLOBAL_QUOTE&symbol=" + stockTicker + "&apikey=" + CREDENTIALS_ALPHA_VANTAGE_APIKEY;

  Serial.print("requesting URL: ");
  Serial.println(host + Link);

  httpsClient.print(String("GET ") + Link + " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" +               
              "Connection: close\r\n\r\n");

  Serial.println("request sent");

  // Wait for response with timeout
  unsigned long timeout = millis();
  while (httpsClient.available() == 0) {
    if (millis() - timeout > 10000) {  // 10 seconds
      Serial.println(">>> Client Timeout !");
      httpsClient.stop();
      return "Timeout";
    }
    delay(10);  // Small delay to prevent tight loop
  }

  // Read HTTP status
  char status[64];
  size_t len = httpsClient.readBytesUntil('\r', status, sizeof(status) - 1);
  status[len] = '\0';
  httpsClient.read(); // Consume '\n'

  int code = 0;
  if (sscanf(status, "HTTP/%*s %d", &code) != 1) {
    Serial.print("Malformed HTTP status: ");
    Serial.println(status);
    httpsClient.stop();  // Close connection
    return status;
  }

  if (code != 200) {
    Serial.print("HTTP error ");
    Serial.print(code);
    Serial.print(": ");
    Serial.println(status);
    httpsClient.stop();  // Close connection
    return String(code);
  }

  Serial.println("reply was:");
  Serial.println("==========");

  // Skip headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!httpsClient.find(endOfHeaders)) {  // Check if headers found
    Serial.println(">>> Headers not found!");
    httpsClient.stop();
    return "NoHeaders";
  }

  // Read response body
  String line = "";
  unsigned long readTimeout = millis();
  while (httpsClient.connected() || httpsClient.available()) {  // Also check available()
    if (httpsClient.available()) {
      char c = httpsClient.read();
      line += c;
      Serial.print(c);
      readTimeout = millis();  // Reset timeout on each byte
    }
    
    // Prevent hanging if connection stalls
    if (millis() - readTimeout > 5000) {
      Serial.println("\n>>> Read timeout!");
      break;
    }
    
    delay(1);  // Small delay
  }
  Serial.println();
  httpsClient.stop();  // Close connection after reading

  Serial.println(line);

  if (line.indexOf("API rate limit")){
    Serial.println(">>> API rate limit exceeded.");
    return "APILIMIT";
  }
  // Parse price from JSON
  int priceIndex = line.indexOf("price\":");

  if (priceIndex < 0) {
    Serial.println(">>> Price field not found in response");
    return "InvPrice";
  }

  priceIndex += 9;  // Skip past '"price":"'
  Serial.println(priceIndex);

  line.remove(0, priceIndex);
  priceIndex = line.indexOf("\",");

  if (priceIndex < 0) {  // Check if closing quote found
    Serial.println(">>> Price end quote not found");
    return "InvPrice";
  }

  line.remove(priceIndex);
  Serial.println(line);
  
  return format_stock_price(line);
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