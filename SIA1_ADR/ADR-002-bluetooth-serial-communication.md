# ADR-002: Bluetooth Serial Communication

- **Status:** Accepted
- **Date:** 2026-10-22

## Context
The Android app made with MIT App Inventor communicates with the Arduino Uno using an HC-05 Bluetooth module. The app needs to send registration or scan commands and receive the result.

## Decision
Use Bluetooth serial communication between the app and Arduino. The documented commands are `REGISTER` and `SCAN`. The Arduino can return messages such as `REGISTERED|<id>`, `REGISTER_FAILED`, `MATCH|<id>|<yyyy-MM-dd>|<HH:mm:ss>`, and `NO_MATCH`.

## Consequences
- The phone must pair and connect to the HC-05 before sending commands.
- Both the app and Arduino sketch must use the same message format.
- Connection failures and unexpected messages should be tested.

## Evidence to check
- `Fingerprint_student_Attendance_System_arduino.ino`
- `Fingerprint_Attendance_System_1.aia`
- `README.md`
