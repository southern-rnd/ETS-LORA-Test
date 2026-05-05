# RAK3112 Multi-Mode IoT Device

A multi-mode embedded application for the **ESP32-S3 + RAK3172 (SX1262)** platform, combining LoRaWAN RFID scanning, RS-485 energy metering, and RS-485 anemometer reading — all selectable via touchscreen at startup.

---

## Table of Contents

- [Overview](#overview)
- [Hardware Requirements](#hardware-requirements)
- [Pin Configuration](#pin-configuration)
- [Modes](#modes)
  - [ETS Mode](#ets-mode)
  - [Careflow Mode](#careflow-mode)
  - [Anemometer Mode](#anemometer-mode)
- [Libraries Required](#libraries-required)
- [LoRaWAN Configuration](#lorawan-configuration)
- [Payload Format](#payload-format)
- [Project Structure](#project-structure)
- [Building](#building)
- [ChirpStack Setup](#chirpstack-setup)
- [Downlink Messages](#downlink-messages)
- [Troubleshooting](#troubleshooting)

---

## Overview

On boot, the device shows a **mode selection screen** with three buttons. The user taps one to enter that mode. There is no way to switch modes without restarting the device.

```
┌─────────────────────────────────────┐
│          SELECT MODE                │
│                                     │
│  ┌────────┐ ┌────────┐ ┌─────────┐ │
│  │  ETS   │ │ CARE-  │ │  WIND   │ │
│  │        │ │ FLOW   │ │  SPD    │ │
│  │RFID+   │ │MFM384  │ │Anemom-  │ │
│  │LoRaWAN │ │RS-485  │ │eter     │ │
│  └────────┘ └────────┘ └─────────┘ │
│       Touch screen to select        │
└─────────────────────────────────────┘
```

---

## Hardware Requirements

| Component | Details |
|---|---|
| MCU | ESP32-S3 development board |
| LoRa Module | RAK3172 (SX1262) |
| Display | TFT LCD 320×240 (ST7789 or ILI9341) |
| Touch | FT6336 capacitive touch controller |
| RFID Reader | UART TTL RFID reader (125kHz, 12-byte frame) |
| Energy Meter | MFM384 via RS-485 Modbus RTU |
| Anemometer | RS-485 Modbus RTU anemometer |
| RS-485 Transceiver | MAX485 or equivalent (DE/RE on pin 34) |

---

## Pin Configuration

### Display & Touch

| Signal | GPIO |
|---|---|
| TFT (SPI) | Configured in `User_Setup.h` (TFT_eSPI) |
| Backlight | GPIO 42 |
| Touch SDA | GPIO 9 |
| Touch SCL | GPIO 40 |
| Touch RST | GPIO 41 |
| Touch INT | Not used (-1) |

### RAK3172 SX1262 (LoRa)

| Signal | GPIO |
|---|---|
| NSS (CS) | GPIO 7 |
| SCK | GPIO 5 |
| MISO | GPIO 3 |
| MOSI | GPIO 6 |
| RESET | GPIO 8 |
| DIO1 | GPIO 47 |
| BUSY | GPIO 48 |

### RFID Reader (UART1)

| Signal | GPIO |
|---|---|
| RX | GPIO 18 |
| TX | GPIO 17 |
| Baud rate | 115200 8N1 |

### RS-485 (UART2 — shared by Careflow and Anemometer)

| Signal | GPIO |
|---|---|
| RX | GPIO 44 |
| TX | GPIO 43 |
| DE/RE | GPIO 34 |

> **Note:** Careflow and Anemometer share the same UART2 and RS-485 pins. The firmware reinitializes UART2 at the correct baud rate when each mode is selected. Only one can be active at a time.

---

## Modes

### ETS Mode

**Purpose:** RFID card scanning with LoRaWAN Class C uplink and downlink display.

**How it works:**
1. Device joins ChirpStack via OTAA (Class C)
2. User scans an RFID card
3. Card UID is sent as a 12-byte LoRaWAN uplink
4. Display shows HEX UID, decimal UID, and send status
5. Any downlink from the gateway is displayed immediately (Class C = always listening)
6. After 4 seconds, returns to waiting screen

**Display screens:**

- **Waiting** — shows scan prompt and LoRa join status
- **Card Detected** — shows HEX UID, DEC UID, LoRa send status
- **Gateway Message** — shows downlink port, raw HEX bytes, and ASCII text if printable

**RFID Frame Format:**

The reader outputs a 12-byte frame:
```
02  [8 ASCII hex chars of UID]  0D  0A  03
STX      UID bytes               CR  LF  ETX
```

---

### Careflow Mode

**Purpose:** Real-time energy monitoring via MFM384 three-phase power meter over RS-485 Modbus RTU.

**How it works:**
1. Reads 23 parameters from the MFM384 every 5 seconds
2. Displays all parameters in a scrollable list (8 rows visible at once)
3. Swipe up/down to scroll through all parameters
4. Error rows shown in red if a register fails to respond

**Parameters monitored:**

| Parameter | Register | Unit |
|---|---|---|
| Voltage V1N | 0 | V |
| Voltage V2N | 2 | V |
| Voltage V3N | 4 | V |
| Avg Voltage L-N | 6 | V |
| Voltage V12 | 8 | V |
| Voltage V23 | 10 | V |
| Voltage V31 | 12 | V |
| Avg Voltage L-L | 14 | V |
| Current I1 | 16 | A |
| Current I3 | 20 | A |
| Average Current | 22 | A |
| kW Phase 1 | 24 | kW |
| kW Phase 2 | 26 | kW |
| kW Phase 3 | 28 | kW |
| kVAr Phase 3 | 42 | kVAr |
| Total kW | 44 | kW |
| Total kVAr | 46 | kVAr |
| Power Factor 1 | 48 | — |
| Power Factor 2 | 50 | — |
| Power Factor 3 | 52 | — |
| Average PF | 54 | — |
| Frequency | 56 | Hz |
| Total kWh | 58 | kWh |

**Modbus settings:** Slave ID 1, 9600 baud, 8N1, Input Registers (FC04), 32-bit float little-endian (low word first).

---

### Anemometer Mode

**Purpose:** Real-time wind speed monitoring from an RS-485 Modbus RTU anemometer.

**How it works:**
1. Reads wind speed holding register 0x0000 every 2 seconds
2. Displays wind speed in m/s in large text
3. Shows Beaufort scale classification
4. Error state shown in red if device does not respond

**Display:**
```
┌─────────────────────────┐
│       ANEMOMETER        │
│           ~             │
│       WIND SPEED        │
│                         │
│         3.45            │
│          m/s            │
│  ───────────────────    │
│  Beaufort: Light Breeze │
│  Next read: 1s  ...     │
└─────────────────────────┘
```

**Beaufort Scale:**

| Speed (m/s) | Description |
|---|---|
| < 0.3 | Calm |
| 0.3 – 1.5 | Light Air |
| 1.6 – 3.3 | Light Breeze |
| 3.4 – 5.4 | Gentle Breeze |
| 5.5 – 7.9 | Moderate Breeze |
| 8.0 – 10.7 | Fresh Breeze |
| 10.8 – 13.8 | Strong Breeze |
| 13.9 – 17.1 | Near Gale |
| 17.2 – 20.7 | Gale |
| 20.8 – 24.4 | Strong Gale |
| 24.5 – 28.4 | Storm |
| ≥ 28.5 | Violent Storm |

**Modbus settings:** Slave ID 1, 4800 baud, 8N1, Holding Register 0x0000 (FC03). Raw value divided by 100 = m/s.

---

## Libraries Required

Install all of these in Arduino IDE via **Sketch → Include Library → Manage Libraries** or by downloading ZIPs:

| Library | Source |
|---|---|
| `SX126x-Arduino` (v2.0.32) | Arduino Library Manager — beegee-tokyo |
| `TFT_eSPI` | Arduino Library Manager — Bodmer |
| `ModbusMaster` | Arduino Library Manager — Doc Walker |
| `FT6336` | Manual install from GitHub |

> **Important:** `LoRaMacHelper.h` is included as `<mac/LoRaMacHelper.h>` — it ships inside the `SX126x-Arduino` library under the `src/mac/` subfolder. No separate installation needed.

---

## LoRaWAN Configuration

Edit these values at the top of `main.ino` to match your ChirpStack device:

```cpp
uint8_t nodeDeviceEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x24, 0x02, 0x98};
uint8_t nodeAppEUI[8]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t nodeAppKey[16]   = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x24, 0x02, 0x98,
                             0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x24, 0x02, 0x98};
```

| Setting | Value |
|---|---|
| Activation | OTAA |
| Device Class | Class C |
| Region | AS923 |
| Data Rate | DR3 |
| TX Power | TX_POWER_5 |
| ADR | Off |
| Duty Cycle | Off |
| Join retries | 8 |

---

## Payload Format

The ETS mode sends a **12-byte uplink** on port 1:

```
Bytes 0–3:  Decimal UID as uint32 big-endian
Bytes 4–11: ASCII HEX UID string (8 characters)
```

**Example** — Card UID `4050B047` (decimal `1079029831`):
```
40 50 B0 47  34 30 35 30 42 30 34 37
[  DEC UID ] [ ASCII: "4050B047"    ]
```

**ChirpStack Codec (JavaScript)** to decode:

```javascript
function decodeUplink(input) {
  var bytes = input.bytes;
  var decUID = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
  var hexUID = "";
  for (var i = 4; i < 12; i++) {
    hexUID += String.fromCharCode(bytes[i]);
  }
  return {
    data: {
      decUID: decUID >>> 0,
      hexUID: hexUID
    }
  };
}
```

---

## Project Structure

```
project/
├── main.ino          ← Main application (this file)
├── touch.h           ← FT6336 touch driver wrapper
├── User_Setup.h      ← TFT_eSPI pin configuration
└── README.md         ← This file
```

---

## Building

### Arduino IDE

1. Install **ESP32 board package** via Boards Manager (esp32 by Espressif, v3.x)
2. Select board: **ESP32S3 Dev Module**
3. Install all libraries listed above
4. Copy `User_Setup.h` into your `TFT_eSPI` library folder
5. Set partition scheme to **Huge APP** (for larger flash)
6. Upload at **921600** baud

### ESP-IDF v5.x (Arduino as component)

1. Add `main/idf_component.yml`:
```yaml
dependencies:
  idf: ">=5.0"
  espressif/arduino-esp32:
    version: ">=3.0.0"
```

2. Add `sdkconfig.defaults`:
```
CONFIG_FREERTOS_HZ=1000
CONFIG_ARDUINO_LOOP_STACK_SIZE=8192
CONFIG_ESP_MAIN_TASK_STACK_SIZE=8192
```

3. Build and flash:
```bash
idf.py set-target esp32s3
idf.py build
idf.py flash monitor
```

---

## ChirpStack Setup

1. Create a **Device Profile**:
   - MAC version: LoRaWAN 1.0.3
   - Regional parameters: AS923
   - Device class: **Class C**
   - Supports OTAA: Yes

2. Create an **Application** and add your device with:
   - DevEUI matching `nodeDeviceEUI` in code
   - AppKey matching `nodeAppKey` in code
   - AppEUI: all zeros

3. Verify join by checking **LoRaWAN Frames** tab — you should see `JoinRequest` followed by `JoinAccept`.

---

## Downlink Messages

Send downlinks from **ChirpStack → Device → Queue → Add downlink**, port 1.

**Text messages** (all ASCII printable bytes):

| Message | HEX Payload |
|---|---|
| `OK` | `4F 4B` |
| `ACCESS GRANTED` | `41 43 43 45 53 53 20 47 52 41 4E 54 45 44` |
| `DENIED` | `44 45 4E 49 45 44` |
| `DOOR OPEN` | `44 4F 4F 52 20 4F 50 45 4E` |

Text messages display in green. Binary messages display as HEX bytes and decimal values in cyan.

The downlink screen auto-dismisses after **6 seconds** and returns to the RFID waiting screen.

---

## Troubleshooting

**Display stays blank after boot**

- Check backlight pin (GPIO 42) — must be HIGH
- Verify `User_Setup.h` has correct SPI pins for your display
- LoRaWAN callbacks must not call display functions directly — use pending flags (already implemented)

**Join failed**

- Verify AppKey in code matches exactly what ChirpStack shows
- Check **LoRaWAN Frames** tab in ChirpStack — if no `JoinRequest` appears, the gateway is not receiving the signal
- Delete and re-add the device in ChirpStack to reset frame counters
- Ensure device profile is set to Class C with OTAA enabled

**Careflow shows ERROR for all rows**

- Check RS-485 wiring: RO→GPIO44, DI→GPIO43, DE/RE→GPIO34
- Verify MFM384 slave ID is 1 (default) — change `MFM_SLAVE_ID` if different
- Check baud rate — MFM384 default is 9600 8N1
- Ensure DE/RE pin transitions are working (preTransmission/postTransmission callbacks)

**Anemometer shows ERROR**

- Verify anemometer slave ID is 1 — change `ANEMOMETER_SLAVE_ID` if different
- Confirm baud rate is 4800 — different from MFM384
- RS-485 shares same pins as Careflow — cannot use both simultaneously
- Check holding register address — default is 0x0000

**Touch not responding**

- Verify I2C pins: SDA→GPIO9, SCL→GPIO40
- Check `touch_init(320, 240, 1)` rotation matches `tft.setRotation(1)`
- FT6336 RST pin is GPIO41 — ensure it's not held low
