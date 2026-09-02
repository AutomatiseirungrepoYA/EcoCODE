#ifndef PH_H
#define PH_H

#include <Arduino.h>

class PHSensor {
public:
    PHSensor(uint8_t pin, float vRef = 5.0f, float adcMax = 1023.0f);

    void begin();
    float read(float tempC);   // tempC is required now — no silent default to 25
    float sampleVoltage() const;

private:
    uint8_t _pin;
    float _vRef;
    float _adcMax;
};

#endif
