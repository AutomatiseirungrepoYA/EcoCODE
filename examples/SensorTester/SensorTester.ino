// ============ SELECT EXACTLY ONE TEST ============
#define TEST_PH
// #define TEST_TDS
// #define TEST_PUMP
// #define TEST_ULTRASONIC
// ===================================================

#if (defined(TEST_PH) + defined(TEST_TDS) + defined(TEST_PUMP) + defined(TEST_ULTRASONIC)) != 1
#error "Uncomment exactly ONE TEST_... define above, not zero, not several."
#endif





// ====================================================================
// ============================ TEST_PH ================================
// ====================================================================
#if defined(TEST_PH)
#include <Sensoren.h>

PHSensor      ph(A1);
DS18B20Sensor temp(5);   // see flagged issue #2 above - confirm this pin

void setup() {
  Serial.begin(9600);
  ph.begin();
  temp.begin();

  Serial.println(F("pH raw voltage + fixed-formula pH + temperature test - no calibration"));
  if (!temp.isFound()) {
    Serial.println(F("DS18B20: kein Sensor gefunden."));
  }
}

void loop() {
  float v = ph.sampleVoltage();
  // Falls back to 25C (the formula's reference temp, i.e. no compensation
  // applied) when no DS18B20 is wired up, so pH is still printed either way.
  float tempC = temp.isFound() ? temp.read() : 25.0f;   // blocks ~750ms if found
  float phVal = ph.read(tempC);   // re-samples voltage internally, close enough to v above

  Serial.print(F("pH probe voltage: "));
  Serial.print(v, 4);
  Serial.print(F(" V   pH: "));
  Serial.print(phVal, 2);
  Serial.print(F("   Temp: "));

  if (temp.isFound()) {
    if (tempC <= -127.0f) {
      Serial.println(F("-- (read failed / CRC error)"));
    } else {
      Serial.print(tempC, 2);
      Serial.println(F(" C"));
    }
  } else {
    Serial.println(F("-- (not found)"));
  }

  delay(1000);
}
#endif // TEST_PH



// ====================================================================
// =========================== TEST_TDS ================================
// ====================================================================
#if defined(TEST_TDS)
#define TdsSensorPin A6
#define VREF 5.0        // ADC reference voltage - 5.0 for Nano/Uno
#define SCOUNT 30        // number of samples in the median filter buffer

int   analogBuffer[SCOUNT];
int   analogBufferTemp[SCOUNT];
int   analogBufferIndex = 0;
int   copyIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;   // fixed at 25C - no DS18B20 involved in this test

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++) bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  } else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void setup() {
  Serial.begin(9600);
  pinMode(TdsSensorPin, INPUT);
  Serial.println(F("TDS test (DFRobot formula) - no calibration, checking noise"));
}

void loop() {
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT) analogBufferIndex = 0;
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();
    int rawNow = analogRead(TdsSensorPin);   // instantaneous, unfiltered
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
    float compensationVoltage = averageVoltage / compensationCoefficient;
    tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage
              - 255.86 * compensationVoltage * compensationVoltage
              + 857.39 * compensationVoltage) * 0.5 * 0.728;

    Serial.print(F("Raw ADC: "));
    Serial.print(rawNow);
    Serial.print(F("   TDS: "));
    Serial.print(tdsValue, 0);
    Serial.println(F(" ppm"));
  }
}
#endif // TEST_TDS



// ====================================================================
// ========================== TEST_PUMP ================================
// ====================================================================
#if defined(TEST_PUMP)
#define PUMP2_PIN   8
#define TASTER_PIN  10
#define PUMP3_PIN   7
#define TASTER2_PIN 11

void setup() {
  pinMode(PUMP2_PIN, OUTPUT);
  pinMode(PUMP3_PIN, OUTPUT);
  pinMode(TASTER_PIN, INPUT_PULLUP);
  pinMode(TASTER2_PIN, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  // INPUT_PULLUP means: not pressed = HIGH, pressed = LOW (button ties pin to GND)
  bool pressed  = (digitalRead(TASTER_PIN)  == LOW);
  bool pressed2 = (digitalRead(TASTER2_PIN) == LOW);

  if (pressed) {
    digitalWrite(PUMP2_PIN, HIGH);
    Serial.println(F("pump is on"));
  } else {
    digitalWrite(PUMP2_PIN, LOW);
    Serial.println(F("pump is off"));
  }

  if (pressed2) {
    digitalWrite(PUMP3_PIN, HIGH);
    Serial.println(F("pump2 is on"));
  } else {
    digitalWrite(PUMP3_PIN, LOW);
    Serial.println(F("pump2 is off"));
  }
}
#endif // TEST_PUMP



// ====================================================================
// ======================== TEST_ULTRASONIC =============================
// ====================================================================
#if defined(TEST_ULTRASONIC)
// Generic HC-SR04 style rangefinder - no library, raw pulseIn().
// TRIG_PIN / ECHO_PIN moved off pin 7 to avoid a physical wiring
// conflict with PUMP3_PIN in the pump test above (see flagged issue #3).
#define TRIG_PIN 6
#define ECHO_PIN 9

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println(F("Ultrasonic rangefinder test - raw distance only"));
}

void loop() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // pulseIn timeout of 30000us (~30ms) caps range at roughly 5m and
  // prevents an infinite block if no echo returns at all
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  float distanceCm = duration * 0.0343f / 2.0f;

  if (duration == 0) {
    Serial.println(F("Distance: -- (no echo / timeout)"));
  } else {
    Serial.print(F("Distance: "));
    Serial.print(distanceCm, 1);
    Serial.println(F(" cm"));
  }

  delay(500);
}
#endif // TEST_ULTRASONIC
