#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <RTClib.h>

// ->  To run this you must install the library adafruit, rtclib in Library Manager <-

// ======================================================
// Protocol used by the MIT App Inventor app
//   App -> Arduino : "REGISTER\n"  |  "SCAN\n"
//                    "CLEARALL\n"  |  "DELETE|<id>\n"
//   Arduino -> App : "REGISTERED|<id>\n"
//                    "MATCH|<id>|<yyyy-MM-dd>|<HH:mm:ss>\n"
//                    "NO_MATCH\n"
//                    "REGISTER_FAILED\n"
//                    "CLEARED\n" | "CLEAR_FAILED\n"
//                    "DELETED|<id>\n" | "DELETE_FAILED\n"
// ONLY these lines go over Bluetooth.
// ALL status / error messages go to the USB Serial Monitor (9600).
//
// USB Serial Monitor commands (set line ending to "Newline"):
//   CLEAR        -> erase ALL fingerprints stored in the sensor
//   DELETE 1     -> erase fingerprint ID 1  only
// ======================================================

// AS608: sensor TX -> D2, sensor RX -> D3
SoftwareSerial fingerSerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

// HC-05: HC-05 TX -> D10, HC-05 RX -> D11 (through a voltage divider)
SoftwareSerial bluetooth(10, 11);

// DS3231 (SDA -> A4, SCL -> A5)
RTC_DS3231 rtc;

const unsigned long SCAN_TIMEOUT_MS = 15000;
const uint32_t FINGER_BAUD = 57600;   // try 9600 if your sensor needs it

bool sensorReady = false;
bool rtcReady = false;

// ---------- debug helpers (Serial Monitor only) ----------
void logError(const __FlashStringHelper *where, uint8_t code)
{
  Serial.print(F("ERROR in "));
  Serial.print(where);
  Serial.print(F(" -> code 0x"));
  Serial.println(code, HEX);
}

// ---------- sensor connection ----------
bool connectSensor()
{
  fingerSerial.listen();
  delay(50);

  sensorReady = finger.verifyPassword();

  if (sensorReady)
  {
    Serial.println(F("FINGERPRINT_READY"));
  }
  else
  {
    Serial.println(F("FINGERPRINT_ERROR: sensor not responding"));
    Serial.println(F("  Check: sensor TX->D2, sensor RX->D3, GND shared, correct power voltage, baud rate"));
  }
  return sensorReady;
}

void setup()
{
  Serial.begin(9600);
  bluetooth.begin(9600);
  finger.begin(FINGER_BAUD);
  Wire.begin();

  rtcReady = rtc.begin();
  if (!rtcReady)
  {
    Serial.println(F("RTC_ERROR: DS3231 not found (check SDA->A4, SCL->A5, power)"));
  }
  else if (rtc.lostPower())
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println(F("RTC lost power: time set to compile time"));
  }

  connectSensor();   // does NOT freeze if it fails; retried on every command

  bluetooth.listen();
  Serial.println(F("SYSTEM_READY"));
  Serial.println(F("USB commands: CLEAR  |  DELETE <id>"));
}

void loop()
{
  // ---------- USB Serial Monitor commands ----------
  if (Serial.available())
  {
    handleUsbCommand();
  }

  // ---------- Bluetooth commands from the app ----------
  if (bluetooth.available())
  {
    String command = bluetooth.readStringUntil('\n');
    command.trim();
    command.toUpperCase();

    if (command == "REGISTER")
    {
      Serial.println(F("Command: REGISTER"));
      if (sensorReady || connectSensor())
      {
        fingerSerial.listen();
        registerFingerprint();
      }
      else
      {
        sendLine(F("REGISTER_FAILED"));
      }
      bluetooth.listen();
    }
    else if (command == "SCAN" || command == "VERIFY")
    {
      Serial.println(F("Command: SCAN"));
      if (sensorReady || connectSensor())
      {
        fingerSerial.listen();
        scanFingerprint();
      }
      else
      {
        sendLine(F("NO_MATCH"));
      }
      bluetooth.listen();
    }
    else if (command == "CLEARALL")
    {
      Serial.println(F("Command: CLEARALL"));
      if (clearSensor()) sendLine(F("CLEARED"));
      else sendLine(F("CLEAR_FAILED"));
    }
    else if (command.startsWith("DELETE|"))
    {
      int id = command.substring(7).toInt();
      Serial.print(F("Command: DELETE "));
      Serial.println(id);
      if (deleteFingerprint(id))
      {
        bluetooth.print(F("DELETED|"));
        bluetooth.println(id);
      }
      else
      {
        sendLine(F("DELETE_FAILED"));
      }
    }
    else if (command.length() > 0)
    {
      Serial.print(F("Unknown command: "));
      Serial.println(command);
    }
  }
}

// ======================================================
// USB Serial Monitor commands
// ======================================================
void handleUsbCommand()
{
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toUpperCase();

  if (cmd == "CLEAR")
  {
    clearSensor();
  }
  else if (cmd.startsWith("DELETE "))
  {
    deleteFingerprint(cmd.substring(7).toInt());
  }
  else if (cmd.length() > 0)
  {
    Serial.println(F("Unknown. Use:  CLEAR   or   DELETE <id>"));
  }
}

// ======================================================
// Delete helpers
// ======================================================

// Erases EVERY fingerprint stored in the sensor
bool clearSensor()
{
  if (!(sensorReady || connectSensor())) return false;

  fingerSerial.listen();
  uint8_t p = finger.emptyDatabase();
  bluetooth.listen();

  if (p == FINGERPRINT_OK)
  {
    Serial.println(F("SENSOR_CLEARED: all fingerprints erased"));
    return true;
  }

  logError(F("emptyDatabase"), p);
  return false;
}

// Erases one fingerprint (id 1..127)
bool deleteFingerprint(int id)
{
  if (id < 1 || id > 127)
  {
    Serial.println(F("DELETE_FAILED: id must be 1 to 127"));
    return false;
  }
  if (!(sensorReady || connectSensor())) return false;

  fingerSerial.listen();
  uint8_t p = finger.deleteModel(id);
  bluetooth.listen();

  if (p == FINGERPRINT_OK)
  {
    Serial.print(F("DELETED fingerprint ID "));
    Serial.println(id);
    return true;
  }

  logError(F("deleteModel"), p);
  return false;
}

// ======================================================
// REGISTER  ->  REGISTERED|<id>   or   REGISTER_FAILED
// ======================================================
void registerFingerprint()
{
  uint8_t id = getNextID();

  if (id == 0)
  {
    sendLine(F("REGISTER_FAILED"));
    return;
  }

  Serial.print(F("Enrolling ID "));
  Serial.println(id);

  if (enrollFingerprint(id))
  {
    bluetooth.print(F("REGISTERED|"));
    bluetooth.println(id);
  }
  else
  {
    sendLine(F("REGISTER_FAILED"));
  }
}

// Finds the first free slot. Returns 0 on communication error or when full,
// so a noisy read can never cause an existing fingerprint to be overwritten.
uint8_t getNextID()
{
  for (uint8_t id = 1; id <= 127; id++)
  {
    uint8_t p = finger.loadModel(id);

    if (p == FINGERPRINT_OK) continue;               // slot in use

    // 0x01 = packet receive error, 0xFE/0xFF = bad packet / timeout
    if (p == 0x01 || p == 0xFE || p == 0xFF)
    {
      logError(F("getNextID (sensor communication)"), p);
      sensorReady = false;
      return 0;
    }

    return id;                                        // slot is free
  }

  Serial.println(F("NO_AVAILABLE_ID: sensor memory is full"));
  return 0;
}

// Waits for a good image; false on repeated errors or timeout
bool waitForImage()
{
  unsigned long start = millis();
  uint8_t errors = 0;
  uint8_t lastCode = 0;

  while (millis() - start < SCAN_TIMEOUT_MS)
  {
    uint8_t p = finger.getImage();

    if (p == FINGERPRINT_OK)
    {
      delay(300);                              // let the finger settle flat
      if (finger.getImage() == FINGERPRINT_OK) return true;
      continue;                                // lifted too early, wait again
    }

    if (p == FINGERPRINT_NOFINGER)
    {
      errors = 0;
    }
    else
    {
      lastCode = p;
      if (++errors > 10)
      {
        logError(F("getImage (SENSOR_ERROR)"), lastCode);
        sensorReady = false;
        return false;
      }
    }
    delay(100);
  }

  Serial.println(F("TIMEOUT: no finger detected"));
  return false;
}

// Captures an image and converts it into buffer `slot`. Retries a bad image 3 times.
bool captureToBuffer(uint8_t slot)
{
  for (uint8_t attempt = 1; attempt <= 3; attempt++)
  {
    if (!waitForImage()) return false;

    uint8_t p = finger.image2Tz(slot);
    if (p == FINGERPRINT_OK) return true;

    logError(F("image2Tz (bad image)"), p);
    Serial.println(F("Press the finger FLAT and firm on the glass, then try again"));

    unsigned long s = millis();                // wait for the finger to be lifted
    while (finger.getImage() != FINGERPRINT_NOFINGER && millis() - s < 5000) delay(100);
    delay(500);
    Serial.println(F("PLACE_FINGER"));
  }
  return false;
}

bool enrollFingerprint(uint8_t id)
{
  uint8_t p;

  // ---- first scan ----
  Serial.println(F("PLACE_FINGER"));
  if (!captureToBuffer(1)) return false;

  Serial.println(F("REMOVE_FINGER"));
  delay(2000);

  unsigned long start = millis();
  while (finger.getImage() != FINGERPRINT_NOFINGER)
  {
    if (millis() - start > SCAN_TIMEOUT_MS)
    {
      Serial.println(F("TIMEOUT: finger not removed"));
      return false;
    }
    delay(100);
  }

  // ---- second scan ----
  Serial.println(F("PLACE_SAME_FINGER"));
  if (!captureToBuffer(2)) return false;

  p = finger.createModel();
  if (p != FINGERPRINT_OK)
  {
    Serial.println(F("FINGERPRINT_NOT_MATCHED: the two scans differ, try again"));
    logError(F("createModel"), p);
    return false;
  }

  p = finger.storeModel(id);
  if (p != FINGERPRINT_OK) { logError(F("storeModel (SAVE_FAILED)"), p); return false; }

  Serial.println(F("FINGERPRINT_SAVED"));
  return true;
}

// ======================================================
// SCAN  ->  MATCH|<id>|<date>|<time>   or   NO_MATCH
// ======================================================
void scanFingerprint()
{
  int id = getFingerprintID();

  if (id < 0)
  {
    sendLine(F("NO_MATCH"));
    return;
  }

  if (!rtcReady)
  {
    Serial.println(F("RTC_ERROR: cannot time-stamp the match"));
    sendLine(F("NO_MATCH"));
    return;
  }

  DateTime now = rtc.now();

  char buf[40];
  snprintf(buf, sizeof(buf), "MATCH|%d|%04d-%02d-%02d|%02d:%02d:%02d",
           id, now.year(), now.month(), now.day(),
           now.hour(), now.minute(), now.second());

  Serial.print(F("Matched ID "));
  Serial.println(id);
  bluetooth.println(buf);
}

int getFingerprintID()
{
  if (!captureToBuffer(1)) return -1;

  uint8_t p = finger.fingerSearch();
  if (p == FINGERPRINT_OK)
  {
    return finger.fingerID;
  }

  if (p == FINGERPRINT_NOTFOUND) Serial.println(F("No match in sensor memory"));
  else logError(F("fingerSearch"), p);

  return -1;
}

void sendLine(const __FlashStringHelper *msg)
{
  bluetooth.println(msg);
}
