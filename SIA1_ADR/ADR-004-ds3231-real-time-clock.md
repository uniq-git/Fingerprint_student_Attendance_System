# ADR-004: DS3231 Real-Time Clock for Attendance Timestamps

- **Status:** Accepted
- **Date:** 2026-10-22

## Context
The fingerprint attendance system uses a DS3231 Real-Time Clock (RTC) module together with an Arduino Uno. Attendance records need a date and time so the app and database can identify when a student was marked present.

## Decision
Use the DS3231 RTC module as the system's date-and-time source for attendance timestamps. The Arduino should read the current date and time from the DS3231 when a fingerprint is matched, then include that timestamp in the message sent to the app.

## Consequences
- The DS3231 must be wired correctly and its time set and checked before testing.
- The team should test that the displayed or stored date and time remain correct after the Arduino is restarted or disconnected from power.
- If the RTC time is incorrect, attendance timestamps may also be incorrect.
- The team should verify that the Arduino sketch actually reads the DS3231 and sends the RTC timestamp before changing this ADR's status to Accepted.

## Evidence to check
- `Fingerprint_student_Attendance_System_arduino.ino`
- `README.md`
