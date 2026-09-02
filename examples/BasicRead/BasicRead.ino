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

// TurbiditySensor(pin, threshold)
// threshold: raw ADC value at or above which water is considered clear.
TurbiditySensor turbidity(A4, 970);

// DS18B20Sensor(pin)
DS18B20Sensor temp(2);

void setup() {
  Serial.begin(9600);

  tds.begin();
  turbidity.begin();
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
  Serial.print(turbidity.isClear() ? F("klar") : F("trueb"));
  Serial.print(F("  ("));
  Serial.print(turbidity.readNTU(), 1);
  Serial.println(F(" NTU)"));

  Serial.print(F("Temperatur: "));
  Serial.print(temp.read(), 1);
  Serial.println(F(" C"));

  Serial.println();

  delay(2000);
}