# Smart Watch Prototype (ESP32)

**Author:** Goda Gutparakytė  
**Course:** Introduction to Robotics — Vilnius University  
**Assignment:** HW4  
**Last Updated:** 2025-11-24  

---

## Overview
This project implements a prototype **smart watch** using the ESP WiFi-enabled microcontroller.  
The device demonstrates essential smartwatch functions, including sensing, step tracking, real-time updates, and user interaction.

### Features
- **240×240 round TFT LCD (GC9A01) display**  
- **GYMAX-30102** heart-rate and pulse oximetry sensor  
- **MPU-6050** accelerometer and gyroscope for step counting  
- **Physical button** for switching UI themes  
- **WiFi connectivity** to obtain current time and date  
- **Weather information** retrieved using the Open-Meteo API
- **3.7 V Li-ion/LiPo battery** for portable power  
- **TP4056 charging module** for safe USB charging  

---

## Circuit Image
![Circuit picture](circuit.jpg)

---

## Build Steps
1. Assemble all components according to the wiring schematic below.  
2. Open the `.ino` file in the Arduino IDE.  
3. Select **Board: ESP32** (or the specific ESP32 variant you are using).  
4. Select the correct **COM port**.  
5. Click **Upload** to flash the program onto the microcontroller.  

---

## Component List

| Label | Quantity | Component |
|-------|----------|-----------|
| U1    | 1        | ESP32 microcontroller |
| L     | 1        | GC9A01 round TFT LCD display |
| H     | 1        | GYMAX-30102 heart-rate & SpO₂ sensor |
| B     | 1        | Push button |
| G     | 1        | MPU-6050 accelerometer & gyroscope |
| BAT   | 1        | 3.7 V Li-ion/LiPo battery |
| CHG   | 1        | TP4056 Li-ion USB charging module |


---

## Wiring

![Wiring schematic](wiring_schematic.pdf)
