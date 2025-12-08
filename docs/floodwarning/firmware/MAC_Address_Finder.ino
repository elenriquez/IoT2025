/*
  ESP8266 MAC Address Finder

  This sketch prints the MAC address of your ESP8266 board
  to the Serial Monitor.
*/
#include <ESP8266WiFi.h>

void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Reading MAC address...");
  Serial.print("ESP8266 MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop(){
  // The MAC address is printed once in setup.
}
