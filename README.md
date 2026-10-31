# Smart Solar-Tracking Environmental Weather Tower

An intelligent, IoT-enabled solar-tracking weather and thermal management system powered by an **ESP32**, dual Light Dependent Resistors (LDRs), an **SG90 servo motor**, a **DHT11 sensor**, an **L298N motor driver**, and a custom DC cooling fan. Telemetry data is streamed in real-time over MQTT (HiveMQ) to a professional web dashboard.

---

## Project Overview
This project addresses two core challenges in renewable energy and embedded systems:
1. **Maximized Solar Efficiency:** Dual LDR sensors detect ambient light gradients, allowing the SG90 servo motor to dynamically orient the sensor tower toward peak sunlight.
2. **Smart Thermal Regulation:** The DHT11 sensor monitors ambient temperature. When temperatures exceed a user-defined potentiometer threshold, an L298N motor driver triggers a cooling fan to protect onboard electronics.
3. **Cloud Telemetry & Web Dashboard:** Live temperature, threshold, and fan status are published via MQTT (`broker.hivemq.com`) and visualized on a custom dark-mode web dashboard built with **Chart.js** and **Paho MQTT**.

---

## 🛠️ Hardware Architecture & Motor Selection Justification

### 1. SG90 Micro Servo Motor (Solar Tracking Mechanism)
* **Selection Justification:** The **SG90 servo** was selected for the solar-tracking head because of its precise angular positioning ($0^\circ$ to $180^\circ$) and lightweight footprint. Solar tracking requires accurate, incremental rotational adjustments rather than continuous high-torque rotation. The PWM control signals from the ESP32 allow fine-grained alignment based on differential analog readings from the dual LDR voltage dividers.

### 2. L298N Motor Driver & DC Cooling Fan (Thermal Management)
* **Selection Justification:** The **L298N dual H-bridge motor driver** was chosen to drive the DC cooling fan because it provides robust isolation and switching capability between the ESP32's 3.3V logic signals and the external higher-voltage power supply required for the motor. This prevents inductive voltage spikes and motor electrical noise from triggering the ESP32's brownout detector or RTC watchdog timer.

### 3. ESP32 Microcontroller & DHT11 Sensor
* **ESP32:** Selected for its integrated Wi-Fi capabilities, dual-core processing, and rich set of ADC/PWM pins, enabling concurrent non-blocking sensor polling and MQTT packet transmission.
* **DHT11:** Chosen for cost-effective, reliable digital temperature and relative humidity sensing within the enclosure.

---

## 👥 Team Information & Individual Contributions
| **M Mansif Morshed** | Firmware development (ESP32 non-blocking loops, MQTT client integration, watchdog stability), CAD mechanical modeling, and Web Dashboard (HTML/JS/Chart.js) architecture. |

## 📺 Project Video Demonstration
Click the link below to watch the full project walkthrough and hardware demo on YouTube:

👉 **[Watch the Smart Solar Tracker Demo on YouTube](https://www.youtube.com/watch?v=3auwEeMrJEg)**
---




