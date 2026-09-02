/*
  BasicRead — Sensoren Library Example

  Reads TDS, turbidity, and DS18B20 temperature sensors and prints to
  Serial Monitor at 9600 baud.

  Wiring:
    TDS probe  → A5
    Turbidity  → A4
    DS18B20    → D2 (needs a 4.7k pull-up resistor to VCC)

  EEPROM note:
    Each TDSSensor is automatically assigned its own 19-byte EEPROM region
    in the order it is declared. Declare sensors in the same order on every
    build so addresses remain stable across power cycles.
*/

#include <Sensoren.h>

// TDSSensor(pin)
// vRef and adcMax default to 5.0 and 1023.0 for Uno/Nano/Mega.
TDSSensor tds(A5);

// TurbiditySensor(pin, fullyClearThreshold)
// Three independent levels, each with its own threshold — tune these three
// against your own probe's readings.
TurbiditySensor turbidity(A4, 970);

// DS18B20Sensor(pin)
DS18B20Sensor temp(2);

void setup() {
  Serial.begin(9600);

  tds.begin();
  turbidity.begin();
  turbidity.setPartiallyClearThreshold(700);
  turbidity.setFullyDarkThreshold(400);
  temp.begin();

  if (!tds.isCalibrated()) {
    Serial.println(F("TDS: keine Kalibrierung gefunden."));
    Serial.println(F("Bitte das Calibrate-Beispiel ausfuehren."));
  }
  if (!temp.isFound()) {
    Serial.println(F("DS18B20: kein Sensor gefunden."));
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

  Serial.print(F("Truebung: "));
  if (turbidity.isFullyClear())          Serial.print(F("klar"));
  else if (turbidity.isPartiallyClear()) Serial.print(F("leicht trueb"));
  else if (turbidity.isFullyDark())      Serial.print(F("sehr trueb"));
  else                                    Serial.print(F("trueb"));
  Serial.print(F("  ("));
  Serial.print(turbidity.readVoltage(), 2);
  Serial.println(F(" V)"));

  Serial.print(F("Temperatur: "));
  Serial.print(temp.read(), 1);
  Serial.println(F(" C"));

  Serial.println();

  delay(2000);
}