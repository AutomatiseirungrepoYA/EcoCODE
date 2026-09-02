#ifndef SENSOREN_TDS_H
#define SENSOREN_TDS_H

#include <Arduino.h>

class TDSSensor {
public:
    // pin    : analog pin the probe connects to (e.g. A5)
    // vRef   : ADC reference voltage. 5.0 for Uno/Nano/Mega, 3.3 for ESP32.
    // adcMax : Highest ADC value. 1023.0 for 10-bit, 4095.0 for 12-bit.
    //
    // EEPROM offset is assigned automatically in construction order (19 bytes
    // per sensor). Declare all TDSSensor globals in the same order on every
    // build so addresses stay stable across power cycles.
    TDSSensor(uint8_t pin,
              float   vRef   = 5.0f,
              float   adcMax = 1023.0f);

    // Call once in setup() — loads calibration from EEPROM.
    // If nothing valid is found, isCalibrated() returns false
    // and read() returns -1 until calibration is loaded or set.
    void  begin();

    // Returns ppm. Takes 20 samples, picks the median, applies temperature
    // compensation, then evaluates the stored polynomial.
    // tempC = 25.0 (default) means no compensation is applied.
    // Returns -1.0 if not calibrated or if a NaN appears.
    float read(float tempC = 25.0f);

    bool  isCalibrated()   const;
    void  printCoeffs()    const;
    bool  coeffsAllZero()  const;
    void  loadFactoryDefaults();

    // Called by Calibration module after a successful run.
    // Updates memory AND writes to EEPROM at _eepromOffset.
    void  setCoefficients(uint8_t degree, const float* coeffs);
    float sampleVoltage(float tempC = 25.0f);
private:
    uint8_t _pin;
    int     _eepromOffset;
    float   _vRef;
    float   _adcMax;
    float   _coeffs[4];   // a0..a3, supports up to degree 3
    uint8_t _degree;
    bool    _calibrated;
};

#endif