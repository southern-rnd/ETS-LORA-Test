# ETS LoRa RFID & RS485 System

A compact embedded system based on **RAK3112 (ESP32-S3 + LoRa)** combining:

- RFID scanning
- RS485 / RS232 communication
- TFT display UI
- Industrial I/O control

---

## 📌 Project Overview

This project integrates:

- Dual-frequency RFID (125kHz + 13.56MHz)
- RS485 & RS232 communication interfaces
- 2.8" TFT display with touch
- User interaction (buttons, buzzer, RGB LED)

Designed for **IoT & industrial applications**.

---

## 🧰 Hardware

- **MCU:** RAK3112 (ESP32-S3 + LoRa)
- **Display:** ILI9341 2.8" TFT (FT6336 Touch)
- **RFID:** UART-based dual-frequency module
- **RS485:** TP8485E
- **RS232:** MAX3232
- **Power:** RT6160A Buck-Boost

---

## ⚙️ Features

- RFID UID display (HEX + Decimal)
- RS485 communication support
- RS232 interface
- Capacitive touch UI
- NeoPixel RGB LED
- Buzzer + Buttons
- Dual power input (USB-C + DC)

---

## 🔌 Pin Configuration (RFID + Display)

| Signal | GPIO |
|------|------|
| LCD_CS | 12 |
| LCD_RST | 39 |
| LCD_DC | 38 |
| MOSI | 11 |
| SCK | 13 |
| MISO | 10 |
| Backlight | 42 |

RFID:
- TX → GPIO 18

---

## 🔋 Power Architecture

- Input: 5V (USB-C / DC)
- Output:
  - 3.3V (logic)
  - 5V (peripherals)

---

## 📂 Files

- `/Firmware` → ESP32 code
- `/Schematic` → PCB & circuit design
- `/Docs` → documentation

---

## 🛠️ Build & Flash

### PlatformIO
```bash
pio run
pio run --target upload
pio device monitor
```

### Arduino IDE
- Select ESP32S3 Dev Module
- Configure TFT_eSPI
- Upload

---

## 🚀 Applications

- Industrial RFID systems
- Access control
- IoT node with RS485 network