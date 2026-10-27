# ADR-001: Fingerprint Processing on the Arduino

- **Status:** Accepted
- **Date:** 2026-10-22

## Context
The system uses an AS608 fingerprint sensor with an Arduino Uno. The Arduino communicates with the sensor and handles fingerprint registration and matching.

## Decision
Use the Arduino Uno and AS608 fingerprint sensor to handle fingerprint registration and matching. The mobile app sends commands over Bluetooth, and the Arduino returns the result.

## Consequences
- Fingerprint matching can happen on the device without sending fingerprint images to Firebase.
- The Arduino sketch and sensor wiring must be configured correctly.
- The team must test registration and matching with the actual hardware.

## Evidence to check
- `Fingerprint_student_Attendance_System_arduino.ino`
- `Fingerprint_attendance_system(flowchart).drawio`
