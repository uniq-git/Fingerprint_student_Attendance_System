# Fingerprint Student Attendance System

A fingerprint-based attendance system for students using an Arduino Uno, AS608 fingerprint sensor, HC-05 Bluetooth module, DS3231 Real-Time Clock (RTC), MIT App Inventor Android application, and Firebase Realtime Database.

The teacher operates the Android application to register students, manage attendance scanning, view attendance records, and export reports. Students use their fingerprints on the sensor to record their attendance. Each student can be recorded once per day.

**Course:** BSIT

**Group/Section:** Group 2 - OLCA311N072

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

| Tier               | Description                                                                   | Location                                    |
|--------------------|-------------------------------------------------------------------------------|---------------------------------------------|
| Edge               | Arduino Uno, AS608 fingerprint sensor, HC-05 Bluetooth module, and DS3231 RTC | `Fingerprint_Attendance_System_arduino.ino` |
| Gateway / Frontend | Android app operated by the teacher                                           | `Fingerprint_Attendance_System_1.aia`       |
| Cloud              | Firebase Realtime Database                                                    | Google Firebase                             |

There is no separate backend server in the current implementation. The Android app communicates with the Arduino through Bluetooth and connects to Firebase to store and retrieve student and attendance data.

### 1.1 User Roles

**Teacher**
- Operates the Android application.
- Registers student information and fingerprints.
- Starts fingerprint attendance scanning.
- Views attendance records.
- Exports attendance reports as CSV files.
- Maintains the system when a student is removed (see section 5.4).

**Students**
- Use their fingerprints on the AS608 sensor to record attendance.
- Do not need individual accounts or a login screen in the Android application.

The fingerprint sensor identifies registered students. It does not authenticate the teacher.

## 2. Repository Layout

The current project files are stored in the root directory of this repository.

```text
Fingerprint_student_Attendance_System/
├── Fingerprint_Attendance_System_arduino.ino   # Arduino sketch (edge tier)
├── Fingerprint_Attendance_System_1.aia         # MIT App Inventor project (Android app source)
├── Fingerprint_Attendance_System_1.apk         # Built Android app, ready to install
├── Diagram.zip                                 # System diagrams (see section 8.1)
├── SIA1_ADR.zip                                # Architecture Decision Records (see section 8.2)
└── README.md
```

The diagrams and ADRs are stored as zip archives. Download and extract them to view their contents.

Unzip `Diagram.zip` and open the `.drawio` files with [diagrams.net](https://app.diagrams.net). Unzip `SIA1_ADR.zip` to read the ADRs.

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
| HC-05 RX | D11 | The HC-05 RX pin is 3.3 V logic; use a voltage divider on this line   |
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
5. Open the Serial Monitor at **9600 baud** and set the line ending to **Newline**.
6. Check the startup messages and confirm that the fingerprint sensor and RTC initialize correctly.
7. Pair the HC-05 with the phone through the phone's Bluetooth settings.

**USB Serial Monitor commands (maintenance):**

| Command | Effect |
|---|---|
| `CLEAR` | Erases **all** fingerprints stored in the sensor |
| `DELETE <id>` | Erases only the fingerprint with that ID (1 to 127), for example `DELETE 3` |

The Arduino confirms with `SENSOR_CLEARED` or `DELETED fingerprint ID <id>`.

Troubleshooting:

- If `FINGERPRINT_ERROR` appears, check the sensor wiring, power supply, and sensor baud rate.
- If the RTC reports that it lost power, check the DS3231 battery and verify its date and time.
- If Bluetooth does not connect, check the pairing status, wiring, and serial communication settings.
- If scans keep matching a fingerprint ID that no longer exists in Firebase, clear that fingerprint from the sensor (see section 5.4).

Only the documented protocol messages are intended for communication between the app and Arduino over Bluetooth. Other diagnostic messages may appear in the USB Serial Monitor.

### 4.2 Gateway: Android App

**Quick install:** download `Fingerprint_Attendance_System_1.apk` from the repository root, copy it to the Android phone, and install it (you may need to allow installs from unknown sources). The steps below are for editing or rebuilding the app from the `.aia` source.

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

The sensor assigns the first free fingerprint slot as the ID, and the app saves the student under that same ID in Firebase.

### 5.2 Taking Attendance

1. The teacher opens **Main Menu → Attendance**.
2. Tap **Start Scan**.
3. The student places their registered finger on the sensor.
4. If the fingerprint matches, verify that the app displays the student's details, date, time, and attendance status.

Each student is recorded **once per day**. If the same student scans again on the same day, the system does not create another attendance record for that day.

### 5.3 Viewing Attendance Reports

1. The teacher opens **Main Menu → Attendance Report**.
2. Select the date to view.
3. Review the available attendance records.
4. Tap **Export / Download CSV** to export the report.
5. Check the exported `AttendanceReport.csv` file in the phone's Downloads folder.

### 5.4 Deleting a Student or Resetting the Sensor

Student data lives in two separate places: Firebase (name, student number, attendance) and the fingerprint sensor's own memory (the fingerprint itself). Deleting in one place does **not** delete from the other, so both must be updated.

**To remove one student:**

1. In the Firebase Console, delete `Registered Student/<id>`.
2. Delete that student's records under `Attendance` and `AttendanceRecords`, if they are no longer needed.
3. In the Arduino IDE Serial Monitor (9600 baud, Newline), type `DELETE <id>` using the same ID.

**To reset everything:**

1. Delete the `Registered Student`, `Attendance`, and `AttendanceRecords` nodes in Firebase.
2. In the Serial Monitor, type `CLEAR`.
3. Register the students again.

If step 3 (or `CLEAR`) is skipped, the old fingerprint stays in the sensor. A scan will still match its ID, but Firebase has no student for it, so the attendance entry can show **"Name not found"**.

## 6. Bluetooth Protocol (Arduino ↔ App)

The Arduino and app exchange plain-text messages over Bluetooth serial communication.

The protocol uses a baud rate of **9600** in the documented implementation.

| Direction     | Message                                 | Meaning                                                                        |
|---------------|-----------------------------------------|--------------------------------------------------------------------------------|
| App → Arduino | `REGISTER`                              | Requests fingerprint registration                                              |
| App → Arduino | `SCAN`                                  | Requests fingerprint matching                                                  |
| App → Arduino | `CLEARALL`                              | Requests erasing all fingerprints from the sensor                              |
| App → Arduino | `DELETE\|<id>`                          | Requests erasing one fingerprint from the sensor                               |
| Arduino → App | `REGISTERED\|<id>`                      | Registration succeeded; `<id>` is the assigned fingerprint ID                  |
| Arduino → App | `REGISTER_FAILED`                       | Registration failed or timed out                                               |
| Arduino → App | `MATCH\|<id>\|<yyyy-MM-dd>\|<HH:mm:ss>` | Fingerprint matched, with date and time                                        |
| Arduino → App | `NO_MATCH`                              | No matching fingerprint was found or no finger was detected within the timeout |
| Arduino → App | `CLEARED` / `CLEAR_FAILED`              | Result of `CLEARALL`                                                           |
| Arduino → App | `DELETED\|<id>` / `DELETE_FAILED`       | Result of `DELETE\|<id>`                                                       |

The Arduino and Android app must use the same command names and message format. The current app sends only `REGISTER` and `SCAN`. `CLEARALL` and `DELETE|<id>` are supported by the Arduino for future use, and the same actions are available today through the USB Serial Monitor (section 4.1).

## 7. Database Structure (Firebase Realtime Database)

The application uses Firebase Realtime Database to store student information and attendance records.

| Node | Key | Fields |
|---|---|---|
| `Registered Student/{FingerprintID}` | Fingerprint ID | `FingerprintID`, `Name`, `StudentNumber` |
| `StudentCount` | Top-level value | Stores the most recently assigned fingerprint ID. It is **not** a live count of students and is not updated when students are deleted |
| `Attendance/{Date}_{FingerprintID}` | Date and fingerprint ID | `FingerprintID`, `Name`, `StudentNumber`, `Date`, `Time`, `Status` |
| `AttendanceRecords/{Date}_{FingerprintID}` | Date and fingerprint ID | Same attendance fields; used by the report screen |

The fingerprint ID connects an attendance record to the registered student. Firebase Realtime Database does not enforce relational foreign keys, so the application is responsible for maintaining this connection.

### 7.1 One Attendance Record per Student per Day

The record key combines the date and fingerprint ID (for example `2026-10-22_4`), so each student has at most one attendance record per day. A repeat scan by the same student on the same day does not create an additional record.

## 8. Documentation

The system diagrams and Architecture Decision Records (ADRs) are stored as zip files in the repository root. Download and extract them to view the contents.

### 8.1 System Diagrams

`Diagram.zip` contains the system diagrams, which are `.drawio` files:

- Level 0 Data Flow Diagram
- Level 1 Data Flow Diagram
- Entity Relationship Diagram (ERD)
- Database schema diagram
- System flowchart

Extract the archive, then open the `.drawio` files using [diagrams.net](https://app.diagrams.net).

### 8.2 Architecture Decision Records (ADRs)

`SIA1_ADR.zip` contains the ADRs documenting the main design decisions of the system:

- ADR-001: Fingerprint Processing on Arduino
- ADR-002: Bluetooth Serial Communication
- ADR-003: Firebase Realtime Database
- ADR-004: DS3231 Real-Time Clock

Each ADR reflects the implementation and decisions confirmed by the group.

## 9. Known Limitations

- The application is designed for teacher-operated attendance management, while students use fingerprint scanning for attendance. A teacher login system is not currently implemented.
- Firebase access depends on the database rules and configuration. Database security should be checked before real student information is used.
- The current architecture does not use a separate backend server or custom HTTP API.
- Fingerprints are stored in the sensor's internal memory, separate from Firebase. Deleting a student in Firebase does not remove their fingerprint from the sensor; it must be removed with `DELETE <id>` or `CLEAR` (section 5.4). The app does not yet have a delete-student screen.
- `StudentCount` holds the last assigned fingerprint ID, not the current number of students.
- Resetting or replacing the sensor may require students to register their fingerprints again.
- If the DS3231 loses power, the sketch may set the clock to the compile time. The date and time should be checked after RTC power loss.
- Bluetooth communication depends on correct pairing, wiring, and matching serial settings.
- The project should undergo hardware, database, and application testing before deployment.

## 10. Credits

- [Adafruit Fingerprint Sensor Library](https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library)
- [Adafruit RTClib](https://github.com/adafruit/RTClib)
- [MIT App Inventor](https://appinventor.mit.edu)
- [Google Firebase](https://firebase.google.com)
- [diagrams.net](https://app.diagrams.net) for viewing and editing system diagrams
