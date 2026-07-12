# Reddit Post Options

## Post 1: r/esp32

**Title:** I built a CrewTrack - RFID attendance tracker for small work crews using ESP32

**Body:**

Hey everyone,

I built an open-source attendance system for my father's small electrical crew. The problem: every month he'd ask "how many days did George work?" and nobody remembered.

**What it does:**
- Workers tap their RFID card on the reader each morning
- LCD shows their name + confirmation
- Buzzer beeps to confirm
- Attendance saved to SD card (one CSV per day)
- Connect your phone to its WiFi → full web dashboard at 192.168.4.1
- Monthly salary calc: days worked × daily wage
- Export CSV for records

**Hardware (~€15 total):**
- ESP32 DevKit
- RC522 RFID reader
- ST7789V 2" LCD
- MicroSD card module
- Active buzzer

**Cost per unit at scale: ~€9**

The whole thing is one .ino file, compiles with Arduino IDE, and the web dashboard is a single HTML page embedded in PROGMEM.

GitHub: https://github.com/0x-Shadow/CrewTrack

Built this for electricians, plumbers, HVAC techs, painters, builders - anyone with a small crew and no budget for enterprise attendance systems.

Would love feedback on the code, ideas for improvements, or contributions. The GPS module and RTC are planned as upgrades.

---

## Post 2: r/arduino

**Title:** CrewTrack - Open source RFID attendance tracker for work crews (ESP32 + Arduino)

**Body:**

Just finished the first version of my CrewTrack project. It's a standalone attendance terminal for small businesses.

**Stack:**
- ESP32 + Arduino IDE
- MFRC522 RFID reader
- ST7789V TFT display via TFT_eSPI
- SD card storage
- WebServer for WiFi dashboard
- ArduinoJson for API responses

Everything is in a single .ino file (~800 lines). The web dashboard is embedded as a PROGMEM string.

Features:
- RFID tap-in with duplicate detection
- LCD display with multiple screens (ready/success/duplicate/unknown)
- SD card CSV logging (one file per day)
- WiFi AP mode → phone dashboard
- Employee management, monthly salary calc, CSV export

GitHub: https://github.com/0x-Shadow/CrewTrack

Looking for code review, suggestions, and contributors. MIT licensed.

---

## Post 3: r/electronics (shorter, image-focused)

**Title:** Built a €15 RFID attendance tracker for my dad's work crew

**Body:**

[Banner image]

Every morning before work, the crew taps their card. Device logs who showed up. At month end, open the dashboard on your phone - days worked, salary calculation, CSV export.

Total hardware cost: ~€15
- ESP32
- RC522 RFID reader
- 2" TFT LCD
- SD card module
- Buzzer

All open source, MIT licensed. Code and wiring on GitHub.

https://github.com/0x-Shadow/CrewTrack

---

## Post 4: r/homelab

**Title:** CrewTrack - Self-hosted attendance system for small crews (no cloud, no subscription)

**Body:**

Built a completely self-contained attendance system. No internet required, no cloud service, no monthly fees.

- ESP32 runs a WiFi access point
- Connect your phone → web dashboard at 192.168.4.1
- All data stored on local SD card
- RFID cards for each worker
- Full salary calculation built in

Perfect for small businesses that don't want to pay for SaaS attendance tools.

GitHub: https://github.com/0x-Shadow/CrewTrack

---

## Post 5: r/opensource

**Title:** CrewTrack - Open source RFID attendance & payroll tracker for small work crews

**Body:**

CrewTrack is an open source hardware project that solves a real problem for small businesses: tracking which workers showed up and calculating pay.

**What makes it special:**
- Total cost: ~€15 per unit
- No cloud, no subscription, no internet needed
- Works offline, stores everything on SD card
- Web dashboard runs on the device itself
- One .ino file, compiles with Arduino IDE

**Target users:**
Electricians, plumbers, HVAC techs, painters, builders, cleaning companies, garden maintenance crews.

Looking for contributors, especially for:
- PCB design
- 3D printable enclosure
- GPS integration
- Mobile app

GitHub: https://github.com/0x-Shadow/CrewTrack

---

## Posting Strategy

**Recommended subreddits (in order of relevance):**
1. r/esp32 (most relevant - ESP32 community)
2. r/arduino (Arduino community)
3. r/electronics (broader hardware audience)
4. r/homelab (self-hosted angle)
5. r/opensource (open source angle)
6. r/raspberry_pi (sometimes overlaps with DIY)
7. r/functionalprint (if you design a case)
8. r/DIY (general maker audience)

**Tips:**
- Post the banner image with every post
- Reply to every comment
- Answer questions quickly
- Don't spam - one post per subreddit, spaced out
- Best time: weekday mornings (US/EU timezone)
