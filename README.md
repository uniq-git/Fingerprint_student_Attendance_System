# Fingerprint Student Attendance System

A fingerprint-based attendance system for students using an Arduino Uno, AS608 fingerprint sensor, HC-05 Bluetooth module, DS3231 Real-Time Clock (RTC), MIT App Inventor Android application, and Firebase Realtime Database.

The teacher operates the Android application to register students, manage attendance scanning, view attendance records, and export reports. Students use their fingerprints on the sensor to record their attendance.

**Course:** Systems Integration and Architecture 1 (SIA1), College of Computer Studies  
**Group/Section:** Group 2

---

## 1. System Overview

```text
AS608 Fingerprint Sensor ──┐
                           │
DS3231 Real-Time Clock ────┼── Arduino Uno
                           │       │
HC-05 Bluetooth Module ────┘       │ Bluetooth Serial
                                   ▼
                            Android App
                         (MIT App Inventor)
                                   │
                                   │ Firebase connection
                                   ▼
                         Firebase Realtime Database
```

| Tier | Description | Location |
|---|---|---|
| Edge | Arduino Uno, AS608 fingerprint sensor, HC-05 Bluetooth module, and DS3231 RTC | Arduino sketch in the repository root |
| Gateway / Frontend | Android app operated by the teacher | `.aia` project file in the repository root |
| Cloud | Firebase Realtime Database | Google Firebase |

There is no separate backend server in the current implementation. The Android app communicates with the Arduino through Bluetooth and connects to Firebase to store and retrieve student and attendance data.

### 1.1 User Roles

**Teacher**
- Operates the Android application.
- Registers student information and fingerprints.
- Starts fingerprint attendance scanning.
- Views attendance records.
- Exports attendance reports as CSV files.

**Students**
- Use their fingerprints on the AS608 sensor to record attendance.
- Do not need individual accounts or a login screen in the Android application.

The fingerprint sensor identifies registered students. It does not authenticate the teacher.

## 2. Repository Layout

The current project files are stored in the root directory of this repository.

```text
Fingerprint_student_Attendance_System/
├── Fingerprint_Attendance_System_arduino.ino
├── Fingerprint_Attendance_System_1.aia
├── DFD_Level0.drawio
├── DFD_Level1.drawio
├── ERD.drawio
├── Database_Schema.drawio
├── Fingerprint_attendance_system(flowchart).drawio
├── README.md
└── docs/
    └── adr/
        ├── ADR-001-fingerprint-processing-on-arduino.md
        ├── ADR-002-bluetooth-serial-communication.md
        ├── ADR-003-firebase-realtime-database.md
        └── ADR-004-ds3231-real-time-clock.md
```

The `docs/adr/` files must be uploaded before the ADR links in this README will work.

## 3. Hardware Requirements

| Component | Purpose |
|---|---|
| Arduino Uno | Main controller |
| AS608 fingerprint sensor | Registers and matches fingerprints |
| HC-05 Bluetooth module | Communicates with the Android phone |
| DS3231 RTC module | Provides the date and time for attendance scans |
| Android phone | Runs the MIT App Inventor application |

### 3.1 Wiring

| Module Pin | Arduino Pin | Notes |
|---|---|---|
| AS608 TX | D2 | Arduino receives sensor data |
| AS608 RX | D3 | Arduino sends commands to the sensor |
| AS608 VCC / GND | According to module specifications / GND | Check the sensor's rated voltage |
| HC-05 TX | D10 | Arduino receives Bluetooth serial data |
| HC-05 RX | D11 | Use an appropriate voltage divider if required |
| HC-05 VCC / GND | According to module specifications / GND | Check the module's power requirements |
| DS3231 SDA | A4 | I²C data |
| DS3231 SCL | A5 | I²C clock |
| DS3231 VCC / GND | According to module specifications / GND | Check the RTC module's power requirements |

All modules must share a common ground.

**Note:** Verify the actual wiring and voltage requirements of your specific modules before powering the system.

## 4. Setup and Running Each Tier

### 4.1 Edge: Arduino

1. Install the Arduino IDE.
2. Install the **Adafruit Fingerprint Sensor Library** and **RTClib** through the Arduino Library Manager. `SoftwareSerial` and `Wire` are included with the Arduino environment.
3. Open `Fingerprint_Attendance_System_arduino.ino` from the repository root.
4. Select the correct Arduino board and port, then upload the sketch.
5. Open the Serial Monitor at **9600 baud**.
6. Check the startup messages and confirm that the fingerprint sensor and RTC initialize correctly.
7. Pair the HC-05 with the phone through the phone's Bluetooth settings.

Troubleshooting:

- If `FINGERPRINT_ERROR` appears, check the sensor wiring, power supply, and sensor baud rate.
- If the RTC reports that it lost power, check the DS3231 battery and verify its date and time.
- If Bluetooth does not connect, check the pairing status, wiring, and serial communication settings.

Only the documented protocol messages are intended for communication between the app and Arduino over Bluetooth. Other diagnostic messages may appear in the USB Serial Monitor.

### 4.2 Gateway: Android App

1. Open [MIT App Inventor](https://appinventor.mit.edu).
2. Select **Projects → Import project (.aia)**.
3. Import `Fingerprint_Attendance_System_1.aia` from the repository root.
4. Configure the Firebase URL and any required settings in the relevant `FirebaseDB` components.
5. Build the app using **Build → Android App (.apk)** or test it using the MIT AI2 Companion.
6. Install the app on the Android phone and grant the necessary permissions.
7. Pair and connect the app to the HC-05 module.

The app is designed for the teacher to operate. Students use the fingerprint sensor and do not need to sign in to the Android application.

### 4.3 Cloud: Firebase

1. Open the [Firebase Console](https://console.firebase.google.com).
2. Create a Firebase project and configure a Realtime Database.
3. Set the database URL in the app.
4. Configure and test Firebase security rules before using real student information.
5. Verify that student registration and attendance records can be saved and retrieved.

**Security note:** The Android application is operated by the teacher, while students use the fingerprint sensor to record attendance. Students do not need individual app accounts. The current application does not include a teacher login system, so Firebase security rules should be properly configured to protect student information.

## 5. How to Use

### 5.1 Student Registration

1. The teacher opens the app.
2. Tap **Tap to Continue**.
3. Select the paired HC-05 Bluetooth device.
4. Open **Main Menu → Student Registration**.
5. Enter the student's name and student number.
6. Continue and tap **Start Register**.
7. Ask the student to place the same finger on the sensor as prompted.
8. Confirm that the app displays a successful registration message and assigned fingerprint ID.

### 5.2 Taking Attendance

1. The teacher opens **Main Menu → Attendance**.
2. Tap **Start Scan**.
3. The student places their registered finger on the sensor.
4. If the fingerprint matches, verify that the app displays the student's details, date, time, and attendance status.

### 5.3 Viewing Attendance Reports

1. The teacher opens **Main Menu → Attendance Report**.
2. Select the date to view.
3. Review the available attendance records.
4. Tap **Export / Download CSV** to export the report.
5. Check the exported `AttendanceReport.csv` file in the phone's Downloads folder.

## 6. Bluetooth Protocol (Arduino ↔ App)

The Arduino and app exchange plain-text messages over Bluetooth serial communication.

The protocol uses a baud rate of **9600** in the documented implementation.

| Direction | Message | Meaning |
|---|---|---|
| App → Arduino | `REGISTER` | Requests fingerprint registration |
| App → Arduino | `SCAN` | Requests fingerprint matching |
| Arduino → App | `REGISTERED\|<id>` | Registration succeeded; `<id>` is the assigned fingerprint ID |
| Arduino → App | `REGISTER_FAILED` | Registration failed or timed out |
| Arduino → App | `MATCH\|<id>\|<yyyy-MM-dd>\|<HH:mm:ss>` | Fingerprint matched, with date and time |
| Arduino → App | `NO_MATCH` | No matching fingerprint was found or no finger was detected within the timeout |

The Arduino and Android app must use the same command names and message format.

## 7. Database Structure (Firebase Realtime Database)

The application uses Firebase Realtime Database to store student information and attendance records.

| Node | Key | Fields |
|---|---|---|
| `Registered Student/{FingerprintID}` | Fingerprint ID | `FingerprintID`, `Name`, `StudentNumber` |
| `StudentCount` | Top-level counter | Integer |
| `Attendance/{Date}_{FingerprintID}` | Date and fingerprint ID | `FingerprintID`, `Name`, `StudentNumber`, `Date`, `Time`, `Status` |
| `AttendanceRecords/{Date}_{FingerprintID}` | Date and fingerprint ID | Same attendance fields; used by the report screen |

The fingerprint ID connects an attendance record to the registered student. Firebase Realtime Database does not enforce relational foreign keys, so the application is responsible for maintaining this connection.

### 7.1 Attendance Record Limitation

The current record key combines the date and fingerprint ID. Therefore, another scan by the same student on the same day can overwrite the earlier record under that key.

This behavior should be considered when evaluating whether the system's attendance-recording logic meets the project requirements.

## 8. Documentation

The repository includes the following system diagrams:

- `DFD_Level0.drawio` — Level 0 Data Flow Diagram
- `DFD_Level1.drawio` — Level 1 Data Flow Diagram
- `ERD.drawio` — Entity Relationship Diagram
- `Database_Schema.drawio` — Database schema diagram
- `Fingerprint_attendance_system(flowchart).drawio` — System flowchart

Open the `.drawio` files using [diagrams.net](https://app.diagrams.net).

### 8.1 Architecture Decision Records (ADRs)

The ADRs document the main design decisions made for the system.

Once uploaded, the files should be available at these paths:

- [ADR-001: Fingerprint Processing on Arduino](docs/adr/ADR-001-fingerprint-processing-on-arduino.md)
- [ADR-002: Bluetooth Serial Communication](docs/adr/ADR-002-bluetooth-serial-communication.md)
- [ADR-003: Firebase Realtime Database](docs/adr/ADR-003-firebase-realtime-database.md)
- [ADR-004: DS3231 Real-Time Clock](docs/adr/ADR-004-ds3231-real-time-clock.md)

Each ADR should reflect the actual implementation and decisions confirmed by the group.

## 9. Known Limitations

- The application is designed for teacher-operated attendance management, while students use fingerprint scanning for attendance. A teacher login system is not currently implemented.
- Firebase access depends on the database rules and configuration. Database security should be checked before real student information is used.
- The current architecture does not use a separate backend server or custom HTTP API.
- Attendance records use a date-and-fingerprint-ID key, so repeated scans on the same day may overwrite earlier records.
- Fingerprint templates are stored in the sensor's internal memory. Resetting or replacing the sensor may require students to register their fingerprints again.
- If the DS3231 loses power, the sketch may set the clock to the compile time. The date and time should be checked after RTC power loss.
- Bluetooth communication depends on correct pairing, wiring, and matching serial settings.
- The project should undergo hardware, database, and application testing before deployment.

## 10. Credits

- [Adafruit Fingerprint Sensor Library](https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library)
- [Adafruit RTClib](https://github.com/adafruit/RTClib)
- [MIT App Inventor](https://appinventor.mit.edu)
- [Google Firebase](https://firebase.google.com)
- [diagrams.net](https://app.diagrams.net) for viewing and editing system diagrams
