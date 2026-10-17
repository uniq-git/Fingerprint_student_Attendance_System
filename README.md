# Fingerprint Student Attendance System

A fingerprint-based attendance system for students. An Arduino with an AS608 fingerprint sensor enrolls and matches fingerprints, and a DS3231 clock time-stamps every scan. Results travel over Bluetooth (HC-05) to an Android app built in MIT App Inventor, which saves records to a Firebase Realtime Database. Teachers can view any day's attendance in the app and export it as a CSV file.

**Course:** Systems Integration and Architecture 1 (SIA1), College of Computer Studies
**Group:** _Group No. / Section: Group 2

---

## 1. System overview

```
 AS608 sensor ──┐
 DS3231 clock ──┼── Arduino ──(Bluetooth HC-05, serial text)── Android app ──(HTTPS)── Firebase Realtime DB
                │   (edge)                                    (App Inventor)              (cloud)
```

| Tier | What it is | Where |
|---|---|---|
| Edge | Arduino + AS608 + HC-05 + DS3231, runs the sketch | `firmware/` |
| Gateway / frontend | Android app made in MIT App Inventor (8 screens) | `app/` |
| Cloud | Firebase Realtime Database | Google Firebase |

There is **no separate backend server**. The app is the gateway: it sends commands to the Arduino, reads the replies and writes records to Firebase.

## 2. Repository layout

```
firmware/   Fingerprint_Attendance_System_arduino.ino
app/        Fingerprint_Attendance_System_1.aia   (import into MIT App Inventor)
docs/       DFD Level 0, DFD Level 1, ERD, Database Schema, flowchart (.drawio) and the SIA1 submission packet
README.md
```

## 3. Hardware

| Part | Purpose |
|---|---|
| Arduino Uno | Main controller |
| AS608 fingerprint sensor | Enroll and match fingerprints |
| HC-05 Bluetooth module | Link to the phone |
| DS3231 RTC module | Date and time for each scan |
| Android phone | Runs the app |

### Wiring

| Module pin | Arduino pin | Note |
|---|---|---|
| AS608 TX | D2 | Arduino receives |
| AS608 RX | D3 | Arduino sends |
| AS608 VCC / GND | 5 V (or 3.3 V, check your module) / GND | Check your sensor's rated voltage |
| HC-05 TX | D10 | Arduino receives |
| HC-05 RX | D11 | **Through a voltage divider** (HC-05 RX is 3.3 V) |
| HC-05 VCC / GND | 5 V / GND | |
| DS3231 SDA | A4 | I2C |
| DS3231 SCL | A5 | I2C |
| DS3231 VCC / GND | 5 V / GND | |

All grounds must be shared.

## 4. Setup and running each tier

### 4.1 Edge: Arduino

1. Install the Arduino IDE.
2. In the Library Manager install **Adafruit Fingerprint Sensor Library** and **RTClib** (Adafruit). `SoftwareSerial` and `Wire` are built in.
3. Open `firmware/Fingerprint_Attendance_System_arduino.ino`, select your board and port, and upload.
4. Open the Serial Monitor at **9600 baud**. You should see `FINGERPRINT_READY` and `SYSTEM_READY`.
   - If you see `FINGERPRINT_ERROR`, check the sensor wiring, power voltage and baud rate (the sketch uses 57600; try 9600 if your sensor needs it).
   - If you see `RTC lost power`, the clock was set to the sketch's compile time.
5. Pair the HC-05 with your phone in the phone's Bluetooth settings (the PIN is usually `1234` or `0000`).

Only protocol messages go over Bluetooth. All status and error messages go to the USB Serial Monitor.

### 4.2 Gateway: Android app

1. Go to [MIT App Inventor](https://appinventor.mit.edu) and choose **Projects → Import project (.aia)**, then select `app/Fingerprint_Attendance_System_1.aia`.
2. In each screen that has a `FirebaseDB` component, set **FirebaseURL** (and token, if you use one) to **your own** Firebase project.
3. Build the app (**Build → Android App (.apk)**) or test it with the MIT AI2 Companion.
4. On your phone, allow Bluetooth and storage permissions when asked.

### 4.3 Cloud: Firebase

1. In the [Firebase console](https://console.firebase.google.com) create a project and add a **Realtime Database**.
2. Set the database rules for your testing or deployment needs. **Authentication is not implemented yet** (see Known limitations).
3. Put the database URL into the app (step 4.2).

## 5. How to use

1. Open the app and tap **Tap to Continue**, then pick the paired HC-05.
2. **Register a student:** Main Menu → *Student Registration* → enter name and student number → *Continue* → *Start Register*. Place the same finger on the sensor twice when prompted. The app shows *Registration Successful* with the assigned ID.
3. **Take attendance:** Main Menu → *Attendance* → *Start Scan* → place a finger. If it matches, the app shows name, number, date, time and status *Present*.
4. **Report:** Main Menu → *Attendance Report* → pick a date → the day's records appear. Tap *Export / Download CSV* to save `AttendanceReport.csv` to the phone's Download folder.

## 6. Bluetooth protocol (Arduino ⇄ app)

Messages are plain text, one per line, at 9600 baud.

| Direction | Message | Meaning |
|---|---|---|
| App → Arduino | `REGISTER` | Enroll a new fingerprint |
| App → Arduino | `SCAN` | Match a fingerprint |
| Arduino → App | `REGISTERED\|<id>` | Enrollment succeeded; `<id>` is the sensor slot (1–127) |
| Arduino → App | `REGISTER_FAILED` | Enrollment failed or timed out |
| Arduino → App | `MATCH\|<id>\|<yyyy-MM-dd>\|<HH:mm:ss>` | Fingerprint matched, with clock date and time |
| Arduino → App | `NO_MATCH` | No match, or no finger within 15 seconds |

## 7. Database structure (Firebase Realtime Database)

| Node | Key | Fields |
|---|---|---|
| `Registered Student/{FingerprintID}` | Fingerprint ID | `FingerprintID`, `Name`, `StudentNumber` |
| `StudentCount` | (top-level counter) | Integer |
| `Attendance/{Date}_{FingerprintID}` | Date + ID | `FingerprintID`, `Name`, `StudentNumber`, `Date`, `Time`, `Status` ("Present") |
| `AttendanceRecords/{Date}_{FingerprintID}` | Date + ID | Same fields; read by the report screen |

`FingerprintID` links an attendance record to its student. Firebase does not enforce foreign keys, so the app maintains this link.

## 8. Documentation

The `docs/` folder contains the DFD Level 0 and Level 1, the ERD, the database schema and the application flowchart (open the `.drawio` files at [diagrams.net](https://app.diagrams.net)).

## 9. Known limitations

- No API key or JWT layer. The edge link relies on Bluetooth pairing, and Firebase access depends on the database rules you set.
- The record key is `Date_FingerprintID`, so a second scan on the same day overwrites the first.
- The sensor stores fingerprints on its own memory (up to 127 here). Resetting or replacing the sensor means students must re-register.
- If the DS3231 loses its battery power, the clock restarts at the sketch's compile time.

## 10. Credits

- [Adafruit Fingerprint Sensor Library](https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library) and [RTClib](https://github.com/adafruit/RTClib) (Adafruit)
- MIT App Inventor and Google Firebase
