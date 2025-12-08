# Flood Warning Web Page

Project page for the IoT Flood Monitoring system, served from the `docs/floodwarning/` path on GitHub Pages.

## Contents
- `index.html` – main page with project overview, documentation carousel, schematic, libraries, and firmware links
- `style.css` – layout and theme styles
- `script.js` / `scene.js` – animations and interactions (GSAP + Three.js)
- `firmware/` – Arduino sketches:
  - `Float_Sensor.ino` (ESP32-C3 sensor unit)
  - `Traffic_Receiver.ino` (ESP32 receiver/traffic-light unit with web dashboard)
  - `MAC_Address_Finder.ino` (Utility to read a board's MAC address)
- Images live in `docs/imgs/` with names `flood_doc1.jpg` … `flood_doc6.jpg` and `flood_schema.png`.

## Accessing the project page
- GitHub Pages: navigate via the main `docs/index.html` landing page (Flood Warning link).
- Local: open `docs/floodwarning/index.html` in a browser with `docs/imgs/` present for images.

## Firmware quick start
- Sensor unit: flash `Float_Sensor.ino` to ESP32-C3. Update `broadcastAddress` with the receiver MAC.
- Receiver unit: flash `Traffic_Receiver.ino` to ESP32. Hosts a dashboard and exposes control endpoints. Update `www_user`/`www_pass`, and set MACs to match your hardware.
- MAC Address Helper: Use `MAC_Address_Finder.ino` on any target board (e.g., your ESP32 receiver) to easily find its MAC address via the Serial Monitor.
- **Board Compatibility:** Whatever board you are using for a specific role (Sensor, Receiver, or Helper), you must ensure you have the correct board support package installed in your IDE. The firmware is designed for specific hardware and relies on compatible libraries, communication protocols (like ESP-NOW), and correct device-specific parameter configuration (like MAC addresses and pinouts).

## Hardware & wiring (summary)
- Sensor unit (ESP32-C3): two float switches on GPIO 4 (low) and GPIO 5 (high), `INPUT_PULLUP`.
- Receiver unit (ESP32): traffic LEDs on GPIO 22 (red), 21 (yellow), 18 (green), 19 (blue).
- Communications: ESP-NOW between sensor and receiver; receiver also runs a SoftAP (`IoT_Flood_Monitor_AP` / `floodmon`) for the dashboard.

## Dashboard controls (receiver)
- Login served at `/` (basic auth values set in the sketch).
- Status dashboard at `/dashboard`.
- Control endpoints: `/control/reset`, `/control/blueled?state=on|off`, `/control/light?color=red|yellow|green&state=on|off`, `/control/traffic/auto`.

## Notes
- External libraries via CDN: Three.js and GSAP/ScrollTrigger.
- Wi-Fi credentials in firmware are placeholders; change for deployment.
- Repository (web + firmware): https://github.com/elenriquez/IoT2025/tree/main/docs/floodwarning
