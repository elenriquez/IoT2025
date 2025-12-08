/*
 * PROJECT: IoT Flood Monitor (Sensor Unit)
 * BOARD:   ESP32-C3
 * PURPOSE: Reads two float switches to detect water levels and sends their
 *          status to a receiver unit via ESP-NOW communication.
 * AUTHOR:  Lance Edward Dela Rosa
 */

#include <esp_now.h>
#include <WiFi.h>

// WiFi credentials for connecting to the Soft AP
const char *ssid = "IoT_Flood_Monitor_AP";
const char *password = "floodmon";

//==============================================================================
// IMPORTANT: Receiver MAC Address
// You must update this array with the MAC address of your receiver board.
//==============================================================================
uint8_t broadcastAddress[] = {0xF0, 0x24, 0xF9, 0x0E, 0xE5, 0xB4};
//==============================================================================


// Pin Definitions
const int LOW_SWITCH_PIN = 4;  // Float switch for low water level
const int HIGH_SWITCH_PIN = 5; // Float switch for high water level


// ESP-NOW Data Structure
// NOTE: This structure must exactly match the receiver's structure.
typedef struct struct_message {
    bool isLowSwitchActive;
    bool isHighSwitchActive;
} struct_message;

// Create a message variable to hold the switch states.
struct_message myData;


// ESP-NOW Peer Information
esp_now_peer_info_t peerInfo;

//==============================================================================
// ESP-NOW Send Callback
// This function is executed after a message is sent, providing status.
//==============================================================================
void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//==============================================================================
// Main Setup
//==============================================================================
void setup() {
  Serial.begin(115200);

  // Configure float switch pins as inputs with internal pull-up resistors.
  // This inverts the logic:
  // - Pin reads HIGH (1) when the switch is open (DRY).
  // - Pin reads LOW (0) when the switch is closed (WET).
  pinMode(LOW_SWITCH_PIN, INPUT_PULLUP);
  pinMode(HIGH_SWITCH_PIN, INPUT_PULLUP);

  // Set device to Wi-Fi Station mode and connect to the Soft AP.
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to AP...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP!");

  // Initialize ESP-NOW.
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the callback function to get send status.
  esp_now_register_send_cb(OnDataSent);

  // Register the receiver as an ESP-NOW peer.
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Initialize the data structure with a default 'dry' state.
  myData.isLowSwitchActive = false;
  myData.isHighSwitchActive = false;
}

//==============================================================================
// Main Loop
//==============================================================================
void loop() {
  // Read the raw state of the digital input pins.
  int lowSwitchState = digitalRead(LOW_SWITCH_PIN);
  int highSwitchState = digitalRead(HIGH_SWITCH_PIN);

  // Convert the raw state to a boolean 'active' status.
  // Because we use INPUT_PULLUP, a LOW signal means the switch is active (WET).
  bool lowIsActive = (lowSwitchState == LOW);
  bool highIsActive = (highSwitchState == LOW);

  // Only send an update if the state of either switch has changed.
  if (lowIsActive != myData.isLowSwitchActive || highIsActive != myData.isHighSwitchActive) {
    
    Serial.println("Switch state changed! Sending update...");
    
    // Update the data structure with the new states.
    myData.isLowSwitchActive = lowIsActive;
    myData.isHighSwitchActive = highIsActive;

    // Send the updated data to the receiver.
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
     
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }
  }

  // Simple debounce delay. This pause prevents the code from misreading
  // rapid, noisy signals that occur as the mechanical switch contacts settle.
  delay(100); 
}
