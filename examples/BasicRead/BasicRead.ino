/*
  BasicRead — Sensoren Library Example

  Reads TDS and LDR sensors and prints to Serial Monitor at 9600 baud.

  Wiring:
    TDS probe → A5
    LDR       → A4

  EEPROM note:
    Each calibrated sensor needs its own EEPROM region.
    TDS uses 19 bytes starting at the offset you provide.
    If you add a second calibrated sensor, start it at offset 20 or later.
*/

#include <Sensoren.h>

// TDSSensor(pin, eepromOffset)
// vRef and adcMax default to 5.0 and 1023.0 for Uno/Nano/Mega.
TDSSensor tds(A5, 0);

// LDRSensor(pin, threshold)
// threshold: raw ADC value at or above which water is considered clear.
LDRSensor ldr(A4, 970);

void setup() {
  Serial.begin(9600);

  tds.begin();
  ldr.begin();

  if (!tds.isCalibrated()) {
    Serial.println(F("TDS: keine Kalibrierung gefunden."));
    Serial.println(F("Bitte das Calibrate-Beispiel ausfuehren."));
  }
}

void loop() {
  float ppm = tds.read();

  Serial.print(F("TDS: "));
  if (ppm < 0) {
    Serial.println(F("nicht kalibriert"));
  } else {
    Serial.print(ppm, 1);
    Serial.println(F(" ppm"));
  }

  Serial.print(F("LDR: "));
  Serial.println(ldr.isClear() ? F("klar") : F("trueb"));
  Serial.println();

  delay(2000);
}