<p align="center">
  <img src="CrewTrack.png" alt="CrewTrack Banner" width="100%" />
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Platform-ESP32-blue?logo=espressif" />
  <img src="https://img.shields.io/badge/Language-C++-00599C?logo=cplusplus" />
  <img src="https://img.shields.io/badge/License-MIT-green" />
  <img src="https://img.shields.io/badge/Version-1.0.0-orange" />
</p>

<h1 align="center">CREWTRACK</h1>

<p align="center">
  <b>Smart RFID Attendance & Payroll Tracker for Small Work Crews</b><br>
  <sub>Built for electricians, plumbers, HVAC techs, painters, builders, and any crew with 2-20 workers</sub>
</p>

---

## The Problem

> *"How many days did George work this month?"*
> *"Did Nikos work 8 or 10 days?"*
> *"How much should I pay Kostas?"*

Small businesses with temporary workers track attendance on paper or from memory. At month-end, nobody remembers the exact numbers.

## The Solution

**CrewTrack** is a standalone device installed in your work van. Every assistant has an RFID card. Every morning, before leaving for work, tap each card. The device handles the rest.

```
┌─────────────────────────────┐
│         CREWTRACK           │
│                             │
│            OK               │
│                             │
│          George             │
│     Attendance Saved        │
│                             │
│     12 Jul 2026  07:34      │
└─────────────────────────────┘
```

---

## Features

| Feature | Description |
|---------|-------------|
| **RFID Scan-In** | Tap card → instant attendance log |
| **Duplicate Detection** | Won't double-count same-day scans |
| **Unknown Card Alert** | Flags unregistered cards immediately |
| **LCD Display** | 2" ST7789 shows scan results in real-time |
| **Audio Feedback** | Buzzer confirms each scan |
| **SD Card Storage** | Attendance saved as CSV, one file per day |
| **WiFi Dashboard** | Connect phone to manage everything wirelessly |
| **Employee Management** | Add/edit/delete employees from your phone |
| **Monthly Salary** | Auto-calculates days worked × daily wage |
| **CSV Export** | Download full attendance history |
| **Search & Filter** | Find any worker by name or UID |
| **NTP Time Sync** | Accurate timestamps without RTC module |

---

## Hardware

| Component | Model | Approx Cost |
|-----------|-------|-------------|
| Microcontroller | ESP32 DevKit | €3-5 |
| RFID Reader | RC522 | €2-3 |
| Display | ST7789V 2" LCD | €4-6 |
| Storage | MicroSD Card Module | €1-2 |
| Feedback | Active Buzzer | €0.50 |
| **Total** | | **~€12-18** |

---

## Wiring

All three SPI devices (RFID, LCD, SD) share one SPI bus with separate chip select pins.

```
                    ESP32
                 ┌──────────┐
    SCK  ────────┤ GPIO 18  ├──────── Shared SPI Clock
    MISO ────────┤ GPIO 19  ├──────── Shared SPI MISO
    MOSI ────────┤ GPIO 23  ├──────── Shared SPI MOSI
                 │          │
    RFID SS ─────┤ GPIO  5  │
    RFID RST ────┤ GPIO 22  │
                 │          │
    LCD CS  ─────┤ GPIO 15  │
    LCD DC  ─────┤ GPIO  2  │
    LCD RST ─────┤ GPIO  4  │
                 │          │
    SD CS   ─────┤ GPIO 10  │
                 │          │
    Buzzer  ─────┤ GPIO 25  │
                 └──────────┘
```

### RFID RC522

| Module | ESP32 |
|--------|-------|
| SDA (SS) | GPIO 5 |
| SCK | GPIO 18 |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| RST | GPIO 22 |
| 3.3V | 3.3V |
| GND | GND |

### ST7789V 2" LCD

| Module | ESP32 |
|--------|-------|
| VCC | 3.3V |
| GND | GND |
| SCL (CLK) | GPIO 18 |
| SDA (MOSI) | GPIO 23 |
| CS | GPIO 15 |
| DC | GPIO 2 |
| RST (RES) | GPIO 4 |
| BL (Backlight) | 3.3V |

### MicroSD Card Module

| Module | ESP32 |
|--------|-------|
| VCC | 3.3V |
| GND | GND |
| CLK | GPIO 18 |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| CS | GPIO 10 |

### Buzzer

| Module | ESP32 |
|--------|-------|
| + | GPIO 25 |
| - | GND |

---

## Setup

### 1. Install Arduino IDE

Download from [arduino.cc](https://www.arduino.cc/en/software)

### 2. Install ESP32 Board

1. Go to **File > Preferences**
2. Add to "Additional Boards Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools > Board > Boards Manager**
4. Search "esp32" and install **esp32 by Espressif Systems**

### 3. Install Libraries

**Sketch > Include Library > Manage Libraries**, install:

| Library | Author |
|---------|--------|
| `MFRC522` | miguelbalboa |
| `TFT_eSPI` | bodmer |
| `ArduinoJson` | Benoit Blanchon |
| `ESPAsyncWebServer` | me-no-dev |
| `AsyncTCP` | me-no-dev |

### 4. Configure TFT_eSPI

Navigate to your Arduino libraries folder:
```
Windows:  C:\Users\<you>\Documents\Arduino\libraries\TFT_eSPI\
Mac:      ~/Documents/Arduino/libraries/TFT_eSPI/
Linux:    ~/Arduino/libraries/TFT_eSPI/
```

Open `User_Setup.h` and replace the ESP32 pin section with:

```c
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4

#define SPI_FREQUENCY  40000000
```

### 5. Upload

1. Open `CrewTrack.ino`
2. **Tools > Board > ESP32 Dev Module**
3. **Tools > Port** → select your COM port
4. Click **Upload**

### 6. Prepare SD Card

1. Format SD card as **FAT32**
2. Create folder: `/attendance/`
3. Copy `employees_example.json` to root, rename to `employees.json`
4. Edit `employees.json` with your workers' card UIDs

---

## First Use

1. Power on the device
2. LCD shows **"CREWTRACK - Ready"**
3. Connect your phone to WiFi: **`CrewTrack`** / **`12345678`**
4. Open browser: **`192.168.4.1`**
5. Add employees via the dashboard
6. Scan each employee's RFID card to register it
7. Done! Every morning, just tap cards

---

## Web Dashboard

Connect to the device's WiFi and open `192.168.4.1`:

```
┌──────────────────────────────────────────┐
│              CREWTRACK                   │
│   Smart Attendance & Payroll Tracker     │
├──────────────────────────────────────────┤
│                                          │
│  [Dashboard] [Workers] [Attendance]      │
│  [Salary]    [+ Add]                     │
│                                          │
│  ┌──────┐  ┌──────┐  ┌──────┐           │
│  │  3   │  │  5   │  │07:36 │           │
│  │Today │  │ Empl │  │Last  │           │
│  │      │  │      │  │Scan  │           │
│  └──────┘  └──────┘  └──────┘           │
│                                          │
│  Recent Scans                            │
│  ─────────────────────────               │
│  George    E4913A21         07:34        │
│  Nikos     AABBCCDD         07:36        │
│  John      11223344         07:40        │
│                                          │
│  [Download CSV]                          │
└──────────────────────────────────────────┘
```

### Dashboard Tabs

- **Dashboard** — Today's stats, recent scans, last scan time
- **Workers** — All employees with days worked, searchable
- **Attendance** — Full history of all scans with dates
- **Salary** — Monthly report with days × wage, total, progress bars
- **+ Add** — Register new employees or edit existing ones

---

## Data Storage

### SD Card Structure
```
/
├── employees.json          # Employee database
└── attendance/
    ├── att_2026_07_01.csv  # July 1st logs
    ├── att_2026_07_02.csv  # July 2nd logs
    ├── att_2026_07_03.csv  # ...
    └── att_2026_07_12.csv  # Today
```

### Daily CSV Format
```csv
E4913A21,07:34
AABBCCDD,07:36
11223344,07:40
```

### Employee JSON Format
```json
[
  {"uid": "E4913A21", "name": "George", "wage": 50.0, "phone": ""},
  {"uid": "AABBCCDD", "name": "Nikos",  "wage": 45.0, "phone": ""}
]
```

---

## Project Structure

```
CrewTrack/
├── CrewTrack.ino              Main application (single file)
├── TFT_eSPI_User_Setup.h      Copy this to TFT_eSPI library folder
├── employees_example.json     Example employee data for SD card
└── README.md                  This file
```

---

## How It Works

```
  ┌──────────┐     ┌──────────┐     ┌──────────┐
  │ Employee │────▶│  RFID    │────▶│  ESP32   │
  │ taps card│     │  RC522   │     │          │
  └──────────┘     └──────────┘     └────┬─────┘
                                         │
                    ┌────────────────────┤
                    │                    │
                    ▼                    ▼
              ┌──────────┐        ┌──────────┐
              │   LCD    │        │ SD Card  │
              │ Display  │        │  Log CSV │
              └──────────┘        └──────────┘
                    │
                    ▼
              ┌──────────┐        ┌──────────┐
              │  Buzzer  │        │  WiFi    │
              │  Beep!   │        │Dashboard │
              └──────────┘        └──────────┘
```

---

## Target Customers

- Electricians
- Plumbers
- HVAC technicians
- Painters
- Builders
- Cleaning companies
- Garden maintenance crews
- Any small business with 2-20 employees

---

## Future Upgrades

- [ ] **GPS (GY-NEO6MV2)** — Log where attendance was recorded
- [ ] **RTC Module** — Keep time when WiFi is unavailable
- [ ] **Multiple Vehicles** — Track Van 1, Van 2 separately
- [ ] **Cloud Sync** — Backup to cloud automatically
- [ ] **Mobile App** — Live attendance, push notifications
- [ ] **Admin Login** — Password-protect dashboard settings
- [ ] **PDF Reports** — Generate monthly payroll documents

---

## License

MIT License - use freely for personal or commercial projects.

---

<p align="center">
  <b>Built with ESP32 + Arduino</b><br>
  <sub>Star this repo if it helps your business</sub>
</p>
