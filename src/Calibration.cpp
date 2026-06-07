#include "Calibration.h"
#include "internal/polyfit.h"
#include <Arduino.h>

#define MAX_RETRIES  5
#define MAX_POINTS   15
#define MAX_DEGREE   3

// Stored in static memory rather than on the stack —
// 15 floats × 2 arrays = 120 bytes on AVR's limited SRAM.
static float vBuf[MAX_POINTS];
static float pBuf[MAX_POINTS];

// ── Serial helpers (private to this file) ─────────────────────

static void flushSerial() {
    while (Serial.available() > 0) Serial.read();
}

static void waitForNonWhitespace() {
    while (true) {
        while (!Serial.available()) { delay(10); }
        char c = Serial.peek();
        if (c == '\r' || c == '\n' || c == ' ' || c == '\t') {
            Serial.read(); continue;
        }
        break;
    }
}

static float readFloat() {
    waitForNonWhitespace();
    float v = Serial.parseFloat();
    flushSerial();
    return v;
}

static int readInt() {
    waitForNonWhitespace();
    int v = Serial.parseInt();
    flushSerial();
    return v;
}

static float readFloatInRange(float lo, float hi) {
    for (int i = 0; i < MAX_RETRIES; i++) {
        float v = readFloat();
        if (v >= lo && v <= hi) return v;
        Serial.print(F("  Ungueltige Eingabe. Bitte Wert zwischen "));
        Serial.print(lo, 0); Serial.print(F(" und "));
        Serial.print(hi, 0); Serial.println(F(" eingeben:"));
    }
    Serial.println(F(">> Zu viele Eingaben. Kalibrierung wird neu gestartet..."));
    delay(2000);
    return -1.0f;
}

static int readIntInRange(int lo, int hi) {
    for (int i = 0; i < MAX_RETRIES; i++) {
        int v = readInt();
        if (v >= lo && v <= hi) return v;
        Serial.print(F("  Ungueltige Eingabe. Bitte Wert zwischen "));
        Serial.print(lo); Serial.print(F(" und "));
        Serial.print(hi); Serial.println(F(" eingeben:"));
    }
    Serial.println(F(">> Zu viele Eingaben. Kalibrierung wird neu gestartet..."));
    delay(2000);
    return -1;
}

// ── Main procedure ────────────────────────────────────────────

bool runCalibration(TDSSensor &sensor) {
    flushSerial();

    Serial.println(F("=================================================="));
    Serial.println(F("  TDS Kalibrierung"));
    Serial.println(F("=================================================="));
    Serial.println(F("Aktuelle Kalibrierung:"));
    sensor.printCoeffs();
    Serial.println();

    Serial.print(F("Polynomgrad eingeben (2-"));
    Serial.print(MAX_DEGREE); Serial.println(F("):"));
    int degree = readIntInRange(2, MAX_DEGREE);
    if (degree < 0) return false;
    Serial.print(F("  -> Grad = ")); Serial.println(degree);

    int minPts = degree + 1;
    Serial.print(F("Anzahl Kalibrierpunkte ("));
    Serial.print(minPts); Serial.print(F("-"));
    Serial.print(MAX_POINTS); Serial.println(F("):"));
    int nPoints = readIntInRange(minPts, MAX_POINTS);
    if (nPoints < 0) return false;
    Serial.print(F("  -> Punkte = ")); Serial.println(nPoints);

    Serial.println(F("Loesungstemperatur in Grad Celsius (5-50):"));
    float tempC = readFloatInRange(5.0f, 50.0f);
    if (tempC < 0) return false;
    Serial.print(F("  -> T = ")); Serial.print(tempC, 1); Serial.println(F(" C"));

    for (int k = 0; k < nPoints; k++) {
        Serial.println();
        Serial.print(F("Punkt ")); Serial.print(k + 1);
        Serial.print(F(" von ")); Serial.println(nPoints);
        Serial.println(F("  Sonde eintauchen, dann ppm eingeben:"));

        float knownPpm = readFloatInRange(10.0f, 10000.0f);
        if (knownPpm < 0) return false;

        float vComp = sensor.sampleVoltage(tempC);
        vBuf[k] = vComp;
        pBuf[k] = knownPpm;

        Serial.print(F("    ppm=")); Serial.print(knownPpm, 2);
        Serial.print(F("  Vc="));   Serial.println(vComp, 4);
    }

    float coeffs[MAX_DEGREE + 1] = {0};
    polyFit(vBuf, pBuf, nPoints, degree, coeffs);

    Serial.println();
    Serial.println(F("--------------------------------------------------"));
    for (int i = 0; i <= degree; i++) {
        Serial.print(F("  a")); Serial.print(i);
        Serial.print(F(" = ")); Serial.println(coeffs[i], 6);
    }

    float sse = 0;
    for (int k = 0; k < nPoints; k++) {
        float yhat = 0;
        for (int i = 0; i <= degree; i++) yhat += coeffs[i] * pow(vBuf[k], i);
        float e = yhat - pBuf[k];
        sse += e * e;
    }
    Serial.print(F("  RMSE = "));
    Serial.print(sqrt(sse / nPoints), 3);
    Serial.println(F(" ppm"));
    Serial.println(F("--------------------------------------------------"));

    sensor.setCoefficients((uint8_t)degree, coeffs);

    Serial.println(F("Kalibrierung gespeichert."));
    Serial.println(F("Neue Kalibrierung:"));
    sensor.printCoeffs();
    Serial.println(F("=================================================="));

    return true;
}