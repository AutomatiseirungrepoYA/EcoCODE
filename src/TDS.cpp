#include "TDS.h"
#include "internal/median.h"
#include "internal/eeprom_utils.h"

static const float TEMP_COEFF  = 0.02f;
static const int   SCOUNT      = 20;
static const int   SAMPLE_DELAY = 40;

TDSSensor::TDSSensor(uint8_t pin, int eepromOffset, float vRef, float adcMax)
    : _pin(pin), _eepromOffset(eepromOffset),
      _vRef(vRef), _adcMax(adcMax),
      _degree(0), _calibrated(false)
{
    for (int i = 0; i < 4; i++) _coeffs[i] = 0.0f;
}

void TDSSensor::begin() {
    pinMode(_pin, INPUT);
    uint8_t deg = 0;
    _calibrated = eeprom_loadCalib(_eepromOffset, deg, _coeffs, 4);
    if (_calibrated) _degree = deg;
}

float TDSSensor::read(float tempC) {
    if (!_calibrated) return -1.0f;

    int buf[SCOUNT];
    for (int i = 0; i < SCOUNT; i++) {
        buf[i] = analogRead(_pin);
        delay(SAMPLE_DELAY);
    }

    float vRaw  = medianOf(buf, SCOUNT) * _vRef / _adcMax;
    float vComp = vRaw / (1.0f + TEMP_COEFF * (tempC - 25.0f));

    float ppm = 0.0f;
    for (int i = 0; i <= _degree; i++) {
        if (isnan(_coeffs[i])) return -1.0f;
        ppm += _coeffs[i] * powf(vComp, (float)i);
    }
    return isnan(ppm) ? -1.0f : ppm;
}

bool TDSSensor::isCalibrated()  const { return _calibrated; }

void TDSSensor::printCoeffs() const {
    Serial.println(F("--- Aktuelle Kalibrierung ---"));
    if (!_calibrated) {
        Serial.println(F("Keine gueltige Kalibrierung geladen."));
        return;
    }
    Serial.print(F("Grad: ")); Serial.println(_degree);
    for (int i = 0; i <= _degree; i++) {
        Serial.print(F("  a")); Serial.print(i);
        Serial.print(F(" = ")); Serial.println(_coeffs[i], 6);
    }
    Serial.println(F("-----------------------------"));
}

bool TDSSensor::coeffsAllZero() const {
    for (int i = 0; i <= _degree; i++)
        if (_coeffs[i] != 0.0f) return false;
    return true;
}

void TDSSensor::loadFactoryDefaults() {
    _degree    = 3;
    _coeffs[0] = 0.0f;
    _coeffs[1] = 428.695f;
    _coeffs[2] = -127.93f;
    _coeffs[3] = 66.71f;
    _calibrated = true;
}

void TDSSensor::setCoefficients(uint8_t degree, const float* coeffs) {
    _degree = degree;
    for (int i = 0; i < 4; i++)
        _coeffs[i] = (i <= degree) ? coeffs[i] : 0.0f;
    _calibrated = true;
    eeprom_saveCalib(_eepromOffset, degree, _coeffs, degree + 1);
}
// Returns the temperature-compensated voltage sample from the pin.
// Used by runCalibration() to collect calibration data points.
float TDSSensor::sampleVoltage(float tempC) {
    int buf[SCOUNT];
    for (int i = 0; i < SCOUNT; i++) {
        buf[i] = analogRead(_pin);
        delay(SAMPLE_DELAY);
    }
    float vRaw = medianOf(buf, SCOUNT) * _vRef / _adcMax;
    return vRaw / (1.0f + TEMP_COEFF * (tempC - 25.0f));
}