/*
 * PROJECT: IoT Flood Monitor (Receiver Unit)
 * BOARD:   ESP32
 * PURPOSE: Controls a traffic light, listens for sensor data, relays status
 *          to a display unit, and hosts a secure web server to show status.
 * AUTHOR:  Lance Edward Dela Rosa
 */

#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>

// --- Web Server ---
WebServer server(80);
const char* www_user = "admin";
const char* www_pass = "password";

// --- WiFi & AP Credentials ---
const char *ssid = "IoT_Flood_Monitor_AP";
const char *password = "floodmon"; // Min 8 characters

//==============================================================================
// Peer MAC Addresses
//==============================================================================
uint8_t displayAddress[] = {0x34, 0x94, 0x54, 0x95, 0x7C, 0xC9};

//==============================================================================
// Pin Definitions
//==============================================================================
const int GREEN_LED_PIN = 18;
const int YELLOW_LED_PIN = 21;
const int RED_LED_PIN = 22;
const int BLUE_LED_PIN = 19;

//==============================================================================
// ESP-NOW Data Structures
//==============================================================================
typedef struct struct_message { bool isLowSwitchActive; bool isHighSwitchActive; } struct_message;
struct_message myData;
typedef struct struct_display_message { int displayState; } struct_display_message;
struct_display_message displayMessage;

//==============================================================================
// ESP-NOW Peer Info
//==============================================================================
esp_now_peer_info_t displayPeerInfo;

//==============================================================================
// Program State
//==============================================================================
#define STATE_NORMAL_CYCLE  0
#define STATE_FLOOD_LOW     1
#define STATE_FLOOD_HIGH    2
int currentState = STATE_NORMAL_CYCLE;
int previousState = STATE_NORMAL_CYCLE;
bool blueLedManualOverride = false; // Flag to indicate if blue LED is under manual control
bool manualTrafficOverride = false; // When true, manual colors stay and auto cycle pauses
bool manualRedState = false;
bool manualYellowState = false;
bool manualGreenState = false;

// --- Yellow LED breathing (PWM-style via analogWrite) ---
int yellowBreathDuty = 0;
int yellowBreathStep = 5;
unsigned long lastBreathUpdate = 0;
const int breathInterval = 20; // Update interval in ms for breathing effect
bool yellowBreathingActive = false;

//==============================================================================
// Traffic Light State
//==============================================================================
#define TRAFFIC_GREEN  0
#define TRAFFIC_YELLOW 1
#define TRAFFIC_RED    2
int currentTrafficLight = TRAFFIC_RED;
unsigned long previousMillis = 0;
const long greenDuration = 8000;
const long yellowDuration = 2000;
const long redDuration = 3000; // Set initial red light duration to 3 seconds

//==============================================================================
// ESP-NOW Callbacks
//==============================================================================
void OnDataRecv(const esp_now_recv_info * info, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  
  int previousStateForReset = currentState;
  blueLedManualOverride = false; // New sensor data means automatic control again

  if (!myData.isLowSwitchActive && !myData.isHighSwitchActive) {
    currentState = STATE_NORMAL_CYCLE;
  } else if (myData.isLowSwitchActive && !myData.isHighSwitchActive) {
    currentState = STATE_FLOOD_LOW;
  } else if (myData.isLowSwitchActive && myData.isHighSwitchActive) {
    currentState = STATE_FLOOD_HIGH;
  }

  // If the state has changed back to normal, reset the traffic light cycle
  if (previousStateForReset != STATE_NORMAL_CYCLE && currentState == STATE_NORMAL_CYCLE) {
    currentTrafficLight = TRAFFIC_RED;
    setTrafficLights(HIGH, LOW, LOW);
    digitalWrite(BLUE_LED_PIN, LOW); // Turn off blue LED
    previousMillis = millis(); // Reset timer for the red light
  }
}

void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("\r\nPacket to display status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

//==============================================================================
// Helper Functions
//==============================================================================
void setTrafficLights(bool r, bool y, bool g) { 
  digitalWrite(RED_LED_PIN, r); 
  digitalWrite(YELLOW_LED_PIN, y); 
  digitalWrite(GREEN_LED_PIN, g); 
}

// Apply manual traffic light states and stop breathing if active.
void applyManualTrafficLights() {
  if (yellowBreathingActive) {
    analogWrite(YELLOW_LED_PIN, 0);
    yellowBreathingActive = false;
    pinMode(YELLOW_LED_PIN, OUTPUT);
  }
  setTrafficLights(manualRedState, manualYellowState, manualGreenState);
}

String getStatusString() {
  switch (currentState) {
    case STATE_NORMAL_CYCLE: return "NORMAL";
    case STATE_FLOOD_LOW: return "FLOOD: LOW";
    case STATE_FLOOD_HIGH: return "FLOOD: HIGH";
    default: return "UNKNOWN";
  }
}

// Web Server Handlers

//==============================================================================

void handleResetControl() {

  currentState = STATE_NORMAL_CYCLE;

  digitalWrite(BLUE_LED_PIN, LOW); // Turn off blue LED

  blueLedManualOverride = false; // Reset blue LED manual override

  manualTrafficOverride = false;
  manualRedState = true;
  manualYellowState = false;
  manualGreenState = false;

  currentTrafficLight = TRAFFIC_RED; // Start traffic cycle with red

  setTrafficLights(HIGH, LOW, LOW);

  previousMillis = millis(); // Reset timer for the red light

  server.send(200, "text/plain", "System reset to normal.");

}



void handleBlueLedControl() {

  if (server.hasArg("state")) {

    String state = server.arg("state");

    if (state == "on") {

      digitalWrite(BLUE_LED_PIN, HIGH);

      blueLedManualOverride = true;

      server.send(200, "text/plain", "Blue LED ON.");

    } else if (state == "off") {

      digitalWrite(BLUE_LED_PIN, LOW);

      blueLedManualOverride = true;

      server.send(200, "text/plain", "Blue LED OFF.");

    } else {

      server.send(400, "text/plain", "Invalid blue LED state. Use 'on' or 'off'.");

    }

  } else {

    server.send(400, "text/plain", "Missing blue LED state parameter.");

  }

}



void handleDashboard() {

  String html = "<!DOCTYPE html><html><head><title>IoT Flood Monitor</title>";

  html += "<meta http-equiv='refresh' content='5'>"; // Auto-refresh for status, buttons will handle their own updates

  html += "<style>";

  html += "body{font-family: Arial, sans-serif; text-align: center; background-color: #282c34; color: #f4f4f9;}";

  html += "h1{color: #61dafb;} h2{color: #ffffff;}";

  html += ".button-container button {";

  html += "  background-color: #007bff; color: white; border: none; padding: 10px 20px; margin: 5px;";

  html += "  border-radius: 5px; cursor: pointer; font-size: 16px; transition: background-color 0.3s ease;";

  html += "}";

  html += ".button-container button:hover { background-color: #0056b3; }";

  html += ".button-container { margin-top: 20px; }";

  html += "</style>";

  html += "</head><body>";

  html += "<h1>IoT Flood Monitor</h1>";

  html += "<h2>Current Status: " + getStatusString() + "</h2>";

  html += "<div class='button-container'>";

  html += "  <button onclick=\"fetch('/control/reset').then(response => { if(response.ok) alert('System Reset'); else alert('Reset Failed'); location.reload(); });\">Reset to Normal</button>";

  html += "  <button onclick=\"fetch('/control/blueled?state=on').then(response => { if(response.ok) alert('Blue LED ON'); else alert('Control Failed'); location.reload(); });\">Blue LED ON</button>";

  html += "  <button onclick=\"fetch('/control/blueled?state=off').then(response => { if(response.ok) alert('Blue LED OFF'); else alert('Control Failed'); location.reload(); });\">Blue LED OFF</button>";

  html += "  <button onclick=\"fetch('/control/light?color=red&state=on').then(r => { if(r.ok) alert('Red ON'); else alert('Failed'); location.reload(); });\">Red ON</button>";
  html += "  <button onclick=\"fetch('/control/light?color=red&state=off').then(r => { if(r.ok) alert('Red OFF'); else alert('Failed'); location.reload(); });\">Red OFF</button>";

  html += "  <button onclick=\"fetch('/control/light?color=yellow&state=on').then(r => { if(r.ok) alert('Yellow ON'); else alert('Failed'); location.reload(); });\">Yellow ON</button>";
  html += "  <button onclick=\"fetch('/control/light?color=yellow&state=off').then(r => { if(r.ok) alert('Yellow OFF'); else alert('Failed'); location.reload(); });\">Yellow OFF</button>";

  html += "  <button onclick=\"fetch('/control/light?color=green&state=on').then(r => { if(r.ok) alert('Green ON'); else alert('Failed'); location.reload(); });\">Green ON</button>";
  html += "  <button onclick=\"fetch('/control/light?color=green&state=off').then(r => { if(r.ok) alert('Green OFF'); else alert('Failed'); location.reload(); });\">Green OFF</button>";

  html += "  <button onclick=\"fetch('/control/traffic/auto').then(r => { if(r.ok) alert('Auto cycle resumed'); else alert('Failed'); location.reload(); });\">Resume Auto Cycle</button>";

  html += "</div>";

  html += "</body></html>";

  server.send(200, "text/html", html);

}



void handleLogin() {

  if (server.hasArg("username") && server.hasArg("password") &&

      server.arg("username") == www_user && server.arg("password") == www_pass) {

    server.sendHeader("Location", "/dashboard", true);

    server.send(302, "text/plain", "");

  } else {

    String html = "<!DOCTYPE html><html><head><title>Login Failed</title>";

    html += "<style>body{font-family: Arial, sans-serif; text-align: center; background-color: #282c34; color: white;} a{color: #61dafb;}</style>";

    html += "</head><body>";

    html += "<h1>401 - Unauthorized</h1><p>Invalid username or password.</p>";

    html += "<a href='/'>Try again</a>";

    html += "</body></html>";

    server.send(401, "text/html", html);

  }

}



void handleRoot() {

  String html = "<!DOCTYPE html><html><head><title>Login</title>";

  html += "<style>body{font-family: Arial, sans-serif; text-align: center; background-color: #282c34; color: white;}";

  html += "form{display: inline-block; margin-top: 50px;}</style>";

  html += "</head><body>";

  html += "<h1>IoT Flood Monitor Login</h1>";

  html += "<form method='POST' action='/login'>";

  html += "<label>Username:</label><br><input type='text' name='username'><br><br>";

  html += "<label>Password:</label><br><input type='password' name='password'><br><br>";

  html += "<input type='submit' value='Login'>";

  html += "</form></body></html>";

  server.send(200, "text/html", html);

}



void handleNotFound() {

  server.send(404, "text/plain", "404: Not Found");

}


// Manual traffic light control
void handleTrafficAuto() {
  manualTrafficOverride = false;
  manualRedState = false;
  manualYellowState = false;
  manualGreenState = false;

  // Reset to normal cycle start
  setTrafficLights(HIGH, LOW, LOW);
  currentTrafficLight = TRAFFIC_RED;
  previousMillis = millis();
  server.send(200, "text/plain", "Traffic lights set to auto cycle.");
}

void handleLightControl() {
  if (!server.hasArg("color") || !server.hasArg("state")) {
    server.send(400, "text/plain", "Missing parameters. Use color=red|yellow|green and state=on|off.");
    return;
  }

  String color = server.arg("color");
  String state = server.arg("state");
  bool on = (state == "on");

  if (color == "red") {
    manualRedState = on;
  } else if (color == "yellow") {
    manualYellowState = on;
  } else if (color == "green") {
    manualGreenState = on;
  } else {
    server.send(400, "text/plain", "Invalid color. Use red, yellow, or green.");
    return;
  }

  manualTrafficOverride = true;
  applyManualTrafficLights();
  server.send(200, "text/plain", "Manual traffic light update applied.");
}



//==============================================================================

// Traffic Cycle Logic

//==============================================================================

void runNormalTrafficCycle() {

  unsigned long currentMillis = millis();

  long interval = redDuration;



  if (currentTrafficLight == TRAFFIC_GREEN) {

    interval = greenDuration;

  } else if (currentTrafficLight == TRAFFIC_YELLOW) {

    interval = yellowDuration;

  } else if (currentTrafficLight == TRAFFIC_RED) {

    interval = redDuration;

  }



  if (currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis;

    if (currentTrafficLight == TRAFFIC_GREEN) {

      currentTrafficLight = TRAFFIC_YELLOW;

      setTrafficLights(LOW, HIGH, LOW);

    } else if (currentTrafficLight == TRAFFIC_YELLOW) {

      currentTrafficLight = TRAFFIC_RED;

      setTrafficLights(HIGH, LOW, LOW);

    } else if (currentTrafficLight == TRAFFIC_RED) {

      currentTrafficLight = TRAFFIC_GREEN;

      setTrafficLights(LOW, LOW, HIGH);

    }

  }

}



//==============================================================================

// Main Setup

//==============================================================================

void setup() {

  Serial.begin(115200);

  pinMode(GREEN_LED_PIN, OUTPUT);

  pinMode(YELLOW_LED_PIN, OUTPUT);

  pinMode(RED_LED_PIN, OUTPUT);

  pinMode(BLUE_LED_PIN, OUTPUT);

  

  // Start with red light on for 3 seconds

  setTrafficLights(HIGH, LOW, LOW);

  digitalWrite(BLUE_LED_PIN, LOW);

  previousMillis = millis();

  currentTrafficLight = TRAFFIC_RED;



  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP(ssid, password);

  Serial.print("AP IP address: ");

  Serial.println(WiFi.softAPIP());



  if (esp_now_init() != ESP_OK) { Serial.println("Error initializing ESP-NOW"); return; }

  esp_now_register_recv_cb(OnDataRecv);

  esp_now_register_send_cb(OnDataSent);

  memcpy(displayPeerInfo.peer_addr, displayAddress, 6);

  displayPeerInfo.channel = 0;  

  displayPeerInfo.encrypt = false;

  if (esp_now_add_peer(&displayPeerInfo) != ESP_OK){ Serial.println("Failed to add display peer"); return; }



  server.on("/", handleRoot);

  server.on("/login", HTTP_POST, handleLogin);

  server.on("/dashboard", HTTP_GET, handleDashboard);

  server.on("/control/reset", HTTP_GET, handleResetControl); // New control endpoint

  server.on("/control/blueled", HTTP_GET, handleBlueLedControl); // New control endpoint
  server.on("/control/light", HTTP_GET, handleLightControl); // Manual traffic light control
  server.on("/control/traffic/auto", HTTP_GET, handleTrafficAuto); // Resume auto cycle

  server.onNotFound(handleNotFound);

  server.begin();

  Serial.println("HTTP server started");

}

 

//==============================================================================

// Main Loop

//==============================================================================

void loop() {

  server.handleClient();

  unsigned long currentMillis = millis();

  // Release breathing control when not in flood low
  if (currentState != STATE_FLOOD_LOW && yellowBreathingActive) {
    analogWrite(YELLOW_LED_PIN, 0);
    yellowBreathingActive = false;
    pinMode(YELLOW_LED_PIN, OUTPUT);
  }


  // Only update blue LED based on currentState if not manually overridden

  if (!blueLedManualOverride) {

    if (currentState == STATE_FLOOD_LOW || currentState == STATE_FLOOD_HIGH) {

      digitalWrite(BLUE_LED_PIN, HIGH);

    } else {

      digitalWrite(BLUE_LED_PIN, LOW);

    }

  }



  switch (currentState) {

    case STATE_NORMAL_CYCLE:

      // Blue LED handled by manual override logic above
      if (manualTrafficOverride) {
        applyManualTrafficLights();
      } else {
        runNormalTrafficCycle();
      }

      break;

      

    case STATE_FLOOD_LOW:

      if (manualTrafficOverride) {
        applyManualTrafficLights();
      } else {
        // Keep red/green off
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, LOW);

        // Start breathing if not active
        if (!yellowBreathingActive) {
          yellowBreathingActive = true;
          yellowBreathDuty = 0;
          yellowBreathStep = 5;
          lastBreathUpdate = currentMillis;
          pinMode(YELLOW_LED_PIN, OUTPUT);
        }

        // Breathing effect: ramp up and down
        if (currentMillis - lastBreathUpdate >= breathInterval) {
          lastBreathUpdate = currentMillis;
          yellowBreathDuty += yellowBreathStep;
          if (yellowBreathDuty >= 255 || yellowBreathDuty <= 0) {
            yellowBreathStep = -yellowBreathStep;
            if (yellowBreathDuty > 255) yellowBreathDuty = 255;
            if (yellowBreathDuty < 0) yellowBreathDuty = 0;
          }
          analogWrite(YELLOW_LED_PIN, yellowBreathDuty);
        }
      }

      break;

      

    case STATE_FLOOD_HIGH:

      if (manualTrafficOverride) {
        applyManualTrafficLights();
      } else {
        setTrafficLights(HIGH, LOW, LOW); // Red LED is solid ON

        // Blue LED handled by manual override logic above

        digitalWrite(YELLOW_LED_PIN, LOW);

        digitalWrite(GREEN_LED_PIN, LOW);
      }

      break;

  }



  if (currentState != previousState) {

    displayMessage.displayState = currentState;

    esp_now_send(displayAddress, (uint8_t *) &displayMessage, sizeof(displayMessage));

    previousState = currentState;

  }

}
