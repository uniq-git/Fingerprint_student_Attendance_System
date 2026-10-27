# ADR-003: Firebase Realtime Database

- **Status:** Accepted
- **Date:** 2026-10-22

## Context
The system needs to store registered student details and attendance records so the mobile app can display and retrieve them.

## Decision
Use Firebase Realtime Database for storing student registration and attendance data. The repository README documents paths including `Registered Student/{FingerprintID}`, `StudentCount`, `Attendance/{Date}_{FingerprintID}`, and `AttendanceRecords/{Date}_{FingerprintID}`.

## Consequences
- The app depends on an internet connection for Firebase operations.
- Firebase rules must be reviewed and tested to prevent unauthorized access.
- The current date-and-fingerprint-ID key format may overwrite a student's earlier scan on the same date; the team should confirm whether this is intended.

## Evidence to check
- `Fingerprint_Attendance_System_1.aia`
- `README.md`
