/*
  MonitorMenu — Sensoren Library Example

  Menu-driven sketch for an Arduino Nano with an SSD1306 I2C OLED display:
    'c' — Calibration menu: TDS only (pH uses a fixed formula, no calibration).
    'r' — Run mode: continuously reads TDS, temperature (DS18B20), and pH,
          and shows all three on the OLED and the Serial Monitor.
          Press any key to return to the menu.

  Wiring:
    TDS probe    → A0
    pH probe     → A1
    DS18B20      → D2   (needs a 4.7k pull-up resistor between data and VCC)
    OLED (SSD1306) → A4 (SDA), A5 (SCL) — the Nano's fixed hardware I2C pins.
                   A4/A5 are reserved for I2C here, so TDS and pH use A0/A1
                   instead of the pins used in the other examples.

  Requires the "SSD1306Ascii" library (Library Manager, by Bill Greiman) —
  a lightweight text-only driver, not the Adafruit_GFX/Adafruit_SSD1306
  graphics stack. Written for a common 128x64 module; if yours is 128x32,
  swap Adafruit128x64 for Adafruit128x32 below. If the screen stays blank,
  run an I2C scanner sketch to find your module's address and update
  OLED_I2C_ADDR — 0x3C and 0x3D are the two common ones.

  EEPROM note:
    TDSSensor is assigned its EEPROM region automatically. PHSensor uses a
    fixed formula and has no calibration or EEPROM storage of its own.
*/

#include <Sensoren.h>
#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

#define OLED_I2C_ADDR 0x3C

SSD1306AsciiWire oled;

TDSSensor      tds(A1);
PHSensor       ph(A0);
DS18B20Sensor  temp(A7);

// ── Serial helpers ────────────────────────────────────────────────────────────

static char waitForKey() {
  while (!Serial.available()) { delay(10); }
  char c = Serial.read();
  while (Serial.available()) Serial.read();
  return c;
}

static bool keyPressed() {
  if (!Serial.available()) return false;
  while (Serial.available()) Serial.read();
  return true;
}

static void showMenu() {
  Serial.println(F(""));
  Serial.println(F("=== Sensoren Monitor ==="));
  Serial.println(F("'c' - Kalibrierung"));
  Serial.println(F("'r' - Messung anzeigen"));
  Serial.print(F("Auswahl: "));
}

// ── Calibration menu ─────────────────────────────────────────────────────────

static void calibrationMenu() {
  Serial.println(F(""));
  Serial.println(F("--- Kalibrierung ---"));
  Serial.println(F("'t' - TDS kalibrieren"));
  Serial.println(F("'q' - zurueck zum Hauptmenue"));

  char c = waitForKey();
  if (c == 't' || c == 'T') {
    while (!runCalibration(tds)) {}
    tds.begin();
  } else {
    Serial.println(F("Zurueck zum Hauptmenue."));
  }
}

// ── Run mode ──────────────────────────────────────────────────────────────────

static void runMode() {
  Serial.println(F(""));
  Serial.println(F("--- Messung laeuft. Beliebige Taste = zurueck zum Menue ---"));
  oled.clear();

  while (!keyPressed()) {
    float tempC = temp.read();
    float ppm   = tds.isCalibrated() ? tds.read(tempC) : -1.0f;
    float phVal = ph.read(tempC);

    Serial.print(F("TDS: "));
    if (ppm < 0) Serial.print(F("--")); else Serial.print(ppm, 0);
    Serial.print(F(" ppm  pH: "));
    if (phVal < 0) Serial.print(F("--")); else Serial.print(phVal, 2);
    Serial.print(F("  T: "));
    if (temp.isFound()) Serial.print(tempC, 1); else Serial.print(F("--"));
    Serial.println(F(" C"));

    oled.setCursor(0, 0);
    oled.print(F("TDS:  "));
    if (ppm < 0) oled.print(F("--       ")); else { oled.print(ppm, 0); oled.print(F(" ppm   ")); }

    oled.setCursor(0, 1);
    oled.print(F("pH:   "));
    if (phVal < 0) oled.print(F("--       ")); else { oled.print(phVal, 2); oled.print(F("       ")); }

    oled.setCursor(0, 2);
    oled.print(F("Temp: "));
    if (temp.isFound()) { oled.print(tempC, 1); oled.print(F(" C     ")); }
    else oled.print(F("--       "));

    delay(2000);
  }

  oled.clear();
}

// ── Setup / loop ──────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(9600);
  Wire.begin();

  oled.begin(&Adafruit128x64, OLED_I2C_ADDR);
  oled.setFont(Adafruit5x7);
  oled.clear();
  oled.print(F("Sensoren Monitor"));

  tds.begin();
  ph.begin();
  temp.begin();

  if (!tds.isCalibrated()) {
    Serial.println(F("TDS: keine Kalibrierung gefunden."));
  }
  if (!temp.isFound()) {
    Serial.println(F("DS18B20: kein Sensor gefunden."));
  }

  delay(1500);
  oled.clear();
}

void loop() {
  showMenu();
  char key = waitForKey();
  Serial.println(key);

  if (key == 'c' || key == 'C') {
    calibrationMenu();
  } else if (key == 'r' || key == 'R') {
    runMode();
  }
}
