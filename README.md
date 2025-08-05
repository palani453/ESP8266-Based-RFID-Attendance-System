# ESP8266-Based RFID Attendance System

An IoT attendance system using an ESP8266 and MFRC522 RFID reader. Student IDs (MIFARE 4K cards) are scanned and logged directly to Google Sheets via HTTPS requests. New UIDs get registered in the **user_data** sheet; returning users trigger attendance updates in the **attendance** sheet. A Google Apps Script backend formats and displays Name, UID (User Id) , Registration No., Date, Time In, and Time Out.

---

##  Project Overview

- Automate student attendance using RFID and ESP8266 → Google Sheets.
- **Microcontroller**: ESP8266 (e.g., NodeMCU or Wemos D1 Mini).
- **RFID Module**: MFRC522.
- **Backend**: Google Apps Script + Google Sheets.
---

##  Key Features

- **Automatic Registration**: New card UIDs stored in **user_data** sheet.
- **Real-Time Attendance**: Existing users get “Time In” or “Time Out” stamped in **attendance** sheet.
- **Cloud-Hosted**: All logs live in Google Sheets—no local server required.

---

##  Hardware Components

- ESP8266 development board 
- MFRC522 RFID reader
- RFID cards (MIFARE 4K)
- Jumper wires
- 3.3 V power supply (USB or regulated)
---

##  Connections

| ESP8266 Pin | MFRC522 Pin |
|-------------|--------------|
| GPIO 14 (D5)| SCK          |
| GPIO 15 (D8)| SDA (SS)     |
| GPIO 13 (D7)| MOSI         |
| GPIO 12 (D6)| MISO         |
| GPIO 0 (D3) | RST          |
| GND         | GND          |
| 3.3 V       | 3.3 V        |

---
