/*
  Board MAC Address Finder

  This universal sketch prints the MAC address of your WiFi-enabled
  board (e.g., ESP32, ESP32-C3, ESP8266) to the Serial Monitor.
  
  Instructions:
  1. Select your specific board from the Arduino IDE's "Tools" menu.
  2. Upload this sketch.
  3. Open the Serial Monitor at 115200 baud.
*/
#include <WiFi.h>

void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Reading board's MAC address...");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop(){
  // The MAC address is printed once in setup, so this loop is empty.
  // You can close the Serial Monitor after copying the address.
}
