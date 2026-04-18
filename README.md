# Rov_TechAni 🚀
**Tele-Operated Inspection & Autonomous Robotics Platform**

Rov_TechAni is a distributed-system robotics platform initially inspired by Mars rover designs, evolved into a terrestrial inspection and disaster-response vehicle. It features an isolated triple-rail power architecture, a 4-DOF robotic arm, skid-steering locomotion, and a responsive web-based command center with live FPV video streaming.

---

## 🌟 Features
* **Dual Operating Modes:**
  * **Manual Override:** Full tele-operation via a responsive web dashboard. Control skid-steering locomotion (W,A,S,D) and articulate the 4-DOF robotic arm.
  * **Autonomous Radar (Auto Mode):** The ESP32 takes over, polling a 4-point ultrasonic array to navigate environments and actively evade obstacles without user input.
* **Live FPV Streaming:** Integrated low-latency video feed using a mobile IP Webcam, directly embedded into the command dashboard.
* **Proximity Alarms:** Onboard hardware buzzer for imminent collision warnings.
* **Asynchronous Telemetry:** Ultra-fast UDP network communication ensures zero-blocking hardware control.

---

## 🛠️ Tech Stack

### **Hardware (The Brawn)**
* **Core Brain:** ESP32 Microcontroller (DOIT DevKit v1)
* **Locomotion:** 4x Yellow TT Gear Motors (Skid Steering) driven by an L298N Motor Controller
* **Manipulation:** 4x Micro Servos (Base, Shoulder, Elbow, Claw)
* **Spatial Awareness:** 4x HC-SR04 Ultrasonic Sensors
* **Power Architecture:** * 7.4V Li-Ion pack (Drive System)
  * 5V Power Bank (Logic System)
  * 5.5V Buck Converter rail (Servo Power)

### **Software (The Brains)**
* **Ground Control Station:** macOS / Python 3.11
* **Backend:** Flask (Python) bridging HTTP UI inputs to UDP hardware packets
* **Frontend:** HTML5, CSS3 (Glass-morphism UI), Vanilla JavaScript
* **Firmware:** C++ (Arduino IDE) utilizing the `ESP32Servo` library

---

## ⚙️ Circuit & Power Architecture Warnings
**CRITICAL:** This project mixes logic boards with heavy-draw inductive motors. 
1. **Common Ground:** The grounds of the Li-Ion battery, Power Bank, Buck Converter, ESP32, and L298N **must** be tied together.
2. **Servo Isolation:** The servos draw massive current spikes. They are powered strictly by the buck converter, *never* from the ESP32 5V pin.
3. **Sensor Logic Level:** The HC-SR04 sensors output 5V. A voltage divider (1kΩ and 2kΩ resistors) is used on all `ECHO` pins to step the signal down to a safe 3.3V for the ESP32.

---

## 🚀 Installation & Setup

### 1. Hardware Firmware (ESP32)
1. Open the Arduino IDE.
2. Go to **Sketch > Include Library > Manage Libraries** and install `ESP32Servo` by Kevin Harrington.
3. Open `ESP32_Brain.ino` (your main hardware code).
4. Edit the network credentials:
   ```cpp
   const char* ssid = "YOUR_HOTSPOT_NAME";
   const char* password = "YOUR_HOTSPOT_PASSWORD";
