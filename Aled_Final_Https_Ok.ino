/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/

#include <Arduino.h>
#include "Adafruit_HTU21DF.h"
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ACROBOTIC_SSD1306.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>

Adafruit_HTU21DF htu = Adafruit_HTU21DF();
const char* ssid = "iPhone de Léo";
const char* password = "20102020";
const char* server = "https://www.aled-weather.fr:8080/reports";

ESP8266WiFiMulti WiFiMulti;

void setup() {

  Wire.begin(0, 2);
  Serial.begin(9600);

  oled.init(); 
  oled.clearDisplay();              
  oled.setTextXY(1, 5);              
  oled.putString("A LED");
  oled.setTextXY(2, 5);              
  oled.putString("WEATHER");
  delay(5000);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  while ((WiFiMulti.run() != WL_CONNECTED)) {

    oled.clearDisplay();              
    oled.setTextXY(2,0);              
    oled.putString("CONNECTION :");
    Serial.print(".");
    oled.setTextXY(3,0);              
    oled.putString(".");
    delay(500);
    Serial.print(".");
    oled.setTextXY(3,1);              
    oled.putString(".");
    delay(500);
    Serial.print(".");
    oled.setTextXY(3,2);              
    oled.putString(".");
    delay(500);
    oled.setTextXY(3,3);              
    oled.putString(".");
    delay(500);
    oled.setTextXY(3,4);              
    oled.putString(".");
    delay(500);
    oled.setTextXY(3,5);              
    oled.putString(".");
    delay(500);
  }
  
  Serial.println("Connected to WiFi");

  if (!htu.begin(&Wire)) {
    Serial.println("Couldn't find sensor!");
    while (1);
  }
  
  oled.clearDisplay(); 
  oled.setTextXY(1, 6);              
  oled.putString("A LED");
  oled.setTextXY(2, 5);              
  oled.putString("WEATHER");
}

void loop() {


  float temp = htu.readTemperature();
  float hum = htu.readHumidity();
  
  char tempBuffer[20];
  sprintf(tempBuffer, "%.2f", temp);
  String tempStr = String(tempBuffer);

  char humBuffer[20];
  sprintf(humBuffer, "%.2f", hum);
  String humStr = String(humBuffer);

  Serial.print("Temp: "); Serial.print(tempStr); Serial.print(" C");
  Serial.print("\t\t");
  Serial.print("Humidity: "); Serial.print(humStr); Serial.println(" \%");

  oled.setTextXY(3,0); 
  oled.putString("Temp: ");
  oled.setTextXY(3,7);             
  oled.putFloat(temp);
  oled.setTextXY(3,13);            
  oled.putString("C");
  oled.setTextXY(4,0);              
  oled.putString("Hum:");
  oled.setTextXY(4,7);             
  oled.putFloat(hum);
  oled.setTextXY(4,13);             
  oled.putString("%");

  String json = "{\"temperature\": \"" + tempStr + "\", \"humidity\": \"" + humStr + "\", \"deviceUuid\": \"63bffd15e209b\", \"locationName\": \"Lyon\" }";

  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

  //client->setFingerprint(fingerprint);
  // Or, if you happy to ignore the SSL certificate, then use the following line instead:
  client->setInsecure();

  HTTPClient https;

  Serial.print("[HTTPS] begin...\n");
  if (https.begin(*client, server)) {  // HTTPS

    Serial.print("[HTTPS] POST...\n");
    // start connection and send HTTP header
    int httpCode = https.POST(json);

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  

  Serial.println("Wait 60s before next round...");
  delay(60000);
}
