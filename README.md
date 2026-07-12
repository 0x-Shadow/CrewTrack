<p align="center">
  <img src="CrewTrack.png" alt="CrewTrack" width="100%" />
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Version-1.0.0-blue" />
  <img src="https://img.shields.io/badge/Platform-ESP32-000?logo=espressif" />
  <img src="https://img.shields.io/badge/Language-C++-00599C?logo=cplusplus" />
  <img src="https://img.shields.io/badge/License-MIT-green" />
  <img src="https://img.shields.io/badge/Status-Working-orange" />
</p>

<h1 align="center">CrewTrack</h1>

<p align="center">
  <b>RFID Attendance for Work Vans</b><br>
  <sub>Know who worked, how many days, and with whom — every month, automatically</sub>
</p>

---

## The Problem

> *"How many days did George work this month?"*
> *"Did Nikos work 8 or 10 days?"*
> *"How much should I pay Kostas?"*

You run a small crew. Workers come and go. At month-end, nobody remembers the exact numbers. Paper sheets get lost. Memory fails. Pay gets argued about.

## The Solution

**CrewTrack** goes in the van. Every morning, each worker taps their RFID card before leaving for work. At month-end, open the dashboard — it shows exactly who worked how many days and how much to pay.

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

**No internet required.** No cloud. No subscription. Just tap and go.

---

## Features

| Feature | Description |
|---------|-------------|
| **RFID Tap-In** | Workers tap their card each morning. Logged instantly. |
| **Duplicate Detection** | Won't double-count if someone taps twice. |
| **Unknown Card Alert** | Flags unregistered cards immediately. |
| **LCD Display** | 2" screen shows scan result in real-time. |
| **Audio Feedback** | Buzzer confirms each tap. |
| **SD Card Storage** | One CSV file per day. Your data stays local. |
| **WiFi Dashboard** | Connect your phone to manage everything. |
| **Employee Management** | Add, edit, delete workers from your phone. |
| **Monthly Salary** | Days worked × daily wage. Automatic. |
| **CSV Export** | Download full attendance history. |
| **Search & Filter** | Find any worker by name or UID. |
| **Manual Time Set** | Set clock from dashboard (no internet needed). |

---

## Hardware

Total cost: **~€40-50** (first build, includes shipping)

| Component | Model | Cost |
|-----------|-------|------|
| Microcontroller | ESP32 DevKit V1 | €5-8 |
| RFID Reader | RC522 MFRC522 | €3-5 |
| Display | ST7789V 2" IPS LCD | €6-10 |
| Storage | MicroSD Module | €2-3 |
| Feedback | Active Buzzer 5V | €0.50 |
| Cards | MIFARE Classic 1K (x10) | €2 |
| MicroSD Card | Any 1GB+ FAT32 | €3-5 |
| Breadboard + Wires | — | €5 |

See [BOM.md](BOM.md) for full shopping list with links.

---

## Wiring

```
                    ESP32
                 ┌──────────┐
    SCK  ────────┤ GPIO 18  ├──────┐
    MISO ────────┤ GPIO 19  ├──────┤ Shared SPI Bus
    MOSI ────────┤ GPIO 23  ├──────┘
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

| Module Pin | ESP32 |
|------------|-------|
| **RFID RC522** | |
| SDA (SS) | GPIO 5 |
| SCK | GPIO 18 |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| RST | GPIO 22 |
| 3.3V / GND | 3.3V / GND |
| **ST7789V LCD** | |
| SCK (CLK) | GPIO 18 |
| MOSI (SDA) | GPIO 23 |
| CS | GPIO 15 |
| DC | GPIO 2 |
| RST | GPIO 4 |
| VCC / BL | 3.3V |
| **MicroSD** | |
| CLK | GPIO 18 |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| CS | GPIO 10 |
| VCC / GND | 3.3V / GND |
| **Buzzer** | |
| + / - | GPIO 25 / GND |

---

## Quick Start

### 1. Install Arduino IDE
Download from [arduino.cc](https://www.arduino.cc/en/software)

### 2. Install ESP32 Board
**File > Preferences** → Additional Boards URL:
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
**Tools > Board > Boards Manager** → Search "esp32" → Install

### 3. Install Libraries
**Sketch > Include Library > Manage Libraries**:
- `MFRC522` by miguelbalboa
- `TFT_eSPI` by bodmer
- `ArduinoJson` by Benoit Blanchon

### 4. Configure TFT_eSPI
Copy contents of `TFT_eSPI_User_Setup.h` into your TFT_eSPI library's `User_Setup.h`:
```
Windows: C:\Users\<you>\Documents\Arduino\libraries\TFT_eSPI\User_Setup.h
Mac:     ~/Documents/Arduino/libraries/TFT_eSPI/User_Setup.h
```

### 5. Upload
Open `CrewTrack.ino` → **ESP32 Dev Module** → Upload

### 6. Connect
1. Power on device
2. Connect phone to WiFi: **`CrewTrack`** / **`12345678`**
3. Open browser: **`192.168.4.1`**
4. Set date/time on Time tab (no internet available)
5. Add workers, scan cards, done

---

## Web Dashboard

Connect to the device's WiFi and open `192.168.4.1`:

```
┌──────────────────────────────────────────┐
│              CREWTRACK                   │
├──────────────────────────────────────────┤
│  [Dashboard] [Workers] [Attendance]      │
│  [Salary]    [+ Add]  [Time]             │
│                                          │
│  ┌──────┐  ┌──────┐  ┌──────┐           │
│  │  3   │  │  5   │  │07:36 │           │
│  │Today │  │ Empl │  │Last  │           │
│  └──────┘  └──────┘  └──────┘           │
│                                          │
│  Recent Scans                            │
│  George    E4913A21         07:34        │
│  Nikos     AABBCCDD         07:36        │
│  John      11223344         07:40        │
│                                          │
│  [Download CSV]                          │
└──────────────────────────────────────────┘
```

**Dashboard tabs:**
- **Dashboard** — Today's stats, recent scans
- **Workers** — All employees, searchable
- **Attendance** — Full scan history
- **Salary** — Monthly report with progress bars
- **+ Add** — Register new employees
- **Time** — Set date/time manually (no internet needed)

---

## How It Works

```
  ┌──────────┐     ┌──────────┐     ┌──────────┐
  │ Worker   │────▶│  RFID    │────▶│  ESP32   │
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

## Roadmap

### ✅ Working
- RFID card scanning with RC522
- ST7789V LCD display (4 screens)
- SD card CSV logging
- Buzzer audio feedback
- WiFi AP + web dashboard
- Employee CRUD (add/edit/delete)
- Monthly salary calculation
- CSV export
- Duplicate scan detection
- Search & filter workers
- Manual time set via dashboard

### 🔄 In Progress
- Hardware integration testing
- Reliable SD card detection
- Power management (car USB / battery)
- Enclosure design

### 🔲 Planned
- FRAM module (replaces SD for critical data)
- BLE passive detection (no tap needed)
- Clock-out tracking (hours, not just days)
- Multiple van support
- GPS tracking (GY-NEO6MV2)
- RTC module for accurate offline time
- Mobile app
- Admin password protection
- OTA firmware updates

---

## Project Structure

```
CrewTrack/
├── CrewTrack.ino              Firmware (single file, Arduino IDE)
├── TFT_eSPI_User_Setup.h      Copy to TFT_eSPI library folder
├── employees_example.json     Example employee data for SD card
├── CrewTrack.png              Project banner
├── index.html                 Landing page (GitHub Pages)
├── README.md                  This file
├── LICENSE                    MIT License
├── CONTRIBUTING.md            How to contribute
├── CHANGELOG.md               Version history
├── BOM.md                     Shopping list with links
└── .github/
    ├── ISSUE_TEMPLATE/        Bug report & feature request forms
    └── workflows/
        └── compile.yml        CI - compiles on every push
```

---

## Contributing

Contributions welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

**Ways to help:**
- Report bugs → [Issues](https://github.com/0x-Shadow/CrewTrack/issues)
- Suggest features → [Issues](https://github.com/0x-Shadow/CrewTrack/issues)
- Fix bugs → Pull Request
- Improve docs → Pull Request
- Test hardware → Share results
- Design enclosure → 3D printable case

---

## License

[MIT License](LICENSE) — Free for personal and commercial use.

---

<p align="center">
  Built with ESP32 + Arduino<br>
  <sub>Star this repo if it helps your crew ⭐</sub>
</p>
