/*
  Calibrate — Sensoren Library Example

  Runs an interactive TDS calibration over Serial Monitor at 9600 baud.
  Follow the prompts. You will need reference solutions of known ppm
  and the water temperature.

  Coefficients are stored in EEPROM and loaded automatically
  by tds.begin() on every subsequent power-up.

  Wiring:
    TDS probe → A5
*/

#include <Sensoren.h>

TDSSensor tds(A5);

void setup() {
  Serial.begin(9600);
  tds.begin();

  Serial.println(F("=== Sensoren Kalibrierung ==="));
  Serial.println(F("Aktuelle Kalibrierung:"));
  tds.printCoeffs();
  Serial.println();
  Serial.println(F("Beliebige Taste zum Starten..."));
  while (!Serial.available()) { delay(10); }
  while (Serial.available()) Serial.read();

  while (!runCalibration(tds)) {
    Serial.println(F("Fehlgeschlagen. Erneut versuchen..."));
  }

  Serial.println(F("Kalibrierung abgeschlossen. Starte Messung..."));
}

void loop() {
  float ppm = tds.read();
  Serial.print(F("TDS: "));
  Serial.print(ppm, 1);
  Serial.println(F(" ppm"));
  delay(2000);
}