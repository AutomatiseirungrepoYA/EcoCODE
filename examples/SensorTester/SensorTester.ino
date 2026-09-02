  // ============ SELECT EXACTLY ONE TEST ============
  //#define TEST_PH
  // #define TEST_TDS
  // #define TEST_TURBIDITY
  // #define TEST_PUMP
  // #define TEST_ULTRASONIC
  // ===================================================

  #if (defined(TEST_PH) + defined(TEST_TDS) + defined(TEST_TURBIDITY) + defined(TEST_PUMP) + defined(TEST_ULTRASONIC)) != 1
  #error "Uncomment exactly ONE TEST_... define above, not zero, not several."
  #endif


  // ====================================================================
  // ============ SHARED I2C OLED DISPLAY (SSD1306Ascii) =================
  // ====================================================================
  // On the Arduino Nano / Uno the hardware I2C bus is fixed to:
  //     A4 = SDA        A5 = SCL
  // The Wire library cannot remap those pins on AVR, so these two defines
  // are documentation only - they keep the screen wiring in one obvious
  // place and are what every test below talks to.
  #define OLED_SDA_PIN     A4
  #define OLED_SCL_PIN     A5
  #define OLED_I2C_ADDRESS 0x3C     // typical SSD1306 address (some modules use 0x3D)

  #include <Wire.h>
  #include <SSD1306Ascii.h>
  #include <SSD1306AsciiWire.h>

  SSD1306AsciiWire oled;

  // Bring the OLED up over I2C (A4/A5) and print a fixed title on the top
  // line. Call once from each test's setup().
  void displayBegin(const __FlashStringHelper *title) {
    Wire.begin();                        // hardware I2C -> A4 = SDA, A5 = SCL
    Wire.setClock(400000L);
    oled.begin(&Adafruit128x64, OLED_I2C_ADDRESS);   // use &Adafruit128x32 for a 128x32 panel
    oled.setFont(System5x7);
    oled.clear();
    oled.println(title);
    oled.println();
  }

  // Park the cursor just below the title so a test's loop() can redraw its
  // live readout without disturbing the title. Follow every printed value
  // with oled.clearToEOL() so leftover digits from a longer previous
  // reading get wiped.
  void displayHome() {
    oled.setCursor(0, 2);                // row 2 = third text line (rows are 0-based)
  }


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
    displayBegin(F("pH + Temp"));

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

    // ---- mirror the reading onto the shared OLED ----
    displayHome();
    oled.print(F("U : ")); oled.print(v, 3);   oled.print(F(" V")); oled.clearToEOL(); oled.println();
    oled.print(F("pH: ")); oled.print(phVal, 2);                    oled.clearToEOL(); oled.println();
    oled.print(F("T : "));
    if (temp.isFound() && tempC > -127.0f) { oled.print(tempC, 1); oled.print(F(" C")); }
    else                                     oled.print(F("--"));
    oled.clearToEOL(); oled.println();

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
    displayBegin(F("TDS"));
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

      // ---- mirror the reading onto the shared OLED ----
      displayHome();
      oled.print(F("Raw: ")); oled.print(rawNow);              oled.clearToEOL(); oled.println();
      oled.print(F("TDS: ")); oled.print(tdsValue, 0); oled.print(F(" ppm")); oled.clearToEOL(); oled.println();
    }
  }
  #endif // TEST_TDS



  // ====================================================================
  // ======================== TEST_TURBIDITY ==============================
  // ====================================================================
  #if defined(TEST_TURBIDITY)
  #include <Sensoren.h>

  #define TURB_FULLY_CLEAR_THRESHOLD     900   // must be defined
  #define TURB_PARTIALLY_CLEAR_THRESHOLD 600   // must be defined
  #define TURB_FULLY_DARK_THRESHOLD      300   // must be defined

  TurbiditySensor turb(A7, TURB_FULLY_CLEAR_THRESHOLD);   // A7 = Truebung pin

  void setup() {
    Serial.begin(9600);
    turb.begin();
    turb.setPartiallyClearThreshold(TURB_PARTIALLY_CLEAR_THRESHOLD);
    turb.setFullyDarkThreshold(TURB_FULLY_DARK_THRESHOLD);
    displayBegin(F("Turbidity"));

    Serial.println(F("Turbidity raw ADC + three-level threshold test - no calibration"));
  }

  void loop() {
    int raw = turb.read();

    Serial.print(F("Raw ADC: "));
    Serial.print(raw);
    Serial.print(F("   V: "));
    Serial.print(turb.readVoltage(), 3);
    Serial.print(F("   Status: "));

    if (turb.isFullyClear())          Serial.println(F("klar"));
    else if (turb.isPartiallyClear()) Serial.println(F("leicht trueb"));
    else if (turb.isFullyDark())      Serial.println(F("sehr trueb"));
    else                                Serial.println(F("trueb"));

    // ---- mirror the reading onto the shared OLED ----
    displayHome();
    oled.print(F("Raw: ")); oled.print(raw);                    oled.clearToEOL(); oled.println();
    oled.print(F("V  : ")); oled.print(turb.readVoltage(), 2);  oled.clearToEOL(); oled.println();
    oled.print(F("St : "));
    if (turb.isFullyClear())          oled.print(F("klar"));
    else if (turb.isPartiallyClear()) oled.print(F("leicht trueb"));
    else if (turb.isFullyDark())      oled.print(F("sehr trueb"));
    else                              oled.print(F("trueb"));
    oled.clearToEOL(); oled.println();

    delay(500);
  }
  #endif // TEST_TURBIDITY



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
    displayBegin(F("Pump"));
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

    // ---- mirror the state onto the shared OLED ----
    // This loop() has no delay, so only repaint when something actually
    // changed - otherwise the I2C bus would be hammered every iteration.
    static int lastP1 = -1, lastP2 = -1;
    if ((int)pressed != lastP1 || (int)pressed2 != lastP2) {
      lastP1 = pressed;
      lastP2 = pressed2;
      displayHome();
      oled.print(F("Pump1: ")); oled.print(pressed  ? F("ON")  : F("OFF")); oled.clearToEOL(); oled.println();
      oled.print(F("Pump2: ")); oled.print(pressed2 ? F("ON")  : F("OFF")); oled.clearToEOL(); oled.println();
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
  #define TRIG_PIN 2
  #define ECHO_PIN 3

  void setup() {
    Serial.begin(9600);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    displayBegin(F("Ultrasonic"));
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

    // ---- mirror the reading onto the shared OLED ----
    displayHome();
    if (duration == 0) {
      oled.print(F("Dist: -- (no echo)"));
    } else {
      oled.print(F("Dist: ")); oled.print(distanceCm, 1); oled.print(F(" cm"));
    }
    oled.clearToEOL(); oled.println();

    delay(500);
  }
  #endif // TEST_ULTRASONIC
